unit uSockets;

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

uses Winapi.Winsock2, Classes;

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
    function Close : Boolean; override;
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
    //function socket(af, type_, protocol: Integer): TSocket; stdcall;
    FSocket := Winapi.WinSock2.socket(Integer(AF_INET), FSocketType, FProtocol);
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
    if Winapi.WinSock2.closesocket(FSocket) = SOCKET_ERROR then begin
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
    //function bind(s: TSocket; var name: TSockAddr; namelen: Integer): Integer; stdcall;
    if Winapi.WinSock2.bind(FSocket, addr, sizeof(addr)) <> 0 then begin
      FLastError := WSAGetLastError;
    end;
  end;
  Result := FlastError = 0;
end;

function TBaseIpSocket.SendTo(var Buffer; Buffersize: Integer; ToAddr: TSockAddr; Flags: Integer): Integer;
begin
  FLastError := 0;
  //function sendto(s: TSocket; const buf; len, flags: Integer; toaddr: PSockAddr; tolen: Integer): Integer; stdcall;
  Result := Winapi.WinSock2.sendto(FSocket, Buffer, BufferSize, Flags, @ToAddr, sizeof(ToAddr));
  if Result = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;
end;

function TBaseIpSocket.ReceiveFrom(var Buffer; BufferSize: Integer; var FromAddr: TSockAddr; var Len: Integer; Flags: Integer): Integer;
begin
  FLastError := 0;
  //function recvfrom(s: TSocket; var buf; len, flags: Integer; var from: TSockAddr; var fromlen: Integer): Integer; stdcall;
  Result := Winapi.WinSock2.recvfrom(FSocket, Buffer, BufferSize, Flags, FromAddr, Len);
  if Result = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;
end;

function TBaseIpSocket.ReceiveBuf(var Buffer; BufferSize: Integer; Flags: Integer): Integer;
begin
  FLastError := 0;
  //function recv(s: TSocket; var buf; len, flags: Integer): Integer; stdcall;
  Result := Winapi.WinSock2.recv(FSocket, Buffer, BufferSize, Flags);
  if Result = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;
end;

function TBaseIpSocket.SendBuf(var Buffer; BufferSize: Integer; Flags: Integer): Integer;
begin
  FLastError := 0;
  //function send(s: TSocket; const buf; len, flags: Integer): Integer; stdcall;
  Result := Winapi.WinSock2.send(FSocket, Buffer, BufferSize, Flags);
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
  //function ioctlsocket(s: TSocket; cmd: Longint; var argp: u_long): Integer; stdcall;
  if Winapi.WinSock2.ioctlsocket(FSocket, Longint(FIONBIO), NonBlock) = SOCKET_ERROR then begin
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
  if FSocket <> INVALID_SOCKET then begin
    OptLen := 4;
    //function getsockopt(s: TSocket; level, optname: Integer; optval: MarshaledAString; var optlen: Integer): Integer; stdcall;
    if Winapi.WinSock2.getsockopt(FSocket, SOL_SOCKET, SO_RCVBUF, MarshaledAString(@OptVal), OptLen) = SOCKET_ERROR then begin
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
  if FSocket <> INVALID_SOCKET then begin
    OptLen := 4;
    //function getsockopt(s: TSocket; level, optname: Integer; optval: MarshaledAString; var optlen: Integer): Integer; stdcall;
    if Winapi.WinSock2.getsockopt(FSocket, SOL_SOCKET, SO_SNDBUF, MarshaledAString(@OptVal), OptLen) = SOCKET_ERROR then begin
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
  if FSocket = INVALID_SOCKET then Exit;

  OptVal := size;
  OptLen := 4;
  //function setsockopt(s: TSocket; level, optname: Integer; optval: MarshaledAString; optlen: Integer): Integer; stdcall;
  if Winapi.WinSock2.setsockopt(FSocket, SOL_SOCKET, SO_RCVBUF, MarshaledAString(@OptVal), OptLen) = SOCKET_ERROR then begin
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
  if FSocket = INVALID_SOCKET then Exit;

  OptVal := size;
  OptLen := 4;
  //function setsockopt(s: TSocket; level, optname: Integer; optval: MarshaledAString; optlen: Integer): Integer; stdcall;
  if Winapi.WinSock2.setsockopt(FSocket, SOL_SOCKET, SO_SNDBUF, MarshaledAString(@OptVal), OptLen) = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;
  Result := FlastError = 0;
