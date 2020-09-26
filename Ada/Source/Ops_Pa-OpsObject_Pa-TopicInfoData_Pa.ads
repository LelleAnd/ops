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
use  Ops_Pa.ArchiverInOut_Pa;

with
  Ops_Pa.OpsObject_Pa.Topic_Pa;
use
  Ops_Pa.OpsObject_Pa.Topic_Pa;

package Ops_Pa.OpsObject_Pa.TopicInfoData_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type TopicInfoData_Class is new OpsObject_Class with
    record
      TopicInfoData_version : Byte := 0;
      name : String_At := null;
      dataType : String_At := null;
      transport : String_At := null;
      address : String_At := null;
      port : Int32 := 0;
      keys : String_Arr_At := null;

    end record;
  type TopicInfoData_Class_At is access all TopicInfoData_Class'Class;

  -- Help types used in case other idl's has vector's of the class
  type TopicInfoData_Class_Arr is array(Integer range <>) of aliased TopicInfoData_Class;
  type TopicInfoData_Class_Arr_At is access all TopicInfoData_Class_Arr;
  type TopicInfoData_Class_At_Arr is array(Integer range <>) of TopicInfoData_Class_At;
  type TopicInfoData_Class_At_Arr_At is access all TopicInfoData_Class_At_Arr;

  -- Constructors
  function Create return TopicInfoData_Class_At;
  function Create(top : Topic_Class_At) return TopicInfoData_Class_At;

  overriding procedure Serialize( Self : in out TopicInfoData_Class; archiver : ArchiverInOut_Class_At);

  -- Returns a newely allocated deep copy/clone of Self.
  overriding function Clone( Self : TopicInfoData_Class ) return OpsObject_Class_At;

  -- Fills the parameter obj with all values from Self.
  overriding procedure FillClone( Self : TopicInfoData_Class; obj : OpsObject_Class_At );

private
-- ==========================================================================
--
-- ==========================================================================
  procedure InitInstance( Self : in out TopicInfoData_Class );
  procedure InitInstance( Self : in out TopicInfoData_Class; top : Topic_Class_At );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out TopicInfoData_Class );

end Ops_Pa.OpsObject_Pa.TopicInfoData_Pa;

