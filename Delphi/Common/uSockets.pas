unit uSockets;

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

uses WinSock, Classes;

type
(**************************************************************************
*
**************************************************************************)
  TBaseIpSocket = class(TObject)
  private
    FSocketType : Integer;
    FProtocol : Integer;
    FSocket : TSocket;
    FLastError : Integer;

    FLocalHost : AnsiString;
    FLocalPort : Integer;

    function GetIsOpen : Boolean;

  public
    // SocketType = SOCK_STREAM, SOCK_DGRAM, ...
    // Protocol   = IPPROTO_TCP, IPPROTO_UDP
    constructor Create(SocketType : Integer; Protocol : Integer);
    destructor Destroy; override;

    // ------------------------------------------------------
    // Winsock API Helpers
    function Open: Boolean;
    function Close: Boolean; virtual;
    function Bind: Boolean;
    function SendTo(var Buffer; BufferSize: Integer; ToAddr: TSockAddr; Flags: Integer = 0): Integer;
    function ReceiveFrom(var Buffer; BufferSize: Integer; var FromAddr: TSockAddr; var Len: Integer; Flags: Integer = 0): Integer;
    function ReceiveBuf(var Buffer; BufferSize: Integer; Flags: Integer = 0): Integer;
    function SendBuf(var Buffer; BufferSize: Integer; Flags: Integer = 0): Integer;

    // True --> Non-Blocking
    function SetNonBlocking(Value : Boolean) : Boolean;

    function SetReceiveBufferSize(Size : Integer) : Boolean;
    function SetSendBufferSize(Size : Integer) : Boolean;
    function GetReceiveBufferSize: Integer;
    function GetSendBufferSize: Integer;

    // ------------------------------------------------------
    // Utility functions
    function GetLocalAddress(var LocalIP : AnsiString; var LocalPort : Integer) : Boolean;
    function GetPeerAddress(var PeerIP : AnsiString; var PeerPort : Integer) : Boolean;

    function MakeSockAddr(IPAddress : AnsiString; Port : Integer): TSockAddr;

    class function GetIpAddress(Addr : TSockAddr) : AnsiString;
    class function GetPort(Addr : TSockAddr) : Integer;

    // ------------------------------------------------------
    // Properties
    property LocalHost : AnsiString read FLocalHost write FLocalHost;
    property LocalPort : Integer read FLocalPort write FLocalPort;

    property IsOpen : Boolean read GetIsOpen;
    property Handle : TSocket read FSocket;
    property LastError : Integer read FLastError;
  end;

(**************************************************************************
*
**************************************************************************)
  TUdpSocket = class(TBaseIpSocket)
  public
    constructor Create;

    // ------------------------------------------------------
    // Winsock API Helpers
    function Close : Boolean; override;

    function SetReuseAddress(Value : Boolean) : Boolean;

    function SetMulticastTtl(Value : Integer) : Boolean;
    function SetMulticastInterface(IFaddr : AnsiString) : Boolean;
    procedure AddMulticastMembership(MCaddr : AnsiString; IFaddr : AnsiString);
    procedure DropMulticastMembership(MCaddr : AnsiString; IFaddr : AnsiString);
  end;

(**************************************************************************
*
**************************************************************************)
  TTcpSocket = class(TBaseIpSocket)
  public
    constructor Create;

    // ------------------------------------------------------
    // Winsock API Helpers
    function SetTcpNoDelay(Value: Boolean) : Boolean;
  end;

(**************************************************************************
*
**************************************************************************)
  TTcpClientSocket = class(TTcpSocket)
  private
    FConnected : Boolean;
    FRemoteHost : AnsiString;
    FRemotePort : Integer;

  public
    // ------------------------------------------------------
    // Winsock API Helpers
    function Connect : Boolean;
    function Disconnect : Boolean;

    // ------------------------------------------------------
    // Properties
    property Connected : Boolean read FConnected;
    property RemoteHost : AnsiString read FRemoteHost write FRemoteHost;
    property RemotePort : Integer read FRemotePort write FRemotePort;
  end;

(**************************************************************************
*
**************************************************************************)
  TTcpServerSocket = class(TTcpSocket)
  private
    FListening : Boolean;

  public
    // ------------------------------------------------------
    // Winsock API Helpers
    function Close : Boolean; override;
    function Listen : Boolean;
    function Accept(var ClientSocket: TTcpClientSocket): Boolean;

    // ------------------------------------------------------
    // Properties
    property Listening : Boolean read FListening;
  end;


