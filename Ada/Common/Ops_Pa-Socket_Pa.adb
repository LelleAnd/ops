--
-- Copyright (C) 2017-2021 Lennart Andersson.
--
-- This file is part of OPS (Open Publish Subscribe).
--
-- OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Lesser General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public License
-- along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.

with Ada.Characters.Latin_1,
     Ada.Text_IO,
     Ada.Integer_Text_IO,
     Ada.Strings.Fixed;

with Interfaces; use Interfaces;
with Interfaces.C;

with GNAT.Sockets;

package body Ops_Pa.Socket_Pa is

  use type GNAT.Sockets.Socket_Type;

  function GetHostName return String is
  begin
    return GNAT.Sockets.Host_Name;
  end;

  -- ===========================================================================

  function Create( SocketType : GNAT.Sockets.Mode_Type )  return Socket_Class_At is
     Self : Socket_Class_At := null;
  begin
    Self := new Socket_Class;
    InitInstance( Self.all, Self, SocketType );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out Socket_Class;
                          SelfAt : Socket_Class_At;
                          SocketType : GNAT.Sockets.Mode_Type ) is
  begin
    Self.SelfAt := SelfAt;
    Self.SocketType := SocketType;
  end;

  overriding procedure Finalize( Self : in out Socket_Class ) is
    dummy : Boolean;
  begin
    if Self.IsOpen then
      dummy := Self.Shutdown;
      dummy := Self.Close;
    end if;
    if Self.Timeout_Used then
      GNAT.Sockets.Close_Selector( Self.Timeout_Selector );
    end if;
  end;

  procedure ExtractErrorCode( Self : in out Socket_Class; e : Ada.Exceptions.Exception_Occurrence ) is
    Msg : constant String := Ada.Exceptions.Exception_Message( e );
    First : Natural;
    Last  : Natural;
  begin
    --  When Socket_Error or Host_Error are raised, the exception message
    --  contains the error code between brackets and a string describing the
    --  error code. Resolve_Error extracts the error code from an exception
    --  message and translate it into an enumeration value.

    First := Msg'First;
    while First <= Msg'Last and then Msg(First) not in '0' .. '9' loop
      First := First + 1;
    end loop;
    if First > Msg'Last then
      return; --Cannot_Resolve_Error;
    end if;

    Last := First;
    while Last < Msg'Last and then Msg(Last+1) in '0' .. '9' loop
      Last := Last + 1;
    end loop;

    Self.LatestErrorCode := Integer'Value(Msg(First..Last));
  end;

  function Open( Self : in out Socket_Class ) return Boolean is
  begin
    GNAT.Sockets.Create_Socket( Self.SocketId, GNAT.Sockets.Family_Inet, Self.SocketType );
    return Self.IsOpen;
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return False;
  end;

  function IsOpen( Self : in out Socket_Class ) return Boolean is
  begin
    return Self.SocketId /= GNAT.Sockets.No_Socket;
  end;

  function Shutdown( Self : in out Socket_Class ) return Boolean is
  begin
    if Self.IsOpen then
      begin
        GNAT.Sockets.Shutdown_Socket( Self.SocketID );
      exception
        when e: others =>
          Self.ExtractErrorCode( e );
          return False;
      end;
    end if;
    return True;
  end;

  function Close( Self : in out Socket_Class ) return Boolean is
  begin
    if Self.IsOpen then
      begin
        GNAT.Sockets.Close_Socket( Self.SocketID );
      exception
        when e: others =>
          Self.ExtractErrorCode( e );
          return False;
      end;
    end if;
    Self.SocketId := GNAT.Sockets.No_Socket;
    return True;
  end;

  function GetLatestError( Self : Socket_Class ) return Integer is
  begin
    return Self.LatestErrorCode;
  end;

  function SetNonBlocking( Self : in out Socket_Class; Value : Boolean ) return Boolean is
    Req : GNAT.Sockets.Request_Type := (Name => GNAT.Sockets.Non_Blocking_IO, Enabled => Value);
  begin
    GNAT.Sockets.Control_Socket( Self.SocketID, Request => Req );
    return True;
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return False;
  end;

  function SetReuseAddress( Self : in out Socket_Class; Value : Boolean ) return Boolean is
    Option : GNAT.Sockets.Option_Type := (Name => GNAT.Sockets.Reuse_Address, Enabled => Value);
  begin
    GNAT.Sockets.Set_Socket_Option( Self.SocketID, GNAT.Sockets.Socket_Level, Option );
    return True;
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return False;
  end;

  function Bind( Self : in out Socket_Class; Ip : String; Port : Integer ) return Boolean is
    addr : GNAT.Sockets.Sock_Addr_Type :=
      (Family => GNAT.Sockets.Family_Inet,
       Addr => GNAT.Sockets.Inet_Addr( Ip ),
       Port => GNAT.Sockets.Port_Type( Port ));
  begin
    GNAT.Sockets.Bind_Socket( Self.SocketID, addr );
    return True;
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return False;
  end;

  function GetBoundIP( Self : in out Socket_Class ) return String is
    addr : GNAT.Sockets.Sock_Addr_Type;
  begin
    addr := GNAT.Sockets.Get_Socket_Name( Self.SocketID );
    return GNAT.Sockets.Image(addr.Addr);
  end;

  function GetBoundPort( Self : in out Socket_Class ) return Integer is
    addr : GNAT.Sockets.Sock_Addr_Type;
  begin
    addr := GNAT.Sockets.Get_Socket_Name( Self.SocketID );
    return Integer(addr.Port);
  end;

  function GetPeerIP( Self : Socket_Class ) return String is
    addr : GNAT.Sockets.Sock_Addr_Type;
  begin
    addr := GNAT.Sockets.Get_Peer_Name( Self.SocketID );
    return GNAT.Sockets.Image(addr.Addr);
  end;

  function GetPeerPort( Self : Socket_Class ) return Integer is
    addr : GNAT.Sockets.Sock_Addr_Type;
  begin
    addr := GNAT.Sockets.Get_Peer_Name( Self.SocketID );
    return Integer(addr.Port);
  end;

  function SetReceiveBufferSize( Self : in out Socket_Class; Value : Integer ) return Boolean is
    Option : GNAT.Sockets.Option_Type := (Name => GNAT.Sockets.Receive_Buffer, Size => Natural(Value));
  begin
    GNAT.Sockets.Set_Socket_Option( Self.SocketID, GNAT.Sockets.Socket_Level, Option );
    return True;
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return False;
  end;

  function GetReceiveBufferSize( Self : in out Socket_Class ) return Integer is
    Option : GNAT.Sockets.Option_Type;
  begin
    Option := GNAT.Sockets.Get_Socket_Option( Self.SocketID, GNAT.Sockets.Socket_Level, GNAT.Sockets.Receive_Buffer );
    return Integer(Option.Size);
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return -1;
  end;

  function SetSendBufferSize( Self : in out Socket_Class; Value : Integer ) return Boolean is
    Option : GNAT.Sockets.Option_Type := (Name => GNAT.Sockets.Send_Buffer, Size => Natural(Value));
  begin
    GNAT.Sockets.Set_Socket_Option( Self.SocketID, GNAT.Sockets.Socket_Level, Option );
    return True;
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return False;
  end;

  function GetSendBufferSize( Self : in out Socket_Class ) return Integer is
    Option : GNAT.Sockets.Option_Type;
  begin
    Option := GNAT.Sockets.Get_Socket_Option( Self.SocketID, GNAT.Sockets.Socket_Level, GNAT.Sockets.Send_Buffer );
    return Integer(Option.Size);
  exception
    when others =>
      return -1;
  end;

  function SendBuf( Self : in out Socket_Class; Buf : Ada.Streams.Stream_Element_Array ) return Integer is
    Last : Ada.Streams.Stream_Element_Offset;
  begin
    GNAT.Sockets.Send_Socket( Self.SocketID,
                              Buf,
                              Last );
    -- Last is the index to the last element sent, but we want to return the number of elements so add 1
    return Integer(Last)-Integer(Buf'First)+1;
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return -1;
  end;

  function ReceiveBuf( Self : in out Socket_Class; Buf : out Ada.Streams.Stream_Element_Array ) return Integer is
    Last : Ada.Streams.Stream_Element_Offset;
  begin
    GNAT.Sockets.Receive_Socket( Self.SocketID,
                                 Buf,
                                 Last );
    -- Last is the index to the last element read, but we want to return the number of elements so add 1
    return Integer(Last)-Integer(Buf'First)+1;
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return -1;
  end;

  function ReceiveBuf( Self : in out Socket_Class;
                       Buf : out Ada.Streams.Stream_Element_Array;
                       Dur : GNAT.Sockets.Timeval_Duration;
                       Timedout : out Boolean ) return Integer is
    Last : Ada.Streams.Stream_Element_Offset;
    sst_R : GNAT.Sockets.Socket_Set_Type;
    sst_W : GNAT.Sockets.Socket_Set_Type;
    Status : GNAT.Sockets.Selector_Status;
    use type GNAT.Sockets.Selector_Status;
  begin
    if not Self.Timeout_Used then
      GNAT.Sockets.Create_Selector( Self.Timeout_Selector );
      Self.Timeout_Used := True;
    end if;
    GNAT.Sockets.Set( sst_R, Self.SocketID );
    GNAT.Sockets.Check_Selector( Self.Timeout_Selector,
                                 sst_R, sst_W,
                                 Status,
                                 Dur );
    if Status = GNAT.Sockets.Expired then
      Timedout := True;
      return 0;
    end if;
    Timedout := False;

    GNAT.Sockets.Receive_Socket( Self.SocketID,
                                 Buf,
                                 Last );
    -- Last is the index to the last element read, but we want to return the number of elements so add 1
    return Integer(Last)-Integer(Buf'First)+1;
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return -1;
  end;

  -- Used to Abort a ReceiveBuf() with timeout
  procedure AbortReceiveBuf( Self : in out Socket_Class ) is
  begin
    if Self.Timeout_Used then
      GNAT.Sockets.Abort_Selector( Self.Timeout_Selector );
    end if;
  end;

  -- Saves from address internally and it is available via API
  function ReceiveFrom( Self : in out Socket_Class; Buf : out Ada.Streams.Stream_Element_Array ) return Integer is
    Last : Ada.Streams.Stream_Element_Offset;
  begin
    GNAT.Sockets.Receive_Socket( Self.SocketID,
                                 Buf,
                                 Last,
                                 Self.FromAddress );
    -- Last is the index to the last element read, but we want to return the number of elements so add 1
    return Integer(Last)-Integer(Buf'First)+1;
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return -1;
  end;

  function GetSourceIP( Self : in out Socket_Class ) return String is
  begin
    return GNAT.Sockets.Image(Self.FromAddress.Addr);
  end;

  function GetSourcePort( Self : in out Socket_Class ) return Integer is
  begin
    return Integer(Self.FromAddress.Port);
  end;

  function SendTo( Self : in out Socket_Class; Buf : Ada.Streams.Stream_Element_Array; Ip : String; Port : Integer ) return Integer is
    addr : GNAT.Sockets.Sock_Addr_Type :=
      (Family => GNAT.Sockets.Family_Inet,
       Addr => GNAT.Sockets.Inet_Addr( Ip ),
       Port => GNAT.Sockets.Port_Type( Port ));
    Last : Ada.Streams.Stream_Element_Offset;
  begin
    GNAT.Sockets.Send_Socket( Self.SocketID,
                              Buf,
                              Last,
                              addr );
    -- Last is the index to the last element sent, but we want to return the number of elements so add 1
    return Integer(Last)-Integer(Buf'First)+1;
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return -1;
  end;

  --------------------------------------------------------------------------

  function Create return Selector_Class_At is
     Self : Selector_Class_At := null;
  begin
    Self := new Selector_Class;
    InitInstance( Self.all );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out Selector_Class ) is
  begin
    GNAT.Sockets.Create_Selector( Self.Selector );
  end;

  overriding procedure Finalize( Self : in out Selector_Class ) is
  begin
    GNAT.Sockets.Close_Selector( Self.Selector );
  end;

  procedure Clear( Self : in out Selector_Class ) is
  begin
    GNAT.Sockets.Empty( Self.sst_R );
    GNAT.Sockets.Empty( Self.sst_W );
    GNAT.Sockets.Empty( Self.sst_latest );
  end;

  procedure Add( Self : in out Selector_Class; Socket : Socket_Class_At ) is
  begin
    GNAT.Sockets.Set( Self.sst_R, Socket.SocketID );
  end;

  procedure Remove( Self : in out Selector_Class; Socket : Socket_Class_At ) is
  begin
    GNAT.Sockets.CLear( Self.sst_R, Socket.SocketID );
  end;

  procedure Wait( Self : in out Selector_Class;
                  Dur : GNAT.Sockets.Timeval_Duration := GNAT.Sockets.Forever;
                  Timedout : out Boolean ) is
    Status : GNAT.Sockets.Selector_Status;
    use type GNAT.Sockets.Selector_Status;
  begin
    GNAT.Sockets.Empty( Self.sst_latest );
    GNAT.Sockets.Copy( Self.sst_R, Self.sst_latest );
    GNAT.Sockets.Check_Selector( Self.Selector,
                                 Self.sst_latest, Self.sst_W,
                                 Status,
                                 Dur );
    Timedout := Status = GNAT.Sockets.Expired;
  end;

  procedure AbortWait( Self : in out Selector_Class ) is
  begin
    GNAT.Sockets.Abort_Selector( Self.Selector );
  end;

  function IsSet( Self : Selector_Class; Socket : Socket_Class_At ) return Boolean is
  begin
    return GNAT.Sockets.Is_Set( Self.sst_latest, Socket.SocketID );
  end;

  --------------------------------------------------------------------------

  function Create return UDPSocket_Class_At is
     Self : UDPSocket_Class_At := null;
  begin
    Self := new UDPSocket_Class;
    InitInstance( Self.all, Self );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out UDPSocket_Class;
                          SelfAt : UDPSocket_Class_At ) is
  begin
    InitInstance( Socket_Class(Self), Socket_Class_At(SelfAt), GNAT.Sockets.Socket_Datagram );
  end;

  overriding procedure Finalize( Self : in out UDPSocket_Class ) is
  begin
    Finalize( Socket_Class(Self) );
  end;

  function SetMulticastTtl( Self : in out UDPSocket_Class; Ttl : Integer ) return Boolean is
    Option : GNAT.Sockets.Option_Type := (Name => GNAT.Sockets.Multicast_TTL, Time_To_Live => Natural(Ttl));
  begin
    GNAT.Sockets.Set_Socket_Option( Self.SocketID, GNAT.Sockets.IP_Protocol_For_IP_Level, Option );
    return True;
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return False;
  end;

  function SetMulticastInterface( Self : in out UDPSocket_Class; Ifc : String ) return Boolean is
    Option : GNAT.Sockets.Option_Type :=
      (Name => GNAT.Sockets.Multicast_If,
       Outgoing_If => GNAT.Sockets.Inet_Addr( Ifc ));
  begin
    GNAT.Sockets.Set_Socket_Option( Self.SocketID, GNAT.Sockets.IP_Protocol_For_IP_Level, Option );
    return True;
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return False;
  end;

  function AddMulticastMembership( Self : in out UDPSocket_Class; McAddr : String; McIfc : String ) return Boolean is
    Option : GNAT.Sockets.Option_Type :=
      (Name              => GNAT.Sockets.Add_Membership,
       Multicast_Address => GNAT.Sockets.Inet_Addr( McAddr ),
       Local_Interface   => GNAT.Sockets.Inet_Addr( McIfc ));
  begin
    GNAT.Sockets.Set_Socket_Option( Self.SocketID, GNAT.Sockets.IP_Protocol_For_IP_Level, Option );
    return True;
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return False;
  end;

  function DropMulticastMembership( Self : in out UDPSocket_Class; McAddr : String; McIfc : String ) return Boolean is
    Option : GNAT.Sockets.Option_Type :=
      (Name              => GNAT.Sockets.Drop_Membership,
       Multicast_Address => GNAT.Sockets.Inet_Addr( McAddr ),
       Local_Interface   => GNAT.Sockets.Inet_Addr( McIfc ));
  begin
    GNAT.Sockets.Set_Socket_Option( Self.SocketID, GNAT.Sockets.IP_Protocol_For_IP_Level, Option );
    return True;
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return False;
  end;

  --------------------------------------------------------------------------

  function Create return TCPSocket_Class_At is
     Self : TCPSocket_Class_At := null;
  begin
    Self := new TCPSocket_Class;
    InitInstance( Self.all, Self );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out TCPSocket_Class;
                          SelfAt : TCPSocket_Class_At ) is
  begin
    InitInstance( Socket_Class(Self), Socket_Class_At(SelfAt), GNAT.Sockets.Socket_Stream );
  end;

  overriding procedure Finalize( Self : in out TCPSocket_Class ) is
  begin
    Finalize( Socket_Class(Self) );
  end;

  function SetTcpNoDelay( Self : in out TCPSocket_Class; Value : Boolean ) return Boolean is
    Option : GNAT.Sockets.Option_Type := (Name => GNAT.Sockets.No_Delay, Enabled => Value);
  begin
    GNAT.Sockets.Set_Socket_Option( Self.SocketID, GNAT.Sockets.IP_Protocol_For_TCP_Level, Option );
    return True;
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return False;
  end;

  --------------------------------------------------------------------------

  function Create return TCPClientSocket_Class_At is
     Self : TCPClientSocket_Class_At := null;
  begin
    Self := new TCPClientSocket_Class;
    InitInstance( Self.all, Self );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out TCPClientSocket_Class;
                          SelfAt : TCPClientSocket_Class_At ) is
  begin
    InitInstance( TCPSocket_Class(Self), TCPSocket_Class_At(SelfAt) );
  end;

  overriding procedure Finalize( Self : in out TCPClientSocket_Class ) is
  begin
    Finalize( TCPSocket_Class(Self) );
  end;

  procedure Initialize( Self : in out TCPClientSocket_Class;
                        SocketId : GNAT.Sockets.Socket_Type;
                        Connected : Boolean ) is
  begin
    Self.SocketID := SocketId;
    Self.Connected := Connected;
  end;

  overriding function Close( Self : in out TCPClientSocket_Class ) return Boolean is
    dummy : Boolean;
  begin
    dummy := Self.Disconnect;
    return Close( TCPSocket_Class(Self) );
  end;

  function IsConnected( Self : TCPClientSocket_Class ) return Boolean is
  begin
    return Self.Connected;
  end;

  function Connect( Self : in out TCPClientSocket_Class; Ip : String; Port : Integer ) return Boolean is
  begin
    if Self.IsOpen and not Self.Connected then
      declare
        addr : GNAT.Sockets.Sock_Addr_Type :=
          (Family => GNAT.Sockets.Family_Inet,
           Addr => GNAT.Sockets.Inet_Addr( Ip ),
           Port => GNAT.Sockets.Port_Type( Port ));
      begin
        GNAT.Sockets.Connect_Socket( Self.SocketID, addr );
        Self.Connected := True;
      exception
        when e: others =>
          Self.ExtractErrorCode( e );
      end;
    end if;
    return Self.Connected;
  end;

  function Disconnect( Self : in out TCPClientSocket_Class ) return Boolean is
    dummy : Boolean;
  begin
    if Self.Connected then
      dummy := Self.Shutdown;
    end if;
    Self.Connected := False;
    return True;
  end;


  --------------------------------------------------------------------------

  function Create return TCPServerSocket_Class_At is
     Self : TCPServerSocket_Class_At := null;
  begin
    Self := new TCPServerSocket_Class;
    InitInstance( Self.all, Self );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out TCPServerSocket_Class;
                          SelfAt : TCPServerSocket_Class_At ) is
  begin
    InitInstance( TCPSocket_Class(Self), TCPSocket_Class_At(SelfAt) );
  end;

  overriding procedure Finalize( Self : in out TCPServerSocket_Class ) is
  begin
    Finalize( TCPSocket_Class(Self) );
  end;

  overriding function Close( Self : in out TCPServerSocket_Class ) return Boolean is
  begin
    Self.Listening := False;
    return Close( TCPSocket_Class(Self) );
  end;

  function Listen( Self : in out TCPServerSocket_Class; MaxBackLog : Integer ) return Boolean is
  begin
    if Self.IsOpen and not Self.Listening then
      begin
        GNAT.Sockets.Listen_Socket( Self.SocketID, Natural(MaxBackLog) );
        Self.Listening := True;
      exception
        when e: others =>
          Self.ExtractErrorCode( e );
      end;
    end if;
    return Self.Listening;
  end;

  function IsListening( Self : TCPServerSocket_Class ) return Boolean is
  begin
    return Self.Listening;
  end;

  AbortSelector : aliased GNAT.Sockets.Selector_Type;

  function AcceptClient( Self : in out TCPServerSocket_Class; Client : TCPClientSocket_Class_At ) return Boolean is
    Address : GNAT.Sockets.Sock_Addr_Type;
    Socket : GNAT.Sockets.Socket_Type;
    Status : GNAT.Sockets.Selector_Status;
    use type GNAT.Sockets.Selector_Status;
  begin
    -- GNAT.Sockets.Accept_Socket( Server => Self.SocketID,
    --                             Socket => Socket,
    --                             Address => Address );
    GNAT.Sockets.Accept_Socket( Server => Self.SocketID,
                                Socket => Socket,
                                Address => Address,
                                Timeout => 1.0,
                                Selector => AbortSelector'Access,
                                Status => Status );
    if Status = GNAT.Sockets.Completed then
      client.Initialize( Socket, True );
      return True;
    else
      return False;
    end if;
  exception
    when e: others =>
      Self.ExtractErrorCode( e );
      return False;
  end;

  package x is new Ada.Text_IO.Modular_IO(Unsigned_32);

  function part(str : String; f : in Natural; Last : in out Natural) return Integer is
    First : Natural := f;
  begin
    while First <= str'Last and then str(First) not in '0' .. '9' loop
      First := First + 1;
    end loop;
    if First > str'Last then
      return -1; --Cannot_Resolve_Error;
    end if;
    Last := First;
    while Last < str'Last and then str(Last+1) in '0' .. '9' loop
      Last := Last + 1;
    end loop;
    return Integer'Value(str(First..Last));
  end;

  function To_Unsigned_32(str : String) return Unsigned_32 is
    Last  : Natural := str'First;
    tmp : Integer;
    res : Unsigned_32 := 0;
  begin
    tmp := part(str, Last, Last);
    if tmp >= 0 and tmp <= 255 then
      res := res + Shift_Left(Unsigned_32(tmp), 24);
      tmp := part(str, Last+1, Last);
      if tmp >= 0 and tmp <= 255 then
        res := res + Shift_Left(Unsigned_32(tmp), 16);
        tmp := part(str, Last+1, Last);
        if tmp >= 0 and tmp <= 255 then
          res := res + Shift_Left(Unsigned_32(tmp), 8);
          tmp := part(str, Last+1, Last);
          if tmp >= 0 and tmp <= 255 then
            res := res + Shift_Left(Unsigned_32(tmp), 0);
            return res;
          end if;
        end if;
      end if;
    end if;
    return 0;
  end;


