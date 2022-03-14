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

package body Ops_Pa.Notifier_Pa is

  use type MyVector_Pa.Cursor;

  --------------------------------------------------------------------------
  --
  --------------------------------------------------------------------------
  function Equal( Left, Right : Listener_T ) return Boolean is
  begin
    return Left.Proc = Right.Proc and 
      Left.Arg = Right.Arg and
      Left.ClassAt = Right.ClassAt;
  end;

  --------------------------------------------------------------------------
  --
  --------------------------------------------------------------------------
  function Create( Owner : Ops_Class_At ) return Notifier_Class_At is
    Self : Notifier_Class_At := null;
  begin
    Self := new Notifier_Class;
    InitInstance( Self.all, Owner );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  --------------------------------------------------------------------------
  -- Called by "owner" that wishes to notify its listeners.
  --------------------------------------------------------------------------
  procedure doNotify( Self : in out Notifier_Class; Item : in Item_T ) is
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.Mutex'Access);
  begin
    Self.Value := Item;
    Self.ValueValid := True;
    for i in Self.Listeners.First_Index .. Self.Listeners.Last_Index loop
      -- We don't allow a client to stop us from notify all clients
      begin
        if Self.Listeners.Element(i).ClassAt /= null then
          OnNotify( Self.Listeners.Element(i).ClassAt.all, Self.Owner, Item );
        else
          Self.Listeners.Element(i).Proc.all( Self.Owner, Item, Self.Listeners.Element(i).Arg );
        end if;
      exception
        when others =>
          null;  
      end;
    end loop;
  end;

  --------------------------------------------------------------------------
  -- Register a Listener for callback via a procedure call
  --------------------------------------------------------------------------
  procedure addListener( Self  : in out Notifier_Class; 
                         Proc  : in OnNotifyEvent_T;
                         Arg   : in Ops_Class_At ) is
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.Mutex'Access);
  begin
    if Proc /= null then
      Self.Listeners.Append(Listener_T'(Proc => Proc, Arg => Arg, ClassAt => null));
      if LateArrivals and then Self.ValueValid then
        Proc.all( Self.Owner, Self.Value, Arg );
      end if;
    end if;
  end;

  --------------------------------------------------------------------------
  --
  --------------------------------------------------------------------------
  procedure removeListener( Self  : in out Notifier_Class;
                            Proc  : in OnNotifyEvent_T;
                            Arg   : in Ops_Class_At ) is
    Cursor : MyVector_Pa.Cursor;
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.Mutex'Access);
  begin
    if Proc /= null then
      Cursor := Self.Listeners.Find(Listener_T'(Proc => Proc, Arg => Arg, ClassAt => null));
      if Cursor /= MyVector_Pa.No_Element then
        Self.Listeners.Delete(Cursor);
      end if;
    end if;
  end;

  --------------------------------------------------------------------------
  -- Register a Listener for callback using a "listener" class
  --------------------------------------------------------------------------
  procedure addListener( Self     : in out Notifier_Class; 
                         Listener : in Listener_Interface_At) is
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.Mutex'Access);
  begin
    if Listener /= null then
      Self.Listeners.Append(Listener_T'(Proc => null, Arg => null, ClassAt => Listener));
      if LateArrivals and then Self.ValueValid then
        OnNotify( Listener.all, Self.Owner, Self.Value );
      end if;
    end if;
  end;

  --------------------------------------------------------------------------
  --
  --------------------------------------------------------------------------
  procedure removeListener( Self     : in out Notifier_Class;
                            Listener : in Listener_Interface_At ) is
    Cursor : MyVector_Pa.Cursor;
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.Mutex'Access);
  begin
    if Listener /= null then
      Cursor := Self.Listeners.Find(Listener_T'(Proc => null, Arg => null, ClassAt => Listener));
      if Cursor /= MyVector_Pa.No_Element then
        Self.Listeners.Delete(Cursor);
      end if;
    end if;
  end;

  --------------------------------------------------------------------------
  --
  --------------------------------------------------------------------------
  function numListeners(Self : in out Notifier_Class) return Integer is
  begin
    return Integer(Self.Listeners.Length);
  end;

  --------------------------------------------------------------------------
  --
  --------------------------------------------------------------------------
  procedure InitInstance( Self : in out Notifier_Class; Owner : Ops_Class_At ) is
  begin
    Self.Owner := Owner;
    Self.Listeners.Reserve_Capacity(Ada.Containers.Count_Type(MinCapacity));
  end InitInstance;

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out Notifier_Class ) is
  begin
    null;
  end Finalize;

  --------------------------------------------------------------------------
  --
  --------------------------------------------------------------------------
  function Create( Owner : Ops_Class_At ) return SingleNotifier_Class_At is
    Self : SingleNotifier_Class_At := null;
  begin
    Self := new SingleNotifier_Class( Owner );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  --------------------------------------------------------------------------
  -- Called by "owner" that wishes to notify its connected listener
  --------------------------------------------------------------------------
  procedure doNotify( Self : in out SingleNotifier_Class; Item : in Item_T ) is
  begin
    if Self.Listener /= null then
      OnNotify( Self.Listener.all, Self.Owner, Item );
    end if;
  end;

  --------------------------------------------------------------------------
  -- Connect a Listener for callback using a "listener" class
  --------------------------------------------------------------------------
  procedure connectListener( Self     : in out SingleNotifier_Class;
                             Listener : in Listener_Interface_At ) is
  begin
    if Self.Listener /= null and Listener /= null then
      raise ApiViolation;
    end if;
    Self.Listener := Listener;
  end;

  --------------------------------------------------------------------------
  --
  --------------------------------------------------------------------------
  procedure disconnectListener( Self : in out SingleNotifier_Class ) is
  begin
    Self.Listener := null;
  end;

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out SingleNotifier_Class ) is
  begin
    null;
  end Finalize;

end Ops_Pa.Notifier_Pa;