end;

class function TBaseIpSocket.GetIpAddress(Addr : TSockAddr) : AnsiString;
begin
  //function inet_ntoa(inaddr: in_addr): MarshaledAString; stdcall;
  Result := Winapi.WinSock2.inet_ntoa(TSockAddrIn(addr).sin_addr);
end;

class function TBaseIpSocket.GetPort(Addr : TSockAddr) : Integer;
begin
  //function ntohs(netshort: u_short): u_short; stdcall;
  Result := Winapi.WinSock2.ntohs(TSockAddrIn(addr).sin_port);
end;

function TBaseIpSocket.GetPeerAddress(var PeerIP : AnsiString; var PeerPort : Integer) : Boolean;
var
  len : Integer;
  addr : TSockAddr;
begin
  FLastError := 0;
  len := SizeOf(addr);

  //function getpeername(s: TSocket; var name: TSockAddr; var namelen: Integer): Integer; stdcall;
  if Winapi.WinSock2.getpeername(FSocket, addr, len) = SOCKET_ERROR then begin
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
  if Winapi.WinSock2.getsockname(FSocket, addr, len) = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;

  LocalIP := GetIpAddress(addr);
  LocalPort := GetPort(addr);

  Result := FLastError = 0;
end;

function TBaseIpSocket.MakeSockAddr(IPAddress : AnsiString; Port : Integer) : TSockAddr;
begin
  //sockaddr = record
  //  sa_family: u_short;                 // address family
  //  sa_data: array [0..13] of a_char; // up to 14 bytes of direct address
  //end;
  //TSockAddr = sockaddr;

  //sockaddr_in = record
  //  sin_family: Smallint;
  //  sin_port: u_short;
  //  sin_addr: in_addr;
  //  sin_zero: array [0..7] of a_char;
  //end;
  //TSockAddrIn = sockaddr_in;

  TSockAddrIn(Result).sin_family := AF_INET;
  //function htons(hostshort: u_short): u_short; stdcall;
  TSockAddrIn(Result).sin_port := Winapi.WinSock2.htons(Port);
  //function inet_addr(cp: MarshaledAString): u_long; stdcall;
  TSockAddrIn(Result).sin_addr.s_addr := Winapi.WinSock2.inet_addr(MarshaledAString(IPAddress));
  FillChar(TSockAddrIn(Result).sin_zero, SizeOf(TSockAddrIn(Result).sin_zero), 0);
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
  Winapi.WinSock2.shutdown(FSocket, SD_BOTH);
  Result := inherited Close;
end;

function TUdpSocket.SetReuseAddress(Value : Boolean) : Boolean;
var
  Flag : BOOL;
begin
  Result := False;
  FLastError := 0;
  if FSocket = INVALID_SOCKET then Exit;

  Flag := value;
  //function setsockopt(s: TSocket; level, optname: Integer; optval: MarshaledAString; optlen: Integer): Integer; stdcall;
  if Winapi.WinSock2.setsockopt(FSocket, SOL_SOCKET, SO_REUSEADDR, MarshaledAString(@Flag), SizeOf(Flag)) = SOCKET_ERROR then begin
    FLastError := WSAGetLastError;
  end;
  Result := FlastError = 0;
end;

