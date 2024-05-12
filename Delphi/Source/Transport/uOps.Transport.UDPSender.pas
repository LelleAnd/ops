unit uOps.Transport.UDPSender;

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

uses uOps.Transport.Sender,
     uSockets;

type
  // A sender implementation that dispatches messages over ip based UDP.
  TUDPSender = class(TSender)
  private
    FLocalInterface : string;
    FTtl : Integer;
    FOutSocketBufferSize : Int64;
    FMulticastSocket : Boolean;

    FSocket: TUdpSocket;

    procedure Report(method : string; mess : string);

  public
    // Constructs a new UDPSender and binds its underlying socket to local host
    // and a dynamically allocated local port.
		// This class accepts synchronous write operations through sendTo().
    constructor Create(localInterface : string = '0.0.0.0';
                       ttl : Integer = 1;
                       outSocketBufferSize : Int64 = 16000000;
                       multicastSocket : Boolean = False);
    destructor Destroy; override;

    procedure Open(); override;
    procedure Close(); override;

    function sendTo(buf : PByte; size : Integer; ip : string; port : Integer) : Boolean; override;

    function getPort() : Integer; override;
    function getAddress() : string; override;

  end;

implementation

uses SysUtils,
     Winapi.Winsock2,
     uOps.Error;

{ TUDPSender }

constructor TUDPSender.Create(localInterface: string; ttl: Integer;
  outSocketBufferSize: Int64; multicastSocket: Boolean);
begin
  inherited Create;

  FLocalInterface := localInterface;
  FTtl := ttl;
  FOutSocketBufferSize := outSocketBufferSize;
  FMulticastSocket := multicastSocket;

  Open;
end;

destructor TUDPSender.Destroy;
begin
  Close;
  inherited;
end;

procedure TUDPSender.Report(method : string; mess : string);
begin
  if Assigned(FErrorService) then begin
    FErrorService.Report(TSocketError.Create('UDPSender', method, mess, FLastErrorCode));
  end;
end;

procedure TUDPSender.Open;
begin
  if Assigned(FSocket) then Exit;

  FSocket := TUdpSocket.Create;

  if not FSocket.Open then begin
    FLastErrorCode := Fsocket.LastError;
    Report('Open', 'Socket could not be created');
    Exit;
  end;

  FSocket.SetNonBlocking(True);

  if FOutSocketBufferSize > 0 then begin
    FSocket.SetSendBufferSize(Integer(FOutSocketBufferSize));
    if FSocket.GetSendBufferSize <> Integer(FOutSocketBufferSize) then begin
      FLastErrorCode := SOCKET_ERROR;
      Report('Open', 'Socket buffer size could not be set');
    end;
  end;

  if FMulticastSocket then begin
    FSocket.SetMulticastTtl(FTtl);
    FSocket.SetMulticastInterface(AnsiString(FLocalInterface));
  end;

  FSocket.Bind;
end;

procedure TUDPSender.Close;
begin
  FreeAndNil(FSocket);
end;

function TUDPSender.getAddress: string;
var
  dummy : Integer;
  str : AnsiString;
begin
  Result := '';
  if not Assigned(FSocket) then Exit;
  dummy := 0;
  FSocket.GetLocalAddress(str, dummy);
  Result := string(str);
end;

function TUDPSender.getPort: Integer;
var
  dummy : AnsiString;
begin
  Result := 0;
  if not Assigned(FSocket) then Exit;
  FSocket.GetLocalAddress(dummy, Result);
end;

function TUDPSender.sendTo(buf: PByte; size: Integer; ip: string; port: Integer): Boolean;
var
  ToAddr : TSockAddr;
begin
  Result := False;
  if not Assigned(FSocket) then Exit;
  FLastErrorCode := 0;

  try
    ToAddr := FSocket.MakeSockAddr(AnsiString(ip), port);
    if FSocket.SendTo(buf^, size, ToAddr) = SOCKET_ERROR then begin
      FLastErrorCode := FSocket.LastError;
      Report('sendTo', 'sendto() failed');
    end else begin
      Result := True;
    end;

  except
    FLastErrorCode := SOCKET_ERROR;
    Report('sendTo',
           'Exception when sending udp message: Params: buf = ' + IntToHex(Int64(buf),8) +
           ', size = ' + IntToStr(size) + ', ip = ' + ip + ', port = ' + IntToStr(port)
          );
  end;
end;

end.

