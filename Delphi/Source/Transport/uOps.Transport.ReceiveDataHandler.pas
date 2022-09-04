unit uOps.Transport.ReceiveDataHandler;

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
     uNotifier,
     uOps.Types,
     uOps.Error,
     uOps.Topic,
     uOps.OPSMessage,
     uOps.SerializableInheritingTypeFactory,
     uOps.Domain,
     uOps.Transport.ReceiveDataChannel;

type
  TReceiveDataHandler = class(TObject)
  private
    // Borrowed references
    FErrorService : TErrorService;

    // Used for notifications to users of the ReceiveDataHandler
    FDataNotifier : TNotifier<TOPSMessage>;
    FCsNotifier : TNotifierValue<TConnectStatus>;

    // The ReceiveDataChannel(s) used for this ReceiveHandler.
    // The list owns the objects
    FRdc : TObjectList<TReceiveDataChannel>;

    FSampleMaxSize : Integer;

		// Current OPSMessage, valid until next sample arrives.
    FMessage : TOPSMessage;

    //
    FMessageLock : TMutex;

    function GetNumListeners : Integer;
    procedure onConnectStatusChanged(Sender : TObject; arg : TConnectStatus);

  public
    constructor Create(top : TTopic;
                       dom : TDomain;
                       opsObjectFactory : TSerializableInheritingTypeFactory;
                       Reporter : TErrorService;
                       rdc : TReceiveDataChannel);
    destructor Destroy; override;

    function aquireMessageLock : Boolean;
    procedure releaseMessageLock;

		procedure addListener(Proc : TOnNotifyEvent<TOPSMessage>; top : TTopic); overload;
		procedure removeListener(Proc : TOnNotifyEvent<TOPSMessage>; top : TTopic); overload;

		procedure addListener(Proc : TOnNotifyEvent<TConnectStatus>); overload;
		procedure removeListener(Proc : TOnNotifyEvent<TConnectStatus>); overload;

    property SampleMaxSize : Integer read FSampleMaxSize;
    property NumListeners : Integer read GetNumListeners;

	protected
    // Called whenever the ReceiveDataChannel has a new message.
    procedure onNewEvent(Sender : TObject; arg : TOPSMessage);

    procedure addRdc(Rdc : TReceiveDataChannel);

    procedure topicUsage(top : TTopic; used : Boolean); virtual; abstract;
	end;

  // ========================

  TMcReceiveDataHandler = class(TReceiveDataHandler)
  public
    constructor Create(top : TTopic;
                       dom : TDomain;
                       opsObjectFactory : TSerializableInheritingTypeFactory;
                       Reporter : TErrorService);
  protected
    procedure topicUsage(top : TTopic; used : Boolean); override;
  end;

  // ========================

  TTcpReceiveDataHandler = class;

  // Method prototype to call when we want to register a TCP RDH for the participant info data
  TOnTcpTransportInfoProc = procedure(top : TTopic; rdh : TTcpReceiveDataHandler; regist : Boolean) of object;

  TTcpReceiveDataHandler = class(TReceiveDataHandler)
  public
    constructor Create(top : TTopic;
                       dom : TDomain;
                       opsObjectFactory : TSerializableInheritingTypeFactory;
                       Reporter : TErrorService;
                       regProc : TOnTcpTransportInfoProc);
    destructor Destroy; override;
    procedure addReceiveChannel(topicName : string; ip: string; port : Integer);

  protected
    procedure topicUsage(top : TTopic; used : Boolean); override;

  private
    // Borrowed
    FDom : TDomain;
    FOpsObjectFactory : TSerializableInheritingTypeFactory;
    FRegProc : TOnTcpTransportInfoProc;

    // Owned
    FTop : TTopic;
    FUsingPartInfo : Boolean;
    FTopicInfoMap : TDictionary<AnsiString,Integer>;
    FTopicInfoLock : TMutex;
  end;

  // ========================

  TUdpReceiveDataHandler = class(TReceiveDataHandler)
  public
    constructor Create(top : TTopic;
                       dom : TDomain;
                       opsObjectFactory : TSerializableInheritingTypeFactory;
                       Reporter : TErrorService;
                       onUdpTransportInfoProc : TOnUdpTransportInfoProc);
  protected
    procedure topicUsage(top : TTopic; used : Boolean); override;
  end;

implementation

uses SysUtils,
     uOps.Exceptions,
     uOps.Transport.UDPReceiver;

