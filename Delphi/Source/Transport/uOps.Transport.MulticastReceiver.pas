unit uOps.Transport.MulticastReceiver;

(**
*
* Copyright (C) 2016-2024 Lennart Andersson.
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
     Winapi.Winsock2,
     uNotifier,
     uRunner,
     uOps.Types,
     uOps.Topic,
     uOps.MemoryMap,
     uOps.OPSMessage,
     uOps.Domain,
     uOps.Transport.Receiver,
     uSockets;

type
	TMulticastReceiver = class(TReceiver)
	private
    FPort : Integer;
    FIpAddress : string;
    FLocalInterface : string;
    FInSocketBufferSize : Int64;
    FSocket: TUdpSocket;
    // Current read buffer from user
    FBuffer : PByte;
    FBufferSize : Integer;
    // Last source for data
    FFromAddress : TSockAddr;
    FFromAddressLen : Integer;
    // Our thread running our Run() method
    FRunner : TRunner;
    FTerminated : Boolean;
    // Will by called by the FRunner thread
    procedure Run;
    procedure Report(method : string; mess : string);
  public
    constructor Create(mcAddress : string; bindPort : Integer; localInterface : string = '0.0.0.0'; inSocketBufferSize : Int64 = 16000000);
    destructor Destroy; override;
    // Start():
    // Starts the receiver, and reads bytes into given buffer.
    // When a message is read, a callback (notification) will be done with the
    // buffer and actual number of bytes read.
    // When the callback returns a new read is started to the current buffer
    function Start(bytes : PByte; size : Integer) : Boolean; override;

    // GetSource():
    // Used to get the sender IP and port for a received message.
    // Should only be called from the callback.
    procedure GetSource(var address : string; var port : Integer); override;

    // SetReceiveBuffer():
    // Changes the current buffer to use for reads.
    // Should only be called from the callback.
    procedure SetReceiveBuffer(bytes : PByte; size : Integer); override;

    // Stop():
    // Aborts an ongoing read. NOTE: Must NOT be called from the callback.
    procedure Stop; override;

    //
    function ReceiveMessage(o: PByte; size: Integer; var fromAddr : TSockAddr; var len : Integer): Integer;
    function available : Boolean;
    property Port : Integer read FPort;
    property Address : string read FIpAddress;
  end;
implementation
uses SysUtils,
     uOps.Error;
constructor TMulticastReceiver.Create(mcAddress : string; bindPort : Integer; localInterface : string; inSocketBufferSize : Int64);
begin
  inherited Create;
  FPort := bindPort;
  FIpaddress := mcAddress;
  FLocalInterface := localInterface;
  FInSocketBufferSize := inSocketBufferSize;
end;
destructor TMulticastReceiver.Destroy;
begin
	Stop;   // Make sure socket is closed
  inherited;
end;
procedure TMulticastReceiver.Report(method : string; mess : string);
begin
  if Assigned(FErrorService) then begin
    FErrorService.Report(TSocketError.Create('MulticastReceiver', method, mess, FLastErrorCode));
  end;
end;

// Override from Receiver
function TMulticastReceiver.Start(bytes : PByte; size : Integer) : Boolean;
var
  localip : AnsiString;
begin
  Result := False;
  if Assigned(FSocket) then Exit;
  FTerminated := False;
  FBuffer := bytes;
  FBufferSize := size;
  FSocket := TUdpSocket.Create;

  if not FSocket.Open then begin
    FLastErrorCode := FSocket.LastError;
    Report('Start', 'Open error');
    Exit;
  end;

  FSocket.LocalHost := '0.0.0.0';
  FSocket.LocalPort := FPort;

  FSocket.SetNonBlocking(False);
  FSocket.SetReuseAddress(True);

  if not FSocket.Bind then begin
    FLastErrorCode := FSocket.LastError;
    Report('Start', 'Bind error');
    Exit;
  end;

  // Get actual port that socket is bound to (in case bindport = 0)
  FSocket.GetLocalAddress(localip, FPort);

  if FInSocketBufferSize > 0 then begin
    FSocket.SetReceiveBufferSize(Integer(FInSocketBufferSize));
    if FSocket.GetReceiveBufferSize <> Integer(FInSocketBufferSize) then begin
      FLastErrorCode := SOCKET_ERROR;
      Report('Start', 'Socket buffer size could not be set');
    end;
  end;

  FSocket.AddMulticastMembership(AnsiString(FIpAddress), AnsiString(FLocalInterface));
  if Assigned(FBuffer) then begin
    // Start a thread running our run() method
    FRunner := TRunner.Create(Run);
  end;
  Result := True;
end;
// Override from Receiver
// Used to get the sender IP and port for a received message
// Only safe to call in callback
procedure TMulticastReceiver.getSource(var address : string; var port : Integer);
begin
  address := string(TUdpSocket.getIpAddress(FFromAddress));
  port := TUdpSocket.getPort(FFromAddress);
end;
// Override from Receiver
// Only safe to call in callback
procedure TMulticastReceiver.SetReceiveBuffer(bytes : PByte; size : Integer);
begin
  FBuffer := bytes;
  FBufferSize := size;
end;

// Override from Receiver
procedure TMulticastReceiver.Stop;
begin
  if not Assigned(FSocket) then Exit;
  // Tell thread to terminate
  FTerminated := True;
  if Assigned(FRunner) then FRunner.Terminate;
  FSocket.Close;
  // If thread exist, wait for thread to terminate and then delete the object
  FreeAndNil(FRunner);
  FreeAndNil(FSocket);

  FBuffer := nil;
  FBufferSize := 0;
end;
function TMulticastReceiver.available : Boolean;
begin
  Result := Assigned(FSocket);
end;
(**************************************************************************
*
**************************************************************************)
function TMulticastReceiver.ReceiveMessage(o: PByte; size: Integer; var fromAddr : TSockAddr; var len : Integer): Integer;
begin
  Result := 0;
  if not Assigned(FSocket) then Exit;

  FLastErrorCode := 0;
  len := SizeOf(fromAddr);
  Result := FSocket.ReceiveFrom(o^, size, fromAddr, len);
  if Result = SOCKET_ERROR then begin
    FLastErrorCode := FSocket.LastError;
  end;
end;

(**************************************************************************
*
**************************************************************************)
procedure TMulticastReceiver.Run;
var
  Res : Integer;
begin
  while not FTerminated do begin
    Res := ReceiveMessage(FBuffer, FBufferSize, FFromAddress, FFromAddressLen);
    if FTerminated then Break;
    if Res > 0 then begin
      // Got some data, Notify listener
      FDataNotifier.DoNotify(TBytesSizePair.Create(FBuffer, Res));

    end else if Res = 0 then begin
      // Could this happen?
      Sleep(10);    // So we don't hog the cpu on errors

    end else begin
      if FLastErrorCode = WSAEWOULDBLOCK then begin
        // Can't happen since we use a blocking socket

      end else if FLastErrorCode = WSAECONNRESET then begin
        // On a UDP-datagram socket this error indicates a previous send
        // operation resulted in an ICMP Port Unreachable message.
      end else begin
      end;

      Report('Run', 'Receive error');

      Sleep(10);    // So we don't hog the cpu on errors
    end;
  end;
end;
end.
