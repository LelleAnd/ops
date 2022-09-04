unit uOps.Transport.TCPServer;

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
     uRunner,
     uOps.Types,
     uOps.Error,
     uOps.Transport.Sender,
     uSockets;

type
  TTCPServerSender = class(TSender)
  private
    FPort : Integer;
    FActualPort : Integer;
    FIpAddress : string;
    FOutSocketBufferSize : Int64;

    FTcpServer : TTcpServerSocket;

    // List owns objects, i.e. when list is free'd all objects in it is free'd
    FConnectedSockets : TObjectList<TTcpClientSocket>;

    // Our thread running our Run() method
    FRunner : TRunner;
    FTerminated : Boolean;

    // Will by called by the FRunner thread
    procedure Run;

    procedure Report(method : string; mess : string);

  public
    constructor Create(serverIP : string; serverPort : Integer; outSocketBufferSize : Int64 = 16000000);
    destructor Destroy; override;

    procedure Open(); override;
    procedure Close(); override;

    function sendTo(buf : PByte; size : Integer; ip : string; port : Integer) : Boolean; override;

    function getPort() : Integer; override;
    function getAddress() : string; override;
  end;

implementation

uses SysUtils,
{$IF CompilerVersion >= 31}      // Delphi 10.1 Berlin
     AnsiStrings,
{$IFEND}
     WinSock;

constructor TTCPServerSender.Create(serverIP : string; serverPort : Integer; outSocketBufferSize : Int64);
begin
  inherited Create;
  FPort := serverPort;
  FIpAddress := serverIP;
  FOutSocketBufferSize := outSocketBufferSize;

  FConnectedSockets := TObjectList<TTcpClientSocket>.Create;

  fTcpServer := TTcpServerSocket.Create;
  fTcpServer.LocalPort := FPort;
  fTcpServer.LocalHost := AnsiString(FIpAddress);
end;

destructor TTCPServerSender.Destroy;
begin
  Close();

  FreeAndNil(FTcpServer);
  FreeAndNil(FConnectedSockets);
  inherited;
end;

procedure TTCPServerSender.Report(method : string; mess : string);
begin
  if Assigned(FErrorService) then begin
    FErrorService.Report(TSocketError.Create('TCPServerSender', method, mess, FLastErrorCode));
  end;
end;

procedure TTCPServerSender.Open();
var
  LocalIP : AnsiString;
begin
  if Assigned(FRunner) then Exit;

  FTcpServer.Open;
  FTcpServer.Bind;

  FTcpServer.GetLocalAddress(LocalIp, FActualPort);

  FTerminated := False;

  // Start a thread running our run() method
  FRunner := TRunner.Create(Run);
end;

procedure TTCPServerSender.Close();
var
  i : Integer;
  ConnectStatus : TConnectStatus;
begin
  // Tell thread to terminate
  FTerminated := True;
  if Assigned(FRunner) then FRunner.Terminate;

  FTcpServer.Close;

  // If thread exist, wait for thread to terminate and then delete the object
  FreeAndNil(FRunner);

  for i := FConnectedSockets.Count - 1 downto 0 do begin
    ConnectStatus.Address := string(FConnectedSockets[i].RemoteHost);
    ConnectStatus.Port := FConnectedSockets[i].RemotePort;
    ConnectStatus.TotalNo := 0;
    ConnectStatus.Connected := False;
    notifyConnectStatus(ConnectStatus);
  end;

  // Free all connected sockets, which implicitly will close all connections
  FConnectedSockets.Clear;
end;

function TTCPServerSender.getPort() : Integer;
begin
  Result := FActualPort;
end;

function TTCPServerSender.getAddress() : string;
begin
  Result := FIpAddress;
end;

// Sends buf to any Receiver connected to this Sender, ip and port are discarded and can be left blank.
function TTCPServerSender.sendTo(buf : PByte; size : Integer; ip : string; port : Integer) : Boolean;
var
  i : Integer;
  sizeInfo : array[0..100] of AnsiChar;
  errorFlag : Boolean;
  ConnectStatus : TConnectStatus;
begin
  Result := True;

  // First, prepare a package of fixed length 22 with information about the size of the data package
{$IF CompilerVersion >= 31}      // Delphi 10.1 Berlin
  AnsiStrings.
{$IFEND}
  StrPCopy(@sizeInfo[0], AnsiString('opsp_tcp_size_info'));
  PInteger(@sizeInfo[18])^ := size;

  // Send to anyone connected. Loop backwards to avoid problems when removing broken sockets
  for i := FConnectedSockets.Count - 1 downto 0 do begin
    ErrorFlag := False;
    try
      // Send prepared size info
      if FConnectedSockets[i].SendBuf(sizeInfo, 22) <> SOCKET_ERROR then begin
        // Send the actual data
        if FConnectedSockets[i].SendBuf(buf^, size) = SOCKET_ERROR then begin
          ErrorFlag := True;
        end;
      end else begin
        ErrorFlag := True;
      end;
    except
      ErrorFlag := True;
    end;

    if ErrorFlag then begin
      FLastErrorCode := FConnectedSockets[i].LastError;
      Report('sendTo', 'Error sending');

      ConnectStatus.Address := string(FConnectedSockets[i].RemoteHost);
      ConnectStatus.Port := FConnectedSockets[i].RemotePort;
      ConnectStatus.TotalNo := FConnectedSockets.Count-1;
      ConnectStatus.Connected := (ConnectStatus.TotalNo > 0);
      notifyConnectStatus(ConnectStatus);

      FConnectedSockets.Delete(i);
    end;
  end;
end;

procedure TTCPServerSender.Run;
var
  tcpClient : TTcpClientSocket;
  ConnectStatus : TConnectStatus;
begin
  tcpClient := nil;

  while not FTerminated do begin
    try
      // Setup server socket for listening
      FTcpServer.Listen;

      if not FTcpServer.Listening then begin
        Sleep(100);
        Continue;
      end;

      // Keep listening for connecting clients
      while not FTerminated do begin
        // Create a client socket ready for the calling client
        tcpClient := TTcpClientSocket.Create;

        // accept()
        while (not FTerminated) and (not FTcpServer.Accept(tcpClient)) do
          Sleep(10);

        if FTerminated then Break;

        // Now we have a connected client, setup some parameters
        if FOutSocketBufferSize > 0 then begin
          tcpClient.SetSendBufferSize(Integer(FOutSocketBufferSize));
          if tcpClient.GetSendBufferSize <> Integer(FOutSocketBufferSize) then begin
            FLastErrorCode := SOCKET_ERROR;
            Report('Run', 'Socket buffer size could not be set');
          end;
        end;

        // Disable Nagle algorithm
        tcpClient.SetTcpNoDelay(True);

        // and put it in list and then wait for another connection
        FConnectedSockets.Add(tcpClient);

        ConnectStatus.Address := string(tcpClient.RemoteHost);
        ConnectStatus.Port := tcpClient.RemotePort;
        ConnectStatus.Connected := True;
        ConnectStatus.TotalNo := FConnectedSockets.Count;
        notifyConnectStatus(ConnectStatus);

        tcpClient := nil;   // Clear ref since list now owns object
      end;
    except
      FreeAndNil(tcpClient);
    end;
  end;
  FreeAndNil(tcpClient);
end;

end.

