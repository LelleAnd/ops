unit uOps.TopicInfoData;

(**
*
* Copyright (C) 2016-2022 Lennart Andersson.
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

uses Classes,
     uOps.Types,
     uOps.OPSObject,
     uOps.Topic,
     uOps.ArchiverInOut;

type
	/// NOTE. Must be kept in sync with other OPS language implementations
  TTopicInfoData = class(TOPSObject)
  public
    const
      TopicInfoData_idlVersion : Byte = 0;
  public
    TopicInfoData_version : Byte;
    Name : AnsiString;
    DataType : AnsiString;
    Transport : AnsiString;
    Address : AnsiString;
    Port : Integer;
    Keys : TDynAnsiStringArray;
    //std::vector<OPSObject*> filters;
  public
    constructor Create; overload;
    constructor Create(top : TTopic); overload;
    destructor Destroy; override;

    procedure Serialize(archiver : TArchiverInOut); override;

		// Returns a newely allocated deep copy/clone of this object.
		function Clone : TOPSObject; override;

		// Fills the parameter obj with all values from this object.
		procedure FillClone(var obj : TOPSObject); override;
  end;

implementation

uses uOps.Exceptions;

constructor TTopicInfoData.Create;
begin
  inherited;
  TopicInfoData_version := TopicInfoData_idlVersion;
  AppendType('TopicInfoData');
end;

destructor TTopicInfoData.Destroy;
begin
  inherited;
end;

constructor TTopicInfoData.Create(top : TTopic);
begin
  inherited Create;
  TopicInfoData_version := 0;
  AppendType('TopicInfoData');
  Name := top.Name;
  DataType := top.TypeID;
  Transport := top.Transport;
  Address := top.DomainAddress;
  Port := top.Port;
  //keys;
end;

procedure TTopicInfoData.Serialize(archiver : TArchiverInOut);
begin
	inherited Serialize(archiver);
  if FIdlVersionMask <> 0 then begin
    archiver.inout('TopicInfoData_version', TopicInfoData_version);
    if TopicInfoData_version > TopicInfoData_idlVersion then begin
      raise EIdlVersionException.Create('TopicInfoData', TopicInfoData_version, TopicInfoData_idlVersion);
    end;
  end else begin
    TopicInfoData_version := 0;
  end;
  archiver.inout('name', Name);
  archiver.inout('type', DataType);
  archiver.inout('transport', Transport);
  archiver.inout('address', Address);
  archiver.inout('port', Port);
  archiver.inout('keys', Keys);
  //archiver.inout('filters', filters);
end;

// Returns a newely allocated deep copy/clone of this object.
function TTopicInfoData.Clone : TOPSObject;
begin
	Result := TTopicInfoData.Create;
  Self.FillClone(Result);
end;

// Fills the parameter obj with all values from this object.
procedure TTopicInfoData.FillClone(var obj : TOPSObject);
var
  i : Integer;
begin
	inherited FillClone(obj);
  with obj as TTopicInfoData do begin
    TopicInfoData_version := Self.TopicInfoData_version;
    Name := Self.Name;
    DataType := Self.DataType;
    Transport := Self.Transport;
    Address := Self.Address;
    Port := Self.Port;
    SetLength(Keys, Length(Self.Keys));
    for i := 0 to High(Self.Keys) do begin
      Keys[i] := Self.Keys[i];
    end;
	end;
end;

end.

