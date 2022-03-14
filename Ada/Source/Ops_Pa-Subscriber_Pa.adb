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

with Ops_pa.Error_Pa,
     Ops_Pa.Participant_Pa;

use Ops_Pa.Error_Pa;

package body Ops_Pa.Subscriber_Pa is

  use type Ada.Containers.Count_Type;

  function Create( t : Topic_Class_At ) return Subscriber_Class_At is
     Self : Subscriber_Class_At := null;
  begin
    Self := new Subscriber_Class;
    InitInstance( Self.all, Self, t );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  procedure InitInstance( Self : in out Subscriber_Class;
                          SelfAt : Subscriber_Class_At;
                          t : Topic_Class_At ) is
  begin
    Self.SelfAt := SelfAt;
    Self.Topic := t;

    Self.DataNotifier := MessageNotifier_Pa.Create( Ops_Class_At(SelfAt) );
    Self.CsNotifier := Transport_Pa.ConnectStatusNotifier_Pa.Create( Ops_Class_At(SelfAt) );
    Self.DeadlineNotifier := Deadline_Pa.Create( Ops_Class_At(SelfAt), Periodic => True );

    Self.Participant := Participant_Interface_At(Ops_Pa.Participant_Pa.getInstance(t.DomainID, t.ParticipantID));
    Self.TimeLastData := GetTimeInMs;
  end;

  overriding procedure Finalize( Self : in out Subscriber_Class ) is
    o : OPSMessage_Class_At;
  begin
    Self.DeadlineNotifier.Cancel;
    if Self.Started then
      Stop( Self );
    end if;

    -- Unreserve all buffered messages
    while Self.MessageBuffer.Current_Use > 0 loop
      Self.MessageBuffer.Dequeue(Element => o);
      o.UnReserve;
    end loop;

    -- Remove the filters we still own
    declare
      S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.FilterQoSPolicyMutex'Access);
    begin
      for i in Self.FilterQoSPolicies.First_Index .. Self.FilterQoSPolicies.Last_Index loop
        if Self.FilterQoSPolicies.Element(i) /= null then
          Free(Self.FilterQoSPolicies.Element(i));
        end if;
      end loop;
    end;

    Deadline_Pa.Free(Self.DeadlineNotifier);
    Transport_Pa.ConnectStatusNotifier_Pa.Free(Self.CsNotifier);
    MessageNotifier_Pa.Free(Self.DataNotifier);
  end;

  -- ---------------------------------------------------------------------------

  procedure SetUserParam( Self : in out Subscriber_Class; Value : UInt64 ) is
  begin
    Self.UserParam := Value;
  end;

  procedure SetUserRef( Self : in out Subscriber_Class; Value : Ops_Class_At ) is
  begin
    Self.UserRef := Value;
  end;

  function GetUserParam( Self : Subscriber_Class ) return UInt64 is
  begin
    return Self.UserParam;
  end;

  function GetUserRef( Self : Subscriber_Class ) return Ops_Class_At is
  begin
    return Self.UserRef;
  end;

  -- ---------------------------------------------------------------------------

  function Name( Self : Subscriber_Class ) return String is
  begin
    return Self.Name.all;
  end;

  function Topic( Self : Subscriber_Class ) return Topic_Class_At is
  begin
    return Self.Topic;
  end;

  procedure Start( Self : in out Subscriber_Class ) is
  begin
    if not Self.Started then
      Self.ReceiveDataHandler := Self.Participant.GetReceiveDataHandler(Self.Topic);
      Self.ReceiveDataHandler.addListener(Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa.MessageNotifier_Pa.Listener_Interface_At(Self.SelfAt), Self.Topic);
      Self.ReceiveDataHandler.addListener(Ops_Pa.Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface_At(Self.SelfAt));

      if Self.DeadlineTimeout > 0 then
        Self.DeadlineNotifier.Start( Self.DeadlineTimeout );
      end if;
      Self.Started := True;
    end if;
  end;

  procedure Stop( Self : in out Subscriber_Class ) is
    dummy : Boolean;
  begin
    if Self.Started then

      if Self.DeadlineTimeout > 0 then
        Self.DeadlineNotifier.Cancel;
      end if;

      Self.ReceiveDataHandler.removeListener(Ops_Pa.Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface_At(Self.SelfAt));
      Self.ReceiveDataHandler.removeListener(Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa.MessageNotifier_Pa.Listener_Interface_At(Self.SelfAt), Self.Topic);
      Self.ReceiveDataHandler := null;
      Self.Participant.ReleaseReceiveDataHandler(Self.Topic);

      Self.Connected := False;
      Self.Started := False;
    end if;
  end;

  -- Return connect status (currently only valid for TCP)
  function isConnected( Self : Subscriber_Class ) return Boolean is
  begin
    return Self.Connected;
  end;

  -- ---------------------------------------------------------------------------

  function getMessage( Self : in out Subscriber_Class ) return OPSMessage_Class_At is
  begin
    Self.HasUnreadData := False;
    return Self.Message;
  end;

  function getDataReference( Self : in out Subscriber_Class ) return OpsObject_Class_At is
  begin
    Self.HasUnreadData := False;
    return Self.Data;
  end;

  function getData( Self : in out Subscriber_Class ) return OpsObject_Class_At is
  begin
    Self.HasUnreadData := False;
    return Self.Data;
  end;

  -- ---------------------------------------------------------------------------
  -- Methods need protection by e.g. a Mutex, since Apply....() will be called by the receiving thread

  function Equal(Left, Right : FilterQoSPolicy_Class_At) return Boolean is
  begin
    return Left = Right;
  end;

  procedure AddFilterQoSPolicy( Self : in out Subscriber_Class; fqos : FilterQoSPolicy_Class_At ) is
  begin
    if fqos /= null then
      declare
        S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.FilterQoSPolicyMutex'Access);
      begin
        Self.FilterQoSPolicies.Append(fqos);
      end;
    end if;
  end;

  procedure RemoveFilterQoSPolicy( Self : in out Subscriber_Class; fqos : FilterQoSPolicy_Class_At ) is
    Idx : MyVector_Pa.Extended_Index;
  begin
    if fqos /= null then
      declare
        S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.FilterQoSPolicyMutex'Access);
      begin
        Idx := Self.FilterQoSPolicies.Find_Index(fqos);
        if Idx /= MyVector_Pa.No_Index then
          -- Remove from list without freeing object
          Self.FilterQoSPolicies.Delete(Idx);
        end if;
      end;
    end if;
  end;

  function ApplyFilterQoSPolicies( Self : in out Subscriber_Class; o : OpsObject_Class_At ) return Boolean is
    fqos : FilterQoSPolicy_Class_At;
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.FilterQoSPolicyMutex'Access);
  begin
    for i in Self.FilterQoSPolicies.First_Index .. Self.FilterQoSPolicies.Last_Index loop
      fqos := Self.FilterQoSPolicies.Element(i);
      if not fqos.ApplyFilter(o) then
        return False;
      end if;
    end loop;
    return True;
  end;

  -- ---------------------------------------------------------------------------

  procedure SetHistoryMaxSize( Self : in out Subscriber_Class; size : Integer) is
  begin
    Self.MessageBufferMaxSize := size;
  end;

  procedure AddToBuffer( Self : in out Subscriber_Class; mess : OPSMessage_Class_At) is
    o : OPSMessage_Class_At;
  begin
    mess.Reserve;
    Self.MessageBuffer.Enqueue(mess);
    while Integer(Self.MessageBuffer.Current_Use) > Self.MessageBufferMaxSize loop
      Self.MessageBuffer.Dequeue(Element => o);
      o.UnReserve;
    end loop;
  end;