implementation

uses Windows, SysUtils;

// ----------------------------------------------------------------------------

{ TBaseIpSocket }

constructor TBaseIpSocket.Create(SocketType : Integer; Protocol : Integer);
begin
  inherited Create;
  FSocketType := SocketType;
  FProtocol := Protocol;
  FSocket := INVALID_SOCKET;
  FLocalHost := '0.0.0.0';
  FLocalPort := 0;
end;

destructor TBaseIpSocket.Destroy;
begin
  Close;
  inherited;
end;

function TBaseIpSocket.GetIsOpen : Boolean;
begin
  Result := FSocket <> INVALID_SOCKET;
end;

function TBaseIpSocket.Open : Boolean;
begin
  Result := False;
  FLastError := 0;
  if FSocket = INVALID_SOCKET then begin
    //function socket(af, Struct, protocol: Integer): TSocket; stdcall;
    FSocket := socket(Integer(AF_INET), FSocketType, FProtocol);
    Result := FSocket <> INVALID_SOCKET;
    if not Result then begin
      FLastError := WSAGetLastError;
    end;
  end;
end;

function TBaseIpSocket.Close : Boolean;
begin
  FLastError := 0;
  if FSocket <> INVALID_SOCKET then begin
    //function closesocket(s: TSocket): Integer; stdcall;
    if closesocket(FSocket) = SOCKET_ERROR then begin
      FLastError := WSAGetLastError;
    end;
    FSocket := INVALID_SOCKET;
  end;
  Result := FlastError = 0;
end;

function TBaseIpSocket.Bind: Boolean;
var
  addr: TSockAddr;
begin
  FLastError := 0;
  if FSocket <> INVALID_SOCKET then begin
    addr := MakeSockAddr(FLocalHost, FLocalPort);
    //function bind(s: TSocket; var addr: TSockAddr; namelen: Integer): Integer; stdcall;
    if WinSock.bind(FSocket, addr, sizeof(addr)) <> 0 then begin
      FLastError := WSAGetLastError;
    end;
  end;
  Result := FlastError = 0;
end;

function TBaseIpSocket.SendTo(var Buffer; Buffersize: Integer; ToAddr: TSockAddr; Flags: Integer): Integer;
begin
  FLastError := 0;
  //function sendto(s: TSocket; const Buf; len, flags: Integer; var addrto: TSockAddr;
  //  tolen: Integer): Integer; stdcall;
  Result := WinSock.sendto(FSocket, Buffer, BufferSize, Flags, ToAddr, sizeof(ToAddr));
  if Result = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;
end;

function TBaseIpSocket.ReceiveFrom(var Buffer; BufferSize: Integer; var FromAddr: TSockAddr; var Len: Integer; Flags: Integer): Integer;
begin
  FLastError := 0;
  //function recvfrom(s: TSocket; var Buf; len, flags: Integer;
  //  var from: TSockAddr; var fromlen: Integer): Integer; stdcall;
  Result := WinSock.recvfrom(FSocket, Buffer, BufferSize, Flags, FromAddr, Len);
  if Result = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;
end;

function TBaseIpSocket.ReceiveBuf(var Buffer; BufferSize: Integer; Flags: Integer): Integer;
begin
  FLastError := 0;
  //function recv(s: TSocket; var Buf; len, flags: Integer): Integer; stdcall;
  Result := recv(FSocket, Buffer, BufferSize, Flags);
  if Result = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;
end;

function TBaseIpSocket.SendBuf(var Buffer; BufferSize: Integer; Flags: Integer): Integer;
begin
  FLastError := 0;
  //function send(s: TSocket; const Buf; len, flags: Integer): Integer; stdcall;
  Result := send(FSocket, Buffer, BufferSize, Flags);
  if Result = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;
end;

// True --> Non-Blocking
function TBaseIpSocket.SetNonBlocking(Value : Boolean) : Boolean;
var
  NonBlock : u_long;
begin
  FLastError := 0;
  if value then begin
    NonBlock := 1;
  end else begin
    NonBlock := 0;
  end;
  //function ioctlsocket(s: TSocket; cmd: DWORD; var arg: u_long): Integer; stdcall;
  if ioctlsocket(FSocket, FIONBIO, NonBlock) = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;
  Result := FlastError = 0;
end;

function TBaseIpSocket.GetReceiveBufferSize: Integer;
var
  OptLen : Integer;
  OptVal : Integer;
