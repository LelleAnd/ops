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

with Ada.Unchecked_Conversion;

with Ops_Pa.OpsObject_Pa;

use Ops_Pa.OpsObject_Pa;

package body Ops_Pa.ArchiverInOut_Pa.ChecksumArchiver_Pa is

  subtype ByteArr2_T is Byte_Arr(0..1);
  subtype ByteArr4_T is Byte_Arr(0..3);
  subtype ByteArr8_T is Byte_Arr(0..7);

  function ToByte is new Ada.Unchecked_Conversion(Character, Byte);

  function ToByteArr is new Ada.Unchecked_Conversion(Int16, ByteArr2_T);
  function ToByteArr is new Ada.Unchecked_Conversion(Int32, ByteArr4_T);
  function ToByteArr is new Ada.Unchecked_Conversion(Int64, ByteArr8_T);
  function ToByteArr is new Ada.Unchecked_Conversion(Float32, ByteArr4_T);
  function ToByteArr is new Ada.Unchecked_Conversion(Float64, ByteArr8_T);

  function Create(Calculator : Checksum_Calculator_Class_At) return ChecksumArchiver_Class_At is
    Self : ChecksumArchiver_Class_At := null;
  begin
    Self := new ChecksumArchiver_Class;
    InitInstance( Self.all, Self, Calculator );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  procedure InitInstance( Self : in out ChecksumArchiver_Class; SelfAt : ChecksumArchiver_Class_At; Calculator : Checksum_Calculator_Class_At ) is
  begin
    Self.SelfAt := SelfAt;
    Self.Calculator := Calculator;
  end;

  overriding procedure Finalize( Self : in out ChecksumArchiver_Class ) is
  begin
    null;
  end;

  function IsOut( Self : in ChecksumArchiver_Class) return Boolean is
  begin
    return True;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Boolean) is
    Bool_C : array(Boolean) of Byte := (0, 1);
    arr : Byte_Arr(1..1) := (1 => Bool_C(value));
  begin
    Self.Calculator.Calc( arr );
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Byte) is
    arr : Byte_Arr(1..1) := (1 => value);
  begin
    Self.Calculator.Calc( arr );
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int32) is
  begin
    Self.Calculator.Calc( ToByteArr(value) );
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int16) is
  begin
    Self.Calculator.Calc( ToByteArr(value) );
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int64) is
  begin
    Self.Calculator.Calc( ToByteArr(value) );
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Float32) is
  begin
    Self.Calculator.Calc( ToByteArr(value) );
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Float64) is
  begin
    Self.Calculator.Calc( ToByteArr(value) );
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out String_At) is
  begin
    if value /= null then
      if value'Length > 0 then
        declare
          arr : Byte_Arr(1..Byte_Arr_Index_T(value'Length));
        begin
          for I in arr'range loop
            arr(I) := ToByte( value(Integer(I)) );
          end loop;
          Self.Calculator.Calc( arr );
        end;
      end if;
    end if;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Serializable_Class_At) is
  begin
    if value = null then
      raise Null_Object_Not_Allowed;
    end if;
    if value.all in OpsObject_Class'Class then
      value.Serialize(ArchiverInOut_Class_At(Self.SelfAt));
    end if;
  end;

  overriding procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Serializable_Class_At; element : Integer) is
  begin
    if value = null then
      raise Null_Object_Not_Allowed;
    end if;
    if value.all in OpsObject_Class'Class then
      value.Serialize(ArchiverInOut_Class_At(Self.SelfAt));
    end if;
  end;

  function inout2( Self : in out ChecksumArchiver_Class; name : String; value : in out Serializable_Class_At) return Serializable_Class_At is
  begin
    if value = null then
      raise Null_Object_Not_Allowed;
    end if;
    if value.all in OpsObject_Class'Class then
      value.Serialize(ArchiverInOut_Class_At(Self.SelfAt));
    end if;
    return value;
  end;

  function inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Serializable_Class_At; element : Integer) return Serializable_Class_At is
  begin
    if value = null then
      raise Null_Object_Not_Allowed;
    end if;
    if value.all in OpsObject_Class'Class then
      value.Serialize(ArchiverInOut_Class_At(Self.SelfAt));
    end if;
    return value;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Boolean_Arr_At) is
  begin
    if value /= null then
      inout(Self, name, value.all);
    end if;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Boolean_Arr) is
  begin
    if value'Length > 0 then
      for I in value'Range loop
        Self.inout( name, value(I) );
      end loop;
    end if;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Byte_Arr_At) is
  begin
    if value /= null then
      inout(Self, name, value.all);
    end if;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Byte_Arr) is
  begin
    if value'Length > 0 then
      Self.Calculator.Calc( value );
    end if;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int32_Arr_At) is
  begin
    if value /= null then
      inout(Self, name, value.all);
    end if;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int32_Arr) is
  begin
    if value'Length > 0 then
      for I in value'Range loop
        Self.inout( name, value(I) );
      end loop;
    end if;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int16_Arr_At) is
  begin
    if value /= null then
      inout(Self, name, value.all);
    end if;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int16_Arr) is
  begin
    if value'Length > 0 then
      for I in value'Range loop
        Self.inout( name, value(I) );
      end loop;
    end if;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int64_Arr_At) is
  begin
    if value /= null then
      inout(Self, name, value.all);
    end if;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Int64_Arr) is
  begin
    if value'Length > 0 then
      for I in value'Range loop
        Self.inout( name, value(I) );
      end loop;
    end if;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Float32_Arr_At) is
  begin
    if value /= null then
      inout(Self, name, value.all);
    end if;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Float32_Arr) is
  begin
    if value'Length > 0 then
      for I in value'Range loop
        Self.inout( name, value(I) );
      end loop;
    end if;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Float64_Arr_At) is
  begin
    if value /= null then
      inout(Self, name, value.all);
    end if;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out Float64_Arr) is
  begin
    if value'Length > 0 then
      for I in value'Range loop
        Self.inout( name, value(I) );
      end loop;
    end if;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out String_Arr_At) is
  begin
    if value /= null then
      inout(Self, name, value.all);
    end if;
  end;

  procedure inout( Self : in out ChecksumArchiver_Class; name : String; value : in out String_Arr) is
  begin
    if value'Length > 0 then
      for I in value'Range loop
        Self.inout( name, value(I) );
      end loop;
    end if;
  end;

  function beginList( Self : in out ChecksumArchiver_Class; name : String; size : Integer) return Integer is
  begin
    return size;
  end;

  procedure endList( Self : in out ChecksumArchiver_Class; name : String) is
  begin
    null;
  end;

  function Create return Checksum_Calc_8bit_xor_Class_At is
    Self : Checksum_Calc_8bit_xor_Class_At := null;
  begin
    Self := new Checksum_Calc_8bit_xor_Class;
    InitInstance( Self.all );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out Checksum_Calc_8bit_xor_Class ) is
  begin
    null;
  end;

  procedure Finalize( Self : in out Checksum_Calc_8bit_xor_Class ) is
  begin
    null;
  end;

  procedure Calc( Self : in out Checksum_Calc_8bit_xor_Class; Bytes : Byte_Arr ) is
  begin
    Self.TotalBytes := Self.TotalBytes + UInt32(Bytes'Length);
    Self.TotalFields := Self.TotalFields + 1;
    for i in Bytes'range loop
      Self.Sum := Self.Sum xor Bytes(i);
    end loop;
  end;

end Ops_Pa.ArchiverInOut_Pa.ChecksumArchiver_Pa;