--  function TSubscriber.getHistory : TArray<TOpsMessage> is
--  begin
--    Result := FMessageBuffer.ToArray;
--  end;

  -- ---------------------------------------------------------------------------

  procedure addListener( Self : in out Subscriber_Class; Proc : MessageNotifier_Pa.OnNotifyEvent_T; Arg : Ops_Class_At ) is
  begin
    Self.DataNotifier.addListener(Proc, Arg);
  end;

  procedure removeListener( Self : in out Subscriber_Class; Proc : MessageNotifier_Pa.OnNotifyEvent_T; Arg : Ops_Class_At ) is
  begin
    Self.DataNotifier.removeListener(Proc, Arg);
  end;

  procedure addListener( Self : in out Subscriber_Class; Client : MessageNotifier_Pa.Listener_Interface_At ) is
  begin
    Self.DataNotifier.addListener(Client);
  end;

  procedure removeListener( Self : in out Subscriber_Class; Client : MessageNotifier_Pa.Listener_Interface_At ) is
  begin
    Self.DataNotifier.removeListener(Client);
  end;

  procedure addListener( Self : in out Subscriber_Class; Client : Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface_At ) is
  begin
    Self.CsNotifier.addListener( Client );
  end;

  procedure removeListener( Self : in out Subscriber_Class; Client : Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface_At ) is
  begin
    Self.CsNotifier.removeListener( Client );
  end;

  procedure acquireMessageLock( Self : in out Subscriber_Class ) is
  begin
    Self.ReceiveDataHandler.acquireMessageLock;
  end;

  procedure releaseMessageLock( Self : in out Subscriber_Class ) is
  begin
    Self.ReceiveDataHandler.releaseMessageLock;
  end;

  procedure OnNotify( Self : in out Subscriber_Class; Sender : in Ops_Class_At; Item : in Transport_Pa.ConnectStatus_T ) is
  begin
    Self.Connected := Item.Connected;
    Self.CsNotifier.doNotify( Item );
  end;

  -- Message listener callback (called from receive thread)
  -- Called with receiveDataHandler MessageLock taken
  procedure OnNotify( Self : in out Subscriber_Class; Sender : in Ops_Class_At; Item : in OPSMessage_Class_At ) is
    o : OpsObject_Class_At;
  begin
    -- Perform a number of checks on incomming data to be sure we want to deliver it to the application layer

    -- Check that this message is delivered on the same topic as this Subscriber use
    if Item.TopicName /= Self.Topic.Name then
      -- This is a normal case when several Topics use the same port
      return;

    -- Check that the type of the delivered data can be interpreted as the type we expect in this Subscriber
    elsif Ada.Strings.Fixed.Index(Item.Data.TypesString, Self.Topic.TypeID) = 0 then
      Self.Participant.ReportError(Error_Class_At(BasicError(
          "Subscriber", "onNewMessage", "Received message with wrong data type for Topic")));
      return;
    end if;

    -- OK, we passed the basic checks, lets go on and filter on data content...

    o := Item.Data;
    if applyFilterQoSPolicies(Self, o) then
      -- Apply timebased filter (if any)
      if (Self.TimeBaseMinSeparationTime = 0) or
         ((GetTimeInMs - Self.TimeLastDataForTimeBase) > Self.TimeBaseMinSeparationTime)
      then