begin
  FLastError := 0;
  OptVal := 0;
  if Handle <> INVALID_SOCKET then begin
    OptLen := 4;
    // function getsockopt(s: TSocket; level, optname: Integer; optval: PAnsiChar; var optlen: Integer): Integer; stdcall;
    if getsockopt(FSocket, SOL_SOCKET, SO_RCVBUF, PAnsiChar(@OptVal), OptLen) = SOCKET_ERROR then begin
      FLastError := WSAGetLastError;
    end;
  end;
  Result := OptVal;
end;

function TBaseIpSocket.GetSendBufferSize: Integer;
var
  OptLen : Integer;
  OptVal : Integer;
begin
  FLastError := 0;
  OptVal := 0;
  if Handle <> INVALID_SOCKET then begin
    OptLen := 4;
    // function getsockopt(s: TSocket; level, optname: Integer; optval: PAnsiChar; var optlen: Integer): Integer; stdcall;
    if getsockopt(FSocket, SOL_SOCKET, SO_SNDBUF, PAnsiChar(@OptVal), OptLen) = SOCKET_ERROR then begin
      FLastError := WSAGetLastError;
    end;
  end;
  Result := OptVal;
end;

function TBaseIpSocket.SetReceiveBufferSize(Size: Integer) : Boolean;
var
  OptLen : Integer;
  OptVal : Integer;
begin
  Result := False;
  FLastError := 0;
  if Handle = INVALID_SOCKET then Exit;

  OptVal := size;
  OptLen := 4;
  // function setsockopt(s: TSocket; level, optname: Integer; optval: PAnsiChar; optlen: Integer): Integer; stdcall;
  if setsockopt(FSocket, SOL_SOCKET, SO_RCVBUF, PAnsiChar(@OptVal), OptLen) = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;
  Result := FlastError = 0;
end;

function TBaseIpSocket.SetSendBufferSize(Size: Integer) : Boolean;
var
  OptLen : Integer;
  OptVal : Integer;
begin
  Result := False;
  FLastError := 0;
  if Handle = INVALID_SOCKET then Exit;

  OptVal := size;
  OptLen := 4;
  // function setsockopt(s: TSocket; level, optname: Integer; optval: PAnsiChar; optlen: Integer): Integer; stdcall;
  if setsockopt(FSocket, SOL_SOCKET, SO_SNDBUF, PAnsiChar(@OptVal), OptLen) = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;
  Result := FlastError = 0;
end;

class function TBaseIpSocket.GetIpAddress(Addr : TSockAddr) : AnsiString;
begin
  //function inet_ntoa(inaddr: TInAddr): PAnsiChar; stdcall;
  Result := inet_ntoa(addr.sin_addr);
end;

class function TBaseIpSocket.GetPort(Addr : TSockAddr) : Integer;
begin
  //function ntohs(netshort: u_short): u_short; stdcall;
  Result := ntohs(addr.sin_port);
end;

function TBaseIpSocket.GetPeerAddress(var PeerIP : AnsiString; var PeerPort : Integer) : Boolean;
var
  len : Integer;
  addr : TSockAddr;
begin
  FLastError := 0;
  len := SizeOf(addr);

  if getpeername(Handle, addr,len) = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;

  PeerIP := GetIpAddress(addr);
  PeerPort := GetPort(addr);

  Result := FLastError = 0;
end;

function TBaseIpSocket.GetLocalAddress(var LocalIP : AnsiString; var LocalPort : Integer) : Boolean;
var
  len : Integer;
  addr : TSockAddr;
begin
  FLastError := 0;
  len := SizeOf(addr);

  //function getsockname(s: TSocket; var name: TSockAddr; var namelen: Integer): Integer; stdcall;
  if getsockname(Handle, addr, len) = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;

  LocalIP := GetIpAddress(addr);
  LocalPort := GetPort(addr);

  Result := FLastError = 0;
end;

function TBaseIpSocket.MakeSockAddr(IPAddress : AnsiString; Port : Integer) : TSockAddr;
begin
  //TSockAddr = sockaddr_in = record
  //  case Integer of
  //    0: (sin_family: u_short;
  //        sin_port: u_short;
  //        sin_addr: TInAddr;
  //        sin_zero: array[0..7] of AnsiChar);
  //    1: (sa_family: u_short;
  //        sa_data: array[0..13] of AnsiChar)
  //end;
  Result.sin_family := AF_INET;
  //function htons(hostshort: u_short): u_short; stdcall;
  Result.sin_port := htons(Port);
  //function inet_addr(cp: PAnsiChar): u_long; stdcall; {PInAddr;}  { TInAddr }
  Result.sin_addr.s_addr := inet_addr(PAnsiChar(IPAddress));
  FillChar(Result.sin_zero, SizeOf(Result.sin_zero), 0);
