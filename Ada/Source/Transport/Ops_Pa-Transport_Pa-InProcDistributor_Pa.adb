--
-- Copyright (C) 2024 Lennart Andersson.
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

package body Ops_Pa.Transport_Pa.InProcDistributor_Pa is

  use type MyMap_Pa.cursor;
  use type MyVector_Pa.Vector;
  use type MyVector_Pa.Cursor;

  function Equal( Left, Right : Listener_T ) return Boolean is
  begin
    return Left.Rcv = Right.Rcv and Left.Proc = Right.Proc;
  end;

  function Less (Left, Right : String) return Boolean is
  begin
    return Left < Right;
  end;

  function Equal (Left, Right : HandlerInfo) return Boolean is
  begin
    return Left.Vec = Right.Vec;
  end;

  function Create return InProcDistributor_Class_At is
     Self : InProcDistributor_Class_At := null;
  begin
    Self := new InProcDistributor_Class;
    InitInstance( Self.all, Self );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out InProcDistributor_Class; SelfAt : InProcDistributor_Class_At ) is
  begin
    Self.SelfAt := SelfAt;
  end;

  overriding procedure Finalize( Self : in out InProcDistributor_Class ) is
  begin
    null;
  end;

  procedure regInProcReceiver( Self : in out InProcDistributor_Class; Topic : Topic_Class_At; Rcv : Ops_Class_At; Proc : OnNotifyEvent_T ) is

    procedure Update(Key     : in String;
                     Element : in out HandlerInfo) is
    begin
      Element.Vec.Append(Listener_T'(Rcv => Rcv, Proc => Proc));
    end;

    pos : MyMap_Pa.Cursor;
  begin
    if Topic /= null and Rcv /= null and Proc /= null then
      pos := Self.Listeners.Find(Topic.Name);
      if pos /= MyMap_Pa.No_Element then
        -- Topic exists
        Self.Listeners.Update_Element(pos, Update'Access);
      else
        -- Topic is new
        declare
          info : HandlerInfo;
        begin
          info.Vec.Append(Listener_T'(Rcv => Rcv, Proc => Proc));
          Self.Listeners.Insert(Topic.Name, info);
        end;
      end if;
    end if;
  end;

  procedure unregInProcReceiver( Self : in out InProcDistributor_Class; Topic : Topic_Class_At; Rcv : Ops_Class_At; Proc : OnNotifyEvent_T ) is

    VecEmpty : Boolean := False;

    procedure Update(Key     : in String;
                     Element : in out HandlerInfo) is
      Cursor : MyVector_Pa.Cursor;
    begin
      Cursor := Element.Vec.Find(Listener_T'(Rcv => Rcv, Proc => Proc));
      if Cursor /= MyVector_Pa.No_Element then
        Element.Vec.Delete(Cursor);
      end if;
      VecEmpty := Element.Vec.Is_Empty;
    end;

    pos : MyMap_Pa.Cursor;
  begin
    if Topic /= null and Rcv /= null then
      pos := Self.Listeners.Find(Topic.Name);
      if pos /= MyMap_Pa.No_Element then
        -- Topic exists
        Self.Listeners.Update_Element(pos, Update'Access);
        -- If vector empty, remove topic
        if VecEmpty then
          Self.Listeners.Delete(pos);
        end if;
      end if;
    end if;
  end;

  function sendMessage( Self : in InProcDistributor_Class; topic : in Topic_Class_At; msg : in OPSMessage_Class_At) return Boolean is

    procedure SendTo(Element : in HandlerInfo) is

      procedure CallProc(Position : MyVector_Pa.Cursor) is
        listener : Listener_T := MyVector_Pa.Element(Position);
      begin
          -- procedure nnn( Sender : in InProcDistributor_Class_At; Rcv : Ops_Class_At; msg : OPSMessage_Class_At );
        listener.Proc.all(Self.SelfAt, listener.Rcv, msg);
      end;

    begin
      Element.Vec.Iterate(CallProc'Access);
    end;

    pos : MyMap_Pa.Cursor;
  begin
    if topic /= null and msg /= null then
      pos := Self.Listeners.Find(Topic.Name);
      if pos /= MyMap_Pa.No_Element then
        -- Topic exists
        SendTo(MyMap_Pa.Element(pos));
      end if;
    end if;
    return True;
  end;

end Ops_Pa.Transport_Pa.InProcDistributor_Pa;

