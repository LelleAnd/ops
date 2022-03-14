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

package body Ops_Pa.PublisherAbs_Pa is

  function Name( Self : PublisherAbs_Class ) return String is
  begin
    if Self.Name /= null then
      return Self.Name.all;
    else
      return "";
    end if;
  end;

  procedure SetName( Self : in out PublisherAbs_Class; Value : String ) is
  begin
    Replace(Self.Name, Value);
  end;

  function Key( Self : PublisherAbs_Class ) return String is
  begin
    if Self.Key /= null then
      return Self.Key.all;
    else
      return "";
    end if;
  end;

  procedure SetKey( Self : in out PublisherAbs_Class; Value : String ) is
  begin
    Replace(Self.Key, Value);
  end;

  function Topic ( Self : PublisherAbs_Class ) return Topic_Class_At is
  begin
    return Self.Topic;
  end;

  -- Send behavior parameters
  function SendSleepTime( Self : PublisherAbs_Class ) return Int64 is
  begin
    return Self.SendSleepTime;
  end;

  procedure SetSendSleepTime( Self : in out PublisherAbs_Class; Value : Int64 ) is
  begin
    Self.SendSleepTime := Value;
  end;

  function SleepEverySendPacket( Self : PublisherAbs_Class ) return Int32 is
  begin
    return Self.SleepEverySendPacket;
  end;

  procedure SetSleepEverySendPacket( Self : in out PublisherAbs_Class; Value : Int32 ) is
  begin
    Self.SleepEverySendPacket := Value;
  end;

  function SleepOnSendFailed( Self : PublisherAbs_Class ) return Boolean is
  begin
    return Self.SleepOnSendFailed;
  end;

  procedure SetSleepOnSendFailed( Self : in out PublisherAbs_Class; Value : Boolean ) is
  begin
    Self.SleepOnSendFailed := Value;
  end;

  overriding procedure Finalize( Self : in out PublisherAbs_Class ) is
  begin
    Dispose(Self.Name);
    Dispose(Self.Key);
  end;

end Ops_Pa.PublisherAbs_Pa;

