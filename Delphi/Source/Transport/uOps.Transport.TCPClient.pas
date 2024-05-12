unit uOps.Transport.TCPClient;

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
  TTCPClientReceiver = class(TReceiver)
  private
    FPort : Integer;
    FIpAddress : string;
    FInSocketBufferSize : Int64;

    FTcpClient : TTcpClientSocket;
    FConnectStatus : TConnectStatus;

    // Current read buffer from user
    FBuffer : PByte;
    FBufferSize : Integer;

    // Our thread running our Run() method
    FRunner : TRunner;
    FTerminated : Boolean;

    // Will by called by the FRunner thread
    procedure Run;

    procedure Report(method : string; mess : string);

  public
    constructor Create(serverIP : string; serverPort : Integer; inSocketBufferSize : Int64 = 16000000);
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

  end;

implementation

uses SysUtils,
     uOps.Error;

constructor TTCPClientReceiver.Create(serverIP : string; serverPort : Integer; inSocketBufferSize : Int64 = 16000000);
begin
  inherited Create;
  FIpAddress := serverIP;
  FPort := serverPort;

  FInSocketBufferSize := inSocketBufferSize;

  fTcpClient := TTcpClientSocket.Create;
  fTcpClient.RemoteHost := AnsiString(FIpAddress);
  fTcpClient.RemotePort := FPort;
end;

destructor TTCPClientReceiver.Destroy;
begin
  // Make sure socket is closed
  Stop;

  FreeAndNil(FTcpClient);
  inherited;
end;

procedure TTCPClientReceiver.Report(method : string; mess : string);
begin
  if Assigned(FErrorService) then begin
    FErrorService.Report(TSocketError.Create('TCPClientReceiver', method, mess, FLastErrorCode));
  end;
end;

// Start():
// Starts the receiver, and reads bytes into given buffer.
// When a message is read, a callback (notification) will be done with the
// buffer and actual number of bytes read.
// When the callback returns a new read is started to the current buffer
function TTCPClientReceiver.Start(bytes : PByte; size : Integer) : Boolean;
begin
  Result := False;
  if Assigned(FRunner) then Exit;

  FTerminated := False;
  FBuffer := bytes;
  FBufferSize := size;

  if Assigned(FBuffer) then begin
    // Start a thread running our run() method
    FRunner := TRunner.Create(Run);
  end;
  Result := True;
end;

// GetSource():
// Used to get the sender IP and port for a received message.
// Should only be called from the callback.
procedure TTCPClientReceiver.GetSource(var address : string; var port : Integer);
begin
  address := FIpAddress;
  port := FPort;
end;

// SetReceiveBuffer():
// Changes the current buffer to use for reads.
// Should only be called from the callback.
procedure TTCPClientReceiver.SetReceiveBuffer(bytes : PByte; size : Integer);
begin
  FBuffer := bytes;
  FBufferSize := size;
end;

// Stop():
// Aborts an ongoing read. NOTE: Must NOT be called from the callback.
procedure TTCPClientReceiver.Stop;
begin
  // Tell thread to terminate
  FTerminated := True;
  if Assigned(FRunner) then FRunner.Terminate;

  FTcpClient.Disconnect;
  FTcpClient.Close;

  // If thread exist, wait for thread to terminate and then delete the object
  FreeAndNil(FRunner);

  FBuffer := nil;
  FBufferSize := 0;
end;
(**************************************************************************
*
**************************************************************************)
procedure TTCPClientReceiver.Run;
const
  cSizeInfoSize = 22;
type
  TPhase = (phSize, phPayload);
var
  Res : Integer;
  BufferIdx : Integer;
  BytesToRead : Integer;
  Phase : TPhase;
  ErrorDetected : Boolean;

  procedure HandleSizeInfo;
  begin
    // Get size of data packet from the received size packet
    BytesToRead := PInteger(@FBuffer[18])^;

    if BytesToRead > FBufferSize then begin
      // This is an error, we are not able to receive more than the buffer size
      FLastErrorCode := SOCKET_ERROR;
      Report('HandleSizeInfo', 'Error in read size info');
//      notifyNewEvent(BytesSizePair(NULL, -1));
      ErrorDetected := True;
		end;
    Phase := phPayload;
    BufferIdx := 0;
  end;

  procedure HandlePayload;
  begin
    // Notify upper layer with a data packet
    FDataNotifier.DoNotify(TBytesSizePair.Create(FBuffer, BufferIdx));

    // Set up for reading a new size info
    Phase := phSize;
    BytesToRead := cSizeInfoSize;
    BufferIdx := 0;
  end;

  procedure OpenAndConnect;
  begin
    FTcpClient.Open;
    FTcpClient.Connect;
  end;

  procedure DisconnectAndClose;
  begin
    FTcpClient.Disconnect;
    FTcpClient.Close;
  end;

begin
  while not FTerminated do begin
    try
      // Connect loop
      while (not FTerminated) and (not fTcpClient.Connected) do begin
        DisconnectAndClose;
        Sleep(100);
        OpenAndConnect;
      end;
      if FTerminated then Break;

      FTcpClient.SetNonBlocking(False);

      // Connected, Set buffer size
      if FInSocketBufferSize > 0 then begin
        FTcpClient.SetReceiveBufferSize(Integer(FInSocketBufferSize));
        if FTcpClient.GetReceiveBufferSize <> Integer(FInSocketBufferSize) then begin
          FLastErrorCode := SOCKET_ERROR;
          Report('Run', 'Socket buffer size could not be set');
        end;
      end;

      // Disable Nagle algorithm
      FTcpClient.SetTcpNoDelay(True);

      FConnectStatus.Address := FIpAddress;
      FConnectStatus.Port := FPort;
      FConnectStatus.Connected := True;
      FConnectStatus.TotalNo := 1;
      notifyConnectStatus(FConnectStatus);

//      notifyNewEvent(BytesSizePair(NULL, -5)); //Connection was down but has been reastablished.

      BufferIdx := 0;
      Phase := phSize;
      BytesToRead := cSizeInfoSize;
      ErrorDetected := False;

      // Read data loop
      while (not FTerminated and FTcpClient.Connected) do begin
        Res := FTcpClient.ReceiveBuf(FBuffer[BufferIdx], BytesToRead - BufferIdx);
        if FTerminated then Break;
        if Res = 0 then begin
          // Connection closed gracefully
          DisconnectAndClose;
          Break;

        end else if Res < 0 then begin
          // Some error
          FLastErrorCode := FTcpClient.LastError;
          Report('Run', 'Read failed');
//          notifyNewEvent(BytesSizePair(NULL, -2));
          DisconnectAndClose;
          Break;

        end else begin
          // Read OK
          Inc(BufferIdx, Res);
          if BufferIdx = BytesToRead then begin
            // Expected number of bytes read
            case Phase of
              phSize : HandleSizeInfo;
              phPayload : HandlePayload;
            end;
            if ErrorDetected then begin
              DisconnectAndClose;
              Break;
            end;
          end else begin
            // Continue to read bytes
          end;
        end;
      end;
      DisconnectAndClose;
    except
      on E: Exception do begin
        FLastErrorCode := SOCKET_ERROR;
        Report('Run', 'Exception "' + E.Message + '"');
        DisconnectAndClose;
      end;
    end;
    FConnectStatus.Connected := False;
    FConnectStatus.TotalNo := 0;
    notifyConnectStatus(FConnectStatus);
  end;
end;
end.

