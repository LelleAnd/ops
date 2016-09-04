unit uOps.OpsObject;

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

uses uOps.ArchiverInOut;

type
  TOPSObject = class(TSerializable)
    protected
      //Should only be set by the Publisher at publication time and by ByteBuffer at deserialization time.
      FKey : AnsiString;
      FTypesString : AnsiString;

		  procedure AppendType(const _type : AnsiString);

      // Used from archivers
      procedure SetTypesString(types : AnsiString); override;

    public
      ///Bytes that hold unserialized data for this object.
      ///This happens if a type can not be fully understood by a participants type support.
      spareBytes : array of Byte;

      constructor Create;
      destructor Destroy; override;

      procedure Serialize(archiver : TArchiverInOut); override;

      ///Returns a newely allocated deep copy/clone of this object.
      function Clone : TOPSObject; virtual;

      ///Fills the parameter obj with all values from this object.
      procedure FillClone(var obj : TOPSObject); virtual;

      property Key : AnsiString read FKey write FKey;

      property TypesString : AnsiString read FTypesString;
  end;

implementation

{ TOPSObject }

constructor TOPSObject.Create;
begin
  Fkey         := 'k';
  FTypesString := '';
end;

destructor TOPSObject.Destroy;
begin
  inherited;
end;

procedure TOPSObject.AppendType(const _type : AnsiString);
begin
	FTypesString := _type + ' ' + FTypesString;
end;

procedure TOPSObject.SetTypesString(types: AnsiString);
begin
  FTypesString := types;
end;

function TOPSObject.Clone : TOPSObject;
begin
  Result := TOPSObject.Create;
  FillClone(Result);
end;

procedure TOPSObject.FillClone(var obj : TOPSObject);
begin
  obj.FKey := FKey;
  obj.FTypesString := FTypesString;
  obj.spareBytes := Copy(spareBytes);
end;

procedure TOPSObject.Serialize(archiver : TArchiverInOut);
begin
  archiver.inout('key', FKey);
end;

end.

