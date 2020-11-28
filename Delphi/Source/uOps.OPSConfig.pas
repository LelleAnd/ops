unit uOps.OPSConfig;

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

uses System.Generics.Collections,
     uOps.Types,
     uOps.Topic,
     uOps.Domain,
     uOps.OpsObject,
     uOps.ArchiverInOut;

type
	TOPSConfig = class(TOPSObject)
  public
    const
      OPSConfig_idlVersion : Byte = 0;
    type
      TDynDomainArray = array of TDomain;
	protected
    FOPSConfig_version : Byte;
    FDomains : TDynDomainArray;

	public
    constructor Create;
    destructor Destroy; override;

    // Returns a reference to a singleton instance and should NOT be deleted.
    // See also releaseConfig below
    class function getConfig : TOPSConfig; overload;

    // Returns a reference to a unique instance and should be deleted.
    // See also releaseConfig below
    class function getConfig(configFile : string) : TOPSConfig; overload;

    // Help method that deletes the config if it isn't the singleton instance
    // The variable used in the call will always be set to NIL
    class procedure releaseConfig(var cfg : TOPSConfig);

    // Returns a reference to the given Domain
    // NOTE: The OPSConfig still owns it
    function getDomain(domainID : string) : TDomain; virtual;

    // Returns references to the internal Domains
    // NOTE: The OPSConfig still owns them
    function getDomains : TDynDomainArray;

    procedure Serialize(archiver : TArchiverInOut); override;

		// Returns a newely allocated deep copy/clone of this object.
		function Clone : TOPSObject; override;

		// Fills the parameter obj with all values from this object.
		procedure FillClone(var obj : TOPSObject); override;

    property OPSConfig_version : Byte read FOPSConfig_version write FOPSConfig_version;
	end;

  TDefaultOPSConfigImpl = class(TOPSConfig)
  public
    const
      DefaultOPSConfigImpl_idlVersion : Byte = 0;
  protected
    FDefaultOPSConfigImpl_version : Byte;
  public
    constructor Create;
    procedure Serialize(archiver : TArchiverInOut); override;
    property DefaultOPSConfigImpl_version : Byte read FDefaultOPSConfigImpl_version write FDefaultOPSConfigImpl_version;
  end;

implementation

uses Classes,
     SysUtils,
     System.SyncObjs,
     uOps.Exceptions,
     uOps.OPSConfigRepository,
     uOps.OPSObjectFactory,
     uOps.XMLArchiverIn;

constructor TOPSConfig.Create;
begin
  inherited Create;
  FOPSConfig_version := OPSConfig_idlVersion;
end;

destructor TOPSConfig.Destroy;
var
  i : Integer;
begin
  // Delete all domains
  for i := 0 to Length(FDomains)-1 do FreeAndNil(FDomains[i]);
  inherited;
end;

function TOPSConfig.getDomain(domainID : string) : TDomain;
var
  i : Integer;
begin
  Result := nil;
  for i := 0 to Length(FDomains) - 1 do begin
    if string(FDomains[i].DomainID) = domainID then begin
      Result := FDomains[i];
      Break;
    end;
  end;
end;

function TOPSConfig.getDomains : TDynDomainArray;
begin
  Result := Copy(FDomains);
end;

procedure TOPSConfig.Serialize(archiver : TArchiverInOut);
begin
  inherited Serialize(archiver);
  if FIdlVersionMask <> 0 then begin
    archiver.inout('OPSConfig_version', FOPSConfig_version);
    if FOPSConfig_version > OPSConfig_idlVersion then begin
      raise EIdlVersionException.Create('OPSConfig', FOPSConfig_version, OPSConfig_idlVersion);
    end;
  end else begin
    FOPSConfig_version := 0;
  end;

	archiver.inout('domains', TDynSerializableArray(FDomains));
end;

// Returns a newely allocated deep copy/clone of this object.
function TOPSConfig.Clone : TOPSObject;
begin
	Result := TOPSConfig.Create;
  Self.FillClone(Result);
end;

// Fills the parameter obj with all values from this object.
procedure TOPSConfig.FillClone(var obj : TOPSObject);
var
  i : Integer;
begin
	inherited FillClone(obj);
  with obj as TOPSConfig do begin
    FOPSConfig_version := Self.FOPSConfig_version;
    SetLength(FDomains, Length(Self.FDomains));
    for i := 0 to High(Self.FDomains) do begin
      FDomains[i] := Self.FDomains[i].Clone as TDomain;
    end;
  end;
end;

// ---------------------------------------------------------------------------

constructor TDefaultOPSConfigImpl.Create;
begin
  inherited Create;
  AppendType('DefaultOPSConfigImpl');
  FDefaultOPSConfigImpl_version := DefaultOPSConfigImpl_idlVersion;
end;

procedure TDefaultOPSConfigImpl.Serialize(archiver : TArchiverInOut);
begin
  inherited Serialize(archiver);
  if FIdlVersionMask <> 0 then begin
    archiver.inout('DefaultOPSConfigImpl_version', FDefaultOPSConfigImpl_version);
    if FDefaultOPSConfigImpl_version > DefaultOPSConfigImpl_idlVersion then begin
      raise EIdlVersionException.Create('DefaultOPSConfigImpl',
              FDefaultOPSConfigImpl_version, DefaultOPSConfigImpl_idlVersion);
    end;
  end else begin
    FDefaultOPSConfigImpl_version := 0;
  end;
end;

// ---------------------------------------------------------------------------

var
  gConfiguration : TOPSConfig;
  gMutex : TMutex;

class function TOPSConfig.getConfig(configFile : string) : TOPSConfig;
var
  archiver : TXMLArchiverIn;
  list : TStringList;
begin
  Result := nil;

  list := TStringList.Create;
  try
    list.LoadFromFile(configFile);
    archiver := nil;
    try
      archiver := TXMLArchiverIn.Create(list.Text, 'root', TOPSObjectFactory.getInstance);
      Result := TOPSConfig(archiver.inout2('ops_config', TSerializable(Result)));
    finally
      FreeAndNil(archiver);
    end;
  finally
    FreeAndNil(list);
  end;
end;

class function TOPSConfig.getConfig : TOPSConfig;
begin
  gMutex.Acquire;
  try
  	if not Assigned(gConfiguration) then begin
      gConfiguration := TOPSConfigRepository.Instance.getConfig();
    end;
  finally
    gMutex.Release;
  end;
  Result := gConfiguration;
end;

class procedure TOPSConfig.releaseConfig(var cfg : TOPSConfig);
begin
  // We don't want to delete the singleton instance
  if cfg <> gConfiguration then begin
    FreeAndNil(cfg);
  end;
  cfg := nil;
end;

initialization
  gConfiguration := nil;
  gMutex := TMutex.Create;

finalization
  //Don't delete config (gConfiguration) since we got it from the TOPSConfigRepository who will delete it
  //FreeAndNil(gConfiguration);
  FreeAndNil(gMutex);

end.

