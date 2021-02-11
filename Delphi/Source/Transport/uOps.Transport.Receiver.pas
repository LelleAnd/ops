unit uOps.Transport.Receiver;

(**
*
* Copyright (C) 2016-2021 Lennart Andersson.
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

uses uNotifier,
     uOps.Types,
     uOps.Error,
     uOps.Topic,
     uOps.Domain;

type
	TBytesSizePair = record
    Bytes : PByte;
    Size : Integer;
    constructor Create(ABytes : PByte; ASize : Integer);
  end;

  (* ------------------------------------------------------------------------ *)

	TReceiver = class(TObject)
  protected
    // Borrowed reference
    FErrorService : TErrorService;

    // Result from WSAGetLastError() on error
    FLastErrorCode : Integer;

    // Used for notifications to users of the Receiver
    FDataNotifier : TNotifier<TBytesSizePair>;

    FCsClient : TOnNotifyEvent<TConnectStatus>;

    procedure notifyConnectStatus(arg : TConnectStatus);

	public
    constructor Create;
    destructor Destroy; override;

		procedure addListener(Proc : TOnNotifyEvent<TBytesSizePair>);
		procedure removeListener(Proc : TOnNotifyEvent<TBytesSizePair>);

    procedure setConnectStatusListener(Listener : TOnNotifyEvent<TConnectStatus>);

    // Start():
    // Starts the receiver, and reads bytes into given buffer.
    // When a message is read, a callback (notification) will be done with the
    // buffer and actual number of bytes read.
    // When the callback returns a new read is started to the current buffer
    function Start(bytes : PByte; size : Integer) : Boolean; virtual; abstract;

    // GetSource():
    // Used to get the sender IP and port for a received message.
    // Should only be called from the callback.
    procedure GetSource(var address : string; var port : Integer); virtual; abstract;

    // SetReceiveBuffer():
    // Changes the current buffer to use for reads.
    // Should only be called from the callback.
    procedure SetReceiveBuffer(bytes : PByte; size : Integer); virtual; abstract;

    // Stop():
    // Aborts an ongoing read. NOTE: Must NOT be called from the callback.
    procedure Stop; virtual; abstract;

    property LastErrorCode : Integer read FLastErrorCode;
  end;

  (* ------------------------------------------------------------------------ *)

  TReceiverFactory = class(TObject)
  public
    // Creates a receiver based on topic transport information
    class function getReceiver(top : TTopic; dom : TDomain; Reporter : TErrorService) : TReceiver;
  end;

implementation

uses SysUtils,
     uOps.NetworkSupport,
     uOps.Transport.MulticastReceiver,
     uOps.Transport.UDPReceiver,
     uOps.Transport.TCPClient;

{ TBytesSizePair }

constructor TBytesSizePair.Create(ABytes : PByte; ASize : Integer);
begin
  Bytes := ABytes;
  Size := ASize;
end;

{ TReceiver }

constructor TReceiver.Create;
begin
  inherited;
  FDataNotifier := TNotifier<TBytesSizePair>.Create(Self);
  FCsClient := nil;
end;

destructor TReceiver.Destroy;
begin
  FreeAndNil(FDataNotifier);
  inherited;
end;

procedure TReceiver.setConnectStatusListener(Listener : TOnNotifyEvent<TConnectStatus>);
begin
  FCsClient := Listener;
end;

procedure TReceiver.notifyConnectStatus(arg : TConnectStatus);
begin
  if Assigned(FCsClient) then begin
    //TOnNotifyEvent<T> = procedure(Sender : TObject; arg : T) of object;
    FCsClient(Self, arg);
  end;
end;

procedure TReceiver.addListener(Proc : TOnNotifyEvent<TBytesSizePair>);
begin
  FDataNotifier.addListener(Proc);
end;

procedure TReceiver.removeListener(Proc : TOnNotifyEvent<TBytesSizePair>);
begin
  FDataNotifier.removeListener(Proc);
end;

{ TReceiverFactory }

class function TReceiverFactory.getReceiver(top : TTopic; dom : TDomain; Reporter : TErrorService) : TReceiver;
var
  localif : string;
  port : Integer;
begin
  Result := nil;

  localIf := string(doSubnetTranslation(top.LocalInterface));

  if top.Transport = TTopic.TRANSPORT_MC then begin
    Result := TMulticastReceiver.Create(string(top.DomainAddress), top.Port, localIf, top.InSocketBufferSize);

  end else if top.Transport = TTopic.TRANSPORT_TCP then begin
    Result := TTCPClientReceiver.Create(string(top.DomainAddress), top.Port, top.InSocketBufferSize);

  end else if top.Transport = TTopic.TRANSPORT_UDP then begin
    // If UDP topic is configured with my node address, we use the configured port, otherwise
    // we use port 0 which will force the OS to create a unique port that we listen to.
    port := 0;
    if isMyNodeAddress(top.DomainAddress) then begin
      localIf := string(top.DomainAddress);
      port := top.Port;
    end;

    Result := TUDPReceiver.Create(port, localIf, top.InSocketBufferSize);
  end;

  if Assigned(Result) then Result.FErrorService := Reporter;
end;

end.

