unit uOps.NetworkSupport;

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

uses Classes;

  function doSubnetTranslation(addr : AnsiString) : AnsiString;

  function isValidNodeAddress(addr : AnsiString) : Boolean;
  function isMyNodeAddress(addr : AnsiString) : Boolean;

  procedure GetHostAddresses(name : AnsiString; List : TStrings);
  function GetHostAddress(name : AnsiString) : AnsiString;
  function GetHostAddressEx(localInterface : AnsiString) : AnsiString;

implementation

uses SysUtils,
     AnsiStrings,
     Winapi.Windows,
     Winapi.IpHlpApi,
     Winapi.IpRtrMib,
     Winapi.Winsock2;

/// ------------------------------------------
/// Helper to get all IP interfaces

procedure VVGetIpAddrTable(var p: PMibIpAddrTable; var Size: Cardinal; const bOrder: BOOL);
var
  Res: DWORD;
begin
  p := nil;
  Size := 0;
  if @GetIpAddrTable = nil then Exit;   //Not implemented in this windows version
  Res := GetIpAddrTable(p,Size,bOrder);
  if Res=ERROR_INSUFFICIENT_BUFFER then begin
    Getmem(p,Size);
    // Caller must free this buffer when it is no longer used
    FillChar(p^,Size,#0);
    Res := GetIpAddrTable(p,Size,bOrder);
  end;
  if Res <> NO_ERROR then begin
    if Assigned(p) then FreeMem(p);
    p := nil;
    Size := 0;
  end;
end;

function IpAddressToString(Addr: DWORD): AnsiString;
var
  InAddr: TInAddr;
begin
{$R-}
  InAddr.S_addr := Addr;
  Result := AnsiString(inet_ntoa(InAddr));
{$R+}
end;

// If argument contains a "/" we assume it is on the form:  subnet-address/subnet-mask
// e.g "192.168.10.0/255.255.255.0" or "192.168.10.0/24"
// In that case we loop over all interfaces and take the first one that matches
// i.e. the one whos interface address is on the subnet
function doSubnetTranslation(addr : AnsiString) : AnsiString;
var
  Idx : Integer;
  subnet, mask : AnsiString;
  subnetIp, subnetMask : DWORD;
  i: integer;
  List : TStringList;
  dwaddr : DWORD;
begin
  Result := addr;

  // If no '/' we just return the given address
  Idx := Pos('/', string(addr));
  if Idx = 0 then Exit;

  subnet := Copy(addr, 1, Idx-1);
  mask   := Copy(addr, Idx+1, MaxInt);

  subnetIp := DWORD(inet_addr(PAnsiChar(subnet)));
  if Length(mask) <= 2 then begin
    // Expand to the number of bits given
    subnetMask := StrToInt(string(mask));
    subnetMask := (((1 shl subnetMask)-1) shl (32 - subnetMask)) and $FFFFFFFF;
    subnetMask := DWORD(ntohl(u_long(subnetMask)));
  end else begin
    subnetMask := DWORD(inet_addr(PAnsiChar(mask)));
  end;

  List := TStringList.Create;
  try
    GetHostAddresses('', List);
    for i := 0 to List.Count - 1 do begin
      dwaddr := DWORD(List.Objects[i]);
      if (dwAddr and subnetMask) = (subnetIp and subnetMask) then begin
        Result := AnsiString(List.Strings[i]);
        Break;
      end;
    end;
  finally
    List.Free;
  end;
end;

function isValidNodeAddress(addr : AnsiString) : Boolean;
var
  Ip : DWORD;
begin
  Result := False;

  //std::cout << "isValidNodeAddress(): " << addr << std::endl;
  if addr = '' then Exit;

  Ip := DWORD(ntohl(inet_addr(PAnsiChar(addr))));
  //std::cout << "isValidNodeAddress(): " << std::hex << Ip << std::dec << std::endl;
  if Ip = 0 then Exit;
  if Ip >= $E0000000 then Exit;  // Skip multicast and above
  Result := True;
end;

function isMyNodeAddress(addr : AnsiString) : Boolean;
var
  Ip : DWORD;
  Size: ULONG;
  p: PMibIpAddrTable;
  i: integer;
begin
  Result := False;

  //std::cout << "isMyNodeAddress(): " << addr << std::endl;
  if addr = '' then Exit;

  Ip := DWORD(ntohl(inet_addr(PAnsiChar(addr))));
  //std::cout << "isMyNodeAddress(): " << std::hex << Ip << std::dec << std::endl;

  if Ip = $7F000001 then begin
    Result := True;  // localhost
    Exit;
  end;

  VVGetIpAddrTable(p, Size, False);
  if Assigned(p) then begin
    try
      with p^ do begin
{$R-}
        for i := 0 to dwNumEntries - 1 do begin
          with table[i] do begin
            if DWORD(htonl(dwAddr)) = Ip then begin
              Result := True;
              Break;
            end;
          end;
        end;
{$R+}
      end;
    finally
      FreeMem(p);
    end;
  end;
end;

//  Paddrinfo = ^addrinfo;
//  addrinfo = record
//    ai_flags: Integer;     // AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST
//    ai_family: Integer;    // PF_xxx
//    ai_socktype: Integer;  // SOCK_xxx
//    ai_protocol: Integer;  // 0 or IPPROTO_xxx for IPv4 and IPv6
//    ai_addrlen: SIZE_T;    // Length of ai_addr
//    ai_canonname: MarshaledAString; // Canonical name for nodename
//    ai_addr: PSockAddr;    // Binary address
//    ai_next: Paddrinfo;    // Next structure in linked list
//  end;

//  PaddrinfoW = ^addrinfoW;
//  addrinfoW = record
//    ai_flags: Integer;     // AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST
//    ai_family: Integer;    // PF_xxx
//    ai_socktype: Integer;  // SOCK_xxx
//    ai_protocol: Integer;  // 0 or IPPROTO_xxx for IPv4 and IPv6
//    ai_addrlen: SIZE_T;    // Length of ai_addr
//    ai_canonname: LPWSTR;  // Canonical name for nodename
//    ai_addr: PSockAddr;    // Binary address
//    ai_next: Paddrinfo;    // Next structure in linked list
//  end;
//  {$EXTERNALSYM addrinfoW}
//  TAddrInfoW = addrinfoW;

//function GetAddrInfoW(hostname, servname: LPCWSTR; const [Ref] hints: addrinfoW; out res: PaddrinfoW): Integer; stdcall;
//procedure FreeAddrInfoW(var ai: addrinfoW); stdcall;

//  sockaddr = record
//    sa_family: u_short;                 // address family
//    sa_data: array [0..13] of a_char; // up to 14 bytes of direct address
//  end;
//  TSockAddr = sockaddr;
//  PSockAddr = ^sockaddr;
//  LPSOCKADDR = ^sockaddr;

//  sockaddr_in = record
//    sin_family: Smallint;
//    sin_port: u_short;
//    sin_addr: in_addr;
//    sin_zero: array [0..7] of a_char;
//  end;
//  TSockAddrIn = sockaddr_in;
//  PSockAddrIn = ^sockaddr_in;

//  in_addr = record
//    case Integer of
//      0: (S_un_b: SunB);
//      1: (S_un_c: SunC);
//      2: (S_un_w: SunW);
//      3: (S_addr: u_long);
//    // #define s_addr  S_un.S_addr // can be used for most tcp & ip code
//    // #define s_host  S_un.S_un_b.s_b2 // host on imp
//    // #define s_net   S_un.S_un_b.s_b1  // netword
//    // #define s_imp   S_un.S_un_w.s_w2 // imp
//    // #define s_impno S_un.S_un_b.s_b4 // imp #
//    // #define s_lh    S_un.S_un_b.s_b3 // logical host
//  end;
//  TInAddr = in_addr;
//  PInAddr = ^in_addr;

procedure GetHostAddresses(name : AnsiString; List : TStrings);
var
  HostName : array[0..100] of AnsiChar;
  Err : Integer;
  hints : addrinfo;
  Res, Ptr : Paddrinfo;
  Addr : PAnsiChar;
  IpNum : NativeInt;
begin
  AnsiStrings.StrPLCopy(HostName, name, 100);
  FillChar(hints, SizeOf(hints), 0);
  Res := nil;

  //function getaddrinfo(hostname, servname: MarshaledAString; const [Ref] hints: addrinfo; out res: Paddrinfo): Integer; stdcall;
  Err := Winapi.Winsock2.getaddrinfo(HostName, nil, hints, Res);
  try
    if (Err = 0) and Assigned(Res) then begin
      Ptr := Res;
      while Assigned(Ptr) do begin
        if Ptr.ai_family = AF_INET then begin
          Addr := inet_ntoa(TSockAddrIn(Ptr.ai_addr^).sin_addr);
          // Save IP both as string and as number
          IpNum := TSockAddrIn(Ptr.ai_addr^).sin_addr.S_addr;
          List.AddObject(string(addr), TObject(IpNum));
        end;
        Ptr := Ptr.ai_next;
      end;
    end;
  finally
    //procedure freeaddrinfo(var ai: addrinfo); stdcall;
    Winapi.Winsock2.freeaddrinfo(Res^);
  end;
end;

function GetHostAddress(name : AnsiString) : AnsiString;
var
  HostName : array[0..100] of AnsiChar;
  Err : Integer;
  hints : addrinfo;
  Res, Ptr : Paddrinfo;
  Addr : PAnsiChar;
begin
  Result := name;

  AnsiStrings.StrPLCopy(HostName, name, 100);
  FillChar(hints, SizeOf(hints), 0);
  Res := nil;

  //function getaddrinfo(hostname, servname: MarshaledAString; const [Ref] hints: addrinfo; out res: Paddrinfo): Integer; stdcall;
  Err := Winapi.Winsock2.getaddrinfo(HostName, nil, hints, Res);
  try
    if (Err = 0) and Assigned(Res) then begin
      Ptr := Res;
      while Assigned(Ptr) do begin
        if Ptr.ai_family = AF_INET then begin
          Addr := inet_ntoa(TSockAddrIn(Ptr.ai_addr^).sin_addr);
          Result := addr;
          Break;
        end;
        Ptr := Ptr.ai_next;
      end;
    end;
  finally
    //procedure freeaddrinfo(var ai: addrinfo); stdcall;
    Winapi.Winsock2.freeaddrinfo(Res^);
  end;
end;

function GetHostAddressEx(localInterface : AnsiString) : AnsiString;
var
  Idx : Integer;
  mask : AnsiString;
begin
  Result := localInterface;
  mask := '';

  // If '/' we split
  Idx := Pos('/', string(localInterface));
  if Idx > 0 then begin
    Result := Copy(localInterface, 1, Idx-1);
    mask   := Copy(localInterface, Idx+1, MaxInt);
  end;

  if Length(Result) > 0 then begin
    Result := getHostAddress(Result);
  end;

  if Idx > 0 then begin
    Result := Result + '/' + mask;
  end;
end;

end.