constructor TReceiveDataHandler.Create(
              top : TTopic;
              dom : TDomain;
              opsObjectFactory : TSerializableInheritingTypeFactory;
              Reporter : TErrorService;
              rdc : TReceiveDataChannel);
begin
  inherited Create;
  FErrorService := Reporter;
  FDataNotifier := TNotifier<TOPSMessage>.Create(Self);
  FMessageLock := TMutex.Create;
  FCsNotifier := TNotifierValue<TConnectStatus>.Create(Self, True);
  FRdc := TObjectList<TReceiveDataChannel>.Create;

  if Assigned(rdc) then begin
    FSampleMaxSize := rdc.SampleMaxSize;
    addRdc(rdc);
  end;
end;

destructor TReceiveDataHandler.Destroy;
begin
  FreeAndNil(FRdc);
  FreeAndNil(FCsNotifier);
  FreeAndNil(FMessageLock);
  FreeAndNil(FDataNotifier);
  // Need to release the last message we received, if any.
  // (We always keep a reference to the last message received)
  if Assigned(FMessage) then FMessage.Unreserve;
  FMessage := nil;
  inherited;
end;

function TReceiveDataHandler.aquireMessageLock : Boolean;
begin
  Result := True;
  FMessageLock.Acquire;
end;

procedure TReceiveDataHandler.releaseMessageLock;
begin
  FMessageLock.Release;
end;

procedure TReceiveDataHandler.addRdc(Rdc : TReceiveDataChannel);
begin
  FMessageLock.Acquire;
  try
    FRdc.Add(Rdc);
    Rdc.addListener(onNewEvent);
    Rdc.addListener(onConnectStatusChanged);

    if FDataNotifier.numListeners > 0 then begin
      Rdc.Start;
    end;
  finally
    FMessageLock.Release;
  end;
end;

procedure TReceiveDataHandler.addListener(Proc : TOnNotifyEvent<TOPSMessage>; top : TTopic);
var
  i : Integer;
begin
  FMessageLock.Acquire;
  try
    FDataNotifier.addListener(Proc);
    if FDataNotifier.numListeners = 1 then begin
      for i := 0 to FRdc.Count - 1 do begin
        if Assigned(FRdc.Items[i]) then begin
          FRdc.Items[i].Start;
        end;
      end;
    end;
    topicUsage(top, True);
  finally
    FMessageLock.Release;
  end;
end;

procedure TReceiveDataHandler.removeListener(Proc : TOnNotifyEvent<TOPSMessage>; top : TTopic);
var
  i : Integer;
begin
  FMessageLock.Acquire;
  try
    topicUsage(top, False);
    FDataNotifier.removeListener(Proc);
    if FDataNotifier.numListeners = 0 then begin
      for i := 0 to FRdc.Count - 1 do begin
        if Assigned(FRdc.Items[i]) then begin
          FRdc.Items[i].Stop;
        end;
      end;
    end;
  finally
    FMessageLock.Release;
  end;
end;

function TReceiveDataHandler.GetNumListeners : Integer;
begin
  Result := FDataNotifier.numListeners;
end;

procedure TReceiveDataHandler.addListener(Proc : TOnNotifyEvent<TConnectStatus>);
begin
  FCsNotifier.addListener(Proc);
end;

procedure TReceiveDataHandler.removeListener(Proc : TOnNotifyEvent<TConnectStatus>);
begin
  FCsNotifier.removeListener(Proc);
end;

procedure TReceiveDataHandler.onConnectStatusChanged(Sender : TObject; arg : TConnectStatus);
begin
  FCsNotifier.doNotify(arg);
end;

// Called whenever the receiver has new data.
procedure TReceiveDataHandler.onNewEvent(Sender : TObject; arg : TOPSMessage);
var
  oldMessage : TOPSMessage;
begin
  if Assigned(arg) then begin
    FMessageLock.Acquire;
    try
      oldMessage := FMessage;
      FMessage := arg;

      // Increment ref count for message
      FMessage.Reserve;

      // Send it to Subscribers
      FDataNotifier.doNotify(FMessage);

      // This will delete the old message if no one has reserved it in the application layer.
      if Assigned(oldMessage) then oldMessage.Unreserve;
    finally
      FMessageLock.Release;
    end;
  end;
end;

// ========================

constructor TMcReceiveDataHandler.Create(
              top : TTopic;
              dom : TDomain;
              opsObjectFactory : TSerializableInheritingTypeFactory;
              Reporter : TErrorService);
begin
  inherited Create(top, dom, opsObjectFactory, Reporter, TReceiveDataChannel.Create(top, dom, opsObjectFactory, Reporter));
