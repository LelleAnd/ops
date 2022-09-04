unit uOps.ParticipantInfoData;

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
     uOps.TopicInfoData,
     uOps.ArchiverInOut;

type
	/// NOTE. Must be kept in sync with other OPS language implementations
  TParticipantInfoData = class(TOPSObject)
  public
    const
      ParticipantInfoData_idlVersion : Byte = 0;
    type
      TDynTopicInfoDataArray = array of TTopicInfoData;

	public
    ParticipantInfoData_version : Byte;
		Name                   : AnsiString;
		Id                     : AnsiString;
		Domain                 : AnsiString;
		Ip                     : AnsiString;
		LanguageImplementation : AnsiString;
		OpsVersion             : AnsiString;
		mc_udp_port            : Integer;
		mc_tcp_port            : Integer;

		subscribeTopics        : TDynTopicInfoDataArray;
		publishTopics          : TDynTopicInfoDataArray;
		knownTypes             : TDynAnsiStringArray;

  public
    constructor Create;
    destructor Destroy; override;

    procedure Serialize(archiver : TArchiverInOut); override;

		// Returns a newely allocated deep copy/clone of this object.
		function Clone : TOPSObject; override;

		// Fills the parameter obj with all values from this object.
		procedure FillClone(var obj : TOPSObject); override;

    // Help routines for updating the dynamic arrays with TopicInfoData
    procedure addTopic(var arr : TDynTopicInfoDataArray; top : TTopic);
    procedure removeTopic(var arr : TDynTopicInfoDataArray; top : TTopic);

    function existTopic(var arr : TDynTopicInfoDataArray; topicName : AnsiString) : Boolean;

  end;

implementation

uses SysUtils,
     uOps.Exceptions;

constructor TParticipantInfoData.Create;
begin
  inherited Create;
  AppendType('ops.ParticipantInfoData');
  ParticipantInfoData_version := ParticipantInfoData_idlVersion;
end;

destructor TParticipantInfoData.Destroy;
var
  i : Integer;
begin
  for i := 0 to Length(subscribeTopics) - 1 do begin
    FreeAndNil(subscribeTopics[i]);
  end;
  for i := 0 to Length(publishTopics) - 1 do begin
    FreeAndNil(publishTopics[i]);
  end;
  inherited;
end;

procedure TParticipantInfoData.Serialize(archiver : TArchiverInOut);
begin
	inherited Serialize(archiver);
  if FIdlVersionMask <> 0 then begin
    archiver.inout('ParticipantInfoData_version', ParticipantInfoData_version);
    if ParticipantInfoData_version > ParticipantInfoData_idlVersion then begin
      raise EIdlVersionException.Create('ParticipantInfoData',
              ParticipantInfoData_version, ParticipantInfoData_idlVersion);
    end;
  end else begin
    ParticipantInfoData_version := 0;
  end;
  archiver.inout('name', name);
  archiver.inout('domain', domain);
  archiver.inout('id', id);
  archiver.inout('ip', ip);
  archiver.inout('languageImplementation', languageImplementation);
  archiver.inout('opsVersion', opsVersion);
  archiver.inout('mc_udp_port', mc_udp_port);
  archiver.inout('mc_tcp_port', mc_tcp_port);
  archiver.inout('subscribeTopics', TDynSerializableArray(subscribeTopics));
  archiver.inout('publishTopics', TDynSerializableArray(publishTopics));
  archiver.inout('knownTypes', knownTypes);
end;

// Returns a newely allocated deep copy/clone of this object.
function TParticipantInfoData.Clone : TOPSObject;
begin
	Result := TParticipantInfoData.Create;
  Self.FillClone(Result);
end;

// Fills the parameter obj with all values from this object.
procedure TParticipantInfoData.FillClone(var obj : TOPSObject);
var
  i : Integer;
begin
	inherited FillClone(obj);
  with obj as TParticipantInfoData do begin
    ParticipantInfoData_version := Self.ParticipantInfoData_version;
		Name := Self.Name;
		Id := Self.Id;
		Domain := Self.Domain;
		Ip := Self.Ip;
		LanguageImplementation := Self.LanguageImplementation;
		OpsVersion := Self.OpsVersion;
		mc_udp_port := Self.mc_udp_port;
		mc_tcp_port := Self.mc_tcp_port;

    SetLength(subscribeTopics, Length(Self.subscribeTopics));
    for i := 0 to High(Self.subscribeTopics) do begin
      subscribeTopics[i] := Self.subscribeTopics[i].Clone as TTopicInfoData;
    end;

    SetLength(publishTopics, Length(Self.publishTopics));
    for i := 0 to High(Self.publishTopics) do begin
      publishTopics[i] := Self.publishTopics[i].Clone as TTopicInfoData;
    end;

    SetLength(knownTypes, Length(Self.knownTypes));
    for i := 0 to High(Self.knownTypes) do begin
      knownTypes[i] := Self.knownTypes[i];
    end;
  end;
end;

procedure TParticipantInfoData.addTopic(var arr: TDynTopicInfoDataArray; top: TTopic);
var
  Len : Integer;
begin
  /// TODO ref count if same topic??
  Len := Length(arr);
  SetLength(arr, Len + 1);
  arr[Len] := TTopicInfoData.Create(top);
end;

procedure TParticipantInfoData.removeTopic(var arr: TDynTopicInfoDataArray; top: TTopic);
var
  Len, i, Found : Integer;
begin
  Found := -1;
  Len := Length(arr);
  for i := 0 to Len - 1 do begin
    if arr[i].Name = top.Name then begin
      Found := i;
      Break;
    end;
  end;
  if Found >= 0 then begin
    /// TODO ref count if same topic??
    FreeAndNil(arr[Found]);
    for i := Found+1 to Len - 1 do begin
      arr[i-1] := arr[i];
    end;
    SetLength(arr, Len - 1);
  end;
end;

function TParticipantInfoData.existTopic(var arr : TDynTopicInfoDataArray; topicName : AnsiString) : Boolean;
var
  Len, i : Integer;
begin
  Result := False;
  Len := Length(arr);
  for i := 0 to Len - 1 do begin
    if arr[i].Name = topicName then begin
      Result := True;
      Break;
    end;
  end;
end;

end.

