unit uOps.ChecksumArchiver;

(**
*
* Copyright (C) 2020 Lennart Andersson.
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

uses System.Generics.Collections,
     System.SyncObjs,
     SysUtils,
     uOps.Types,
     uOPs.Error,
     uOps.ArchiverInOut;

type
  TChecksum_Calculator = class(TObject)
  public
    procedure Calc(const name : String; var Ptr; Length : Integer); virtual; abstract;
  end;

  TChecksumArchiver = class(TArchiverInOut)
  private
    FCalc : TChecksum_Calculator;
  public
    constructor Create(Calc : TChecksum_Calculator);
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

  // Example calculator
  TCalculator_8bit_xor = class(TChecksum_Calculator)
  public
    Sum : Byte;
    TotalBytes : LongWord;
    TotalFields : LongWord;
    constructor Create;
    destructor Destroy; override;
    procedure Calc(const name : String; var Ptr; Length : Integer); override;
  end;

implementation

uses uOps.OPSObject,
     uOps.Exceptions;

constructor TCalculator_8bit_xor.Create;
begin
  inherited Create;
  Sum := 0;
end;

destructor TCalculator_8bit_xor.Destroy;
begin
  inherited;
end;

procedure TCalculator_8bit_xor.Calc(const name : String; var Ptr; Length : Integer);
var
  i : Integer;
begin
  TotalBytes := TotalBytes + Longword(Length);
  Inc(TotalFields);
  for i := 0 to Length - 1 do begin
    Sum := Sum xor PByte(@Ptr)[i];
  end;
end;

constructor TChecksumArchiver.Create(Calc : TChecksum_Calculator);
begin
  inherited Create;
  FCalc := Calc;
end;

destructor TChecksumArchiver.Destroy;
begin
  inherited;
end;

function TChecksumArchiver.isOut : Boolean;
begin
  Result := True;
end;

procedure TChecksumArchiver.inout(const name : String; var value : Boolean);
begin
  FCalc.Calc(name, value, 1);
end;

procedure TChecksumArchiver.inout(const name : String; var value : Byte);
begin
  FCalc.Calc(name, value, 1);
end;

procedure TChecksumArchiver.inout(const name : String; var value : Int32);
begin
  FCalc.Calc(name, value, 4);
end;

procedure TChecksumArchiver.inout(const name : String; var value : Int16);
begin
  FCalc.Calc(name, value, 2);
end;

procedure TChecksumArchiver.inout(const name : String; var value : Int64);
begin
  FCalc.Calc(name, value, 8);
end;

procedure TChecksumArchiver.inout(const name : String; var value : Single);
begin
  FCalc.Calc(name, value, 4);
end;

procedure TChecksumArchiver.inout(const name : String; var value : Double);
begin
  FCalc.Calc(name, value, 8);
end;

procedure TChecksumArchiver.inout(const name : String; var value : AnsiString);
begin
  if Length(value) > 0 then begin
    FCalc.Calc(name, PByte(@value[1])^, Length(value));
  end else begin
    FCalc.Calc(name, value, 0);
  end;
end;

procedure TChecksumArchiver.inout(const name : String; var value : TSerializable);
begin
  if value is TOPSObject then begin
    value.Serialize(Self);
  end;
end;

procedure TChecksumArchiver.inout(const name : String; var value : TSerializable; element : Integer);
begin
  if value is TOPSObject then begin
    value.Serialize(Self);
  end;
end;

procedure TChecksumArchiver.inout(const name : String; buffer : PByte; bufferSize : Integer);
begin
  FCalc.Calc(name, buffer^, bufferSize);
end;

function TChecksumArchiver.inout2(const name : String; var value : TSerializable) : TSerializable;
begin
  if value is TOPSObject then begin
    value.Serialize(Self);
  end;
  Result := value;
end;

function TChecksumArchiver.inout2(const name : String; var value : TSerializable; element : Integer) : TSerializable;
begin
  if value is TOPSObject then begin
    value.Serialize(Self);
  end;
  Result := value;
end;

procedure TChecksumArchiver.inout(const name : String; var value : TDynBooleanArray);
begin
  if Length(value) > 0 then begin
    FCalc.Calc(name, PByte(@value[0])^, Length(value));
  end else begin
    FCalc.Calc(name, value, 0);
  end;
end;

procedure TChecksumArchiver.inout(const name : String; var value : TDynByteArray);
begin
  if Length(value) > 0 then begin
    FCalc.Calc(name, PByte(@value[0])^, Length(value));
  end else begin
    FCalc.Calc(name, value, 0);
  end;
end;

procedure TChecksumArchiver.inout(const name : String; var value : TDynInt32Array);
begin
  if Length(value) > 0 then begin
    FCalc.Calc(name, PByte(@value[0])^, 4 * Length(value));
  end else begin
    FCalc.Calc(name, value, 0);
  end;
end;

procedure TChecksumArchiver.inout(const name : String; var value : TDynInt16Array);
begin
  if Length(value) > 0 then begin
    FCalc.Calc(name, PByte(@value[0])^, 2 * Length(value));
  end else begin
    FCalc.Calc(name, value, 0);
  end;
end;

procedure TChecksumArchiver.inout(const name : String; var value : TDynInt64Array);
begin
  if Length(value) > 0 then begin
    FCalc.Calc(name, PByte(@value[0])^, 8 * Length(value));
  end else begin
    FCalc.Calc(name, value, 0);
  end;
end;

procedure TChecksumArchiver.inout(const name : String; var value : TDynSingleArray);
begin
  if Length(value) > 0 then begin
    FCalc.Calc(name, PByte(@value[0])^, 4 * Length(value));
  end else begin
    FCalc.Calc(name, value, 0);
  end;
end;

procedure TChecksumArchiver.inout(const name : String; var value : TDynDoubleArray);
begin
  if Length(value) > 0 then begin
    FCalc.Calc(name, PByte(@value[0])^, 8 * Length(value));
  end else begin
    FCalc.Calc(name, value, 0);
  end;
end;

procedure TChecksumArchiver.inout(const name : String; var value : TDynAnsiStringArray);
var
  i : Integer;
begin
  for i := 0 to Length(value) - 1 do begin
    inout(name, value[i]);
  end;
end;

procedure TChecksumArchiver.inout(const name : string; var value : TDynSerializableArray);
var
  i : Integer;
begin
  for i := 0 to Length(value) - 1 do begin
    inout('element', value[i]);
  end;
end;

procedure TChecksumArchiver.inoutfixarr(const name : string; value : Pointer; numElements : Integer; totalSize : Integer);
begin
  FCalc.Calc(name, PByte(value)^, totalSize);
end;

procedure TChecksumArchiver.inoutfixarr(const name : string; var value : array of AnsiString; numElements : Integer);
var
  i : Integer;
begin
  for i := 0 to numElements-1 do begin
    inout(name, value[i]);
  end;
end;

function TChecksumArchiver.beginList(const name : String; size : Integer) : Integer;
begin
  Result := size;
end;

procedure TChecksumArchiver.endList(const name : String);
begin
end;

end.

