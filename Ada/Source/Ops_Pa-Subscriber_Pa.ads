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

with Ada.Containers.Vectors,
     Ada.Containers.Synchronized_Queue_Interfaces,
     Ada.Containers.Unbounded_Synchronized_Queues,
     Ada.Finalization;

with Ops_Pa.Signal_Pa,
     Ops_Pa.Mutex_Pa,
     Ops_Pa.Notifier_Pa,
     Ops_Pa.DeadlineNotifier_Pa,
     Ops_Pa.OpsObject_Pa,
     Ops_Pa.OpsObject_Pa.OPSMessage_Pa,
     Ops_Pa.OpsObject_Pa.Topic_Pa,
     Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa,
     Ops_Pa.FilterQoSPolicy_Pa,
     Ops_Pa.Participant_Interface_Pa;

use Ops_Pa.OpsObject_Pa,
    Ops_Pa.OpsObject_Pa.OPSMessage_Pa,
    Ops_Pa.OpsObject_Pa.Topic_Pa,
    Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa,
    Ops_Pa.FilterQoSPolicy_Pa,
    Ops_Pa.Participant_Interface_Pa;

package Ops_Pa.Subscriber_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type Subscriber_Class is new Ops_Class and
    Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa.MessageNotifier_Pa.Listener_Interface and
    Ops_Pa.Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface with
      private;
  type Subscriber_Class_At is access all Subscriber_Class'Class;

  -- Subscriber just borrows the ref to the topic. Caller still owns it.
  -- Note that the topic need to exist while the subscriber exist.
  function Create( t : Topic_Class_At ) return Subscriber_Class_At;

  -- Starts communication.
  procedure Start( Self : in out Subscriber_Class );

  -- Stops communication, unsubscribe this subscriber from data.
  procedure Stop( Self : in out Subscriber_Class );

  -- Return connect status (currently only valid for TCP)
  function isConnected( Self : Subscriber_Class ) return Boolean;

  package MessageNotifier_Pa is new Ops_Pa.Notifier_Pa(10, OPSMessage_Class_At);

  -- Add notifications (callbacks) when data arrives
  -- NOTE: 'Proc' will be called in the context of an arbitrary thread
  procedure addListener( Self : in out Subscriber_Class; Proc : MessageNotifier_Pa.OnNotifyEvent_T; Arg : Ops_Class_At );
  procedure removeListener( Self : in out Subscriber_Class; Proc : MessageNotifier_Pa.OnNotifyEvent_T; Arg : Ops_Class_At );

  procedure addListener( Self : in out Subscriber_Class; Client : MessageNotifier_Pa.Listener_Interface_At );
  procedure removeListener( Self : in out Subscriber_Class; Client : MessageNotifier_Pa.Listener_Interface_At );

  procedure addListener( Self : in out Subscriber_Class; Client : Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface_At );
  procedure removeListener( Self : in out Subscriber_Class; Client : Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface_At );

  package Deadline_Pa is new Ops_Pa.DeadlineNotifier_Pa(10);

  -- Add notifications (callbacks) when a deadline is missed
  -- NOTE: 'Proc' will be called in the context of an arbitrary thread
  procedure addDeadlineListener( Self : in out Subscriber_Class; Proc : Deadline_Pa.OnNotifyEvent_T; Arg : Ops_Class_At );
  procedure addDeadlineListener( Self : in out Subscriber_Class; Client : Deadline_Pa.DeadlineListener_Interface_At );

  -- Add the given object and take ownership over it.
  procedure AddFilterQoSPolicy( Self : in out Subscriber_Class; fqos : FilterQoSPolicy_Class_At );

  -- Remove the given object from the list and ownership are returned to the caller.
  procedure RemoveFilterQoSPolicy( Self : in out Subscriber_Class; fqos : FilterQoSPolicy_Class_At );

  -- Waits for new data to arrive or timeout.
  -- Returns: true if new data (i.e. unread data) exist.
  function WaitForNewData( Self : in out Subscriber_Class; timeoutMS : Integer) return Boolean;

  -- Methods for handling the message lock.
  -- NOTE: Lock should be held while working with the message using a reference got either by
  --   protected member FData
  --   protected method getData()
  --   public method getMessage()
  --   public method getDataReference()
  --   public method getHistory()
  procedure acquireMessageLock( Self : in out Subscriber_Class );
  procedure releaseMessageLock( Self : in out Subscriber_Class );

  type Scope_MessageLock( Sub : Subscriber_Class_At ) is tagged limited private;

  -- Returns a reference to the latest received OPSMessage (including the latest data object).
  -- Clears the "new data" flag.
  -- NOTE: MessageLock should be held while working with the message, to prevent a
  -- new incomming message to delete the current one while in use.
  function getMessage( Self : in out Subscriber_Class ) return OPSMessage_Class_At;

  function isDeadlineMissed( Self : in out Subscriber_Class ) return Boolean;

  -- Returns an array with the last received OPS messages (0 to HistoryMaxSize messages).
  -- NOTE: MessageLock should be held while working with the messages, to prevent a
  -- new incomming message to delete an older one while in use.
