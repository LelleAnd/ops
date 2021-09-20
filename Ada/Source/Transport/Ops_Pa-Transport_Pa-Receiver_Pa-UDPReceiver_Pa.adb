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

package body Ops_Pa.Transport_Pa.Receiver_Pa.UDPReceiver_Pa is

  function Create( bindPort : Integer;
                   localInterface : string := "0.0.0.0";
                   inSocketBufferSize : Int64 := 16000000) return UDPReceiver_Class_At is
     Self : UDPReceiver_Class_At := null;
  begin
    Self := new UDPReceiver_Class;
    InitInstance( Self.all, Self, bindPort, localInterface, inSocketBufferSize );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out UDPReceiver_Class;
                          SelfAt : UDPReceiver_Class_At;
                          bindPort : Integer;
                          localInterface : String;
                          inSocketBufferSize : Int64 ) is
  begin
    InitInstance( Receiver_Class(Self), Receiver_Class_At(SelfAt) );

    Self.UdpSocket := Ops_Pa.Socket_Pa.Create;

    if not Self.UdpSocket.Open then
      Self.LastErrorCode := Self.UdpSocket.GetLatestError;
      Report(Self, "InitInstance", "Socket could not be created");
      return;
    end if;

    if localInterface = "0.0.0.0" then
      Self.IpAddress := Copy("127.0.0.1");
    else
      Self.IpAddress := Copy(localInterface);
    end if;

    Self.Port := bindPort;

    -- Set blocking calls
    if not Self.UdpSocket.SetNonBlocking( False ) then
      Self.LastErrorCode := Self.UdpSocket.GetLatestError;
      Report(Self, "Start", "Failed to set Blocking calls [" & Self.IpAddress.all & ":" & Integer'Image(Self.Port) & "]");
    end if;

    -- Set reuse address
    if not Self.UdpSocket.SetReuseAddress( True ) then
      Self.LastErrorCode := Self.UdpSocket.GetLatestError;
      Report(Self, "Start", "Failed to set REUSE ADDR [" & Self.IpAddress.all & ":" & Integer'Image(Self.Port) & "]");
    end if;

    -- Bind socket to local address
    if not Self.UdpSocket.Bind( Self.IpAddress.all, Self.Port ) then
      Self.LastErrorCode := Self.UdpSocket.GetLatestError;
      Report(Self, "Start", "Bind error [" & Self.IpAddress.all & ":" & Integer'Image(Self.Port) & "]");
      return;
    end if;

    -- Get actual port that socket is bound to (in case bind port = 0)
    Self.Port := Self.UdpSocket.GetBoundPort;

    -- Set socket buffer size
    if InSocketBufferSize > 0 then
      if not Self.UdpSocket.SetReceiveBufferSize( Integer(InSocketBufferSize) ) then
        Self.LastErrorCode := Self.UdpSocket.GetLatestError;
      end if;

      if Self.UdpSocket.GetReceiveBufferSize /= Integer(InSocketBufferSize) then
        if Self.LastErrorCode = 0 then
          Self.LastErrorCode := Ops_Pa.Socket_Pa.SOCKET_ERROR_C;
        end if;
        Report(Self, "Start", "Socket buffer size could not be set [" & Self.IpAddress.all & ":" & Integer'Image(Self.Port) & "]");
      end if;
    end if;
  end;

  overriding procedure Finalize( Self : in out UDPReceiver_Class ) is
  begin
    Stop( Self );   -- Make sure socket is closed

    Finalize( Receiver_Class(Self) );  -- Make sure thread is terminated

    Ops_Pa.Socket_Pa.Free(Self.UdpSocket);

    if Self.IpAddress /= null then
      Dispose(Self.IpAddress);
    end if;
  end;

  procedure Report( Self : in out UDPReceiver_Class; method : string; mess : string) is
    error : SocketError_Class_At := null;
  begin
    if Self.ErrorService /= null then
      error := SocketError("UDPReceiver", method, mess, Self.LastErrorCode);
      Self.ErrorService.Report(Error_Class_At(error));
    end if;
  end;

  -- Start():
  -- Starts the receiver, and reads bytes into given buffer.
  -- When a message is read, a callback (notification) will be done with the
  -- buffer and actual number of bytes read.
  -- When the callback returns a new read is started to the current buffer
  overriding function Start( Self: in out UDPReceiver_Class; bytes : Byte_Arr_At; size : Integer) return Boolean is
  begin
    Self.LastErrorCode := 0;
    if not Self.UdpSocket.IsOpen then
      -- This is a Start after Stop case. Do we need to handle that???
      Self.LastErrorCode := Ops_Pa.Socket_Pa.SOCKET_ERROR_C;
      Report(Self, "Start", "Can't Start after a Stop (NYI)");
      return False;
    end if;

    Self.StopFlag := False;
    Self.Buffer := bytes;
    Self.BufferSize := size;

    if Self.Buffer /= null then
      -- Start a thread running our run() method
      Self.EventsToTask.Signal(StartEvent_C);
    end if;
    return True;
  end;

  -- Override from Receiver
  -- Used to get the sender IP and port for a received message
  -- Only safe to call in callback
  overriding function GetSourceIP( Self : in out UDPReceiver_Class ) return String is
  begin
    return Self.UdpSocket.GetSourceIP;
  end;

  overriding function GetSourcePort( Self : in out UDPReceiver_Class ) return Integer is
  begin
    return Self.UdpSocket.GetSourcePort;
  end;

  -- SetReceiveBuffer():
  -- Changes the current buffer to use for reads.
  -- Should only be called from the callback.
  overriding procedure SetReceiveBuffer( Self : in out UDPReceiver_Class; bytes : Byte_Arr_At; size : Integer) is
  begin
    Self.Buffer := bytes;
    Self.BufferSize := size;
  end;

  -- Stop():
  -- Aborts an ongoing read. NOTE: Must NOT be called from the callback.
  overriding procedure Stop( Self : in out UDPReceiver_Class ) is
    dummy : Boolean;
  begin
    -- Tell run to exit
    Self.StopFlag := True;

    if Self.UdpSocket.IsOpen then
      -- Thread is probably waiting in a read, so we must close the socket
      dummy := Self.UdpSocket.Shutdown;

      if not Self.UdpSocket.Close then
        Self.LastErrorCode := Self.UdpSocket.GetLatestError;
      end if;

      Self.Buffer := null;
      Self.BufferSize := 0;
    end if;
  end;

  function available( Self : UDPReceiver_Class ) return Boolean is
  begin
    return Self.UdpSocket.IsOpen;
  end;

  function Port( Self : UDPReceiver_Class ) return Integer is
  begin
    return Self.Port;
  end;

  function Address( Self : UDPReceiver_Class ) return String is
  begin
    if Self.IpAddress /= null then
      return Self.IpAddress.all;
    else
      return "";
    end if;
  end;

  function ReceiveMessage( Self : in out UDPReceiver_Class; o: Byte_Arr_At; size: Integer ) return Integer is
    Result : Integer := 0;
  begin
    if Self.UdpSocket.IsOpen then
      Self.LastErrorCode := 0;
      Result := Self.UdpSocket.ReceiveFrom(o.all(o'First..o'First+Byte_Arr_Index_T(size)-1));
      if Result < 0 then
        Self.LastErrorCode := Self.UdpSocket.GetLatestError;
      end if;
    end if;
    return Result;
  end;

  --(**************************************************************************
  --*
  --**************************************************************************)
  overriding procedure Run( Self : in out UDPReceiver_Class ) is
    Res : Integer;
  begin
    while not Self.StopFlag loop
      Res := ReceiveMessage(Self, Self.Buffer, Self.BufferSize);
      exit when Self.StopFlag;

      if Res > 0 then
        -- Got some data, Notify listener
        Self.DataNotifier.DoNotify(BytesSizePair_T'(Self.Buffer, Res));

      elsif Res = 0 then
        -- Could this happen?
        delay 0.010;    -- So we don't hog the cpu on errors

      else
--          if Self.LastErrorCode = Win32.Winsock.WSAEWOULDBLOCK then
--            -- Can't happen since we use a blocking socket
--            null;
--
--          elsif Self.LastErrorCode = Win32.Winsock.WSAECONNRESET then
--            -- On a UDP-datagram socket this error indicates a previous send
--            -- operation resulted in an ICMP Port Unreachable message.
--            null;
--
--          else
--            null;
--
--          end if;

        Report(Self, "Run", "Receive error");

        delay 0.010;    -- So we don't hog the cpu on errors
      end if;
    end loop;
  end;

end Ops_Pa.Transport_Pa.Receiver_Pa.UDPReceiver_Pa;

