--
-- Copyright (C) 2024-2025 Lennart Andersson.
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

with Ops_Pa.Transport_Pa.Sender_Pa;
use Ops_Pa.Transport_Pa.Sender_Pa;

package body Ops_Pa.Transport_Pa.SendDataHandler_Pa.InProc_Pa is

  -- ==========================================================================
  --      C l a s s    D e c l a r a t i o n.
  -- ==========================================================================
  type DummySender_Class    is new Sender_Class with null record;
  type DummySender_Class_At is access all DummySender_Class'Class;

  -- Constructs a new DummySender
  function CreateDummy return DummySender_Class_At;

  -- Interface used to send data
  overriding function sendTo( Self : in out DummySender_Class; buf : Byte_Arr_At; size : Integer; ip : string; port : Integer) return Boolean;

  overriding function getPort( Self : in out DummySender_Class ) return Integer;
  overriding function getAddress( Self : in out DummySender_Class ) return String;

  overriding procedure Open( Self : in out DummySender_Class );
  overriding procedure Close( Self : in out DummySender_Class );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out DummySender_Class );


  function CreateDummy return DummySender_Class_At is
     Self : DummySender_Class_At := null;
  begin
    Self := new DummySender_Class;
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  overriding procedure Finalize( Self : in out DummySender_Class ) is
  begin
    null;
  end;

  overriding procedure Open( Self : in out DummySender_Class ) is
  begin
    null;
  end;

  overriding procedure Close( Self : in out DummySender_Class ) is
  begin
    null;
  end;

  -- Interface used to send data
  overriding function sendTo( Self : in out DummySender_Class; buf : Byte_Arr_At; size : Integer; ip : string; port : Integer) return Boolean is
  begin
    return False;
  end;

  overriding function getPort( Self : in out DummySender_Class ) return Integer is
  begin
    return 0;
  end;

  overriding function getAddress( Self : in out DummySender_Class ) return String is
  begin
    return "0.0.0.0";
  end;

  -----------------------------------------------------------------------------
  -----------------------------------------------------------------------------

  function Create(topic : Topic_Class_At; distributor : InProcDistributor_Class_At ) return InProcSendDataHandler_Class_At is
     Self : InProcSendDataHandler_Class_At := null;
  begin
    Self := new InProcSendDataHandler_Class;
    InitInstance( Self.all, Self, topic, distributor );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  -- Should not be called
  overriding function sendData( Self : in out InProcSendDataHandler_Class; buf : Byte_Arr_At; bufSize : Integer; topic : Topic_Class_At) return Boolean is
  begin
    return False;
  end;

  -- Used for inprocess transport
  overriding function sendMessage( Self : in out InProcSendDataHandler_Class; topic : Topic_Class_At; message : OPSMessage_Class_At) return Boolean is
  begin
    return Self.distributor.sendMessage(topic, message);
  end;

  procedure InitInstance( Self : in out InProcSendDataHandler_Class;
                          SelfAt : InProcSendDataHandler_Class_At;
                          topic : Topic_Class_At;
                          distributor : InProcDistributor_Class_At ) is
  begin
    InitInstance( SendDataHandler_Class(Self), SendDataHandler_Class_At(SelfAt) );
    Self.Sender := Sender_Class_At(CreateDummy);
    Self.distributor := distributor;
  end;

  overriding procedure Finalize( Self : in out InProcSendDataHandler_Class ) is
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

end Ops_Pa.Transport_Pa.SendDataHandler_Pa.InProc_Pa;

