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

with Ops_Pa.ByteBuffer_Pa;
use Ops_Pa.ByteBuffer_Pa;
with Ops_Pa.SerializableFactory_Pa.CompFactory_Pa;
use Ops_Pa.SerializableFactory_Pa.CompFactory_Pa;

package Ops_Pa.ArchiverInOut_Pa.ArchiverIn_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type ArchiverIn_Class    is new ArchiverInOut_Class with private;
  type ArchiverIn_Class_At is access all ArchiverIn_Class'Class;

  -- Constructors
  function Create( buf : ByteBuffer_Class_At; fact : SerializableInheritingTypeFactory_Class_At ) return ArchiverIn_Class_At;

  overriding function IsOut( Self : in ArchiverIn_Class) return Boolean;

  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Boolean);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Byte);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int32);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int16);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int64);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Float32);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Float64);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out String_At);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Serializable_Class_At);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Serializable_Class_At; element : Integer);

  overriding function inout2( Self : in out ArchiverIn_Class; name : String; value : in out Serializable_Class_At) return Serializable_Class_At;

  overriding function inout( Self : in out ArchiverIn_Class; name : String; value : in out Serializable_Class_At; element : Integer) return Serializable_Class_At;

  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Boolean_Arr_At);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Byte_Arr_At);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int32_Arr_At);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int16_Arr_At);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int64_Arr_At);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Float32_Arr_At);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Float64_Arr_At);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out String_Arr_At);

  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Boolean_Arr);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Byte_Arr);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int32_Arr);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int16_Arr);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int64_Arr);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Float32_Arr);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Float64_Arr);
  overriding procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out String_Arr);

--  procedure inout( Self : in out ArchiverIn_Class; name : string; Value : in out Serializable_Class_Arr_At);

  overriding function beginList( Self : in out ArchiverIn_Class; name : String; size : Integer) return Integer;
  overriding procedure endList( Self : in out ArchiverIn_Class; name : String);

  -- Set this flag to enable trace from the ArchiverIn class
  TraceEnabled : Boolean := False;

private
-- ==========================================================================
--
-- ==========================================================================
  type ArchiverIn_Class is new ArchiverInOut_Class with
    record
      SelfAt : ArchiverIn_Class_At := null;
      FBuf : ByteBuffer_Class_At := null;
      Factory : SerializableInheritingTypeFactory_Class_At := null;
    end record;

  procedure InitInstance( Self : in out ArchiverIn_Class;
                          SelfAt : ArchiverIn_Class_At;
                          buf : ByteBuffer_Class_At;
                          fact : SerializableInheritingTypeFactory_Class_At);

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out ArchiverIn_Class );

end Ops_Pa.ArchiverInOut_Pa.ArchiverIn_Pa;
