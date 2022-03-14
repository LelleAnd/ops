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

with Ada.Unchecked_Conversion;
with Ada.Strings.Fixed;

with Ops_Pa.Socket_Pa,
     Ops_Pa.Signal_Pa;

package body Ops_Pa.Transport_Pa.Sender_Pa.TCPServer_Pa is

  use type Ada.Containers.Count_Type;
  use type Ops_Pa.Socket_Pa.TCPClientSocket_Class_At;
  use type Ops_Pa.Socket_Pa.TCPServerSocket_Class_At;
  use type TCPConnection_Pa.TCPServerConnection_Class_At;
  use type Ops_Pa.Signal_Pa.Event_T;

  function Equal( Left, Right : TCPConnection_Pa.TCPServerConnection_Class_At ) return Boolean is
  begin
    return Left = Right;
  end;

  procedure Trace(Self : TCPServerSender_Class; Msg : String) is
    NameStr : String := "TcpServer (" & Integer'Image(Self.ActualPort) & ")";
  begin
    Trace(NameStr, Msg);
  end;

  procedure Report( Self : in out TCPServerSender_Class; method : string; mess : string ) is
    error : SocketError_Class_At := null;
  begin
    if Self.ErrorService /= null then
      error := SocketError("TCPServerSender", method, mess, Self.LastErrorCode);
      Self.ErrorService.Report(Error_Class_At(error));
    end if;
  end;

  -- ==========================================================================

  function Create(serverIP : String;
                  serverPort : Integer;
                  HeartbeatPeriod : Int32;
                  HeartbeatTimeout : Int32;
                  outSocketBufferSize : Int64 := 16000000) return TCPServerSender_Class_At is
    Self : TCPServerSender_Class_At := null;
  begin
    Self := new TCPServerSender_Class;
    InitInstance( Self.all, serverIP, serverPort, HeartbeatPeriod, HeartbeatTimeout, outSocketBufferSize );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out TCPServerSender_Class;
                          serverIP : String;
                          serverPort : Integer;
                          HeartbeatPeriod : Int32;
                          HeartbeatTimeout : Int32;
                          outSocketBufferSize : Int64) is
  begin
    Self.Port := serverPort;
    Self.ActualPort := Self.Port;
    Self.IpAddress := Copy(serverIP);
    Self.OutSocketBufferSize := outSocketBufferSize;
    Self.HeartbeatPeriod := HeartbeatPeriod;
    Self.HeartbeatTimeout := HeartbeatTimeout;
    if TraceEnabled then Self.Trace("InitInstance()"); end if;

    Self.TcpServer := Ops_Pa.Socket_Pa.Create;
    Self.SocketWaits := Ops_Pa.Socket_Pa.Create;

    Self.Server_Pr.Start;
  end;

  overriding procedure Finalize( Self : in out TCPServerSender_Class ) is
  begin
    if TraceEnabled then Self.Trace("Finalize()"); end if;
    Close( Self );

    Self.TerminateFlag := True;
    Self.EventsToTask.Signal(TerminateEvent_C);
    Self.Server_Pr.Finish;

    Ops_Pa.Socket_Pa.Free(Self.SocketWaits);
    Ops_Pa.Socket_Pa.Free(Self.TcpServer);

    if Self.IpAddress /= null then
      Dispose( Self.IpAddress );
    end if;
  end;

  -- ==========================================================================

  overriding procedure Open( Self : in out TCPServerSender_Class ) is
  begin
    if not Self.Opened then
      if TraceEnabled then Self.Trace("Open()"); end if;

      if not Self.TcpServer.Open then
        Self.LastErrorCode := Self.TcpServer.GetLatestError;
        Report(Self, "Run", "Socket could not be created [" & Self.IpAddress.all & ":" & Integer'Image(Self.Port) & "]");
        return;
      end if;
      if not Self.TcpServer.SetReuseAddress(True) then
        Report(Self, "Run", "Failed to set REUSE_ADDRESS for server socket [" & Self.IpAddress.all & ":" & Integer'Image(Self.Port) & "]");
      end if;
      if not Self.TcpServer.Bind( Self.IpAddress.all, Self.Port ) then
        Self.LastErrorCode := Self.TcpServer.GetLatestError;
        Report(Self, "Run", "Socket could not be bound [" & Self.IpAddress.all & ":" & Integer'Image(Self.Port) & "]");
      end if;

      Self.ActualPort := Self.TcpServer.GetBoundPort;

      Self.Opened := True;
      Self.StopFlag := False;

      -- Start a thread running our run() method
      Self.EventsToTask.Signal(StartEvent_C);
    end if;
  end;

  -- Should only be called while lock is held
  procedure DeleteConnection( Self : in out TCPServerSender_Class; i : MyIndex_T; Total : Integer ) is
    tcpClient : Ops_Pa.Socket_Pa.TCPClientSocket_Class_At := Self.ConnectedSockets.Element(i).GetSocket;
    Status : ConnectStatus_T;
  begin
    Self.SocketWaits.Remove( Socket_Pa.Socket_Class_At(tcpClient) );
    if Self.CsClient /= null then
      begin
        Ada.Strings.Fixed.Move( tcpClient.GetPeerIP, Status.Address, Drop => Ada.Strings.Right );
        Status.Port := tcpClient.GetPeerPort;
      exception
        when others =>
          null;
      end;
      Status.TotalNo := Total;
      Self.CsClient.OnDisconnect( null, Status );
    end if;
    TCPConnection_Pa.Free( Self.ConnectedSockets.Element(i) );
  end;

  overriding procedure Close( Self : in out TCPServerSender_Class ) is
    dummy : Boolean;
  begin
    if Self.Opened then
      if TraceEnabled then Self.Trace("Close()"); end if;
      Self.Opened := False;

      -- Tell thread to terminate
      Self.StopFlag := True;
      Self.SocketWaits.AbortWait;

      dummy := Self.TcpServer.Close;

      Self.ActualPort := Self.Port;

      -- Free all connected sockets, which implicitly will close all connections
      declare
        S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.ConnectedSocketsMutex'Access);
      begin
        for i in Self.ConnectedSockets.First_Index .. Self.ConnectedSockets.Last_Index loop
          if Self.ConnectedSockets.Element(i) /= null then
            Self.DeleteConnection( i, 0 );
          end if;
        end loop;
        Self.ConnectedSockets.Clear;
        if TraceEnabled then Self.Trace("Close(): Num: " & Integer'Image(Integer(Self.ConnectedSockets.Length))); end if;
      end;
    end if;
  end;

  -- ==========================================================================

  overriding function getAddress( Self : in out TCPServerSender_Class ) return String is
  begin
    return Self.IpAddress.all;
  end;

  overriding function getPort( Self : in out TCPServerSender_Class ) return Integer is
  begin
    return Self.ActualPort;
  end;

  -- ==========================================================================

  -- Should only be called while lock is held
  procedure HandleErrorAndDeleteConnection( Self : in out TCPServerSender_Class; i : MyIndex_T; method : string; mess : string  ) is
  begin
    Self.LastErrorCode := Self.ConnectedSockets.Element(i).LastErrorCode;
    Report(Self, method, mess);
    Self.DeleteConnection( i, Integer(Self.ConnectedSockets.Length)-1 );
    Self.ConnectedSockets.Delete(i);
  end;

  -- Sends buf to any Receiver connected to this Sender, ip and port are discarded and can be left blank.
  overriding function sendTo( Self : in out TCPServerSender_Class; buf : Byte_Arr_At; size : Integer; ip : string; port : Integer) return Boolean is
    errorFlag : Boolean;
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.ConnectedSocketsMutex'Access);
  begin
    -- Send to anyone connected. Loop backwards to avoid problems when removing broken sockets
    for i in reverse Self.ConnectedSockets.First_Index .. Self.ConnectedSockets.Last_Index loop
      ErrorFlag := False;
      begin
        if Self.ConnectedSockets.Element(i) /= null then
          Self.ConnectedSockets.Element(i).SendData(buf, size, ErrorFlag);
        end if;
      exception
        when others =>
          ErrorFlag := True;
      end;

      if ErrorFlag then
        Self.HandleErrorAndDeleteConnection( i, "sendTo", "Error sending" );
      end if;
    end loop;
    return True;
  end;

  -- ==========================================================================

  task body Server_Pr_T is
    Events : Ops_Pa.Signal_Pa.Event_T;
  begin
    select
      accept Start;
      while not Self.TerminateFlag loop
        begin
          Self.EventsToTask.WaitForAny(Events);
          exit when (Events and TerminateEvent_C) /= Ops_Pa.Signal_Pa.NoEvent_C;
          if (Events and StartEvent_C) /= Ops_Pa.Signal_Pa.NoEvent_C then
            Self.Run;
          end if;
        exception
          when others =>
            Self.ErrorService.Report( "TCPServer", "Server_Pr", "Got exception from Run()" );
        end;
      end loop;
      accept Finish;
    or
      accept Finish;
    end select;
  end Server_Pr_T;

  procedure Run( Self : in out TCPServerSender_Class ) is
    tcpClient : Ops_Pa.Socket_Pa.TCPClientSocket_Class_At := null;
    dummy : Boolean;

    procedure DoAccept is
    begin
      if tcpClient = null then
        -- Create a client socket ready for the calling client
        tcpClient := Ops_Pa.Socket_Pa.Create;
      end if;

      -- accept()
      if not Self.TcpServer.AcceptClient(tcpClient) then
        return;
      end if;
      if Self.StopFlag then
        return;
      end if;

      -- Now we have a connected client, setup some parameters
      if Self.OutSocketBufferSize > 0 then
        dummy:= tcpClient.SetSendBufferSize(Integer(Self.OutSocketBufferSize));
        if tcpClient.GetSendBufferSize /= Integer(Self.OutSocketBufferSize) then
          Self.LastErrorCode := Ops_Pa.Socket_Pa.SOCKET_ERROR_C;
          Report(Self, "Run", "Socket buffer size could not be set for connected client socket");
        end if;
      end if;

      -- Disable Nagle algorithm
      dummy := tcpClient.SetTcpNoDelay(True);

      -- and put it in list and then wait for another connection
      declare
        S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.ConnectedSocketsMutex'Access);
        Port : Integer := 0;
        Conn : TCPConnection_Pa.TCPServerConnection_Class_At := null;
        Status : ConnectStatus_T;
      begin
        Self.SocketWaits.Add( Socket_Pa.Socket_Class_At(tcpClient) );
        Port := tcpClient.GetBoundPort;
        Conn := TCPConnection_Pa.Create( tcpClient, Port, Self.HeartbeatPeriod, Self.HeartbeatTimeout );
        Self.ConnectedSockets.Append( Conn );
        if Self.CsClient /= null then
          Ada.Strings.Fixed.Move( tcpClient.GetPeerIP, Status.Address, Drop => Ada.Strings.Right );
          Status.Port := tcpClient.GetPeerPort;
          Status.TotalNo := Integer(Self.ConnectedSockets.Length);
          Status.Connected := True;
          Self.CsClient.OnConnect( null, Status );
        end if;
      end;
      tcpClient := null;   -- Clear ref since list now owns object
    end;

  begin
    while not Self.StopFlag loop
      begin
        -- Setup server socket for listening
        if not Self.TcpServer.Listen( 10 ) then
          Self.LastErrorCode := Self.TcpServer.GetLatestError;
          Report(Self, "Run", "Socket Listen failed [" & Self.IpAddress.all & ":" & Integer'Image(Self.ActualPort) & "]");
        end if;

        if not Self.TcpServer.IsListening then
          Self.LastErrorCode := Self.TcpServer.GetLatestError;

        else
          Self.SocketWaits.Clear;
          Self.SocketWaits.Add( Socket_Pa.Socket_Class_At(Self.TcpServer) );

          -- Keep listening for connecting clients
          while not Self.StopFlag loop
            -- Wait for accept or data from one or more clients
            if TraceEnabled then Self.Trace("Wait..."); end if;
            Self.SocketWaits.Wait( Dur => 1.0, Timedout => dummy );
            exit when Self.StopFlag;

            if TraceEnabled then Self.Trace("Wait exited"); end if;

            -- Check our server if there is any waiting connects
            if Self.SocketWaits.IsSet( Socket_Pa.Socket_Class_At(Self.TcpServer) ) then
              -- Server ready for an accept call
              if TraceEnabled then Self.Trace("Do Accept..."); end if;
              DoAccept;
            end if;
            exit when Self.StopFlag;

            -- Check if any of the connected sockets have data and also do the periodic check
            declare
              S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.ConnectedSocketsMutex'Access);
              ErrorDetected : Boolean;
            begin
              -- Loop over connected sockets, Loop backwards to avoid problems when removing broken sockets
              for i in reverse Self.ConnectedSockets.First_Index .. Self.ConnectedSockets.Last_Index loop
                ErrorDetected := False;
                begin
                  if Self.SocketWaits.IsSet( Socket_Pa.Socket_Class_At(Self.ConnectedSockets.Element(i).GetSocket) ) then
                    -- This connection have data to read
                    Self.ConnectedSockets.Element(i).Poll( ErrorDetected );
                  end if;
                  Self.ConnectedSockets.Element(i).PeriodicCheck( ErrorDetected );
                exception
                  when others => ErrorDetected := True;
                end;
                if ErrorDetected then
                  Self.HandleErrorAndDeleteConnection( i, "run", "Read/Send error" );
                end if;
              end loop;
            end;
          end loop;
        end if;
      exception
        when others =>
          null;
      end;

      delay 0.100;

      if tcpClient /= null then
        Ops_Pa.Socket_Pa.Free(tcpClient);
        tcpClient := null;
      end if;
    end loop;
  end;

end Ops_Pa.Transport_Pa.Sender_Pa.TCPServer_Pa;

