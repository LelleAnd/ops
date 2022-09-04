unit uOps.Transport.SendDataHandlerFactory;

(**
*
* Copyright (C) 2016-2022 Lennart Andersson.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*)

interface

uses System.Generics.Collections,
     System.SyncObjs,
     uOps.Error,
     uOps.Topic,
     uOps.Domain,
     uOps.Transport.SendDataHandler;

type
  // Method prototype to call when we connect/disconnect UDP topics with the participant info data listener
  TOnUdpConnectDisconnectProc = procedure(top : TTopic; sdh : TSendDataHandler; connect : Boolean) of object;

  TSendDataHandlerFactory = class(TObject)
  private
    type
      THandlerInfo = record
        handler : TSendDataHandler;
        numUsers : Integer;
      end;

  private
    // Borrowed reference
    FErrorService : TErrorService;

    // The Domain to which this Factory belongs (NOTE: we don't own the object)
    FDomain : TDomain;

    // Method to call when we connect/disconnect UDP topics with the participant info data listener
    // normally handled by the participant
    FOnUdpConnectDisconnectProc : TOnUdpConnectDisconnectProc;

    // There is only one McUdpSendDataHandler for each participant
    FUdpSendDataHandler : TSendDataHandler;
    FUdpUsers : Integer;

    // Dictionary with all SendDataHandlers except for UDP-transport
    FSendDataHandlers : TDictionary<string,THandlerInfo>;
    FMutex : TMutex;

    // Generate the key used in the dictionary
    function getKey(top : TTopic) : string;

  public
    constructor Create(Dom : TDomain; Proc : TOnUdpConnectDisconnectProc; Reporter : TErrorService);
    destructor Destroy; override;

    function getSendDataHandler(top : TTopic) : TSendDataHandler;
    procedure releaseSendDataHandler(top : TTopic);
  end;

implementation

uses SysUtils,
     uOps.NetworkSupport,
     uOps.Transport.McSendDataHandler,
     uOps.Transport.McUdpSendDataHandler,
     uOps.Transport.TCPSendDataHandler;

{ TSendDataHandlerFactory }

constructor TSendDataHandlerFactory.Create(
              Dom : TDomain;
              Proc : TOnUdpConnectDisconnectProc;
              Reporter : TErrorService);
begin
  inherited Create;
  FErrorService := Reporter;
  FDomain := dom;
  FOnUdpConnectDisconnectProc := Proc;
  FMutex := TMutex.Create;
  FSendDataHandlers := TDictionary<string,THandlerInfo>.Create;
end;

destructor TSendDataHandlerFactory.Destroy;
var
  value : THandlerInfo;
  handlerExist : Boolean;
begin
  handlerExist := False;
  // Cleanup/Free all senddatahandlers under protection
  FMutex.Acquire;
  try
    if FUdpUsers <> 0 then handlerExist := True;
    FreeAndNil(FUdpSendDataHandler);

    for Value in FSendDataHandlers.Values do begin
      if Value.numUsers <> 0 then handlerExist := True;
      if Assigned(Value.handler) then Value.handler.Free;
    end;
  finally
    FMutex.Release;
  end;
  if handlerExist then begin
    if Assigned(FErrorService) then begin
      FErrorService.Report(TBasicError.Create(
        'TSendDataHandlerFactory', 'Destroy', 'Publishers still alive when deleting factory!!!'));
    end;
  end;
  FreeAndNil(FSendDataHandlers);
  FreeAndNil(FMutex);
  inherited;
end;

function TSendDataHandlerFactory.getKey(top : TTopic) : string;
begin
  if (top.Transport = TTopic.TRANSPORT_TCP) and (top.Port = 0) then begin
    Result := string(top.Transport) + '::' + top.ChannelID + '::' + string(top.DomainAddress) + '::' + IntToStr(top.Port);
  end else begin
    Result := string(top.Transport) + '::' + string(top.DomainAddress) + '::' + IntToStr(top.Port);
  end;
end;

