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

with Ada.Containers.Vectors;
with Ada.Containers.Indefinite_Ordered_Maps;

with Ops_Pa.OpsObject_Pa.OPSMessage_Pa;
use  Ops_Pa.OpsObject_Pa.OPSMessage_Pa;

with Ops_Pa.OpsObject_Pa.Topic_Pa;
use Ops_Pa.OpsObject_Pa.Topic_Pa;

package Ops_Pa.Transport_Pa.InProcDistributor_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type InProcDistributor_Class    is new Ops_Class with private;
  type InProcDistributor_Class_At is access all InProcDistributor_Class'Class;

  function Create return InProcDistributor_Class_At;

  type OnNotifyEvent_T is access procedure( Sender : in InProcDistributor_Class_At; Rcv : Ops_Class_At; msg : OPSMessage_Class_At );

  procedure regInProcReceiver( Self : in out InProcDistributor_Class; Topic : Topic_Class_At; Rcv : Ops_Class_At; Proc : OnNotifyEvent_T );
  procedure unregInProcReceiver( Self : in out InProcDistributor_Class; Topic : Topic_Class_At; Rcv : Ops_Class_At; Proc : OnNotifyEvent_T );

  function sendMessage( Self : in InProcDistributor_Class; Topic : Topic_Class_At; msg : OPSMessage_Class_At) return Boolean;

private
  type Listener_T is
    record
      Rcv : Ops_Class_At := null;
      Proc : OnNotifyEvent_T := null;
    end record;

  function Equal( Left, Right : Listener_T ) return Boolean;

  subtype MyIndex_T is Integer range 0..Integer'Last;
  package MyVector_Pa is new Ada.Containers.Vectors(MyIndex_T, Listener_T, Equal);

  type HandlerInfo is
    record
      Vec : MyVector_Pa.Vector;
    end record;

  function Less (Left, Right : String) return Boolean;
  function Equal (Left, Right : HandlerInfo) return Boolean;

  package MyMap_Pa is new Ada.Containers.Indefinite_Ordered_Maps(String, HandlerInfo, Less, Equal);

-- ==========================================================================
--
-- ==========================================================================
  type InProcDistributor_Class is new Ops_Class with
    record
      SelfAt : InProcDistributor_Class_At;
      Listeners : MyMap_Pa.Map;
    end record;

  procedure InitInstance( Self: in out InProcDistributor_Class; SelfAt : InProcDistributor_Class_At );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out InProcDistributor_Class );

end Ops_Pa.Transport_Pa.InProcDistributor_Pa;

