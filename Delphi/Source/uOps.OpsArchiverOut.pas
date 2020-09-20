unit uOps.OpsArchiverOut;

(**
*
* Copyright (C) 2016-2020 Lennart Andersson.
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
     uOps.OpsObject;

type
  TOPSArchiverOut = class(TArchiverInOut)
  private
    FBuf : TByteBuffer;
    FOptNonVirt : Boolean;

  public
    constructor Create(buf : TByteBuffer; OptNonVirt : Boolean);
    destructor Destroy; override;

    function isOut : Boolean; override;

    procedure inout(const name : String; var value : Boolean); overload; override;
    procedure inout(const name : String; var value : Byte); overload; override;
    procedure inout(const name : String; var value : Int32); overload; override;
    procedure inout(const name : String; var value : Int16); overload; override;
    procedure inout(const name : String; var value : Int64); overload; override;
    procedure inout(const name : String; var value : Single); overload; override;
    procedure inout(const name : String; var value : Double); overload; override;
    procedure inout(const name : String; var value : AnsiString); overload; override;
    procedure inout(const name : String; var value : TSerializable); overload; override;
    procedure inout(const name : String; var value : TSerializable; element : Integer); overload; override;

		procedure inout(const name : String; buffer : PByte; bufferSize : Integer); overload; override;

		function inout2(const name : String; var value : TSerializable) : TSerializable; overload; override;

    function inout2(const name : String; var value : TSerializable; element : Integer) : TSerializable; overload; override;

    procedure inout(const name : String; var value : TDynBooleanArray); overload; override;
    procedure inout(const name : String; var value : TDynByteArray); overload; override;
    procedure inout(const name : String; var value : TDynInt32Array); overload; override;
    procedure inout(const name : String; var value : TDynInt16Array); overload; override;
    procedure inout(const name : String; var value : TDynInt64Array); overload; override;
    procedure inout(const name : String; var value : TDynSingleArray); overload; override;
    procedure inout(const name : String; var value : TDynDoubleArray); overload; override;
    procedure inout(const name : String; var value : TDynAnsiStringArray); overload; override;

    procedure inoutfixarr(const name : string; value : Pointer; numElements : Integer; totalSize : Integer); overload; override;
    procedure inoutfixarr(const name : string; var value : array of AnsiString; numElements : Integer); overload; override;

    procedure inout(const name : string; var value : TDynSerializableArray); overload; override;

    function beginList(const name : String; size : Integer) : Integer; override;
    procedure endList(const name : String); override;
  end;

implementation

uses uOps.Exceptions;

constructor TOPSArchiverOut.Create(buf : TByteBuffer; OptNonVirt : Boolean);
begin
  Fbuf := buf;
  FOptNonVirt := OptNonVirt;
end;

destructor TOPSArchiverOut.Destroy;
begin
  inherited;
end;

function TOPSArchiverOut.isOut : Boolean;
begin
  Result := True;
end;

procedure TOPSArchiverOut.inout(const name : String; var value : Boolean);
begin
  Fbuf.WriteBoolean(value);
end;

procedure TOPSArchiverOut.inout(const name : String; var value : Byte);
begin
  Fbuf.WriteChar(value);
end;

procedure TOPSArchiverOut.inout(const name : String; var value : Int32);
begin
  Fbuf.WriteInt(value);
end;

procedure TOPSArchiverOut.inout(const name : String; var value : Int16);
begin
  Fbuf.WriteShort(value);
end;

procedure TOPSArchiverOut.inout(const name : String; var value : Int64);
begin
  Fbuf.WriteLong(value);
end;

procedure TOPSArchiverOut.inout(const name : String; var value : Single);
begin
  Fbuf.WriteFloat(value);
end;

procedure TOPSArchiverOut.inout(const name : String; var value : Double);
begin
  Fbuf.WriteDouble(value);
end;

procedure TOPSArchiverOut.inout(const name : String; var value : AnsiString);
begin
  Fbuf.WriteString(AnsiString(value));
end;

procedure TOPSArchiverOut.inout(const name : String; var value : TSerializable);
var
  typeS : AnsiString;
begin
  typeS := '';
  if (value as TOPSObject).IdlVersionMask <> 0 then begin
    typeS := '0 ';
  end;
  if not FOptNonVirt then begin
    typeS := typeS + (value as TOPSObject).TypesString;
  end;
  Fbuf.WriteString(typeS);
  value.serialize(Self);
end;

procedure TOPSArchiverOut.inout(const name : String; var value : TSerializable; element : Integer);
var
  typeS : AnsiString;
begin
  typeS := '';
  if (value as TOPSObject).IdlVersionMask <> 0 then begin
    typeS := '0 ';
  end;
  if not FOptNonVirt then begin
    typeS := typeS + (value as TOPSObject).TypesString;
  end;
  Fbuf.WriteString(typeS);
  value.serialize(Self);
end;

procedure TOPSArchiverOut.inout(const name : String; buffer : PByte; bufferSize : Integer);
begin
  Fbuf.WriteChars(buffer, bufferSize);
end;

function TOPSArchiverOut.inout2(const name : String; var value : TSerializable) : TSerializable;
var
  typeS : AnsiString;
begin
  typeS := '';
  if (value as TOPSObject).IdlVersionMask <> 0 then begin
    typeS := '0 ';
  end;
  typeS := typeS + (value as TOPSObject).TypesString;
  Fbuf.WriteString(AnsiString(typeS));
  value.serialize(Self);
  Result := value;
end;

function TOPSArchiverOut.inout2(const name : String; var value : TSerializable; element : Integer) : TSerializable;
var
  typeS : AnsiString;
begin
  typeS := '';
  if (value as TOPSObject).IdlVersionMask <> 0 then begin
    typeS := '0 ';
  end;
  typeS := typeS + (value as TOPSObject).TypesString;
  Fbuf.WriteString(AnsiString(typeS));
  value.serialize(Self);
  Result := value;
end;

procedure TOPSArchiverOut.inout(const name : String; var value : TDynBooleanArray);
begin
  Fbuf.WriteBooleans(value);
end;

procedure TOPSArchiverOut.inout(const name : String; var value : TDynByteArray);
begin
  Fbuf.WriteBytes(value);
end;

procedure TOPSArchiverOut.inout(const name : String; var value : TDynInt32Array);
begin
  Fbuf.WriteInts(value);
end;

procedure TOPSArchiverOut.inout(const name : String; var value : TDynInt16Array);
begin
  Fbuf.WriteShorts(value);
end;

procedure TOPSArchiverOut.inout(const name : String; var value : TDynInt64Array);
begin
  Fbuf.WriteLongs(value);
end;

procedure TOPSArchiverOut.inout(const name : String; var value : TDynSingleArray);
begin
  Fbuf.WriteFloats(value);
end;

procedure TOPSArchiverOut.inout(const name : String; var value : TDynDoubleArray);
begin
  Fbuf.WriteDoubles(value);
end;

procedure TOPSArchiverOut.inout(const name : String; var value : TDynAnsiStringArray);
begin
  Fbuf.WriteStrings(value);
end;

procedure TOPSArchiverOut.inoutfixarr(const name : string; value : Pointer; numElements : Integer; totalSize : Integer);
begin
  FBuf.WriteInt(numElements);
  FBuf.WriteChars(PByte(value), totalSize);
end;

procedure TOPSArchiverOut.inoutfixarr(const name : string; var value : array of AnsiString; numElements : Integer);
var
  i : Integer;
begin
  FBuf.WriteInt(numElements);
  for i := 0 to numElements-1 do begin
    FBuf.WriteString(value[i]);
  end;
end;

function TOPSArchiverOut.beginList(const name : String; size : Integer) : Integer;
begin
  Fbuf.WriteInt(size);
  Result := size;
end;

procedure TOPSArchiverOut.endList(const name : String);
begin
  //Nothing to do in this implementation
end;

procedure TOPSArchiverOut.inout(const name : string; var value : TDynSerializableArray);
var
  size, i : Integer;
begin
  size := beginList(name, Length(value));

  // Now loop over all objects in the array
  for i := 0 to size-1 do begin
    inout(name, value[i], i);
  end;

  endList(name);
end;

end.

