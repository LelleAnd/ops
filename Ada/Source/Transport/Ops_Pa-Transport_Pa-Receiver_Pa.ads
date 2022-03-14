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

with Ops_Pa.Signal_Pa;
with Ops_Pa.Notifier_Pa;
with Ops_Pa.OpsObject_Pa.Topic_Pa;
with Ops_Pa.OpsObject_Pa.Domain_Pa;

use Ops_Pa.OpsObject_Pa.Topic_Pa;
use Ops_Pa.OpsObject_Pa.Domain_Pa;

package Ops_Pa.Transport_Pa.Receiver_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type Receiver_Class    is abstract new Transport_Class with private;
  type Receiver_Class_At is access all Receiver_Class'Class;

  package ReceiverNotifier_Pa is new Notifier_Pa(1, BytesSizePair_T);

  -- Note: Only one listener is allowed
  procedure addListener( Self : in out Receiver_Class; Client : ReceiverNotifier_Pa.Listener_Interface_At );
  procedure removeListener( Self : in out Receiver_Class; Client : ReceiverNotifier_Pa.Listener_Interface_At );

  -- Start():
  -- Starts the receiver, and reads bytes into given buffer.
  -- When a message is read, a callback (notification) will be done with the
  -- buffer and actual number of bytes read.
  -- When the callback returns a new read is started to the current buffer
  function Start( Self: in out Receiver_Class; bytes : Byte_Arr_At; size : Integer) return Boolean is abstract;

  -- GetSource():
  -- Used to get the sender IP and port for a received message.
  -- Should only be called from the callback.
  function GetSourceIP( Self : in out Receiver_Class ) return String is abstract;
  function GetSourcePort( Self : in out Receiver_Class ) return Integer is abstract;

  -- SetReceiveBuffer():
  -- Changes the current buffer to use for reads.
  -- Should only be called from the callback.
  procedure SetReceiveBuffer( Self : in out Receiver_Class; bytes : Byte_Arr_At; size : Integer) is abstract;

  -- Stop():
  -- Aborts an ongoing read. NOTE: Must NOT be called from the callback.
  procedure Stop( Self : in out Receiver_Class ) is abstract;

  function Port( Self : Receiver_Class ) return Integer is abstract;
  function Address( Self : Receiver_Class ) return String is abstract;

  -- ========================================================================

  -- Receiver Factory (creates a receiver)
  function getReceiver(top : Topic_Class_At; dom : Domain_Class_At; Reporter : ErrorService_Class_At) return Receiver_Class_At;

private
-- ==========================================================================
--
-- ==========================================================================
  task type Receiver_Pr_T( Self : access Receiver_Class'Class ) is
    entry Start;
    entry Finish;
  end Receiver_Pr_T;

  TerminateEvent_C : constant Ops_Pa.Signal_Pa.Event_T := Ops_Pa.Signal_Pa.Event1_C;
  StartEvent_C     : constant Ops_Pa.Signal_Pa.Event_T := Ops_Pa.Signal_Pa.Event2_C;

-- ==========================================================================
--
-- ==========================================================================
  type Receiver_Class is abstract new Transport_Class with
    record
      -- Task that read data
      Receiver_Pr : Receiver_Pr_T(Receiver_Class'Access);

      -- Stop flag for Receiver_Pr task
      StopFlag : aliased Boolean := False;
      pragma volatile(StopFlag);
      TerminateFlag : aliased Boolean := False;
      pragma volatile(TerminateFlag);
      EventsToTask : Ops_Pa.Signal_Pa.Signal_T;

      -- Used for notifications to user of the Receiver
      DataNotifier : ReceiverNotifier_Pa.SingleNotifier_Class_At := null;
    end record;

  procedure Run( Self : in out Receiver_Class );

  procedure InitInstance( Self : in out Receiver_Class; SelfAt : Receiver_Class_At );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out Receiver_Class );

end Ops_Pa.Transport_Pa.Receiver_Pa;

