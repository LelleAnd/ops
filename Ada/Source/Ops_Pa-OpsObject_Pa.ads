--
-- Copyright (C) 2016 Lennart Andersson.
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

package Ops_Pa.OpsObject_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type OpsObject_Class    is new Serializable_Class with private;
  type OpsObject_Class_At is access all OpsObject_Class'Class;

  -- Constructors
  function Create return OpsObject_Class_At;

  procedure Serialize( Self : in out OpsObject_Class; archiver : ArchiverInOut_Class_At); 

  -- Getters/Setters
  function Key( Self : OpsObject_Class ) return String;
  procedure Key( Self : in out OpsObject_Class; Value : String );

  function TypesString( Self : OpsObject_Class ) return String;

  function SpareBytes( Self : OpsObject_Class ) return Byte_Arr_At;
  
  -- Returns a newely allocated deep copy/clone of this object.
  function Clone( Self : OpsObject_Class ) return OpsObject_Class_At; 

  -- Fills the parameter obj with all values from this object.
  procedure FillClone( Self : OpsObject_Class; obj : OpsObject_Class_At );

private
-- ==========================================================================
--
-- ==========================================================================
  type OpsObject_Class is new Serializable_Class with
    record
      Key         : String_At := null;
      TypesString : String_At := null;
      -- Bytes that hold unserialized data for this object.
      -- This happens if a type can not be fully understood by a participants type support.
      SpareBytes : Byte_Arr_At := null;
    end record;

  procedure AppendType( Self : in out OpsObject_Class; typ : String );

  -- Used from archivers
  procedure SetTypesString( Self : in out OpsObject_Class; types : String ); 

  procedure InitInstance( Self : in out OpsObject_Class );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  procedure Finalize( Self : in out OpsObject_Class );

end Ops_Pa.OpsObject_Pa;