end;

procedure TMcReceiveDataHandler.topicUsage(top : TTopic; used : Boolean);
begin
  // Nothing to do
end;

// ========================

constructor TTcpReceiveDataHandler.Create(
              top : TTopic;
              dom : TDomain;
              opsObjectFactory : TSerializableInheritingTypeFactory;
              Reporter : TErrorService;
              regProc : TOnTcpTransportInfoProc);
var
  rdc : TReceiveDataChannel;
begin
  inherited Create(top, dom, opsObjectFactory, Reporter, nil);

  FTopicInfoMap := TDictionary<AnsiString,Integer>.Create;
  FTopicInfoLock := TMutex.Create;

  // Make a copy of the topic since we will change it
  FTop := top.Clone as TTopic;

  // Just borrow these
  FDom := dom;
  FOpsObjectFactory := opsObjectFactory;
  FRegProc := regProc;

  // Handle TCP channels specified with address and port
  if (top.Transport = TTopic.TRANSPORT_TCP) and (top.Port <> 0) then begin
    FUsingPartInfo := False;
    rdc := TReceiveDataChannel.Create(top, dom, opsObjectFactory, Reporter);
    rdc.Key := string(top.DomainAddress) + '::' + IntToStr(top.Port);
    addRdc(rdc);
  end else begin
    // Since we use the same "topic" parameters for all created RDC's, we can set sampleMaxSize here
    FSampleMaxSize := calcSampleMaxSize(top);
    FUsingPartInfo := True;
  end;
end;

destructor TTcpReceiveDataHandler.Destroy;
begin
  FreeAndNil(FTop);
  FreeAndNil(FTopicInfoLock);
  FreeAndNil(FTopicInfoMap);
  inherited;
end;

procedure TTcpReceiveDataHandler.addReceiveChannel(topicName : string; ip: string; port : Integer);
var
  key : string;
  i : Integer;
  found : Boolean;
  rdc : TReceiveDataChannel;
begin
  // Create key
  key := ip + '::' + IntToStr(port);

  // Look after it in FRdc
  found := False;
  for i := 0 to FRdc.Count - 1 do begin
    if Assigned(FRdc.Items[i]) then begin
      if FRdc.Items[i].Key = key then begin
        found := True;
        Break;
      end;
    end;
  end;

  // if not found, create a new RDC
  if not found then begin
    FTop.DomainAddress := AnsiString(ip);
    FTop.Port := port;
    rdc := TReceiveDataChannel.Create(FTop, FDom, FOpsObjectFactory, FErrorService);
    rdc.Key := key;
    addRdc(rdc);
  end;
end;

procedure TTcpReceiveDataHandler.topicUsage(top : TTopic; used : Boolean);
var
  Count : Integer;
begin
  if FUsingPartInfo then begin
    FTopicInfoLock.Acquire;
    try
      // Only register topic once
      Count := 0;
      // lookup
  		if FTopicInfoMap.ContainsKey(top.Name) then begin
        Count := FTopicInfoMap.Items[top.Name];
      end;
      if used then begin
        Inc(Count);
        if Count = 1 then begin
          if Assigned(FRegProc) then begin
            FRegProc(top, Self, True);
          end;
        end;
      end else begin
        Dec(Count);
        if Count = 0 then begin
          if Assigned(FRegProc) then begin
            FRegProc(top, Self, False);
          end;
        end;
      end;
      FTopicInfoMap.AddOrSetValue(top.Name, Count);
    finally
      FTopicInfoLock.Release;
    end;
  end;
end;

// ========================

constructor TUdpReceiveDataHandler.Create(
              top : TTopic;
              dom : TDomain;
              opsObjectFactory : TSerializableInheritingTypeFactory;
              Reporter : TErrorService;
              onUdpTransportInfoProc : TOnUdpTransportInfoProc);
var
  rdc : TReceiveDataChannel;
begin
  inherited Create(top, dom, opsObjectFactory, Reporter, TReceiveDataChannel.Create(top, dom, opsObjectFactory, Reporter));

  if Assigned(onUdpTransportInfoProc) then begin
    rdc := FRdc.Items[0];
    onUdpTransportInfoProc(
      (rdc.getReceiver as TUDPReceiver).Address,
      (rdc.getReceiver as TUDPReceiver).Port);
  end;
end;

procedure TUdpReceiveDataHandler.topicUsage(top : TTopic; used : Boolean);
begin
  // Nothing to do
end;

end.

