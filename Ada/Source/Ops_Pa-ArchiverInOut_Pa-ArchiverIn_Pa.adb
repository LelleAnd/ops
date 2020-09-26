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

with Ops_Pa.OpsObject_Pa;
use  Ops_Pa.OpsObject_Pa;

package body Ops_Pa.ArchiverInOut_Pa.ArchiverIn_Pa is

  -- Constructors
  function Create( buf : ByteBuffer_Class_At;
                   fact : SerializableInheritingTypeFactory_Class_At ) return ArchiverIn_Class_At is
    Self : ArchiverIn_Class_At := null;
  begin
    Self := new ArchiverIn_Class;
    InitInstance( Self.all, Self, buf, fact );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  function IsOut( Self : in ArchiverIn_Class) return Boolean is
  begin
    return False;
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Boolean) is
  begin
    Self.FBuf.ReadBoolean(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Byte) is
  begin
    Self.FBuf.ReadChar(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int32) is
  begin
    Self.FBuf.ReadInt(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int16) is
  begin
    Self.FBuf.ReadShort(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int64) is
  begin
    Self.FBuf.ReadLong(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Float32) is
  begin
    Self.FBuf.ReadFloat(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Float64) is
  begin
    Self.FBuf.ReadDouble(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out String_At) is
  begin
    if value /= null then
      Dispose(value);
    end if;
    Self.FBuf.ReadString(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Serializable_Class_At) is
    types : String_At := null;
    verMask : UInt32 := 0;
  begin
    if value = null then
      raise Null_Object_Not_Allowed;
    end if;
    Self.FBuf.ReadString( types );
    if types /= null and then types'Length >= 2 and then types(types'First) = '0' then
      verMask := 1;
    end if;
    OpsObject_Class_At(value).SetIdlVersionMask( verMask );
    value.all.Serialize( ArchiverInOut_Class_At(Self.SelfAt) );
    if types /= null then
      Dispose(types);
    end if;
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Serializable_Class_At; element : Integer) is
  begin
    inout( Self, name, value );
  end;

  function inout2( Self : in out ArchiverIn_Class; name : String; value : in out Serializable_Class_At) return Serializable_Class_At is
    types : String_At := null;
    result : Serializable_Class_At := null;
  begin
    if value /= null then
      Free(value);
    end if;
    Self.FBuf.ReadString( types );
    if types /= null then
      declare
        verMask : UInt32 := 0;
        start : Integer := types'First;
      begin
        if types'Length >= 2 and then types(start) = '0' then
          verMask := 1;
          start := start + 2;
        end if;
        result := Self.Factory.Make( types.all(start..types'Last) );
        if result /= null then
          OpsObject_Class_At(result).SetIdlVersionMask( verMask );

          -- We need to preserve the type information since the factory only can create
          -- objects it knows how to create, and this can be a more generalized (base) object
          -- than the actual one. The rest of the bytes will be placed in the spareBytes member.
          SetTypesString(result.all, types.all(start..types'Last));

          result.Serialize(ArchiverInOut_Class_At(Self.SelfAt));
        end if;
      end;
      Dispose(types);
    end if;
    return result;
  end;

  function inout( Self : in out ArchiverIn_Class; name : String; value : in out Serializable_Class_At; element : Integer) return Serializable_Class_At is
    types : String_At := null;
    result : Serializable_Class_At := null;
  begin
    Self.FBuf.ReadString( types );
    if types /= null then
      declare
        verMask : Int32 := 0;
        start : Integer := types'First;
      begin
        if types'Length >= 2 and then types(start) = '0' then
          verMask := 1;
          start := start + 2;
        end if;
        result := Self.Factory.Make( types.all(start..types'Last) );
        if result /= null then
          result.Serialize(ArchiverInOut_Class_At(Self.SelfAt));
        end if;
      end;
      Dispose(types);
    end if;
    return result;
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Boolean_Arr_At) is
  begin
    if value /= null then
      Dispose(value);
    end if;
    Self.FBuf.ReadBooleans(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Boolean_Arr) is
  begin
    Self.FBuf.ReadBooleans(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Byte_Arr_At) is
  begin
    if value /= null then
      Dispose(value);
    end if;
    Self.FBuf.ReadBytes(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Byte_Arr) is
  begin
    Self.FBuf.ReadBytes(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int32_Arr_At) is
  begin
    if value /= null then
      Dispose(value);
    end if;
    Self.FBuf.ReadInts(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int32_Arr) is
  begin
    Self.FBuf.ReadInts(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int16_Arr_At) is
  begin
    if value /= null then
      Dispose(value);
    end if;
    Self.FBuf.ReadShorts(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int16_Arr) is
  begin
    Self.FBuf.ReadShorts(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int64_Arr_At) is
  begin
    if value /= null then
      Dispose(value);
    end if;
    Self.FBuf.ReadLongs(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Int64_Arr) is
  begin
    Self.FBuf.ReadLongs(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Float32_Arr_At) is
  begin
    if value /= null then
      Dispose(value);
    end if;
    Self.FBuf.ReadFloats(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Float32_Arr) is
  begin
    Self.FBuf.ReadFloats(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Float64_Arr_At) is
  begin
    if value /= null then
      Dispose(value);
    end if;
    Self.FBuf.ReadDoubles(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out Float64_Arr) is
  begin
    Self.FBuf.ReadDoubles(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out String_Arr_At) is
  begin
    Clear(value);
    Dispose(value);
    Self.FBuf.ReadStrings(value);
  end;

  procedure inout( Self : in out ArchiverIn_Class; name : String; value : in out String_Arr) is
  begin
    Clear(value);
    Self.FBuf.ReadStrings(value);
  end;

--  procedure inout( Self : in out ArchiverIn_Class; name : string; Value : in out Serializable_Class_Arr_At) is
--    size : Integer;
--  begin
--    size := Self.beginList(name, 0);
--
--    -- First free ev existing objects in the array
--    for i in Value'Range loop
--      Free(Value(i));
--    end loop;
--    Dispose(Value);
--
--    -- Create new array
--    Value := new Serializable_Class_Arr(0..Integer(size-1));
--
--    -- Now loop over all objects in the array
--    for i in Value'Range loop
--      Value(i) := Self.inout2(name, Value(i));
--    end loop;
--
--    Self.endList(name);
--  end;

  function beginList( Self : in out ArchiverIn_Class; name : String; size : Integer) return Integer is
    Result : Int32;
  begin
    Self.FBuf.ReadInt(Result);
    return Integer(Result);
  end;

  procedure endList( Self : in out ArchiverIn_Class; name : String) is
  begin
    -- Nothing to do in this implementation
    null;
  end;

  procedure InitInstance( Self : in out ArchiverIn_Class;
                          SelfAt : ArchiverIn_Class_At;
                          buf : ByteBuffer_Class_At;
                          fact : SerializableInheritingTypeFactory_Class_At) is
  begin
    Self.SelfAt := SelfAt;
    Self.FBuf := buf;
    Self.Factory := fact;
  end;

  overriding procedure Finalize( Self : in out ArchiverIn_Class ) is
  begin
    null;
  end;

end Ops_Pa.ArchiverInOut_Pa.ArchiverIn_Pa;