--///TODO  function GetHistory : TArray<TOpsMessage>;

  procedure SetHistoryMaxSize( Self : in out Subscriber_Class; size : Integer);

  -- Returns a reference the latest received data object.
  -- Clears the "new data" flag.
  -- NOTE: MessageLock should be held while working with the data object, to prevent a
  -- new incomming message to delete the current one while in use.
  function getDataReference( Self : in out Subscriber_Class ) return OpsObject_Class_At;

  function Name( Self : Subscriber_Class ) return String;
  function Topic( Self : Subscriber_Class ) return Topic_Class_At;

  -- Checks if new data exist (same as 'waitForNewData(0)' but faster)
  -- Returns: true if new data (i.e. unread data) exist.
  function NewDataExist( Self : Subscriber_Class ) return Boolean;

  -- The deadline timout [ms] for this subscriber.
  -- If no message is received within deadline, listeners to deadlines will be notified
  -- == 0, --> Infinite wait
  procedure SetDeadlineQoS( Self : in out Subscriber_Class; millis : TimeMs_T );

  -- Sets the minimum time separation [ms] between to consecutive messages.
  -- Received messages in between will be ignored by this Subscriber
  -- == 0, no filtering on time
  procedure SetTimeBasedFilterQoS( Self : in out Subscriber_Class; TimeMs : Integer );

  -- Methods to store and retrieve user parameters.
  -- These are not used by OPS in any way, the usage is up to the user of OPS.
  procedure SetUserParam( Self : in out Subscriber_Class; Value : UInt64 );
  procedure SetUserRef( Self : in out Subscriber_Class; Value : Ops_Class_At );
  function GetUserParam( Self : Subscriber_Class ) return UInt64;
  function GetUserRef( Self : Subscriber_Class ) return Ops_Class_At;

private
-- ==========================================================================
--
-- ==========================================================================
  function Equal( Left, Right : FilterQoSPolicy_Class_At ) return Boolean;

  subtype MyIndex_T is Integer range 0..Integer'Last;
  package MyVector_Pa is new Ada.Containers.Vectors(MyIndex_T, FilterQoSPolicy_Class_At, Equal);

-- ==========================================================================
--
-- ==========================================================================
  package MyQI is new Ada.Containers.Synchronized_Queue_Interfaces(Element_Type => Ops_Pa.OpsObject_Pa.OPSMessage_Pa.OPSMessage_Class_At);
  package MyQ is new Ada.Containers.Unbounded_Synchronized_Queues(Queue_Interfaces => MyQI);

-- ==========================================================================
--
-- ==========================================================================
  type Subscriber_Class is new Ops_Class and
    Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa.MessageNotifier_Pa.Listener_Interface and
    Ops_Pa.Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface with
    record
      -- Name of this subscriber
      Name : String_At := null;

      -- The Topic this Subscriber subscribes to (NOTE: we don't own the object)
      Topic : Topic_Class_At := null;

      -- Subscriber started or not
      Started : Boolean := False;

      SelfAt : Subscriber_Class_At := null;

      -- Used for notifications to users of the subscriber
      DataNotifier : MessageNotifier_Pa.Notifier_Class_At := null;
      CsNotifier : Transport_Pa.ConnectStatusNotifier_Pa.Notifier_Class_At := null;

      -- Keep status for current ConnectStatus
      Connected : Boolean := False;

      -- Used for notifications to users of the subscriber
      DeadlineNotifier : Deadline_Pa.DeadlineNotifier_Class_At := null;

      -- The Participant to which this Subscriber belongs (NOTE: we don't own the object)
      Participant : Participant_Interface_At := null;

      -- ReceiveDataHandler delivering new data samples to this Subscriber (NOTE: we don't own the object)
      ReceiveDataHandler : ReceiveDataHandler_Class_At := null;

      -- Receiver side filters that will be applied to data from receiveDataHandler
      -- before delivery to application layer.
      FilterQoSPolicies : MyVector_Pa.Vector;
      FilterQoSPolicyMutex : aliased Ops_Pa.Mutex_Pa.Mutex;

      MessageBuffer : MyQ.Queue;
      MessageBufferMaxSize : Integer := 1;

      -- Used for wakeup of thread waiting for new data in 'WaitForNewData()'
      NewDataEvent : Ops_Pa.Signal_Pa.Signal_T;

      -- Time used for deadline missed checks
      TimeLastData : TimeMs_T := 0;
      DeadlineTimeout : TimeMs_T := 0;

      -- Time used for time based filter
      TimeLastDataForTimeBase : TimeMs_T := 0;
      TimeBaseMinSeparationTime : TimeMs_T := 0;

      Message : OPSMessage_Class_At := null;
      Data : OpsObject_Class_At := null;

--not used      FirstDataReceived : Boolean := False;
      HasUnreadData : Boolean := False;

      -- User variables
      UserParam : UInt64 := 0;
      UserRef : Ops_Class_At := null;
    end record;

  procedure AddToBuffer( Self : in out Subscriber_Class; mess : OPSMessage_Class_At);

  function ApplyFilterQoSPolicies( Self : in out Subscriber_Class; o : OpsObject_Class_At) return Boolean;

  function getData( Self : in out Subscriber_Class ) return OpsObject_Class_At;

  -- Message listener callback
  procedure OnNotify( Self : in out Subscriber_Class; Sender : in Ops_Class_At; Item : in OPSMessage_Class_At );

  procedure OnNotify( Self : in out Subscriber_Class; Sender : in Ops_Class_At; Item : in Transport_Pa.ConnectStatus_T );

  procedure InitInstance( Self : in out Subscriber_Class;
                          SelfAt : Subscriber_Class_At;
                          t : Topic_Class_At );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out Subscriber_Class );

-- ==========================================================================
--
-- ==========================================================================
  type Scope_MessageLock( Sub : Subscriber_Class_At ) is
    new Ada.Finalization.Limited_Controlled with null record;

  overriding procedure Initialize(Self : in out Scope_MessageLock);
  overriding procedure Finalize(Self : in out Scope_MessageLock);

end Ops_Pa.Subscriber_Pa;

