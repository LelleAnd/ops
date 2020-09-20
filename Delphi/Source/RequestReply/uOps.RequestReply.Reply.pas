unit uOps.RequestReply.Reply;

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
  TReply = class(TOPSObject)
  public
    Reply_version : Byte;
    requestId : AnsiString;
    requestAccepted : Boolean;
    mess : AnsiString;

    procedure Serialize(archiver : TArchiverInOut); override;

    // Returns a newely allocated deep copy/clone of this object.
    function Clone : TOPSObject; override;

    // Fills the parameter obj with all values from this object.
    procedure FillClone(var obj : TOPSObject); override;
  end;

implementation

procedure TReply.Serialize(archiver : TArchiverInOut);
begin
  inherited Serialize(archiver);
  if FIdlVersionMask <> 0 then begin
    archiver.inout('Reply_version', Reply_version);
  end else begin
    Reply_version := 0;
  end;
  archiver.inout('requestId', requestId);
  archiver.inout('requestAccepted', requestAccepted);
  archiver.inout('message', mess);
end;

// Returns a newely allocated deep copy/clone of this object.
function TReply.Clone : TOPSObject;
begin
	Result := TReply.Create;
  Self.FillClone(Result);
end;

procedure TReply.FillClone(var obj : TOPSObject);
begin
	inherited FillClone(obj);
  with obj as TReply do begin
    Reply_version := Self.Reply_version;
    requestId := Self.requestId;
    requestAccepted := Self.requestAccepted;
    mess := Self.mess;
  end;
end;

end.

