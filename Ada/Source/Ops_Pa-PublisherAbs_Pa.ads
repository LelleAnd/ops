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

with Ops_Pa.OpsObject_Pa;
with Ops_Pa.OpsObject_Pa.Topic_Pa;

use Ops_Pa.OpsObject_Pa;
use Ops_Pa.OpsObject_Pa.Topic_Pa;

package Ops_Pa.PublisherAbs_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type PublisherAbs_Class is abstract new Ops_Class with private;
  type PublisherAbs_Class_At is access all PublisherAbs_Class'Class;

  procedure Start( Self : in out PublisherAbs_Class ) is abstract;
  procedure Stop( Self : in out PublisherAbs_Class ) is abstract;

  procedure WriteOPSObject( Self : in out PublisherAbs_Class; obj : OpsObject_Class_At) is abstract;

  -- Getters/Setters
  function Name( Self : PublisherAbs_Class ) return String;
  procedure SetName( Self : in out PublisherAbs_Class; Value : String );

  function Key( Self : PublisherAbs_Class ) return String;
  procedure SetKey( Self : in out PublisherAbs_Class; Value : String );

  function Topic ( Self : PublisherAbs_Class ) return Topic_Class_At;

  -- Send behavior parameters
  -- time in [ms]
  function SendSleepTime( Self : PublisherAbs_Class ) return Int64;
  procedure SetSendSleepTime( Self : in out PublisherAbs_Class; Value : Int64 );

  function SleepEverySendPacket( Self : PublisherAbs_Class ) return Int32;
  procedure SetSleepEverySendPacket( Self : in out PublisherAbs_Class; Value : Int32 );

  function SleepOnSendFailed( Self : PublisherAbs_Class ) return Boolean;
  procedure SetSleepOnSendFailed( Self : in out PublisherAbs_Class; Value : Boolean );

private
-- ==========================================================================
--
-- ==========================================================================
  type PublisherAbs_Class is abstract new Ops_Class with
    record
      Name : String_At := null;
      Key : String_At := null;

      -- Send behavior parameters
      SendSleepTime : Int64 := 1;
      SleepEverySendPacket : Int32 := 100000;
      SleepOnSendFailed : Boolean := True;

      -- The Topic this Publisher publish on (NOTE: we don't own the object)
      Topic : Topic_Class_At := null;
    end record;

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out PublisherAbs_Class );

end Ops_Pa.PublisherAbs_Pa;

