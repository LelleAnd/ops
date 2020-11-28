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

package body Ops_Pa.ArchiverInOut_Pa.ArchiverOut_Pa is

  -- Constructors
  function Create( buf : ByteBuffer_Class_At; OptNonVirt : Boolean ) return ArchiverOut_Class_At is
    Self : ArchiverOut_Class_At := null;
  begin
    Self := new ArchiverOut_Class;
    InitInstance( Self.all, Self, buf, OptNonVirt );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  function IsOut( Self : in ArchiverOut_Class) return Boolean is
  begin
    return True;
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Boolean) is
  begin
    Self.FBuf.WriteBoolean(value);
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Byte) is
  begin
    Self.FBuf.WriteChar(value);
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Int32) is
  begin
    Self.FBuf.WriteInt(value);
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Int16) is
  begin
    Self.FBuf.WriteShort(value);
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Int64) is
  begin
    Self.FBuf.WriteLong(value);
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Float32) is
  begin
    Self.FBuf.WriteFloat(value);
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Float64) is
  begin
    Self.FBuf.WriteDouble(value);
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out String_At) is
  begin
    Self.FBuf.WriteString(value);
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Serializable_Class_At) is
  begin
    if value = null then
      raise Null_Object_Not_Allowed;
    end if;
    if OpsObject_Class_At(value).IdlVersionMask /= 0 then
      if Self.OptNonVirt then
        Self.FBuf.WriteString( "0 " );
      else
        Self.FBuf.WriteString( "0 " & value.TypesString );
      end if;
    else
      if Self.OptNonVirt then
        Self.FBuf.WriteString( "" );
      else
        Self.FBuf.WriteString( value.TypesString );
      end if;
    end if;
    value.all.Serialize( ArchiverInOut_Class_At(Self.SelfAt) );
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Serializable_Class_At; element : Integer) is
  begin
    inout( Self, name, value );
  end;

  function inout2( Self : in out ArchiverOut_Class; name : String; value : in out Serializable_Class_At) return Serializable_Class_At is
  begin
    if value = null then
      raise Null_Object_Not_Allowed;
    end if;
    if OpsObject_Class_At(value).IdlVersionMask /= 0 then
      Self.FBuf.WriteString( "0 " & value.TypesString );
    else
      Self.FBuf.WriteString( value.TypesString );
    end if;
    value.all.Serialize( ArchiverInOut_Class_At(Self.SelfAt) );
    return value;
  end;

  function inout( Self : in out ArchiverOut_Class; name : String; value : in out Serializable_Class_At; element : Integer) return Serializable_Class_At is
  begin
    return inout2( Self, name, value );
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Boolean_Arr_At) is
  begin
    if value = null then
      Self.FBuf.WriteInt(0);
    else
      Self.FBuf.WriteBooleans(value.all);
    end if;
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Boolean_Arr) is
  begin
    Self.FBuf.WriteBooleans(value);
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Byte_Arr_At) is
  begin
    if value = null then
      Self.FBuf.WriteInt(0);
    else
      Self.FBuf.WriteBytes( value.all );
    end if;
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Byte_Arr) is
  begin
    Self.FBuf.WriteBytes( value );
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Int32_Arr_At) is
  begin
    if value = null then
      Self.FBuf.WriteInt(0);
    else
      Self.FBuf.WriteInts(value.all);
    end if;
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Int32_Arr) is
  begin
    Self.FBuf.WriteInts(value);
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Int16_Arr_At) is
  begin
    if value = null then
      Self.FBuf.WriteInt(0);
    else
      Self.FBuf.WriteShorts(value.all);
    end if;
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Int16_Arr) is
  begin
    Self.FBuf.WriteShorts(value);
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Int64_Arr_At) is
  begin
    if value = null then
      Self.FBuf.WriteInt(0);
    else
      Self.FBuf.Writelongs(value.all);
    end if;
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Int64_Arr) is
  begin
    Self.FBuf.Writelongs(value);
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Float32_Arr_At) is
  begin
    if value = null then
      Self.FBuf.WriteInt(0);
    else
      Self.FBuf.WriteFloats(value.all);
    end if;
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Float32_Arr) is
  begin
    Self.FBuf.WriteFloats(value);
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Float64_Arr_At) is
  begin
    if value = null then
      Self.FBuf.WriteInt(0);
    else
      Self.FBuf.WriteDoubles(value.all);
    end if;
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out Float64_Arr) is
  begin
    Self.FBuf.WriteDoubles(value);
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out String_Arr_At) is
  begin
    if value = null then
      Self.FBuf.WriteInt(0);
    else
      Self.FBuf.WriteStrings(value.all);
    end if;
  end;

  procedure inout( Self : in out ArchiverOut_Class; name : String; value : in out String_Arr) is
  begin
    Self.FBuf.WriteStrings(value);
  end;

  function beginList( Self : in out ArchiverOut_Class; name : String; size : Integer) return Integer is
  begin
    Self.FBuf.WriteInt(Int32(size));
    return size;
  end;

  procedure endList( Self : in out ArchiverOut_Class; name : String) is
  begin
    -- Nothing to do in this implementation
    null;
  end;

--  procedure inout( Self : in out ArchiverOut_Class; name : string; Value : in out Serializable_Class_Arr_At) is
--    size : Integer;
--  begin
--    size := Self.beginList(name, value'Length);
--
--    -- Now loop over all objects in the array
--    for i in Value'Range loop
--      Self.inout(name, Value(i));
--    end loop;
--
--    Self.endList(name);
--  end;

  procedure InitInstance( Self : in out ArchiverOut_Class; SelfAt : ArchiverOut_Class_At;
                          buf : ByteBuffer_Class_At; OptNonVirt : Boolean ) is
  begin
    Self.SelfAt := SelfAt;
    Self.FBuf := buf;
    Self.OptNonVirt := OptNonVirt;
  end;

  overriding procedure Finalize( Self : in out ArchiverOut_Class ) is
  begin
    null;
  end;

end Ops_Pa.ArchiverInOut_Pa.ArchiverOut_Pa;