-- If argument contains a "/" we assume it is on the form:  subnet-address/subnet-mask
-- e.g "192.168.10.0/255.255.255.0" or "192.168.10.0/24"
-- In that case we loop over all interfaces and take the first one that matches
-- i.e. the one whos interface address is on the subnet
  function doSubnetTranslation(addr : String) return String is
    Idx : Natural;
  begin
    --Ada.Text_IO.Put_Line("Translate(), addr: " & addr);
    -- If no '/' we just return the given address
    Idx := Ada.Strings.Fixed.Index( addr, "/" );
    if Idx = 0 then
      return addr;
    end if;

    declare
      subnet : String := addr(addr'First .. Idx-1);
      mask   : String := addr(Idx+1 .. addr'Last);
      numBits : Unsigned_32 := 0;
      subnetIp, subnetMask : Unsigned_32;
      dummy : Positive := 1;
    begin
      --Ada.Text_IO.Put_Line("Translate(), subnet: " & subnet);
      --Ada.Text_IO.Put_Line("Translate(), mask: " & mask);
      subnetIp := To_Unsigned_32(subnet);
      if mask'Length <= 2 then
        -- Expand to the number of bits given
        x.Get(mask, numBits, dummy);
        subnetMask := Shift_Left(1, Natural(numBits)) - 1;
        subnetMask := Shift_Left(subnetMask, Natural(32 - numBits));
        subnetMask := Unsigned_32(Interfaces.C.unsigned_long(subnetMask));
      else
        subnetMask := To_Unsigned_32(mask);
      end if;

      --Ada.Text_IO.Put("Translate(), subnetIp: "); x.Put(subnetIp, 12, 16); Ada.Text_IO.New_Line;
      --Ada.Text_IO.Put("Translate(), subnetMask: "); x.Put(subnetMask, 12, 16); Ada.Text_IO.New_Line;

      declare
        he : GNAT.Sockets.Host_Entry_Type := GNAT.Sockets.Get_Host_By_Name(GNAT.Sockets.Host_Name);
        addr : GNAT.Sockets.Inet_Addr_Type;
        addrNum : Unsigned_32;
      begin
        for i in 1 .. GNAT.Sockets.Addresses_Length( he ) loop
          addr := GNAT.Sockets.Addresses( he, i );
          addrNum := To_Unsigned_32(GNAT.Sockets.Image(addr));
          --Ada.Text_IO.Put_Line("  Address: " & GNAT.Sockets.Image(addr));
          --Ada.Text_IO.Put("Translate(), addrnum: "); x.Put(addrnum, 12, 16); Ada.Text_IO.New_Line;

          if (addrNum and subnetMask) = (subnetIp and subnetMask) then
            --Ada.Text_IO.Put_Line("Translate()  ----> " & GNAT.Sockets.Image(addr));
            return GNAT.Sockets.Image(addr);
          end if;
        end loop;
      end;
      return subnet;
    end;
  end;

  function isValidNodeAddress(addr : String) return Boolean is
    Ip : Unsigned_32;
  begin
    Ip := To_Unsigned_32(addr);

    -- Skip Any and multicast and above
    if Ip = 0 or Ip >= 16#E0000000# then
      return False;
    end if;
    return True;
  end;

  function isMyNodeAddress(addr : String) return Boolean is
    Ip : Unsigned_32;
  begin
    Ip := To_Unsigned_32(addr);

    if Ip = 16#7F000001# then
      return True;
    end if;

    declare
      he : GNAT.Sockets.Host_Entry_Type := GNAT.Sockets.Get_Host_By_Name(GNAT.Sockets.Host_Name);
      addr : GNAT.Sockets.Inet_Addr_Type;
      addrNum : Unsigned_32;
    begin
      for i in 1 .. GNAT.Sockets.Addresses_Length( he ) loop
        addr := GNAT.Sockets.Addresses( he, i );
        addrNum := To_Unsigned_32(GNAT.Sockets.Image(addr));
        --Ada.Text_IO.Put_Line("  Address: " & GNAT.Sockets.Image(addr));
        --Ada.Text_IO.Put("Translate(), addrnum: "); x.Put(addrnum, 12, 16); Ada.Text_IO.New_Line;

        if addrNum = Ip then
          return True;
        end if;
      end loop;
    end;
    return False;
  end;

begin
  GNAT.Sockets.Create_Selector(AbortSelector);
end Ops_Pa.Socket_Pa;

