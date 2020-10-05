--
-- Copyright (C) 2016-2018 Lennart Andersson.
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

with System,
     System.Atomic_Counters,
     Ada.Unchecked_Deallocation,
     Ada.Exceptions,
     Ada.Streams,
     Ada.Finalization,
     Interfaces;

use  System;

package Ops_Pa is

-- ===========================================================================
--          C l a s s   D e c l a r a t i o n
-- ===========================================================================
  -- Base Class for all OPS classes
  type Ops_Class    is abstract new Ada.Finalization.Limited_Controlled with null record;
  type Ops_Class_At is access all Ops_Class'Class;

  -- Get class name for object (May be overridden to shorten class name)
  function ClassName(Self : Ops_Class) return String;

  -- Destructor ( There is no need to override this method anytime, use
  --              Finalize() instead to dealloc memory )
  procedure Free(Self : access Ops_Class);

  -- Destructor
  overriding procedure Finalize(Self : in out Ops_Class) is abstract;

  -- Initialize object (Only used to trace allocation of object)
  overriding procedure Initialize(Self : in out Ops_Class);

  ----------------------------------------------------------------------
  -- Install/Uninstall trace routine to catch allocation/deallocation
  -- of objects, when running.
  type CreateStatus_T is (Alloc, Dealloc);
  type TraceRoutine_At is access procedure( Class         : String;
                                            CreateStatus  : CreateStatus_T;
                                            TotalAllocObj : System.Atomic_Counters.Atomic_Unsigned);
  procedure InstallTrace(   Routine : TraceRoutine_At);
  procedure UnInstallTrace( Routine : TraceRoutine_At);

  -- Debug
  function NumActiveObjects return System.Atomic_Counters.Atomic_Unsigned;

  -- Install generic trace
  type TraceProc_At is access procedure( NameStr : String; ValueStr : String );
  procedure InstallTrace( Proc : TraceProc_At );

  ----------------------------------------------------------------------

  -- Extra parameter in notification callbacks using a procedure call
  subtype NotifyParam_T is Ops_Class_At;

  -- Types
  subtype Byte    is Ada.Streams.Stream_Element;
  subtype UInt8   is Interfaces.Unsigned_8;
  subtype UInt16  is Interfaces.Unsigned_16;
  subtype UInt32  is Interfaces.Unsigned_32;
  subtype UInt64  is Interfaces.Unsigned_64;

  subtype Int8    is Interfaces.Integer_8;
  subtype Int16   is Interfaces.Integer_16;
  subtype Int32   is Interfaces.Integer_32;
  subtype Int64   is Interfaces.Integer_64;

  subtype Float32 is Interfaces.IEEE_Float_32;
  subtype Float64 is Interfaces.IEEE_Float_64;

  type String_At  is access all String;

  -- Types used for implementing OPS static and dynamic arrays
  type Boolean_Arr is array(Integer range <>) of Boolean;
  type Boolean_Arr_At is access all Boolean_Arr;
  type Int16_Arr is array(Integer range <>) of Int16;
  type Int16_arr_At is access all Int16_Arr;
  type Int32_Arr is array(Integer range <>) of Int32;
  type Int32_Arr_At is access all Int32_Arr;
  type Int64_Arr is array(Integer range <>) of Int64;
  type Int64_Arr_At is access all Int64_Arr;
  type Float32_Arr is array(Integer range <>) of Float32;
  type Float32_Arr_At is access all Float32_Arr;
  type Float64_Arr is array(Integer range <>) of Float64;
  type Float64_Arr_At is access all Float64_Arr;
  type String_Arr is array(Integer range <>) of String_At;
  type String_Arr_At is access all String_Arr;

  subtype Byte_Arr is Ada.Streams.Stream_Element_Array;
  type Byte_Arr_At is access all Byte_Arr;
  subtype Byte_Arr_Index_T is Ada.Streams.Stream_Element_Offset;

  -- Access to operations on types
  use type Interfaces.Unsigned_8;
  use type Interfaces.Unsigned_16;
  use type Interfaces.Unsigned_32;
  use type Interfaces.Unsigned_64;

  use type Interfaces.Integer_8;
  use type Interfaces.Integer_16;
  use type Interfaces.Integer_32;
  use type Interfaces.Integer_64;

  use type Interfaces.IEEE_Float_32;
  use type Interfaces.IEEE_Float_64;

  use type Ada.Streams.Stream_Element_Offset;
  use type Ada.Streams.Stream_Element;
  use type Ada.Streams.Stream_Element_Array;

  -- Constants
  PACKET_MAX_SIZE : constant := 60000;
  MAX_DEADLINE_TIMEOUT : constant := Int64'Last;
--  Is_Network_Byte_Order : constant Boolean := System.Default_Bit_Order = System.High_Order_First;

  -- Exceptions
  Not_Yet_Implemented : exception;

  EIdlVersionException : exception;
  EConfigException : exception;
  ENoSuchTopicException : exception;
  ECommException : exception;
  --  EReceiveTimedOutException : Exception;
  Illegal_Array_Length : exception;
  Null_Object_In_Array : exception;
  Null_Object_Not_Allowed : exception;

  -- String helpers
  function Copy( Str : String ) return String_At;
  function Copy( Str : String_At ) return String_At;
  procedure Dispose is new Ada.Unchecked_Deallocation( String, String_At );
  procedure Replace( Str_At : in out String_At; Str : String );
  procedure Replace( Str_At : in out String_At; Str : String_At );

  -- String Array helpers
  procedure Clear( Arr : in out String_Arr );
  procedure Clear( Arr : in out String_Arr_At );

  -- Time helpers
  subtype TimeMs_T is UInt32;
  function GetTimeInMs return TimeMs_T;

  -- Dispose routines for our dynamic arrays
  procedure Dispose is new Ada.Unchecked_Deallocation( Boolean_Arr, Boolean_Arr_At );
  procedure Dispose is new Ada.Unchecked_Deallocation( Byte_Arr, Byte_Arr_At );
  procedure Dispose is new Ada.Unchecked_Deallocation( Int16_Arr, Int16_Arr_At );
  procedure Dispose is new Ada.Unchecked_Deallocation( Int32_Arr, Int32_Arr_At );
  procedure Dispose is new Ada.Unchecked_Deallocation( Int64_Arr, Int64_Arr_At );
  procedure Dispose is new Ada.Unchecked_Deallocation( Float32_Arr, Float32_Arr_At );
  procedure Dispose is new Ada.Unchecked_Deallocation( Float64_Arr, Float64_Arr_At );
  procedure Dispose is new Ada.Unchecked_Deallocation( String_Arr, String_Arr_At );

  -- Debug
  procedure Trace( NameStr, ValueStr : String );
  procedure Trace( NameStr, ValueStr : String; E : Ada.Exceptions.Exception_Occurrence );

  function ToString(ptr : Byte_Arr_At) return String;

private
  -- Get original Class name
  function OriginalClassName(Self : Ops_Class) return String;

end Ops_Pa;