end;

// ----------------------------------------------------------------------------

{ TUdpSocket }

constructor TUdpSocket.Create;
begin
  inherited Create(SOCK_DGRAM, IPPROTO_UDP);
end;

function TUdpSocket.Close : Boolean;
begin
  //function shutdown(s: TSocket; how: Integer): Integer; stdcall;
  shutdown(FSocket, SD_BOTH);
  Result := inherited Close;
end;

function TUdpSocket.SetReuseAddress(Value : Boolean) : Boolean;
var
  Flag : BOOL;
begin
  Result := False;
  FLastError := 0;
  if Handle = INVALID_SOCKET then Exit;

  Flag := value;
  //function setsockopt(s: TSocket; level, optname: Integer; optval: PAnsiChar; optlen: Integer): Integer; stdcall;
  if setsockopt(Handle, SOL_SOCKET, SO_REUSEADDR, PAnsiChar(@Flag), SizeOf(Flag)) = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;
  Result := FlastError = 0;
end;

function TUdpSocket.SetMulticastTtl(Value : Integer) : Boolean;
var
  OptLen : Integer;
  OptVal : Integer;
begin
  Result := False;
  FLastError := 0;
  if Handle = INVALID_SOCKET then Exit;

  OptVal := value;
  OptLen := 4;
  //function setsockopt(s: TSocket; level, optname: Integer; optval: PAnsiChar; optlen: Integer): Integer; stdcall;
  if setsockopt(Handle, IPPROTO_IP, IP_MULTICAST_TTL, PAnsiChar(@OptVal), OptLen) = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;
  Result := FlastError = 0;
end;

function TUdpSocket.SetMulticastInterface(IFaddr : AnsiString) : Boolean;
var
  OptLen : Integer;
  OptVal : Integer;
  addr : TSockAddr;
begin
  Result := False;
  FLastError := 0;
  if Handle = INVALID_SOCKET then Exit;

  addr := MakeSockAddr(ifaddr, 0);
  OptVal := addr.sin_addr.S_addr;
  OptLen := SizeOf(OptVal);

  //function setsockopt(s: TSocket; level, optname: Integer; optval: PAnsiChar; optlen: Integer): Integer; stdcall;
  if setsockopt(Handle, IPPROTO_IP, IP_MULTICAST_IF, PAnsiChar(@OptVal), OptLen) = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;
  Result := FlastError = 0;
end;

type
  //typedef struct ip_mreq {
  //  struct in_addr  imr_multiaddr;
  //  struct in_addr  imr_interface;
  //} IP_MREQ, *PIP_MREQ;
  ip_mreq = record
    imr_multiaddr : in_addr;
    imr_interface : in_addr;
  end;

procedure TUdpSocket.AddMulticastMembership(MCaddr : AnsiString; IFaddr : AnsiString);
var
  OptLen : Integer;
  OptVal : ip_mreq;
  addr : TSockAddr;
begin
  FLastError := 0;
  if Handle = INVALID_SOCKET then Exit;

  addr := MakeSockAddr(mcaddr, 0);
  OptVal.imr_multiaddr := addr.sin_addr;

  addr := MakeSockAddr(ifaddr, 0);
  OptVal.imr_interface := addr.sin_addr;

  OptLen := SizeOf(OptVal);

  //function setsockopt(s: TSocket; level, optname: Integer; optval: PAnsiChar; optlen: Integer): Integer; stdcall;
  if setsockopt(Handle, IPPROTO_IP, IP_ADD_MEMBERSHIP, PAnsiChar(@OptVal), OptLen) = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;
end;

procedure TUdpSocket.DropMulticastMembership(MCaddr : AnsiString; IFaddr : AnsiString);
var
  OptLen : Integer;
  OptVal : ip_mreq;
  addr : TSockAddr;
