unit uOps.OpsArchiverIn;

(**
*
* Copyright (C) 2016 Lennart Andersson.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*)

interface

uses uOps.Types,
     uOps.ArchiverInOut,
     uOps.ByteBuffer,
     uOps.OpsObject,
     uOps.SerializableInheritingTypeFactory;

type
  TOPSArchiverIn = class(TArchiverInOut)
  private
    FBuf : TByteBuffer;
    FFactory : TSerializableInheritingTypeFactory;

  public
    constructor Create(buf : TByteBuffer; factory : TSerializableInheritingTypeFactory);
    destructor Destroy; override;

    procedure inout(const name : String; var value : Boolean); overload; override;
    procedure inout(const name : String; var value : Byte); overload; override;
    procedure inout(const name : String; var value : Int32); overload; override;
    procedure inout(const name : String; var value : Int16); overload; override;
    procedure inout(const name : String; var value : Int64); overload; override;
    procedure inout(const name : String; var value : Single); overload; override;
    procedure inout(const name : String; var value : Double); overload; override;
    procedure inout(const name : String; var value : AnsiString); overload; override;
    procedure inout(const name : String; var value : TSerializable); overload; override;

		procedure inout(const name : String; buffer : PByte; bufferSize : Integer); overload; override;

		function inout2(const name : String; var value : TSerializable) : TSerializable; overload; override;

    function inout(const name : String; var value : TSerializable; element : Integer) : TSerializable; overload; override;

    procedure inout(const name : String; var value : TDynBooleanArray); overload; override;
    procedure inout(const name : String; var value : TDynByteArray); overload; override;
    procedure inout(const name : String; var value : TDynInt32Array); overload; override;
    procedure inout(const name : String; var value : TDynInt16Array); overload; override;
    procedure inout(const name : String; var value : TDynInt64Array); overload; override;
    procedure inout(const name : String; var value : TDynSingleArray); overload; override;
    procedure inout(const name : String; var value : TDynDoubleArray); overload; override;
    procedure inout(const name : String; var value : TDynAnsiStringArray); overload; override;

    procedure inout(const name : string; var value : TDynSerializableArray); overload; override;

  protected
    function beginList(const name : String; size : Integer) : Integer; override;
    procedure endList(const name : String); override;
  end;

implementation

uses SysUtils;

constructor TOPSArchiverIn.Create(buf : TByteBuffer; factory : TSerializableInheritingTypeFactory);
begin
  Fbuf := buf;
  FFactory := factory;
end;

destructor TOPSArchiverIn.Destroy;
begin
  inherited;
end;

procedure TOPSArchiverIn.inout(const name : String; var value : Boolean);
begin
  value := Fbuf.ReadBoolean;
end;

procedure TOPSArchiverIn.inout(const name : String; var value : Byte);
begin
  value := Fbuf.ReadChar;
end;

procedure TOPSArchiverIn.inout(const name : String; var value : Int32);
begin
  value := Fbuf.ReadInt;
end;

procedure TOPSArchiverIn.inout(const name : String; var value : Int16);
begin
  value := Fbuf.ReadShort;
end;

procedure TOPSArchiverIn.inout(const name : String; var value : Int64);
begin
  value := Fbuf.ReadLong;
end;

procedure TOPSArchiverIn.inout(const name : String; var value : Single);
begin
  value := Fbuf.ReadFloat;
end;

procedure TOPSArchiverIn.inout(const name : String; var value : Double);
begin
  value := Fbuf.ReadDouble;
end;

procedure TOPSArchiverIn.inout(const name : String; var value : AnsiString);
begin
  value := Fbuf.ReadString;
end;

procedure TOPSArchiverIn.inout(const name : String; buffer : PByte; bufferSize : Integer);
begin
  Fbuf.ReadChars(buffer, bufferSize);
end;

procedure TOPSArchiverIn.inout(const name : String; var value : TSerializable);
var
  types : string;
begin
  types := string(Fbuf.ReadString);
  value.Serialize(Self);
end;

function TOPSArchiverIn.inout(const name : String; var value : TSerializable; element : Integer) : TSerializable;
var
  types : string;
begin
  types := string(Fbuf.ReadString);
  Result := FFactory.Make(types);
  if Assigned(Result) then begin
    Result.Serialize(Self);
  end;
end;

function TOPSArchiverIn.inout2(const name : String; var value : TSerializable) : TSerializable;
var
  types : AnsiString;
begin
  if Assigned(value) then begin
    FreeAndNil(value);
  end;
  types := Fbuf.ReadString;
  Result := FFactory.Make(string(types));
  if Assigned(Result) then begin
    // We need to preserve the type information since the factory only can create
    // objects it knows how to create, and this can be a more generalized (base) object
    // than the actual one. The rest of the bytes will be placed in the spareBytes member.
    SetTypesString(Result, types);

    Result.Serialize(Self);
  end;
end;

procedure TOPSArchiverIn.inout(const name : String; var value : TDynBooleanArray);
begin
  value := Fbuf.ReadBooleans;
end;

procedure TOPSArchiverIn.inout(const name : String; var value : TDynByteArray);
begin
  value := Fbuf.ReadBytes;
end;

procedure TOPSArchiverIn.inout(const name : String; var value : TDynInt32Array);
begin
  value := Fbuf.ReadInts;
end;

procedure TOPSArchiverIn.inout(const name : String; var value : TDynInt16Array);
begin
  value := Fbuf.ReadShorts;
end;

procedure TOPSArchiverIn.inout(const name : String; var value : TDynInt64Array);
begin
  value := Fbuf.ReadLongs;
end;

procedure TOPSArchiverIn.inout(const name : String; var value : TDynSingleArray);
begin
  value := Fbuf.ReadFloats;
end;

procedure TOPSArchiverIn.inout(const name : String; var value : TDynDoubleArray);
begin
  value := Fbuf.ReadDoubles;
end;

procedure TOPSArchiverIn.inout(const name : String; var value : TDynAnsiStringArray);
begin
  value := Fbuf.ReadStrings;
end;

function TOPSArchiverIn.beginList(const name : String; size : Integer) : Integer;
begin
  Result := Fbuf.ReadInt;
end;

procedure TOPSArchiverIn.endList(const name : String);
begin
  //Nothing to do in this implementation
end;

procedure TOPSArchiverIn.inout(const name : string; var value : TDynSerializableArray);
var
  size, i : Integer;
begin
  size := beginList(name, 0);

  // First free ev existing objects in the array
  for i := 0 to Length(value)-1 do begin
    FreeAndNil(value[i]);
  end;

  // Set new length (elements will be nil)
  SetLength(value, size);

  // Now loop over all objects in the array
  for i := 0 to size-1 do begin
    value[i] := inout2(name, value[i]);
  end;

  endList(name);
end;

end.

