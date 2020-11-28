unit uOps.RequestReply.Request;

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

uses uOps.ArchiverInOut,
     uOps.OPSObject;

type
  TRequest = class(TOPSObject)
  public
    const
      Request_idlVersion : Byte = 0;
  public
    Request_version : Byte;
    requestId : AnsiString;

    constructor Create;

    procedure Serialize(archiver : TArchiverInOut); override;

    // Returns a newely allocated deep copy/clone of this object.
    function Clone : TOPSObject; override;

    // Fills the parameter obj with all values from this object.
    procedure FillClone(var obj : TOPSObject); override;
  end;

implementation

uses uOps.Exceptions;

constructor TRequest.Create;
begin
  inherited;
  Request_version := Request_idlVersion;
  AppendType('Request');
end;

procedure TRequest.Serialize(archiver : TArchiverInOut);
begin
  inherited Serialize(archiver);
  if FIdlVersionMask <> 0 then begin
    archiver.inout('Request_version', Request_version);
    if Request_version > Request_idlVersion then begin
      raise EIdlVersionException.Create('Request', Request_version, Request_idlVersion);
    end;
  end else begin
    Request_version := 0;
  end;
  archiver.inout('requestId', requestId);
end;

// Returns a newely allocated deep copy/clone of this object.
function TRequest.Clone : TOPSObject;
begin
	Result := TRequest.Create;
  Self.FillClone(Result);
end;

procedure TRequest.FillClone(var obj : TOPSObject);
begin
	inherited FillClone(obj);
  with obj as TRequest do begin
    Request_version := Self.Request_version;
		requestId := Self.requestId;
	end;
end;

end.

