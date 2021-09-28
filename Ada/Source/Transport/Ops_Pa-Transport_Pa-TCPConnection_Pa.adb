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
with Ada.Real_Time;

package body Ops_Pa.Transport_Pa.TCPConnection_Pa is

  use type Ops_Pa.Socket_Pa.TCPClientSocket_Class_At;

  subtype ByteArr4_T is Byte_Arr(0..3);
  function ToByteArr is new Ada.Unchecked_Conversion(Integer, ByteArr4_T);
  function ToByte    is new Ada.Unchecked_Conversion(Character, Byte);

  -- =========================================================================

  -- Original TCP ptotocol (version 1)
  -- ---------------------------------
  -- The OPS TCP Protocol has a leading header of 22 bytes with
  --    0..17    "opsp_tcp_size_info"
  --   18..21    uint32_t in little-endian format giving size of data that follows
  --
  -- TCP protocol version 2
  -- ----------------------
  -- The above is still valid for ordinary transport of OPS messages (used by old clients and server).
  -- Version 2 adds probe messages (probe and heartbeat).
  --    0..17    "opsprobeNNNN______"
  --   18..21    uint32_t in little-endian format giving size of data that follows
  --               0  : heartbeat no additional data
  --               1  : probe additional data, 1 byte with value 0
  --

  Protocol_Version_C : constant Integer := 2;
  SizeInfoSize_C     : constant Byte_Arr_Index_T := 22;
  ProbeBufferSize_C  : constant Byte_Arr_Index_T := SizeInfoSize_C + 1;
  MinBufferLength_C  : constant Byte_Arr_Index_T := ProbeBufferSize_C;

  opsp_tcp_size_info_header : constant Byte_Arr(0..17) :=
    (ToByte('o'),
     ToByte('p'),
     ToByte('s'),
     ToByte('p'),
     ToByte('_'),
     ToByte('t'),
     ToByte('c'),
     ToByte('p'),
     ToByte('_'),
     ToByte('s'),
     ToByte('i'),
     ToByte('z'),
     ToByte('e'),
     ToByte('_'),
     ToByte('i'),
     ToByte('n'),
     ToByte('f'),
     ToByte('o')
    );

  opsp_tcp_probe_header : constant Byte_Arr(0..17) :=
    (ToByte('o'),
     ToByte('p'),
     ToByte('s'),
     ToByte('p'),
     ToByte('r'),
     ToByte('o'),
     ToByte('b'),
     ToByte('e'),
     ToByte('N'),
     ToByte('N'),
     ToByte('N'),
     ToByte('N'),
     ToByte('_'),
     ToByte('_'),
     ToByte('_'),
     ToByte('_'),
     ToByte('_'),
     ToByte('_')
    );

  -- =========================================================================

  procedure Trace(Self : TCPConnection_Class; Msg : String) is
    NameStr : String := "TcpConnection (" & Integer'Image(Self.Port) & ")";
  begin
    Trace(NameStr, Msg);
  end;

  procedure Report( Self : in out TCPConnection_Class; method : string; mess : string ) is
    error : SocketError_Class_At := null;
  begin
    if Self.ErrorService /= null then
      error := SocketError("TCPConnection", method, mess, Self.LastErrorCode);
      Self.ErrorService.Report(Error_Class_At(error));
    end if;
    if TraceEnabled then Self.Trace(method & ": " & mess & ", Error: " & Integer'Image(Self.LastErrorCode)); end if;
  end;

  -- =========================================================================

  function Create( Socket : Ops_Pa.Socket_Pa.TCPClientSocket_Class_At;
                   Port : Integer;
                   HeartbeatPeriod : Int32;
                   HeartbeatTimeout : Int32
                  ) return TCPConnection_Class_At is
     Self : TCPConnection_Class_At := null;
  begin
    Self := new TCPConnection_Class;
    InitInstance( Self.all, Self, Socket, Port, HeartbeatPeriod, HeartbeatTimeout );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out TCPConnection_Class;
                          SelfAt : TCPConnection_Class_At;
                          Socket : Ops_Pa.Socket_Pa.TCPClientSocket_Class_At;
                          Port : Integer;
                          HeartbeatPeriod : Int32;
                          HeartbeatTimeout : Int32 ) is
  begin
    Self.Socket := Socket;
    Self.Port := Port;
    Self.HeartbeatPeriod := TimeMs_T(HeartbeatPeriod);
    Self.HeartbeatTimeout := TimeMs_T(HeartbeatTimeout);
    Self.DataNotifier := ReceiveNotifier_Pa.Create( Ops_Class_At(SelfAt) );

    Self.SizeBuffer := new Byte_Arr(0..SizeInfoSize_C-1);
    Self.SizeBuffer(0..17) := opsp_tcp_size_info_header;

    -- In ProbeBuffer the NNNN is the current TCP protocol version
    Self.ProbeBuffer := new Byte_Arr(0..ProbeBufferSize_C-1);
    Self.ProbeBuffer(0..17) := opsp_tcp_probe_header;
    Self.ProbeBuffer(8..11) := ToByteArr(Protocol_Version_C);
  end;

  overriding procedure Finalize( Self : in out TCPConnection_Class ) is
    dummy : Boolean;
  begin
    ReceiveNotifier_Pa.Free(Self.DataNotifier);
    if Self.Socket /= null then
      dummy := Self.Socket.Shutdown;
      dummy := Self.Socket.Disconnect;
      Ops_Pa.Socket_Pa.Free(Self.Socket);
    end if;
    Dispose(Self.ProbeBuffer);
    Dispose(Self.SizeBuffer);
  end;

    -- Return ref to underlaying socket
  function GetSocket( Self : TCPConnection_Class ) return Ops_Pa.Socket_Pa.TCPClientSocket_Class_At is
  begin
    return Self.Socket;
  end;

  -- =========================================================================

  procedure addListener( Self : in out TCPConnection_Class; Client : ReceiveNotifier_Pa.Listener_Interface_At ) is
  begin
    Self.DataNotifier.addListener( Client );
  end;

  procedure removeListener( Self : in out TCPConnection_Class; Client : ReceiveNotifier_Pa.Listener_Interface_At ) is
  begin
    Self.DataNotifier.removeListener( Client );
  end;

  -- =========================================================================

  -- SetReceiveBuffer():
  -- Changes the current buffer to use for reads.
  -- Should only be called from the callback.
  procedure SetReceiveBuffer( Self : in out TCPConnection_Class; bytes : Byte_Arr_At; size : Integer) is
  begin
    Self.Buffer := bytes;
    if bytes /= null then
      -- check that buffer is large enough
      if size < Integer(MinBufferLength_C) then
        raise Buffer_Too_Small;
      end if;
    end if;
    Self.BufferSize := size;
  end;

  -- =========================================================================

  -- size == 0, trigs a periodic check
  procedure SendData( Self : in out TCPConnection_Class; buf : Byte_Arr_At; size : Integer; errorFlag : in out boolean ) is
  begin
    if size = 0 then
      Self.PeriodicCheck( errorFlag );

    else
      Self.TimeSnd := GetTimeInMs;

      -- First, prepare a package of fixed length 22 with information about the size of the data package
      Self.SizeBuffer(18..21) := ToByteArr(size);

      if TraceEnabled then Self.Trace("SendData, length= " & Integer'Image(size) & ")"); end if;

      -- Send prepared size info
      if Self.Socket.SendBuf(Self.SizeBuffer.all) = Integer(SizeInfoSize_C) then
        -- Send the actual data
        if Self.Socket.SendBuf(buf.all(buf'first..buf'first+Byte_Arr_Index_T(size)-1)) /= size then
          Self.LastErrorCode := Self.Socket.GetLatestError;
          errorFlag := True;
        end if;
      else
        Self.LastErrorCode := Self.Socket.GetLatestError;
        errorFlag := True;
      end if;
    end if;
  end;

  -- Should only be called by TCP clients
  -- Returns errorFlag if an error
  procedure SendProbe( Self : in out TCPConnection_Class; errorFlag : in out boolean ) is
  begin
    if TraceEnabled then Self.Trace("SendProbe"); end if;

    Self.TimeSnd := GetTimeInMs;

    -- Fill in size in protocol probe header
    Self.ProbeBuffer(18..21) := ToByteArr(1);

    -- One byte of real data is needed to not get a disconnect if the message is received in an old client.
    -- This will be acceptable by old servers/clients. Clients log an error code and continue to read.
    -- Old servers wont notice since they don't read. This should not be a problem, a small amount of
    -- bytes will be transfered and stored in the stack.
    -- New TCP Servers notice and enable the sending of heartbeat/pubinfo and other new functionality.
    Self.ProbeBuffer(SizeInfoSize_C) := 0;

    -- Send header + 1 byte
    if Self.Socket.SendBuf(Self.ProbeBuffer.all(0..ProbeBufferSize_C-1)) /= Integer(ProbeBufferSize_C) then
      Self.LastErrorCode := Self.Socket.GetLatestError;
      errorFlag := True;
    end if;
  end;

  -- Returns errorFlag if an error
  procedure SendHeartbeat( Self : in out TCPConnection_Class; errorFlag : in out boolean ) is
  begin
    if Self.DetectedVersion > 1 then
      if TraceEnabled then Self.Trace("SendHeartbeat"); end if;

      Self.TimeSnd := GetTimeInMs;
      Self.HbSent := true;

      -- Fill in size in protocol probe header
      Self.ProbeBuffer(18..21) := ToByteArr(0);

      -- Send only a header
      if Self.Socket.SendBuf(Self.ProbeBuffer.all(0..SizeInfoSize_C-1)) /= Integer(SizeInfoSize_C) then
        Self.LastErrorCode := Self.Socket.GetLatestError;
        if TraceEnabled then Self.Trace("SendHeartbeat, Error= " & Integer'Image(Self.LastErrorCode)); end if;
        errorFlag := True;
      end if;
    end if;
  end;

  procedure PeriodicCheck( Self : in out TCPConnection_Class; errorFlag : in out boolean ) is
  begin
    if Self.DetectedVersion > 1 and Self.HeartbeatPeriod > 0 then
      declare
        Now : TimeMs_T := GetTimeInMs;
      begin
        -- Check if we need to send a heartbeat (at least one during a connection)
        if not Self.HbSent or ((Now - Self.TimeSnd) >= Self.HeartbeatPeriod) then
          Self.SendHeartbeat( errorFlag );
        end if;

        -- Check receive timeout
        if Now - Self.TimeRcv >= Self.HeartbeatTimeout then
          if TraceEnabled then Self.Trace("No Data. Connection timed out"); end if;
          errorFlag := True;
        end if;
      end;
    end if;
  end;

  -- =========================================================================

  -- Exit Run as fast as possible
  procedure Stop( Self : in out TCPConnection_Class ) is
  begin
    Self.StopFlag := True;
    -- In case waiting on a ReceiveBuf with timeout
    Self.Socket.AbortReceiveBuf;
  end;

  procedure SetupForReadingSize( Self : in out TCPConnection_Class ) is
  begin
    Self.Phase := phSize;
    Self.BufferIdx := Self.Buffer'First;
    Self.BufferIdxLast := Self.BufferIdx + SizeInfoSize_C - 1;
  end;

  procedure HandleSizeInfo( Self : in out TCPConnection_Class; ErrorDetected : in out Boolean ) is
    Res : Integer := 0;
    BytesToRead : Byte_Arr_Index_T;
  begin
    -- Get size of data packet from the received size packet
    BytesToRead :=
      16#0000_0001# * Byte_Arr_Index_T(Self.Buffer(Self.Buffer'First+18)) +
      16#0000_0100# * Byte_Arr_Index_T(Self.Buffer(Self.Buffer'First+19)) +
      16#0001_0000# * Byte_Arr_Index_T(Self.Buffer(Self.Buffer'First+20)) +
      16#0100_0000# * Byte_Arr_Index_T(Self.Buffer(Self.Buffer'First+21));

    if BytesToRead > Byte_Arr_Index_T(Self.BufferSize) then
      -- This is an error, we are not able to receive more than the buffer size
      Self.LastErrorCode := Ops_Pa.Socket_Pa.SOCKET_ERROR_C;
      Self.Report("HandleSizeInfo", "Error in read size info");
      ErrorDetected := True;

    elsif BytesToRead < 2 then
      -- Check if it's a heartbeat or probe message
      if Self.Buffer(Self.Buffer'First..Self.Buffer'First+7) /= Self.ProbeBuffer(0..7) then
        Self.LastErrorCode := Ops_Pa.Socket_Pa.SOCKET_ERROR_C;
        Self.Report("HandleSizeInfo", "Too short message");
        ErrorDetected := True;
      else
        if BytesToRead = 0 then
          -- Heartbeat, read new message
          Self.DetectedVersion := 2;
          if TraceEnabled then Self.Trace("Got a Heartbeat"); end if;
          Self.SetupForReadingSize;
        else
          -- Probe
          Res := Integer(Self.BufferIdxLast - Self.Buffer'First + 1);
          if Res = Integer(SizeInfoSize_C) then
            -- Probe, one more byte to read, continue in the same buffer
            Self.BufferIdxLast := Self.BufferIdxLast + 1;
          else
            -- Probe, All bytes read, read a new message
            Self.DetectedVersion := 2;
            if TraceEnabled then Self.Trace("Got a Probe message"); end if;
            Self.SetupForReadingSize;
          end if;
        end if;
      end if;

    else
      Self.Phase := phPayload;
      Self.BufferIdx := Self.Buffer'First;
      Self.BufferIdxLast := Self.BufferIdx + BytesToRead - 1;
    end if;
  end;

  procedure HandlePayload( Self : in out TCPConnection_Class ) is
    Res : Integer := Integer(Self.BufferIdxLast - Self.Buffer'First + 1);
  begin
    if TraceEnabled then Self.Trace("Notify(data packet received, length= " & Integer'Image(Res) & ")"); end if;

    -- Notify upper layer with a data packet
    Self.DataNotifier.DoNotify(BytesSizePair_T'(Self.Buffer, Res));

    -- Set up for reading a new size info
    Self.SetupForReadingSize;
  end;

  procedure HandleResult( Self : in out TCPConnection_Class;
                          Res : in Integer;
                          Timedout : in Boolean;
                          ErrorDetected : in out Boolean ) is
  begin
    if Res > 0 then
      -- Read OK
      Self.TimeRcv := GetTimeInMs;
      Self.BufferIdx := Self.BufferIdx + Byte_Arr_Index_T(Res);
      if Self.BufferIdx > Self.BufferIdxLast then
        -- Expected number of bytes read
        case Self.Phase is
          when phSize => Self.HandleSizeInfo( ErrorDetected );
          when phPayload => Self.HandlePayload;
        end case;
      else
        -- Continue to read more bytes
        null;
      end if;

    elsif Res = 0 then
      if Timedout then
        -- Connection timedout due to no data
        if TraceEnabled then Self.Trace("Connection timed out"); end if;
      else
        -- Connection closed gracefully
        if TraceEnabled then Self.Trace("Connection closed gracefully"); end if;
      end if;
      ErrorDetected := True;

    elsif Res < 0 then
      -- Some error
      Self.LastErrorCode := Self.Socket.GetLatestError;
      Self.Report("Run", "Read failed");
      ErrorDetected := True;
    end if;
  end;

  -- Performs the reading and notification of data to the client
  -- Called when the socket is connected, exits on error or stop
  procedure Run( Self : in out TCPConnection_Class ) is
    Res : Integer := 0;
    ErrorDetected : Boolean := False;
    Timedout : Boolean := False;
    TimeSpan  : Ada.Real_Time.Time_Span;
    Dur       : Duration;
  begin
    Self.StopFlag := False;
    Self.LastErrorCode := 0;
    Self.DetectedVersion := 1;
    Self.HbSent := False;

    -- Set up for reading a new size info
    Self.SetupForReadingSize;

    TimeSpan := Ada.Real_Time.Milliseconds( Integer(Self.HeartbeatTimeout) );
    Dur := Ada.Real_Time.To_Duration(TimeSpan);

    -- Read data loop
    while (not Self.StopFlag) and (Self.Socket.IsConnected) loop
      if TraceEnabled then Self.Trace("Wait for Data..."); end if;

      if Self.DetectedVersion > 1 then
        --Wait for data with timeout. If no data, exit which closes the connection
        --This will be the heartbeat check (ie. we don't need to save time for received messages?)
        Res := Self.Socket.ReceiveBuf( Self.Buffer(Self.BufferIdx..Self.BufferIdxLast), Dur, Timedout );
      else
        Res := Self.Socket.ReceiveBuf( Self.Buffer(Self.BufferIdx..Self.BufferIdxLast) );
      end if;

      exit when Self.StopFlag;

      Self.HandleResult( Res, Timedout, ErrorDetected );
      exit when ErrorDetected;
    end loop;
  end;

  -- Performs the reading and notification of data to the client
  -- Called when there is data for the socket, exits when data handled
  procedure Poll( Self : in out TCPConnection_Class; ErrorDetected : in out Boolean ) is
    Res : Integer := 0;
    Timedout : Boolean := False;
  begin
    Res := Self.Socket.ReceiveBuf( Self.Buffer(Self.BufferIdx..Self.BufferIdxLast) );
    if TraceEnabled then Self.Trace("Read: " & Integer'Image(Res)); end if;
    Self.HandleResult( Res, Timedout, ErrorDetected );
  end;

  function GetUserData( Self : TCPConnection_Class ) return Integer is
  begin
    return Self.UserData;
  end;

  procedure SetUserData( Self : in out TCPConnection_Class; Value : Integer ) is
  begin
    Self.UserData := Value;
  end;

  -- =========================================================================

  function Create( Socket : Ops_Pa.Socket_Pa.TCPClientSocket_Class_At;
                   Port : Integer;
                   HeartbeatPeriod : Int32;
                   HeartbeatTimeout : Int32
                  ) return TCPServerConnection_Class_At is
     Self : TCPServerConnection_Class_At := null;
  begin
    Self := new TCPServerConnection_Class;
    InitInstance( Self.all, Self, Socket, Port, HeartbeatPeriod, HeartbeatTimeout );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out TCPServerConnection_Class;
                          SelfAt : TCPServerConnection_Class_At;
                          Socket : Ops_Pa.Socket_Pa.TCPClientSocket_Class_At;
                          Port : Integer;
                          HeartbeatPeriod : Int32;
                          HeartbeatTimeout : Int32 ) is
  begin
    InitInstance( TCPConnection_Class(Self), TCPConnection_Class_At(SelfAt), Socket, Port, HeartbeatPeriod, HeartbeatTimeout );
    Self.ServerBuffer := new Byte_Arr'(0..Byte_Arr_Index_T(1023) => 0);
    Self.SetReceiveBuffer( Self.ServerBuffer, 1024 );
    Self.SetupForReadingSize;
  end;

  overriding procedure Finalize( Self : in out TCPServerConnection_Class ) is
  begin
    Finalize( TCPConnection_Class(Self) );
    Dispose(Self.ServerBuffer);
  end;

end Ops_Pa.Transport_Pa.TCPConnection_Pa;

