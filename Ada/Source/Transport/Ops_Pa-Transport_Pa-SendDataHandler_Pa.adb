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

package body Ops_Pa.Transport_Pa.SendDataHandler_Pa is

  use type Ada.Containers.Count_Type;

  procedure addUser( Self : in out SendDataHandler_Class; client : Ops_Class_At ) is
    Idx : MyVector_Pa.Extended_Index;
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.Mutex'Access);
  begin
    -- Check that it isn't already in the list
    Idx := Self.Users.Find_Index(client);
    if Idx /= MyVector_Pa.No_Index then
      return;
    end if;

    -- Save client in the list
    Self.Users.Append(client);

    -- For the first client, we open the sender
    if Self.Users.Length = 1 then
      Self.Sender.Open;
    end if;
  end;

  procedure removeUser( Self : in out SendDataHandler_Class; client : Ops_Class_At ) is
    Idx : MyVector_Pa.Extended_Index;
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.Mutex'Access);
  begin
    Idx := Self.Users.Find_Index(client);
    if Idx /= MyVector_Pa.No_Index then
      -- Remove from list without freeing object
      Self.Users.Delete(Idx);
    end if;

    -- For the last client, we close the sender
    if Self.Users.Length = 0 then
      Self.Sender.Close;
    end if;
  end;

  -- At least one publisher must be added to us for this call to work correct
  -- Updates topic with the used transport info
  procedure updateTransportInfo( Self : in out SendDataHandler_Class; topic : Topic_Class_At ) is
  begin
    null;
  end;

  function Equal( Left, Right : Ops_Class_At ) return Boolean is
  begin
    return Left = Right;
  end;

  procedure InitInstance( Self : in out SendDataHandler_Class;
                          SelfAt : in SendDataHandler_Class_At ) is
  begin
    Self.SelfAt := SelfAt;
    Self.CsNotifier := ConnectStatusNotifier_Pa.Create( Ops_Class_At(SelfAt) );
  end;

  overriding procedure Finalize( Self : in out SendDataHandler_Class ) is
  begin
    ConnectStatusNotifier_Pa.Free(Self.CsNotifier);
  end;

  procedure addListener( Self : in out SendDataHandler_Class; Client : ConnectStatusNotifier_Pa.Listener_Interface_At ) is
  begin
    Self.CsNotifier.addListener( Client );
  end;

  procedure removeListener( Self : in out SendDataHandler_Class; Client : ConnectStatusNotifier_Pa.Listener_Interface_At ) is
  begin
    Self.CsNotifier.removeListener( Client );
  end;

  procedure OnConnect( Self : in out SendDataHandler_Class;
                       Sender : in Ops_Class_At;
                       Status : in ConnectStatus_T ) is
  begin
    Self.CsNotifier.doNotify( Status );
  end;

  procedure OnDisconnect( Self : in out SendDataHandler_Class;
                          Sender : in Ops_Class_At;
                          Status : in ConnectStatus_T ) is
  begin
    Self.CsNotifier.doNotify( Status );
  end;

end Ops_Pa.Transport_Pa.SendDataHandler_Pa;