begin
  FLastError := 0;
  if Handle = INVALID_SOCKET then Exit;

  addr := MakeSockAddr(mcaddr, 0);
  OptVal.imr_multiaddr := addr.sin_addr;

  addr := MakeSockAddr(ifaddr, 0);
  OptVal.imr_interface := addr.sin_addr;

  OptLen := SizeOf(OptVal);

  //function setsockopt(s: TSocket; level, optname: Integer; optval: PAnsiChar; optlen: Integer): Integer; stdcall;
  if setsockopt(Handle, IPPROTO_IP, IP_DROP_MEMBERSHIP, PAnsiChar(@OptVal), OptLen) = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;
end;

// ----------------------------------------------------------------------------

{ TTcpSocket }

constructor TTcpSocket.Create;
begin
  inherited Create(SOCK_STREAM, IPPROTO_TCP);
end;

function TTcpSocket.SetTcpNoDelay(Value: Boolean) : Boolean;
var
  Flag : BOOL;
begin
  Result := False;
  FLastError := 0;
  if Handle = INVALID_SOCKET then Exit;

  Result := True;
  Flag := value;
  //function setsockopt(s: TSocket; level, optname: Integer; optval: PAnsiChar; optlen: Integer): Integer; stdcall;
  if setsockopt(Handle, IPPROTO_TCP, TCP_NODELAY, PAnsiChar(@Flag), SizeOf(Flag)) = SOCKET_ERROR then begin
    Result := False;
    FLastError := WSAGetLastError;
  end;
end;

// ----------------------------------------------------------------------------

{ TTcpClientSocket }

function TTcpClientSocket.Connect : Boolean;
var
  addr: TSockAddr;
begin
  FLastError := 0;
  if IsOpen and not FConnected then begin
    addr := MakeSockAddr(FRemoteHost, FRemotePort);
    //function connect(s: TSocket; var name: TSockAddr; namelen: Integer): Integer; stdcall;
    FConnected := WinSock.connect(FSocket, addr, sizeof(addr)) = 0;
    if not FConnected then begin
      FLastError := WSAGetLastError;
    end;
  end;
  Result := FConnected;
end;

function TTcpClientSocket.Disconnect : Boolean;
begin
  FLastError := 0;
  Result := True;
  if FConnected then begin
    //function shutdown(s: TSocket; how: Integer): Integer; stdcall;
    shutdown(FSocket, SD_BOTH);
    FConnected := False;
  end;
end;

// ----------------------------------------------------------------------------

{ TTcpServerSocket }

function TTcpServerSocket.Close : Boolean;
begin
  FListening := False;
  Result := inherited Close;
end;

function TTcpServerSocket.Listen : Boolean;
begin
  FLastError := 0;
  if IsOpen and not FListening then begin
    //function listen(s: TSocket; backlog: Integer): Integer; stdcall;
    FListening := WinSock.listen(FSocket, SOMAXCONN) = 0;
    if not FListening then begin
      FLastError := WSAGetLastError;
    end;
  end;
  Result := FListening;
end;

function TTcpServerSocket.Accept(var ClientSocket: TTcpClientSocket): Boolean;
var
  sock : TSocket;
  addr : TSockAddr;
  len : Integer;
begin
  FLastError := 0;
  Result := False;

  Fillchar(addr, sizeof(addr), 0);
  len := sizeof(addr);

  //function accept(s: TSocket; addr: PSockAddr; addrlen: PInteger): TSocket; stdcall;
  Sock := WinSock.accept(FSocket, @addr, @len);

  if Sock <> INVALID_SOCKET then begin
    Result := True;
    ClientSocket.FSocket := Sock;
    ClientSocket.FSocketType := FSocketType;
    ClientSocket.FProtocol := FProtocol;
    ClientSocket.FConnected := True;
    ClientSocket.FRemoteHost := GetIpAddress(addr);
    ClientSocket.FRemotePort := GetPort(addr);
  end else begin
    FLastError := WSAGetLastError;
  end;
end;

// ----------------------------------------------------------------------------

var
  WSAData: TWSAData;

initialization
  //TODO change to $0202 ?
  //function WSAStartup(wVersionRequired: word; var WSData: TWSAData): Integer; stdcall;
  if WSAStartup($0101, WSAData) <> 0 then begin
    raise Exception.Create('WSAStartup: Error: ' + IntToStr(WSAGetLastError));
  end;

finalization
  //function WSACleanup: Integer; stdcall;
  if WSACleanup <> 0 then begin
    raise Exception.Create('WSACleanup: Error: ' + IntToStr(WSAGetLastError));
  end;

end.

