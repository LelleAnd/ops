unit uOps.Transport;

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
     uOps.OPSObject;

type
  TTransport = class(TOPSObject)
  public
    Transport_version : Byte;
    channelID : AnsiString;
    topics : TDynAnsiStringArray;

    constructor Create;

    procedure Serialize(archiver : TArchiverInOut); override;

		// Returns a newely allocated deep copy/clone of this object.
		function Clone : TOPSObject; override;

		// Fills the parameter obj with all values from this object.
		procedure FillClone(var obj : TOPSObject); override;
  end;

implementation

uses SysUtils;

{ TTransport }

constructor TTransport.Create;
begin
  inherited;
  Transport_version := 0;
  AppendType('Transport');
end;

procedure TTransport.Serialize(archiver: TArchiverInOut);
begin
  inherited Serialize(archiver);
  if FIdlVersionMask <> 0 then begin
    archiver.inout('Transport_version', Transport_version);
  end else begin
    Transport_version := 0;
  end;
  archiver.inout('channelID', channelID);
  archiver.inout('topics', topics);
end;

function TTransport.Clone: TOPSObject;
begin
	Result := TTransport.Create;
  Self.FillClone(Result);
end;

procedure TTransport.FillClone(var obj: TOPSObject);
var
  i : Integer;
begin
	inherited FillClone(obj);
  with obj as TTransport do begin
    Transport_version := Self.Transport_version;
    channelID := Self.channelID;
    SetLength(topics, Length(Self.topics));
    for i := 0 to High(topics) do topics[i] := Self.topics[i];
  end;
end;

end.

