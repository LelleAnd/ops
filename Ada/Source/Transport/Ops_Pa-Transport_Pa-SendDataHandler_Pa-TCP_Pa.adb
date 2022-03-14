--
-- Copyright (C) 2017-2019 Lennart Andersson.
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

with Ops_Pa.Socket_Pa;

package body Ops_Pa.Transport_Pa.SendDataHandler_Pa.TCP_Pa is

  function Create( topic : Topic_Class_At; Reporter : ErrorService_Class_At ) return TcpSendDataHandler_Class_At is
     Self : TcpSendDataHandler_Class_At := null;
  begin
    Self := new TcpSendDataHandler_Class;
    InitInstance( Self.all, Self, topic, Reporter );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out TcpSendDataHandler_Class;
                          SelfAt : in TcpSendDataHandler_Class_At;
                          topic : Topic_Class_At;
                          Reporter : ErrorService_Class_At ) is
  begin
    InitInstance( SendDataHandler_Class(Self), SendDataHandler_Class_At(SelfAt) );
    if Socket_Pa.isValidNodeAddress( topic.DomainAddress ) then
      Self.Sender := createTCPServer( ip                  => topic.DomainAddress,
                                      port                => Integer(topic.Port),
                                      HeartbeatPeriod     => topic.HeartbeatPeriod,
                                      HeartbeatTimeout    => topic.HeartbeatTimeout,
                                      outSocketBufferSize => topic.OutSocketBufferSize );
    else
      Self.Sender := createTCPServer( ip                  => "0.0.0.0",
                                      port                => Integer(topic.Port),
                                      HeartbeatPeriod     => topic.HeartbeatPeriod,
                                      HeartbeatTimeout    => topic.HeartbeatTimeout,
                                      outSocketBufferSize => topic.OutSocketBufferSize );
    end if;
    Self.Sender.SetErrorService( Reporter );
    Self.Sender.SetConnectStatusClient( ConnectStatus_Interface_At(SelfAt) );
  end;

  overriding procedure Finalize( Self : in out TcpSendDataHandler_Class ) is
  begin
    declare
      S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.Mutex'Access);
    begin
      Free(Self.Sender);
    exception
      when others =>
        null;
    end;

    Finalize( SendDataHandler_Class(Self) );
  end;

  -- At least one publisher must be added to us for this call to work correct
  -- ie. sender must be opened for this to be correct
  overriding procedure updateTransportInfo( Self : in out TcpSendDataHandler_Class; topic : Topic_Class_At ) is
  begin
    -- Set port to the one actually used (for tcp server where OS defines port)
    topic.SetPort( Int32(Self.Sender.getPort) );
    if not Socket_Pa.isValidNodeAddress( topic.DomainAddress ) then
      topic.setDomainAddress(Socket_Pa.doSubnetTranslation(topic.LocalInterface));
    end if;
  end;

  overriding function sendData( Self : in out TcpSendDataHandler_Class; buf : Byte_Arr_At; bufSize : Integer; topic : Topic_Class_At) return Boolean is
  begin
    -- We don't "sendTo" but rather lets the server (sender) send to all conncted clients.
    return Self.Sender.sendTo(buf, bufSize, "", 0);
  end;

end Ops_Pa.Transport_Pa.SendDataHandler_Pa.TCP_Pa;

