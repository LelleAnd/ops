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

with System;
with Ada.Streams;
with Ada.Exceptions;
with GNAT.Sockets;

package Ops_Pa.Socket_Pa is

  SOCKET_ERROR_C : constant Integer := -1;

  function GetHostName return String;
  function doSubnetTranslation(addr : String) return String;
  function isValidNodeAddress(addr : String) return Boolean;
  function isMyNodeAddress(addr : String) return Boolean;

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type Socket_Class    is new Ops_Class with private;
  type Socket_Class_At is access all Socket_Class'Class;

  function Create( SocketType : GNAT.Sockets.Mode_Type ) return Socket_Class_At;

  function Open( Self : in out Socket_Class ) return Boolean;
  function IsOpen( Self : in out Socket_Class ) return Boolean;

  function Shutdown( Self : in out Socket_Class ) return Boolean;
  function Close( Self : in out Socket_Class ) return Boolean;

  function GetLatestError( Self : Socket_Class ) return Integer;

  function SetNonBlocking( Self : in out Socket_Class; Value : Boolean ) return Boolean;
  function SetReuseAddress( Self : in out Socket_Class; Value : Boolean ) return Boolean;

  function Bind( Self : in out Socket_Class; Ip : String; Port : Integer ) return Boolean;
  function GetBoundIP( Self : in out Socket_Class ) return String;
  function GetBoundPort( Self : in out Socket_Class) return Integer;

  function GetPeerIP( Self : Socket_Class ) return String;
  function GetPeerPort( Self : Socket_Class ) return Integer;

  function SetReceiveBufferSize( Self : in out Socket_Class; Value : Integer ) return Boolean;
  function GetReceiveBufferSize( Self : in out Socket_Class ) return Integer;

  function SetSendBufferSize( Self : in out Socket_Class; Value : Integer ) return Boolean;
  function GetSendBufferSize( Self : in out Socket_Class ) return Integer;

  -- Saves from address internally and it is available via API GetSourceIP/Port
  function ReceiveFrom( Self : in out Socket_Class; Buf : out Ada.Streams.Stream_Element_Array ) return Integer;
  function GetSourceIP( Self : in out Socket_Class ) return String;
  function GetSourcePort( Self : in out Socket_Class ) return Integer;

  function ReceiveBuf( Self : in out Socket_Class; Buf : out Ada.Streams.Stream_Element_Array ) return Integer;
  function ReceiveBuf( Self : in out Socket_Class;
                       Buf : out Ada.Streams.Stream_Element_Array;
                       Dur : GNAT.Sockets.Timeval_Duration;
                       Timedout : out Boolean ) return Integer;
  -- Used to Abort a ReceiveBuf() with timeout
  procedure AbortReceiveBuf( Self : in out Socket_Class );

  function SendTo( Self : in out Socket_Class; Buf : Ada.Streams.Stream_Element_Array; Ip : String; Port : Integer ) return Integer;

  function SendBuf( Self : in out Socket_Class; Buf : Ada.Streams.Stream_Element_Array ) return Integer;

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  -- Helper class to be able to wait for several sockets to be ready
  -- for accept or read of data
  type Selector_Class is new Ops_Class with private;
  type Selector_Class_At is access all Selector_Class'Class;

  function Create return Selector_Class_At;

  -- Empty list of sockets to wait for
  procedure Clear( Self : in out Selector_Class );
  procedure Add( Self : in out Selector_Class; Socket : Socket_Class_At );
  procedure Remove( Self : in out Selector_Class; Socket : Socket_Class_At );

  -- Wait for any of the sockets to be ready
  procedure Wait( Self : in out Selector_Class;
                  Dur : GNAT.Sockets.Timeval_Duration := GNAT.Sockets.Forever;
                  Timedout : out Boolean );
  procedure AbortWait( Self : in out Selector_Class );

  -- Check if the specific socket was ready after the last wait call
  function IsSet( Self : Selector_Class; Socket : Socket_Class_At ) return Boolean;

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type UDPSocket_Class    is new Socket_Class with private;
  type UDPSocket_Class_At is access all UDPSocket_Class'Class;

  function Create return UDPSocket_Class_At;

  function SetMulticastTtl( Self : in out UDPSocket_Class; Ttl : Integer ) return Boolean;
  function SetMulticastInterface( Self : in out UDPSocket_Class; Ifc : String ) return Boolean;

  function AddMulticastMembership( Self : in out UDPSocket_Class; McAddr : String; McIfc : String ) return Boolean;
  function DropMulticastMembership( Self : in out UDPSocket_Class; McAddr : String; McIfc : String ) return Boolean;

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type TCPSocket_Class    is new Socket_Class with private;
  type TCPSocket_Class_At is access all TCPSocket_Class'Class;

  function Create return TCPSocket_Class_At;

  function SetTcpNoDelay( Self : in out TCPSocket_Class; Value : Boolean ) return Boolean;

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type TCPClientSocket_Class    is new TCPSocket_Class with private;
  type TCPClientSocket_Class_At is access all TCPClientSocket_Class'Class;

  function Create return TCPClientSocket_Class_At;

  overriding function Close( Self : in out TCPClientSocket_Class ) return Boolean;

  function Connect( Self : in out TCPClientSocket_Class; Ip : String; Port : Integer ) return Boolean;
  function Disconnect( Self : in out TCPClientSocket_Class ) return Boolean;

  function IsConnected( Self : TCPClientSocket_Class ) return Boolean;


