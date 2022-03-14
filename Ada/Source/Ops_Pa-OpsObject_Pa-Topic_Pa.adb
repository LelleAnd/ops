--
-- Copyright (C) 2016-2021 Lennart Andersson.
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

with Ops_Pa.ArchiverInOut_Pa,
     Ops_Pa.Error_Pa;
use  Ops_Pa.ArchiverInOut_Pa,
     Ops_Pa.Error_Pa;

package body Ops_Pa.OpsObject_Pa.Topic_Pa is

  -- Constructors
  function Create(namee : String; portt : Int32; typeIDd : String; domainAddresss : String) return Topic_Class_At is
    Self : Topic_Class_At := null;
  begin
    Self := new Topic_Class;
    InitInstance( Self.all, namee, portt, typeIDd, domainAddresss );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  function Create return Topic_Class_At is
    Self : Topic_Class_At := null;
  begin
    Self := new Topic_Class;
    InitInstance( Self.all );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  -- Getters/Setters
  function Name( Self : Topic_Class ) return String is
  begin
    return Self.Name.all;
  end;

  function TypeID( Self : Topic_Class ) return String is
  begin
    return Self.TypeID.all;
  end;

  function DomainID( Self : Topic_Class ) return String is
  begin
    return Self.DomainID.all;
  end;

  procedure SetDomainID( Self : in out Topic_Class; Value : String ) is
  begin
    Replace(Self.DomainID, Value);
  end;

  function ParticipantID( Self : Topic_Class ) return String is
  begin
    return Self.ParticipantID.all;
  end;

  procedure SetParticipantID( Self : in out Topic_Class; Value : String ) is
  begin
    Replace(Self.ParticipantID, Value);
  end;

  function Transport( Self : Topic_Class ) return String is
  begin
    return Self.Transport.all;
  end;

  procedure SetTransport( Self : in out Topic_Class; Value : String ) is
  begin
    Replace(Self.Transport, Value);
  end;

  function DomainAddress( Self : Topic_Class ) return String is
  begin
    return Self.DomainAddress.all;
  end;

  procedure SetDomainAddress( Self : in out Topic_Class; Value : String ) is
  begin
    Replace(Self.DomainAddress, Value);
  end;

  function LocalInterface( Self : Topic_Class ) return String is
  begin
    return Self.LocalInterface.all;
  end;

  procedure SetLocalInterface( Self : in out Topic_Class; Value : String ) is
  begin
    Replace(Self.LocalInterface, Value);
  end;

  function SampleMaxSize( Self : Topic_Class ) return Int32 is
  begin
    return Self.SampleMaxSize;
  end;

  procedure SetSampleMaxSize( Self : in out Topic_Class; Value : Int32) is
  begin
    if Value < PACKET_MAX_SIZE then
      Self.SampleMaxSize := PACKET_MAX_SIZE;
    else
      Self.SampleMaxSize := Value;
    end if;
  end;

  function Port( Self : Topic_Class ) return Int32 is
  begin
    return Self.Port;
  end;

  procedure SetPort( Self : in out Topic_Class; Value : Int32) is
  begin
    Self.Port := Value;
  end;

  function TimeToLive( Self : Topic_Class ) return Int32 is
  begin
    return Self.TimeToLive;
  end;

  procedure SetTimeToLive( Self : in out Topic_Class; Value : Int32) is
  begin
    Self.TimeToLive := Value;
  end;

  function OutSocketBufferSize( Self : Topic_Class ) return Int64 is
  begin
    return Self.OutSocketBufferSize;
  end;

  procedure SetOutSocketBufferSize( Self : in out Topic_Class; Value : Int64) is
  begin
    Self.OutSocketBufferSize := Value;
  end;

  function InSocketBufferSize( Self : Topic_Class ) return Int64 is
  begin
    return Self.InSocketBufferSize;
  end;

  procedure SetInSocketBufferSize( Self : in out Topic_Class; Value : Int64) is
  begin
    Self.InSocketBufferSize := Value;
  end;

  function OptNonVirt( Self : Topic_Class ) return Boolean is
  begin
    return Self.OptNonVirt;
  end;

  procedure SetOptNonVirt( Self : in out Topic_Class; Value : Boolean) is
  begin
    Self.OptNonVirt := Value;
  end;

  function HeartbeatPeriod( Self : Topic_Class ) return Int32 is
  begin
    return Self.HeartbeatPeriod;
  end;

  procedure SetHeartbeatPeriod( Self : in out Topic_Class; Value : Int32) is
  begin
    Self.HeartbeatPeriod := Value;
  end;

  function HeartbeatTimeout( Self : Topic_Class ) return Int32 is
  begin
    return Self.HeartbeatTimeout;
  end;

  procedure SetHeartbeatTimeout( Self : in out Topic_Class; Value : Int32) is
  begin
    Self.HeartbeatTimeout := Value;
  end;

  function ChannelId( Self : Topic_Class ) return String is
  begin
    return Self.ChannelId.all;
  end;

  procedure SetChannelId( Self : in out Topic_Class; Value : String ) is
  begin
    Replace(Self.ChannelId, Value);
  end;

  function Topic_version( Self : Topic_Class ) return Byte is
  begin
    return Self.Topic_version;
  end;

  procedure SetTopic_version( Self : in out Topic_Class; Version : Byte ) is
  begin
    Self.Topic_version := Version;
  end;

  overriding procedure Serialize( Self : in out Topic_Class; archiver : ArchiverInOut_Class_At) is
    tSampleMaxSize : Int32;
  begin
    Serialize( OpsObject_Class(Self), archiver );
    if Self.IdlVersionMask /= 0 then
      archiver.inout("Topic_version", Self.Topic_Version);
      ValidateVersion("Topic", Self.Topic_version, Topic_idlVersion);
    else
      Self.Topic_Version := 0;
    end if;
    archiver.Inout("name", Self.Name);
    archiver.Inout("dataType", Self.TypeID);
    archiver.Inout("port", Self.Port);
    archiver.Inout("address", Self.DomainAddress);

    archiver.Inout("outSocketBufferSize", Self.OutSocketBufferSize);
    archiver.Inout("inSocketBufferSize", Self.InSocketBufferSize);

    -- Limit this value
    tSampleMaxSize := Self.SampleMaxSize;
    archiver.Inout("sampleMaxSize", tSampleMaxSize);
    SetSampleMaxSize( Self, tSampleMaxSize );

    archiver.Inout("transport", Self.Transport);
    if Self.Transport = null then
      Self.Transport := Copy(TRANSPORT_MC);

    elsif Self.Transport.all = "" then
      Replace(Self.Transport, TRANSPORT_MC);

    elsif (Self.Transport.all /= TRANSPORT_MC) and
      (Self.Transport.all /= TRANSPORT_TCP) and
      (Self.Transport.all /= TRANSPORT_UDP)
    then
      StaticErrorService.
        Report( "Domain", "CheckTransport",
                "Illegal transport: '" & Self.Transport.all &
                  "'. Transport for topic must be either 'multicast', 'tcp', 'udp' or left blank( = multicast)" );
      raise EConfigException;
    end if;
  end;

  -- Returns a newely allocated deep copy/clone of Self.
  overriding function Clone( Self : Topic_Class ) return OpsObject_Class_At is
    Result : Topic_Class_At := null;
  begin
    Result := Create;
    Self.FillClone( OpsObject_Class_At(Result) );
    return OpsObject_Class_At(Result);
  end Clone;

  -- Fills the parameter obj with all values from Self.
  overriding procedure FillClone( Self : Topic_Class; obj : OpsObject_Class_At ) is
  begin
    FillClone( OpsObject_Class(Self), obj );
    if obj.all in Topic_Class'Class then
      Topic_Class(obj.all).Topic_version := Self.Topic_version;
      Replace(Topic_Class(obj.all).Name, Self.Name);
      Topic_Class(obj.all).Port := Self.Port;
      Topic_Class(obj.all).TimeToLive := Self.TimeToLive;
      Replace(Topic_Class(obj.all).TypeID, Self.TypeID);
      Replace(Topic_Class(obj.all).DomainAddress, Self.DomainAddress);
      Replace(Topic_Class(obj.all).LocalInterface, Self.LocalInterface);
      Replace(Topic_Class(obj.all).ParticipantID, Self.ParticipantID);
      Replace(Topic_Class(obj.all).DomainID, Self.DomainID);
      Topic_Class(obj.all).SampleMaxSize := Self.SampleMaxSize;
      Topic_Class(obj.all).Deadline := Self.Deadline;
      Topic_Class(obj.all).MinSeparation := Self.MinSeparation;
      Replace(Topic_Class(obj.all).Transport, Self.Transport);
      Topic_Class(obj.all).OutSocketBufferSize := Self.OutSocketBufferSize;
      Topic_Class(obj.all).InSocketBufferSize := Self.InSocketBufferSize;
      Topic_Class(obj.all).HeartbeatPeriod := Self.HeartbeatPeriod;
      Topic_Class(obj.all).HeartbeatTimeout := Self.HeartbeatTimeout;
      Replace(Topic_Class(obj.all).ChannelId, Self.ChannelId);
    end if;
  end;

  procedure InitInstance( Self : in out Topic_Class; namee : String; portt : Int32; typeIDd : String; domainAddresss : String) is
  begin
    InitInstance( OpsObject_Class(Self) );
    AppendType( OpsObject_Class(Self), "Topic" );
    Self.Name := Copy(namee);
    Self.Port := portt;
    Self.TypeID := Copy(typeIDd);
    Self.DomainAddress := Copy(domainAddresss);
    Self.LocalInterface := Copy("");
    Self.DomainID := Copy("");
    Self.Transport := Copy("");
    Self.ChannelId := Copy("");
  end;

  procedure InitInstance( Self : in out Topic_Class ) is
  begin
    InitInstance( OpsObject_Class(Self) );
    AppendType( OpsObject_Class(Self), "Topic" );
    Self.Name := Copy("");
    Self.TypeID := Copy("");
    Self.DomainAddress := Copy("");
    Self.LocalInterface := Copy("");
    Self.DomainID := Copy("");
    Self.Transport := Copy("");
    Self.ChannelId := Copy("");
  end;

  overriding procedure Finalize( Self : in out Topic_Class ) is
  begin
    if Self.Name /= null then
      Dispose(Self.Name);
    end if;
    if Self.TypeID /= null then
      Dispose(Self.TypeID);
    end if;
    if Self.DomainAddress /= null then
      Dispose(Self.DomainAddress);
    end if;
    if Self.LocalInterface /= null then
      Dispose(Self.LocalInterface);
    end if;
    if Self.ParticipantID /= null then
      Dispose(Self.ParticipantID);
    end if;
    if Self.DomainID /= null then
      Dispose(Self.DomainID);
    end if;
    if Self.Transport /= null then
      Dispose(Self.Transport);
    end if;
    if Self.ChannelId /= null then
      Dispose(Self.ChannelId);
    end if;

    Finalize( OpsObject_Class(Self) );
  end;

end Ops_Pa.OpsObject_Pa.Topic_Pa;