-- not used        Self.FirstDataReceived := True;

        AddToBuffer(Self, Item);
        Self.Message := Item;
        Self.Data := o;

        Self.DataNotifier.doNotify(Item);

        Self.HasUnreadData := True;
        Self.NewDataEvent.Signal( Ops_Pa.Signal_Pa.Event1_C );

        Self.TimeLastDataForTimeBase := GetTimeInMs;
        Self.TimeLastData := Self.TimeLastDataForTimeBase;

        if Self.DeadlineTimeout > 0 then
          Self.DeadlineNotifier.Start( Self.DeadlineTimeout );
        end if;
      end if;
    end if;
  end;

  -- Waits for new data to arrive or timeout.
  -- Returns: true if new data (i.e. unread data) exist.
  function WaitForNewData( Self : in out Subscriber_Class; timeoutMS : Integer) return Boolean is
    Events : Ops_Pa.Signal_Pa.Event_T;
  begin
    Self.NewDataEvent.Get( Events );  -- Clear events

    if Self.HasUnreadData then
      return True;
    end if;

    select
      Self.NewDataEvent.WaitForAny( Events );
      return True;
    or
      delay (1.0 * timeoutMS) / 1000.0;
    end select;
    return False;
  end;

  function NewDataExist( Self : Subscriber_Class ) return Boolean is
  begin
    return Self.HasUnreadData;
  end;

  procedure SetTimeBasedFilterQoS( Self : in out Subscriber_Class; TimeMs : Integer ) is
  begin
    Self.TimeBaseMinSeparationTime := TimeMs_T(TimeMs);
  end;

  -- ---------------------------------------------------------------------------

  procedure addDeadlineListener( Self : in out Subscriber_Class; Proc : Deadline_Pa.OnNotifyEvent_T; Arg : Ops_Class_At ) is
  begin
    Self.DeadlineNotifier.addListener( Proc, Arg );
  end;

  procedure addDeadlineListener( Self : in out Subscriber_Class; Client : Deadline_Pa.DeadlineListener_Interface_At ) is
  begin
    Self.DeadlineNotifier.addListener( Client );
  end;

  procedure SetDeadlineQoS( Self : in out Subscriber_Class; millis : TimeMs_T) is
  begin
    if millis <= 0 then
      Self.DeadlineTimeout := 0;
      Self.DeadlineNotifier.Cancel;
    else
      Self.DeadlineTimeout := millis;
      Self.DeadlineNotifier.Start( Self.DeadlineTimeout );
    end if;
  end;

  function isDeadlineMissed( Self : in out Subscriber_Class ) return Boolean is
  begin
    if Self.DeadlineTimeout > 0 then
      if (GetTimeInMs - Self.TimeLastData) > Self.DeadlineTimeout then
        return True;
      end if;
    end if;
    return False;
  end;

  procedure Initialize(Self : in out Scope_MessageLock) is
  begin
    Self.Sub.acquireMessageLock;
  end;

  procedure Finalize(Self : in out Scope_MessageLock) is
  begin
    Self.Sub.releaseMessageLock;
  end;

end Ops_Pa.Subscriber_Pa;