const
  /// Constants from ws2ipdef.h (Windows SDK 10.0.22621) missing in Winsock2.pas

  //
  // Options to use with [gs]etsockopt at the IPPROTO_IP level.
  // The values should be consistent with the IPv6 equivalents.
  //
  IP_OPTIONS                =  1; // Set/get IP options.
  IP_HDRINCL                =  2; // Header is included with data.
  IP_TOS                    =  3; // IP type of service.
  IP_TTL                    =  4; // IP TTL (hop limit).
  IP_MULTICAST_IF           =  9; // IP multicast interface.
  IP_MULTICAST_TTL          = 10; // IP multicast TTL (hop limit).
  IP_MULTICAST_LOOP         = 11; // IP multicast loopback.
  IP_ADD_MEMBERSHIP         = 12; // Add an IP group membership.
  IP_DROP_MEMBERSHIP        = 13; // Drop an IP group membership.
  IP_DONTFRAGMENT           = 14; // Don't fragment IP datagrams.
  IP_ADD_SOURCE_MEMBERSHIP  = 15; // Join IP group/source.
  IP_DROP_SOURCE_MEMBERSHIP = 16; // Leave IP group/source.
  IP_BLOCK_SOURCE           = 17; // Block IP group/source.
  IP_UNBLOCK_SOURCE         = 18; // Unblock IP group/source.
  IP_PKTINFO                = 19; // Receive packet information.
  IP_HOPLIMIT               = 21; // Receive packet hop limit.
  IP_RECVTTL                = 21; // Receive packet Time To Live (TTL).
  IP_RECEIVE_BROADCAST      = 22; // Allow/block broadcast reception.
  IP_RECVIF                 = 24; // Receive arrival interface.
  IP_RECVDSTADDR            = 25; // Receive destination address.
  IP_IFLIST                 = 28; // Enable/Disable an interface list.
  IP_ADD_IFLIST             = 29; // Add an interface list entry.
  IP_DEL_IFLIST             = 30; // Delete an interface list entry.
  IP_UNICAST_IF             = 31; // IP unicast interface.
  IP_RTHDR                  = 32; // Set/get IPv6 routing header.
  IP_GET_IFLIST             = 33; // Get an interface list.
  IP_RECVRTHDR              = 38; // Receive the routing header.
  IP_TCLASS                 = 39; // Packet traffic class.
  IP_RECVTCLASS             = 40; // Receive packet traffic class.
  IP_RECVTOS                = 40; // Receive packet Type Of Service (TOS).
  IP_ORIGINAL_ARRIVAL_IF    = 47; // Original Arrival Interface Index.
  IP_ECN                    = 50; // IP ECN codepoint.
  IP_RECVECN                = 50; // Receive ECN codepoints in the IP header.
  IP_PKTINFO_EX             = 51; // Receive extended packet information.
  IP_WFP_REDIRECT_RECORDS   = 60; // WFP's Connection Redirect Records.
  IP_WFP_REDIRECT_CONTEXT   = 70; // WFP's Connection Redirect Context.
  IP_MTU_DISCOVER           = 71; // Set/get path MTU discover state.
  IP_MTU                    = 73; // Get path MTU.
  IP_NRT_INTERFACE          = 74; // Set NRT interface constraint (outbound).
  IP_RECVERR                = 75; // Receive ICMP errors.
  IP_USER_MTU               = 76; // Set/get app defined upper bound IP layer MTU.

function TUdpSocket.SetMulticastTtl(Value : Integer) : Boolean;
var
  OptLen : Integer;
  OptVal : Integer;
