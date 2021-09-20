--
-- Copyright (C) 2016-2021 Lennart Andersson.
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

with Ada.Strings.Fixed;

with Ops_Pa.Socket_Pa;

package body Ops_Pa.Transport_Pa.Receiver_Pa.TCPClient_Pa is

  use type Ops_Pa.Socket_Pa.TCPClientSocket_Class_At;
  use type TCPConnection_Pa.TCPConnection_Class_At;

  procedure Trace(Self : TCPClientReceiver_Class; Msg : String) is
    NameStr : String := "TcpClient (" & Integer'Image(Self.Port) & ")";
  begin
    Trace(NameStr, Msg);
  end;

  function Create( serverIP : string;
                   serverPort : Integer;
                   HeartbeatPeriod : Int32;
                   HeartbeatTimeout : Int32;
                   inSocketBufferSize : Int64 := 16000000) return TCPClientReceiver_Class_At is
     Self : TCPClientReceiver_Class_At := null;
  begin
    Self := new TCPClientReceiver_Class;
    InitInstance( Self.all, Self, serverIP, serverPort, HeartbeatPeriod, HeartbeatTimeout, inSocketBufferSize );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out TCPClientReceiver_Class;
                          SelfAt : TCPClientReceiver_Class_At;
                          serverIP : String;
                          serverPort : Integer;
                          HeartbeatPeriod : Int32;
                          HeartbeatTimeout : Int32;
                          inSocketBufferSize : Int64 ) is
  begin
    Self.SelfAt := SelfAt;
    InitInstance( Receiver_Class(Self), Receiver_Class_At(SelfAt) );

    Self.IpAddress := Copy(serverIP);
    Self.Port := serverPort;
    Self.InSocketBufferSize := inSocketBufferSize;

    -- Create socket
    Self.TcpClient := Ops_Pa.Socket_Pa.Create;
    Self.Connection := TCPConnection_Pa.Create( Self.TcpClient, Self.Port, HeartbeatPeriod, HeartbeatTimeout );
    Self.Connection.addListener( TCPConnection_Pa.ReceiveNotifier_Pa.Listener_Interface_At(SelfAt) );

    Self.Timer := Timer_Pa.Create( Ops_Class_At(SelfAt), Periodic => True );
    Self.Timer.addListener( Timer_Pa.DeadlineListener_Interface_At(SelfAt) );
  end;

  overriding procedure Finalize( Self : in out TCPClientReceiver_Class ) is
  begin
    Self.Timer.removeListener( Timer_Pa.DeadlineListener_Interface_At(Self.SelfAt) );
    Timer_Pa.Free( Self.Timer );

    Stop( Self );   -- Make sure socket is closed

    Finalize( Receiver_Class(Self) );  -- Make sure thread is terminated

    if Self.Connection /= null then
      Self.Connection.removeListener( TCPConnection_Pa.ReceiveNotifier_Pa.Listener_Interface_At(Self.SelfAt) );
      TCPConnection_Pa.Free(Self.Connection);
    end if;

    if Self.IpAddress /= null then
      Dispose(Self.IpAddress);
    end if;
  end;

  -- Called whenever the connection has new data.
  procedure OnNotify( Self : in out TCPClientReceiver_Class; Sender : in Ops_Class_At; Item : in BytesSizePair_T ) is
  begin
    -- Forward notification to upper layer with the data packet
    Self.DataNotifier.DoNotify(Item);
  end;

  procedure Report( Self : in out TCPClientReceiver_Class; method : string; mess : string ) is
    error : SocketError_Class_At := null;
  begin
    if Self.ErrorService /= null then
      error := SocketError("TCPClientReceiver", method, mess, Self.LastErrorCode);
      Self.ErrorService.Report(Error_Class_At(error));
    end if;
    if TraceEnabled then Self.Trace(method & ": " & mess & ", Error: " & Integer'Image(Self.LastErrorCode)); end if;
  end;

  overriding procedure SetErrorService( Self : in out TCPClientReceiver_Class; es : ErrorService_Class_At ) is
  begin
    SetErrorService( Receiver_Class(Self), es );
    Self.Connection.SetErrorService( es );
  end;

  -- Start():
  -- Starts the receiver, and reads bytes into given buffer.
  -- When a message is read, a callback (notification) will be done with the
  -- buffer and actual number of bytes read.
  -- When the callback returns a new read is started to the current buffer
  overriding function Start( Self: in out TCPClientReceiver_Class; bytes : Byte_Arr_At; size : Integer) return Boolean is
  begin
    Self.LastErrorCode := 0;
    if Self.TcpClient.IsOpen then
      return True;
    end if;

    if not Self.TcpClient.Open then
      Self.LastErrorCode := Self.TcpClient.GetLatestError;
      Report(Self, "Start", "Client socket could not be created [Server: " & Self.IpAddress.all & ":" & Integer'Image(Self.Port) & "]");
      return False;
    end if;

    Self.StopFlag := False;
    Self.Connection.SetReceiveBuffer(bytes, size);

    if bytes /= null then
      -- Start a thread running our run() method
      Self.EventsToTask.Signal(StartEvent_C);
    end if;
    return True;
  end;

  -- Override from Receiver
  -- Used to get the sender IP and port for a received message
  -- Only safe to call in callback
  overriding function GetSourceIP( Self : in out TCPClientReceiver_Class ) return String is
  begin
    return Self.IpAddress.all;
  end;

  overriding function GetSourcePort( Self : in out TCPClientReceiver_Class ) return Integer is
  begin
    return Self.Port;
  end;

  -- SetReceiveBuffer():
  -- Changes the current buffer to use for reads.
  -- Should only be called from the callback.
  overriding procedure SetReceiveBuffer( Self : in out TCPClientReceiver_Class; bytes : Byte_Arr_At; size : Integer) is
  begin
    Self.Connection.SetReceiveBuffer(bytes, size);
  end;

  -- Stop():
  -- Aborts an ongoing read. NOTE: Must NOT be called from the callback.
  overriding procedure Stop( Self : in out TCPClientReceiver_Class ) is
    dummy : Boolean;
  begin
    -- Tell run to exit
    Self.StopFlag := True;

    if Self.TcpClient.IsOpen then
      Self.Connection.Stop;
      -- Thread is probably waiting in a read, so we must close the socket
      dummy := Self.TcpClient.Shutdown;
      dummy := Self.TcpClient.Disconnect;

      if not Self.TcpClient.Close then
        Self.LastErrorCode := Self.TcpClient.GetLatestError;
      end if;

      Self.Connection.SetReceiveBuffer(null, 0);
    end if;
  end;

  overriding function Port( Self : TCPClientReceiver_Class ) return Integer is
  begin
    return Self.TcpClient.GetBoundPort;
  end;

  overriding function Address( Self : TCPClientReceiver_Class ) return String is
  begin
    return Self.TcpClient.GetBoundIP;
  end;

  overriding procedure OnDeadlineMissed( Self : in out TCPClientReceiver_Class; Sender : in Ops_Class_At ) is
    errorFlag : Boolean := False;
  begin
    Self.Connection.SendData( null, 0, errorFlag );
  end;

  overriding procedure Run( Self : in out TCPClientReceiver_Class ) is
    dummy : Boolean;
    Status : ConnectStatus_T;

    procedure OpenAndConnect is
    begin
      dummy := Self.TcpClient.Open;
      dummy := Self.TcpClient.Connect( Self.IpAddress.all, Self.Port );
    end;

    procedure DisconnectAndClose is
    begin
      dummy := Self.TcpClient.Disconnect;
      dummy := Self.TcpClient.Close;
    end;

  begin
    if TraceEnabled then Self.Trace("Started"); end if;
    while not Self.StopFlag loop
      begin
        -- Connect loop
        if TraceEnabled then Self.Trace("Connecting..."); end if;
        while (not Self.StopFlag) and (not Self.TcpClient.IsConnected) loop
          DisconnectAndClose;
          delay 0.100;
          exit when Self.StopFlag;
          OpenAndConnect;
        end loop;
        exit when Self.StopFlag;

        if TraceEnabled then Self.Trace("Connected"); end if;

        dummy := Self.TcpClient.SetNonBlocking(False);

        -- Connected, Set buffer size
        if Self.InSocketBufferSize > 0 then
          dummy := Self.TcpClient.SetReceiveBufferSize(Integer(Self.InSocketBufferSize));
          if Self.TcpClient.GetReceiveBufferSize /= Integer(Self.InSocketBufferSize) then
            Self.LastErrorCode := Ops_Pa.Socket_Pa.SOCKET_ERROR_C;
            Self.Report("Run", "Socket buffer size could not be set");
          end if;
        end if;

        -- Disable Nagle algorithm
        dummy := Self.TcpClient.SetTcpNoDelay(True);

--      notifyNewEvent(BytesSizePair(NULL, -5)); //Connection was down but has been reastablished.

        if Self.CsClient /= null then
          Ada.Strings.Fixed.Move(Self.TcpClient.GetPeerIP, Status.Address, Drop => Ada.Strings.Right);
          Status.Port := Self.TcpClient.GetPeerPort;
          Status.TotalNo := 1;
          Status.Connected := True;
          Self.CsClient.OnConnect( null, Status );
        end if;

        -- Send a probe to trig newer versions of TCPServers to enable heartbeats
        Self.Connection.SendProbe( dummy );

        -- Start timer so we send heartbeats on newer versions
        Self.Timer.Start( 1000 );

        -- Do the transfer phase while the connection is established
        Self.Connection.Run;

      exception
        when others =>
          Self.LastErrorCode := Ops_Pa.Socket_Pa.SOCKET_ERROR_C;
          Self.Report("Run", "Exception ");
      end;
      if Self.CsClient /= null then
        Status.TotalNo := 0;
        Status.Connected := False;
        Self.CsClient.OnDisconnect( null, Status );
      end if;
      Self.Timer.Cancel;
      DisconnectAndClose;
    end loop;
    if TraceEnabled then Self.Trace("Stopped"); end if;
  end;

end Ops_Pa.Transport_Pa.Receiver_Pa.TCPClient_Pa;

