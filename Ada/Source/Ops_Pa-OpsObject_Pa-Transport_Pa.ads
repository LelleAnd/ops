--
-- Copyright (C) 2016-2020 Lennart Andersson.
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

with Ops_Pa.ArchiverInOut_Pa;
use Ops_Pa.ArchiverInOut_Pa;

package Ops_Pa.OpsObject_Pa.Transport_Pa is

  Transport_idlVersion : constant Byte := 0;

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type Transport_Class is new OpsObject_Class with
    record
      Transport_version : Byte := Transport_idlVersion;
      ChannelID : String_At := null;
      Topics : String_Arr_At := null;
    end record;
  type Transport_Class_At is access all Transport_Class'Class;

  -- Help types used in case other has vector's of the class
  type Transport_Class_Arr is array(Integer range <>) of aliased Transport_Class;
  type Transport_Class_Arr_At is access all Transport_Class_Arr;
  type Transport_Class_At_Arr is array(Integer range <>) of Transport_Class_At;
  type Transport_Class_At_Arr_At is access all Transport_Class_At_Arr;

  -- Constructors
  function Create return Transport_Class_At;

  overriding procedure Serialize( Self : in out Transport_Class; archiver : ArchiverInOut_Class_At);

  -- Returns a newely allocated deep copy/clone of this object.
  overriding function Clone( Self : Transport_Class ) return OpsObject_Class_At;

  -- Fills the parameter obj with all values from this object.
  overriding procedure FillClone( Self : Transport_Class; obj : OpsObject_Class_At );

private
-- ==========================================================================
--
-- ==========================================================================

  procedure InitInstance( Self : in out Transport_Class );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out Transport_Class );

end Ops_Pa.OpsObject_Pa.Transport_Pa;

