--
-- Copyright (C) 2016-2024 Lennart Andersson.
--
-- This file is part of OPS (Open Publish Subscribe).
--
-- OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Lesser General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public License
-- along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.

with Ops_Pa.Error_Pa,
     Ops_Pa.Socket_Pa;
use  Ops_Pa.Error_Pa;

package body Ops_Pa.OpsObject_Pa.Channel_Pa is

  -- Constructors
  function Create return Channel_Class_At is
    Self : Channel_Class_At := null;
  begin
    Self := new Channel_Class;
    InitInstance( Self.all );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  overriding procedure Serialize( Self : in out Channel_Class; archiver : ArchiverInOut_Class_At) is
  begin
    Serialize( OpsObject_Class(Self), archiver );
    if Self.IdlVersionMask /= 0 then
      archiver.inout("Channel_version", Self.Channel_Version);
      ValidateVersion("Channel", Self.Channel_version, Channel_idlVersion);
    else
      Self.Channel_Version := 0;
    end if;
    archiver.Inout("name", Self.ChannelID);
    archiver.Inout("linktype", Self.Linktype);
    archiver.Inout("localInterface", Self.LocalInterface);
    archiver.Inout("address", Self.DomainAddress);
    archiver.Inout("timeToLive", Self.TimeToLive);
    archiver.Inout("port", Self.Port);
    archiver.Inout("outSocketBufferSize", Self.OutSocketBufferSize);
    archiver.Inout("inSocketBufferSize", Self.InSocketBufferSize);

    Replace(Self.LocalInterface, Ops_Pa.Socket_Pa.GetHostAddressEx(Self.LocalInterface.all));

    if Self.Linktype = null then
      Self.linktype := Copy(LINKTYPE_MC);

    elsif (Self.Linktype.all = "") or (Self.linktype.all = LINKTYPE_MC) then
      Replace(Self.linktype, LINKTYPE_MC);

    elsif (Self.Linktype.all = LINKTYPE_TCP) or (Self.Linktype.all = LINKTYPE_UDP)
    then
      if Self.DomainAddress'Length > 0 then
        Replace(Self.DomainAddress, Ops_Pa.Socket_Pa.GetHostAddress(Self.DomainAddress.all));
      end if;

    else
      StaticErrorService.
        Report( "Domain", "CheckTransport",
                "Illegal linktype: '" & Self.Linktype.all &
                  "'. Linktype for Channel must be either 'multicast', 'tcp', 'udp' or left blank( = multicast)" );
      raise EConfigException;
    end if;
  end;

  -- Returns a newely allocated deep copy/clone of Self.
  overriding function Clone( Self : Channel_Class ) return OpsObject_Class_At is
    Result : Channel_Class_At := null;
  begin
    Result := Create;
    Self.FillClone( OpsObject_Class_At(Result) );
    return OpsObject_Class_At(Result);
  end Clone;

  -- Fills the parameter obj with all values from Self.
  overriding procedure FillClone( Self : Channel_Class; obj : OpsObject_Class_At ) is
  begin
    FillClone( OpsObject_Class(Self), obj );
    if obj.all in Channel_Class'Class then
      Channel_Class(obj.all).Channel_version := Self.Channel_version;
      Replace(Channel_Class(obj.all).ChannelID, Self.ChannelID);
      Replace(Channel_Class(obj.all).Linktype, Self.Linktype);
      Replace(Channel_Class(obj.all).LocalInterface, Self.LocalInterface);
      Replace(Channel_Class(obj.all).DomainAddress, Self.DomainAddress);
      Channel_Class(obj.all).TimeToLive := Self.TimeToLive;
      Channel_Class(obj.all).Port := Self.Port;
      Channel_Class(obj.all).OutSocketBufferSize := Self.OutSocketBufferSize;
      Channel_Class(obj.all).InSocketBufferSize := Self.InSocketBufferSize;
    end if;
  end;

  procedure InitInstance( Self : in out Channel_Class ) is
  begin
    InitInstance( OpsObject_Class(Self) );
    AppendType( OpsObject_Class(Self), "Channel" );
  end;

  overriding procedure Finalize( Self : in out Channel_Class ) is
  begin
    if Self.ChannelID /= null then
      Dispose(Self.ChannelID);
    end if;
    if Self.Linktype /= null then
      Dispose(Self.Linktype);
    end if;
    if Self.LocalInterface /= null then
      Dispose(Self.LocalInterface);
    end if;
    if Self.DomainAddress /= null then
      Dispose(Self.DomainAddress);
    end if;

    Finalize( OpsObject_Class(Self) );
  end;

  procedure PopulateTopic( Self : in out Channel_Class; top : in out Topic_Class_At ) is
  begin
    -- If Topic doesn't specify a transport it will default to 'multicast', therefore
    -- we can't just check for an empty 'top.Transport' to know when to replace value.
    -- Therfore, if a topic is listed in a 'Transport/Channel', we assume it shall
    -- use the channel values, so replace all values.
    top.SetTransport( Self.Linktype.all );
    top.SetLocalInterface( Self.LocalInterface.all );
    top.SetDomainAddress( Self.DomainAddress.all );
    top.SetPort( Self.Port );
    top.SetOutSocketBufferSize( Self.OutSocketBufferSize );
    top.SetInSocketBufferSize( Self.InSocketBufferSize );
    top.SetTimeToLive( Self.TimeToLive );
    top.SetChannelId( Self.ChannelID.all );
  end;

end Ops_Pa.OpsObject_Pa.Channel_Pa;