-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type TCPServerSocket_Class    is new TCPSocket_Class with private;
  type TCPServerSocket_Class_At is access all TCPServerSocket_Class'Class;

  function Create return TCPServerSocket_Class_At;

  overriding function Close( Self : in out TCPServerSocket_Class ) return Boolean;

  function Listen(Self : in out TCPServerSocket_Class; MaxBackLog : Integer) return Boolean;
  function AcceptClient( Self : in out TCPServerSocket_Class; Client : TCPClientSocket_Class_At ) return Boolean;

  function IsListening( Self : TCPServerSocket_Class ) return Boolean;

private
-- ==========================================================================
--
-- ==========================================================================
  type Socket_Class is new Ops_Class with
    record
      SelfAt : Socket_Class_At := null;
      SocketType : GNAT.Sockets.Mode_Type := GNAT.Sockets.Socket_Datagram;
      SocketID : GNAT.Sockets.Socket_Type := GNAT.Sockets.No_Socket;
      StartupOK : Boolean := False;

      -- Latest error
      LatestErrorCode : Integer := 0;

      -- Last source for read data
      FromAddress : GNAT.Sockets.Sock_Addr_Type := GNAT.Sockets.No_Sock_Addr;

      -- For read timeout handling ReceiveBuf( with duration )
      Timeout_Selector : GNAT.Sockets.Selector_Type;
      Timeout_Used : Boolean := False;
    end record;

  procedure ExtractErrorCode( Self : in out Socket_Class; e : Ada.Exceptions.Exception_Occurrence );

  procedure InitInstance( Self : in out Socket_Class;
                          SelfAt : Socket_Class_At;
                          SocketType : GNAT.Sockets.Mode_Type );

  overriding procedure Finalize( Self : in out Socket_Class );


-- ==========================================================================
--
-- ==========================================================================
  type Selector_Class is new Ops_Class with
    record
      Selector : GNAT.Sockets.Selector_Type;

      -- Holder sets
      sst_R : GNAT.Sockets.Socket_Set_Type;
      sst_W : GNAT.Sockets.Socket_Set_Type;

      -- Set from latest wait()
      sst_latest : GNAT.Sockets.Socket_Set_Type;
    end record;

  procedure InitInstance( Self : in out Selector_Class );

  overriding procedure Finalize( Self : in out Selector_Class );


-- ==========================================================================
--
-- ==========================================================================
  type UDPSocket_Class is new Socket_Class with
     record
       null;
     end record;

  procedure InitInstance( Self : in out UDPSocket_Class;
                          SelfAt : UDPSocket_Class_At );

  overriding procedure Finalize( Self : in out UDPSocket_Class );



-- ==========================================================================
--
-- ==========================================================================
  type TCPSocket_Class is new Socket_Class with
     record
       null;
     end record;

  procedure InitInstance( Self : in out TCPSocket_Class;
                          SelfAt : TCPSocket_Class_At );

  overriding procedure Finalize( Self : in out TCPSocket_Class );


-- ==========================================================================
--
-- ==========================================================================
  type TCPClientSocket_Class is new TCPSocket_Class with
     record
       Connected : Boolean := False;
     end record;

  procedure Initialize( Self : in out TCPClientSocket_Class;
                        SocketId : GNAT.Sockets.Socket_Type;
                        Connected : Boolean );

  procedure InitInstance( Self : in out TCPClientSocket_Class;
                          SelfAt : TCPClientSocket_Class_At );

  overriding procedure Finalize( Self : in out TCPClientSocket_Class );


-- ==========================================================================
--
-- ==========================================================================
  type TCPServerSocket_Class is new TCPSocket_Class with
     record
       Listening : Boolean := False;
     end record;

  procedure InitInstance( Self : in out TCPServerSocket_Class;
                          SelfAt : TCPServerSocket_Class_At );

  overriding procedure Finalize( Self : in out TCPServerSocket_Class );

end Ops_Pa.Socket_Pa;

