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

with Ada.Containers.Vectors;

with Ops_Pa.OpsObject_Pa.Domain_Pa,
     Ops_Pa.OpsObject_Pa.Topic_Pa,
     Ops_Pa.Transport_Pa.ReceiveDataChannel_Pa,
     Ops_Pa.SerializableFactory_Pa.CompFactory_Pa,
     Ops_Pa.Error_Pa,
     Ops_Pa.OpsObject_Pa.OPSMessage_Pa,
     Ops_Pa.Notifier_Pa,
     Ops_Pa.Mutex_Pa;

use Ops_Pa.OpsObject_Pa.OPSMessage_Pa;

package Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type ReceiveDataHandler_Class is new Ops_Class and
    ReceiveDataChannel_Pa.MessageNotifier_Pa.Listener_Interface and
    ConnectStatus_Interface with
      private;
  type ReceiveDataHandler_Class_At is access all ReceiveDataHandler_Class'Class;

  procedure acquireMessageLock( Self : in out ReceiveDataHandler_Class );
  procedure releaseMessageLock( Self : in out ReceiveDataHandler_Class );

  -- Set the default capacity to handle many subscribers (may happen when many topics use the same transport and port).
  package MessageNotifier_Pa is new Ops_Pa.Notifier_Pa(100, Ops_Pa.OpsObject_Pa.OPSMessage_Pa.OPSMessage_Class_At);

  procedure addListener( Self : in out ReceiveDataHandler_Class;
                         Client : MessageNotifier_Pa.Listener_Interface_At;
                         top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At );
  procedure removeListener( Self : in out ReceiveDataHandler_Class;
                            Client : MessageNotifier_Pa.Listener_Interface_At;
                            top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At );

  procedure addListener( Self : in out ReceiveDataHandler_Class; Client : ConnectStatusNotifier_Pa.Listener_Interface_At );
  procedure removeListener( Self : in out ReceiveDataHandler_Class; Client : ConnectStatusNotifier_Pa.Listener_Interface_At );

  function getSampleMaxSize( Self : ReceiveDataHandler_Class ) return Int32;
  function getNumListeners( Self : ReceiveDataHandler_Class ) return Int32;

private
  function Equal( Left, Right : ReceiveDataChannel_Pa.ReceiveDataChannel_Class_At ) return Boolean;

  subtype MyIndex_T is Integer range 0..Integer'Last;
  package MyVector_Pa is new Ada.Containers.Vectors(MyIndex_T, ReceiveDataChannel_Pa.ReceiveDataChannel_Class_At, Equal);

-- ==========================================================================
--
-- ==========================================================================
  type ReceiveDataHandler_Class is new Ops_Class and
    ReceiveDataChannel_Pa.MessageNotifier_Pa.Listener_Interface and
    ConnectStatus_Interface with
    record
      SelfAt : ReceiveDataHandler_Class_At := null;

      -- Borrowed references
      ErrorService : Ops_Pa.Error_Pa.ErrorService_Class_At := null;

      -- Used for notifications to users of the ReceiveDataHandler
      DataNotifier : MessageNotifier_Pa.Notifier_Class_At := null;
      CsNotifier : ConnectStatusNotifier_Pa.Notifier_Class_At := null;

      -- The ReceiveDataChannels used for this ReceiveDataHandler
      Rdc : MyVector_Pa.Vector;

      SampleMaxSize : Int32 := 0;

      -- Current OPSMessage, valid until next sample arrives.
      Message : Ops_Pa.OpsObject_Pa.OPSMessage_Pa.OPSMessage_Class_At := null;

      --
      MessageLock : aliased Ops_Pa.Mutex_Pa.Mutex;
    end record;

  procedure AddRdc( Self : in out ReceiveDataHandler_Class; Rdc : ReceiveDataChannel_Pa.ReceiveDataChannel_Class_At );

  -- Tell derived classes which topics that are active
  procedure topicUsage( Self : in out ReceiveDataHandler_Class;
                        top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                        used : Boolean);

  -- Called whenever the RDC has a new message.
  procedure OnNotify( Self : in out ReceiveDataHandler_Class; Sender : in Ops_Class_At; Item : in OPSMessage_Class_At );

  procedure OnConnect( Self : in out ReceiveDataHandler_Class;
                       Sender : in Ops_Class_At;
                       Status : in ConnectStatus_T );

  procedure OnDisconnect( Self : in out ReceiveDataHandler_Class;
                          Sender : in Ops_Class_At;
                          Status : in ConnectStatus_T );

  procedure InitInstance( Self : in out ReceiveDataHandler_Class;
                          SelfAt : ReceiveDataHandler_Class_At;
                          top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                          dom : Ops_Pa.OpsObject_Pa.Domain_Pa.Domain_Class_At;
                          opsObjectFactory : Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.SerializableInheritingTypeFactory_Class_At;
                          Reporter : Ops_Pa.Error_Pa.ErrorService_Class_At;
                          Rdc : Ops_Pa.Transport_Pa.ReceiveDataChannel_Pa.ReceiveDataChannel_Class_At );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out ReceiveDataHandler_Class );

end Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa;

