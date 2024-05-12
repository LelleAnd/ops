unit uOps.Channel;

(**
*
* Copyright (C) 2016-2024 Lennart Andersson.
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
     uOps.OPSObject,
     uOps.Topic;

type
  TChannel = class(TOPSObject)
  public
    const
      Channel_idlVersion : Byte = 0;
      LINKTYPE_MC = 'multicast';
      LINKTYPE_TCP = 'tcp';
      LINKTYPE_UDP = 'udp';

  public
    Channel_version : Byte;
    channelID : AnsiString;
    linktype : AnsiString;
    localInterface : AnsiString;     // If multicast, this specifies interface to use
    domainAddress : AnsiString;
    timeToLive : Integer;                 // if multicast, this specifies the ttl parameter
    port : Integer;
    outSocketBufferSize : Int64;
    inSocketBufferSize : Int64;

    constructor Create;

    procedure Serialize(archiver : TArchiverInOut); override;

		// Returns a newely allocated deep copy/clone of this object.
		function Clone : TOPSObject; override;

		// Fills the parameter obj with all values from this object.
		procedure FillClone(var obj : TOPSObject); override;

    procedure PopulateTopic(top : TTopic);
  end;

implementation

uses SysUtils,
     uOps.NetworkSupport,
     uOps.Exceptions;

{ TChannel }

constructor TChannel.Create;
begin
  inherited;
  Channel_version := Channel_idlVersion;
  timeToLive := -1;
  outSocketBufferSize := -1;
  inSocketBufferSize := -1;
  AppendType('Channel');
end;

procedure TChannel.Serialize(archiver: TArchiverInOut);
begin
  inherited Serialize(archiver);
  if FIdlVersionMask <> 0 then begin
    archiver.inout('Channel_version', Channel_version);
    if Channel_version > Channel_idlVersion then begin
      raise EIdlVersionException.Create('Channel', Channel_version, Channel_idlVersion);
    end;
  end else begin
    Channel_version := 0;
  end;
  archiver.inout('name', channelID);
  archiver.inout('linktype', linktype);
  archiver.inout('localInterface', localInterface);
  archiver.inout('address', domainAddress);
  archiver.inout('timeToLive', timeToLive);
  archiver.inout('port', port);
  archiver.inout('outSocketBufferSize', outSocketBufferSize);
  archiver.inout('inSocketBufferSize', inSocketBufferSize);

  localInterface := GetHostAddressEx(localInterface);

  if (linktype = '') or (linktype = LINKTYPE_MC) then begin
    linktype := LINKTYPE_MC;

  end else if (linktype = LINKTYPE_TCP) or (linktype = LINKTYPE_UDP) then begin
    if Length(domainAddress) > 0 then begin
      domainAddress := GetHostAddress(domainAddress);
    end;

  end else begin
    raise EConfigException.Create(
            'Illegal linktype: "' + string(linktype) +
            '". Linktype for Channel must be either "multicast", "tcp", "udp" or left blank( = multicast)');
  end;
end;

function TChannel.Clone: TOPSObject;
begin
	Result := TChannel.Create;
  Self.FillClone(Result);
end;

procedure TChannel.FillClone(var obj: TOPSObject);
begin
	inherited FillClone(obj);
  with obj as TChannel do begin
    Channel_version := Self.Channel_version;
    channelID := Self.channelID;
    linktype := Self.linktype;
		LocalInterface := Self.LocalInterface;
		DomainAddress := Self.DomainAddress;
		TimeToLive := Self.TimeToLive;
    Port := Self.port;
		InSocketBufferSize := Self.InSocketBufferSize;
		OutSocketBufferSize := Self.OutSocketBufferSize;
  end;
end;

procedure TChannel.PopulateTopic(top: TTopic);
begin
  // If Topic doesn't specify a transport it will default to 'multicast', therefore
  // we can't just check for an empty 'top.Transport' to know when to replace value.
  // Therfore, if a topic is listed in a 'Transport/Channel', we assume it shall
  // use the channel values, so replace all values.
  top.Transport := linktype;
  top.LocalInterface := localInterface;
  top.DomainAddress := domainAddress;
  top.Port := port;
  top.OutSocketBufferSize := outSocketBufferSize;
  top.InSocketBufferSize := inSocketBufferSize;
  top.TimeToLive := timeToLive;
  top.ChannelID := string(channelID);
end;

end.