function TSendDataHandlerFactory.getSendDataHandler(top: TTopic): TSendDataHandler;
var
  key : string;
  localIf : string;
  ttl : Integer;
  info : THandlerInfo;
  topName : string;
	destAddress : string;
	destPort : Integer;
begin
  Result := nil;

  // We need to store SendDataHandlers with more than just the name as key.
  // Since topics can use the same port, we need to return the same SendDataHandler.
  // Make a key with the transport info that uniquely defines the receiver.
  key := getKey(top);

  FMutex.Acquire;
  try
		if FSendDataHandlers.ContainsKey(key) then begin
      // Increment usage count
			info := FSendDataHandlers.Items[key];
      Inc(info.numUsers);
			FSendDataHandlers.Items[key] := info;
      // Return found handler
      Result := info.handler;
      Exit;
		end;

    localIf := string(doSubnetTranslation(top.LocalInterface));
    ttl := top.TimeToLive;

		if top.Transport = TTopic.TRANSPORT_MC then begin
      Result := TMcSendDataHandler.Create(top, localIf, ttl, FErrorService);
      info.handler := Result;
      info.numUsers := 1;
      FSendDataHandlers.Add(key, info);

    end else if top.Transport = TTopic.TRANSPORT_UDP then begin
      if not Assigned(FUdpSendDataHandler) then begin
        // We have only one sender for all topics, so use the domain value for buffer size
        FUdpSendDataHandler := TMcUdpSendDataHandler.Create(localIf,
                                 ttl,
                                 FDomain.OutSocketBufferSize,
                                 FErrorService);
			end;

			// If topic specifies a valid node address, add that as a static destination address for topic
			if isValidNodeAddress(top.DomainAddress) then begin
				topName := string(top.Name);
				destAddress := string(top.DomainAddress);
				destPort := top.Port;
        TMcUdpSendDataHandler(FUdpSendDataHandler).addSink(topName, destAddress, destPort, True);

			end else begin
        // Setup a listener on the participant info data published by participants on our domain.
        // We use the information for topics with UDP as transport, to know the destination for UDP sends
        // ie. we extract ip and port from the information and add it to our McUdpSendDataHandler
        if Assigned(FOnUdpConnectDisconnectProc) then FOnUdpConnectDisconnectProc(top, FUdpSendDataHandler, True);
      end;

      Inc(FUdpUsers);

      Result := FUdpSendDataHandler;

		end else if top.Transport = TTopic.TRANSPORT_TCP then begin
      Result := TTCPSendDataHandler.Create(top, FErrorService);
      info.handler := Result;
      info.numUsers := 1;
      FSendDataHandlers.Add(key, info);

		end;
  finally
    FMutex.Release;
  end;
end;

procedure TSendDataHandlerFactory.releaseSendDataHandler(top: TTopic);
var
  key : string;
  info : THandlerInfo;
  ent : TPair<string,THandlerInfo>;
begin
  FMutex.Acquire;
  try
    if top.Transport = TTopic.TRANSPORT_UDP then begin
			if not isValidNodeAddress(top.DomainAddress) then begin
        if Assigned(FOnUdpConnectDisconnectProc) then FOnUdpConnectDisconnectProc(top, FUdpSendDataHandler, False);
      end;
      Dec(FUdpUsers);
      if FUdpUsers = 0 then begin
        // This is the last matching call, so now no one is using the SendDataHandler
        // Delete it
        FreeAndNil(FUdpSendDataHandler);
      end;

		end else begin
      key := getKey(top);

	  	if FSendDataHandlers.ContainsKey(key) then begin
        // Decrement usage count
	  		info := FSendDataHandlers.Items[key];
        Dec(info.numUsers);
			  FSendDataHandlers.Items[key] := info;

        if info.numUsers = 0 then begin
          // This is the last matching call, so now no one is using the SendDataHandler
          // Remove it from the dictionary and then delete it
          ent := FSendDataHandlers.ExtractPair(key);
          FreeAndNil(ent.Value.handler);
        end;
	  	end;
    end;
  finally
    FMutex.Release;
  end;
end;

end.