begin
  Result := False;
  FLastError := 0;
  if FSocket = INVALID_SOCKET then Exit;

  OptVal := value;
  OptLen := 4;
  //function setsockopt(s: TSocket; level, optname: Integer; optval: MarshaledAString; optlen: Integer): Integer; stdcall;
  if Winapi.WinSock2.setsockopt(FSocket, IPPROTO_IP, IP_MULTICAST_TTL, MarshaledAString(@OptVal), OptLen) = SOCKET_ERROR then begin
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
  if FSocket = INVALID_SOCKET then Exit;

  addr := MakeSockAddr(IFaddr, 0);
  OptVal := TSockAddrIn(addr).sin_addr.S_addr;
  OptLen := SizeOf(OptVal);

  //function setsockopt(s: TSocket; level, optname: Integer; optval: MarshaledAString; optlen: Integer): Integer; stdcall;
  if Winapi.WinSock2.setsockopt(FSocket, IPPROTO_IP, IP_MULTICAST_IF, MarshaledAString(@OptVal), OptLen) = SOCKET_ERROR then begin
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
  if FSocket = INVALID_SOCKET then Exit;

  addr := MakeSockAddr(MCaddr, 0);
  OptVal.imr_multiaddr := TSockAddrIn(addr).sin_addr;

  addr := MakeSockAddr(IFaddr, 0);
  OptVal.imr_interface := TSockAddrIn(addr).sin_addr;

  OptLen := SizeOf(OptVal);

  //function setsockopt(s: TSocket; level, optname: Integer; optval: MarshaledAString; optlen: Integer): Integer; stdcall;
  if Winapi.WinSock2.setsockopt(FSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, MarshaledAString(@OptVal), OptLen) = SOCKET_ERROR then begin
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
  if FSocket = INVALID_SOCKET then Exit;

  addr := MakeSockAddr(MCaddr, 0);
  OptVal.imr_multiaddr := TSockAddrIn(addr).sin_addr;

  addr := MakeSockAddr(IFaddr, 0);
  OptVal.imr_interface := TSockAddrIn(addr).sin_addr;

  OptLen := SizeOf(OptVal);

  //function setsockopt(s: TSocket; level, optname: Integer; optval: MarshaledAString; optlen: Integer): Integer; stdcall;
  if Winapi.WinSock2.setsockopt(FSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, MarshaledAString(@OptVal), OptLen) = SOCKET_ERROR then begin
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
  if FSocket = INVALID_SOCKET then Exit;

  Result := True;
  Flag := value;
  //function setsockopt(s: TSocket; level, optname: Integer; optval: MarshaledAString; optlen: Integer): Integer; stdcall;
  if Winapi.WinSock2.setsockopt(FSocket, IPPROTO_TCP, TCP_NODELAY, MarshaledAString(@Flag), SizeOf(Flag)) = SOCKET_ERROR then begin
    Result := False;
    FLastError := WSAGetLastError;
  end;
end;

// ----------------------------------------------------------------------------

{ TTcpClientSocket }

function TTcpClientSocket.Close : Boolean;
begin
  FConnected := False;
  Result := inherited Close;
end;

function TTcpClientSocket.Connect : Boolean;
var
  addr: TSockAddr;
begin
  FLastError := 0;
  if IsOpen and not FConnected then begin
    addr := MakeSockAddr(FRemoteHost, FRemotePort);
    //function connect(s: TSocket; var name: TSockAddr; namelen: Integer): Integer; stdcall;
    FConnected := Winapi.WinSock2.connect(FSocket, addr, sizeof(addr)) = 0;
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
    Winapi.WinSock2.shutdown(FSocket, SD_BOTH);
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
    FListening := Winapi.WinSock2.listen(FSocket, SOMAXCONN) = 0;
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

  //function accept(s: TSocket; addr: PSockAddr; addrlen: PINT): TSocket; stdcall;
  Sock := Winapi.WinSock2.accept(FSocket, @addr, @len);

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
  //function WSAStartup(wVersionRequired: WORD; var lpWSAData: TWSAData): Integer; stdcall;
  if Winapi.Winsock2.WSAStartup(Winapi.Winsock2.WINSOCK_VERSION, WSAData) <> 0 then begin
    raise Exception.Create('WSAStartup: Error: ' + IntToStr(WSAGetLastError));
  end;

finalization
  //function WSACleanup: Integer; stdcall;
  if Winapi.Winsock2.WSACleanup <> 0 then begin
    raise Exception.Create('WSACleanup: Error: ' + IntToStr(WSAGetLastError));
  end;

end.

