--
-- Copyright (C) 2020 Lennart Andersson.
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

package Ops_Pa.ArchiverInOut_Pa.ChecksumArchiver_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type Checksum_Calculator_Class    is abstract new Ops_Class with null record;
  type Checksum_Calculator_Class_At is access all Checksum_Calculator_Class'Class;

  procedure Calc( Self : in out Checksum_Calculator_Class; Bytes : Byte_Arr ) is abstract;


  -- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type ChecksumArchiver_Class    is new ArchiverInOut_Class with private;
  type ChecksumArchiver_Class_At is access all ChecksumArchiver_Class'Class;

  -- Constructors
  function Create(Calculator : Checksum_Calculator_Class_At) return ChecksumArchiver_Class_At;

  --
  overriding function IsOut( Self : in ChecksumArchiver_Class) return Boolean;

  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Boolean);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Byte);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int32);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int16);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int64);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Float32);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Float64);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out String_At);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Serializable_Class_At);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Serializable_Class_At; element : Integer);

  overriding function inout2( Self : in out ChecksumArchiver_Class; name : String; value : in out Serializable_Class_At) return Serializable_Class_At;

  overriding function inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Serializable_Class_At; element : Integer) return Serializable_Class_At;

  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Boolean_Arr_At);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Byte_Arr_At);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int32_Arr_At);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int16_Arr_At);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int64_Arr_At);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Float32_Arr_At);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Float64_Arr_At);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out String_Arr_At);

  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Boolean_Arr);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Byte_Arr);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int32_Arr);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int16_Arr);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int64_Arr);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Float32_Arr);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Float64_Arr);
  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out String_Arr);

--  procedure inout( Self : in out ChecksumArchiver_Class; name : string; Value : in out Serializable_Class_Arr_At);

  overriding function beginList( Self : in out ChecksumArchiver_Class; name : String; size : Integer) return Integer;
  overriding procedure endList( Self : in out ChecksumArchiver_Class; name : String);

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type Checksum_Calc_8bit_xor_Class    is new Checksum_Calculator_Class with
     record
       Sum : Byte := 0;
       TotalBytes : UInt32 := 0;
       TotalFields : UInt32 := 0;
     end record;
  type Checksum_Calc_8bit_xor_Class_At is access all Checksum_Calc_8bit_xor_Class'Class;

  function Create return Checksum_Calc_8bit_xor_Class_At;
  overriding procedure Calc( Self : in out Checksum_Calc_8bit_xor_Class; Bytes : Byte_Arr );


private
-- ==========================================================================
--
-- ==========================================================================
  type ChecksumArchiver_Class is new ArchiverInOut_Class with
    record
      SelfAt     : ChecksumArchiver_Class_At := null;
      Calculator : Checksum_Calculator_Class_At := null;
    end record;

  procedure InitInstance( Self : in out ChecksumArchiver_Class; SelfAt : ChecksumArchiver_Class_At; Calculator : Checksum_Calculator_Class_At );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out ChecksumArchiver_Class );


-- ==========================================================================
--
-- ==========================================================================
  procedure InitInstance( Self : in out Checksum_Calc_8bit_xor_Class );
  overriding procedure Finalize( Self : in out Checksum_Calc_8bit_xor_Class );

end Ops_Pa.ArchiverInOut_Pa.ChecksumArchiver_Pa;

