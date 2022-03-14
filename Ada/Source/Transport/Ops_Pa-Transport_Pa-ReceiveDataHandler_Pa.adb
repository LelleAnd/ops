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

with Ops_Pa.Error_Pa,
     Ops_Pa.OpsObject_Pa,
     Ops_Pa.OpsObject_Pa.OPSMessage_Pa;

use Ops_Pa.Error_Pa,
    Ops_Pa.OpsObject_Pa,
    Ops_Pa.OpsObject_Pa.OPSMessage_Pa;

package body Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa is

  use type ReceiveDataChannel_Pa.ReceiveDataChannel_Class_At;
  use type MyVector_Pa.Cursor;

  --------------------------------------------------------------------------
  --
  --------------------------------------------------------------------------
  function Equal( Left, Right : ReceiveDataChannel_Pa.ReceiveDataChannel_Class_At ) return Boolean is
  begin
    return Left = Right;
  end;

  procedure InitInstance( Self : in out ReceiveDataHandler_Class;
                          SelfAt : ReceiveDataHandler_Class_At;
                          top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                          dom : Ops_Pa.OpsObject_Pa.Domain_Pa.Domain_Class_At;
                          opsObjectFactory : Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.SerializableInheritingTypeFactory_Class_At;
                          Reporter : Ops_Pa.Error_Pa.ErrorService_Class_At;
                          Rdc : Ops_Pa.Transport_Pa.ReceiveDataChannel_Pa.ReceiveDataChannel_Class_At ) is
  begin
    Self.SelfAt := SelfAt;
    Self.ErrorService := Reporter;

    Self.DataNotifier := MessageNotifier_Pa.Create( Ops_Class_At(SelfAt) );
    Self.CsNotifier := ConnectStatusNotifier_Pa.Create( Ops_Class_At(SelfAt) );

    Self.Rdc.Reserve_Capacity( 10 );

    if Rdc /= null then
      Self.SampleMaxSize := Rdc.getSampleMaxSize;
      Self.AddRdc( Rdc );
    end if;
  end;

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out ReceiveDataHandler_Class ) is
  begin
    -- TODO Do we need to ensure Rdc's stopped?

    -- Free all objects in the list
    for i in Self.Rdc.First_Index .. Self.Rdc.Last_Index loop
      if Self.Rdc.Element(i) /= null then
        ReceiveDataChannel_Pa.Free( Self.Rdc.Element(i) );
      end if;
    end loop;
    Self.Rdc.Clear;

    ConnectStatusNotifier_Pa.Free(Self.CsNotifier);
    MessageNotifier_Pa.Free(Self.DataNotifier);

    -- Need to release the last message we received, if any.
    -- (We always keep a reference to the last message received)
    if Self.Message /= null then
      Self.Message.Unreserve;
    end if;
    Self.Message := null;
  end;

  procedure AddRdc( Self : in out ReceiveDataHandler_Class; Rdc : ReceiveDataChannel_Pa.ReceiveDataChannel_Class_At ) is
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.MessageLock'Access);
  begin
    Self.Rdc.Append( Rdc );
    Rdc.addListener( ReceiveDataChannel_Pa.MessageNotifier_Pa.Listener_Interface_At(Self.SelfAt) );
    Rdc.SetConnectStatusClient( ConnectStatus_Interface_At(Self.SelfAt) );

    if Self.DataNotifier.numListeners > 0 then
      Rdc.Start;
    end if;
  end;

  -- Tell derived classes which topics that are active
  procedure topicUsage( Self : in out ReceiveDataHandler_Class;
                        top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                        used : Boolean) is
  begin
    null;
  end;

  procedure acquireMessageLock( Self : in out ReceiveDataHandler_Class ) is
  begin
    Self.MessageLock.Acquire;
  end;

  procedure releaseMessageLock( Self : in out ReceiveDataHandler_Class ) is
  begin
    Self.MessageLock.Release;
  end;

  procedure addListener( Self : in out ReceiveDataHandler_Class;
                         Client : MessageNotifier_Pa.Listener_Interface_At;
                         top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At ) is
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.MessageLock'Access);
  begin
    Self.DataNotifier.addListener(Client);
    if Self.DataNotifier.numListeners = 1 then
      for i in Self.Rdc.First_Index .. Self.Rdc.Last_Index loop
        if Self.Rdc.Element(i) /= null then
          Self.Rdc.Element(i).Start;
        end if;
      end loop;
    end if;

    Self.SelfAt.topicUsage( top, True );
  end;

  procedure removeListener( Self : in out ReceiveDataHandler_Class;
                            Client : MessageNotifier_Pa.Listener_Interface_At;
                            top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At ) is
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.MessageLock'Access);
  begin
    Self.SelfAt.topicUsage( top, False );

    -- By taking the lock (above), we also make sure that the receiveDataHandler::onNewEvent() can't be in
    -- the callback of a listener
    Self.DataNotifier.removeListener(Client);
    if Self.DataNotifier.numListeners = 0 then
      for i in Self.Rdc.First_Index .. Self.Rdc.Last_Index loop
        if Self.Rdc.Element(i) /= null then
          Self.Rdc.Element(i).Stop;
        end if;
      end loop;
    end if;
  end;

  procedure addListener( Self : in out ReceiveDataHandler_Class; Client : ConnectStatusNotifier_Pa.Listener_Interface_At ) is
  begin
    Self.CsNotifier.addListener( Client );
  end;

  procedure removeListener( Self : in out ReceiveDataHandler_Class; Client : ConnectStatusNotifier_Pa.Listener_Interface_At ) is
  begin
    Self.CsNotifier.removeListener( Client );
  end;

  function getSampleMaxSize( Self : ReceiveDataHandler_Class ) return Int32 is
  begin
    return Self.SampleMaxSize;
  end;

  function getNumListeners( Self : ReceiveDataHandler_Class ) return Int32 is
  begin
    return Int32(Self.DataNotifier.numListeners);
  end;

  procedure OnConnect( Self : in out ReceiveDataHandler_Class;
                       Sender : in Ops_Class_At;
                       Status : in ConnectStatus_T ) is
  begin
    Self.CsNotifier.doNotify( Status );
  end;

  procedure OnDisconnect( Self : in out ReceiveDataHandler_Class;
                          Sender : in Ops_Class_At;
                          Status : in ConnectStatus_T ) is
  begin
    Self.CsNotifier.doNotify( Status );
  end;

  -- Called whenever the RDC has a new message.
  procedure OnNotify( Self : in out ReceiveDataHandler_Class; Sender : in Ops_Class_At; Item : in OPSMessage_Class_At ) is
    oldMessage : Ops_Pa.OpsObject_Pa.OPSMessage_Pa.OPSMessage_Class_At;
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.MessageLock'Access);
  begin
    oldMessage := Self.Message;
    Self.Message := Item;

    -- Increment ref count for message
    Self.Message.Reserve;

    -- Send it to Subscribers
    Self.DataNotifier.doNotify(Self.Message);

    -- This will delete the old message if no one has reserved it in the application layer.
    if oldMessage /= null then
      oldMessage.Unreserve;
    end if;
  end;

end Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa;

