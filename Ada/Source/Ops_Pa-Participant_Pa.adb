--
-- Copyright (C) 2016-2025 Lennart Andersson.
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

with Ada.Containers.Indefinite_Ordered_Maps,
     Ada.Strings.Fixed;

with GNAT.OS_Lib;

with Ops_Pa.Socket_Pa;

with Ops_Pa.Error_Pa,
     Ops_Pa.SerializableFactory_Pa.CompFactory_Pa,
     Ops_Pa.PublisherAbs_Pa.Publisher_Pa;
use  Ops_Pa.Error_Pa,
     Ops_Pa.SerializableFactory_Pa.CompFactory_Pa,
     Ops_Pa.PublisherAbs_Pa.Publisher_Pa;

package body Ops_Pa.Participant_Pa is

  use type Ops_Pa.Signal_Pa.Event_T;

  -- ===========================================================================

  function Less (Left, Right : String) return Boolean;
  function Equal (Left, Right : Participant_Class_At) return Boolean;

  package MyMap is new Ada.Containers.Indefinite_Ordered_Maps(String, Participant_Class_At, Less, Equal);

  use type MyMap.cursor;

  function Less (Left, Right : String) return Boolean is
  begin
    return Left < Right;
  end;

  function Equal (Left, Right : Participant_Class_At) return Boolean is
  begin
    return Left = Right;
  end;

  -- By Singelton, one Participant per 'domainId + participantID'
  gInstances : MyMap.Map;
  gMutex : aliased Ops_Pa.Mutex_Pa.Mutex;

  -- Get a Participant instance
  function getInstance(domainID : String) return Participant_Class_At is
  begin
    return getInstance(domainID, "DEFAULT_PARTICIPANT");
  end;

  function getInstance(domainID : String; participantID : String) return Participant_Class_At is
  begin
    return getInstance(domainID, participantID, "");
  end;

  function getInstance(domainID : String; participantID : String; configFile : String) return Participant_Class_At is
    key : String := domainID & "::" & participantID;
    result : Participant_Class_At := null;
    pos : MyMap.Cursor;
    S : Ops_Pa.Mutex_Pa.Scope_Lock(gMutex'Access);
  begin
    pos := gInstances.Find( key );

    if pos = MyMap.No_Element then
      begin
        Result := Create(domainID, participantID, configFile);
        gInstances.Insert(key, Result);
      exception
        when EConfigException =>
          StaticErrorService.Report( "Participant", "Participant", "Configuration Error" );
        when others =>
          StaticErrorService.Report( "Participant", "Participant", "Unknown Exception" );
      end;
    else
      Result := MyMap.Element(pos);
    end if;
    return Result;
  end;

  procedure releaseInstance( part : Participant_Class_At ) is
    key : String := part.domainID.all & "::" & part.participantID.all;
    pos : MyMap.Cursor;
    S : Ops_Pa.Mutex_Pa.Scope_Lock(gMutex'Access);
  begin
    pos := gInstances.Find( key );

    if pos /= MyMap.No_Element then
      gInstances.Delete(pos);
    end if;

    Free(part);
  end;

  -- ===========================================================================

  -- Constructors
  function Create( domainID : String; participantID : String; configFile : String ) return Participant_Class_At is
    Self : Participant_Class_At := null;
  begin
    Self := new Participant_Class;
    InitInstance( Self.all, Self, domainID, participantID, configFile );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  procedure addTypeSupport( Self: in out Participant_Class; typeSupport : SerializableFactory_Class_At ) is
  begin
    Self.ObjectFactory.Add( typeSupport );
  end;

  overriding function getSendDataHandler( Self: in out Participant_Class; top : Topic_Class_At) return SendDataHandler_Class_At is
    Result : SendDataHandler_Class_At := null;
  begin
    Result := Self.SendDataHandlerFactory.getSendDataHandler(top);
    -- We can't update Participant Info here, delayed until updateSendPartInfo()
    return Result;
  end;

  overriding procedure updateSendPartInfo( Self: in out Participant_Class; top : Topic_Class_At ) is
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.PartInfoDataMutex'Access);
	begin
    -- Need to add topic to partInfoData.publishTopics
    addTopic(Self.PartInfoData.publishTopics, top);
	end;

  overriding procedure releaseSendDataHandler( Self: in out Participant_Class; top : Topic_Class_At ) is
  begin
    Self.SendDataHandlerFactory.releaseSendDataHandler(top);
    declare
      S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.PartInfoDataMutex'Access);
    begin
      removeTopic(Self.PartInfoData.publishTopics, top);
    end;
  end;

  -- Should only be used by Subscribers
  overriding function getReceiveDataHandler( Self : in out Participant_Class; top : Topic_Class_At) return ReceiveDataHandler_Class_At is
    Result : ReceiveDataHandler_Class_At := null;
  begin
    Result := Self.ReceiveDataHandlerFactory.getReceiveDataHandler( top, Self.Domain, SerializableInheritingTypeFactory_Class_At(Self.ObjectFactory) );
    if Result /= null then
      declare
        S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.PartInfoDataMutex'Access);
      begin
        -- Need to add topic to partInfoData.subscribeTopics
        addTopic(Self.PartInfoData.subscribeTopics, top);
      end;
    end if;
    return Result;
  end;

  overriding procedure releaseReceiveDataHandler( Self: in out Participant_Class; top : Topic_Class_At ) is
  begin
    Self.ReceiveDataHandlerFactory.releaseReceiveDataHandler( top );
    declare
      S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.PartInfoDataMutex'Access);
    begin
      removeTopic(Self.PartInfoData.subscribeTopics, top);
    end;
  end;

  function getErrorService( Self : in out Participant_Class ) return ErrorService_Class_At is
  begin
    return Self.ErrorService;
  end;

  overriding procedure ReportError( Self : in out Participant_Class; Error : Error_Class_At ) is
  begin
    Self.ErrorService.Report( Error );
  end;

  function getTopic( Self: Participant_Class; name : string) return Topic_Class_At is
    top : Topic_Class_At;
  begin
    top := Self.Domain.getTopic(name);
    if top /= null then
      top.SetParticipantID( Self.ParticipantID.all);
      top.SetDomainID( Self.DomainID.all );
    end if;
    return top;
  end;

  -- Create a Topic for subscribing or publishing on ParticipantInfoData
  function createParticipantInfoTopic( Self : Participant_Class ) return Topic_Class_At is
    Result : Topic_Class_At;
  begin
    -- ops::Topic infoTopic("ops.bit.ParticipantInfoTopic", 9494, "ops.ParticipantInfoData", domain->getDomainAddress());
    Result := Create("ops.bit.ParticipantInfoTopic", Self.Domain.MetaDataMcPort, "ops.ParticipantInfoData", Self.Domain.DomainAddress);
    Result.SetLocalInterface( Self.Domain.LocalInterface );
    Result.SetTimeToLive( Self.Domain.TimeToLive );
    Result.SetDomainID( Self.DomainID.all );
    Result.SetParticipantID( Self.ParticipantID.all );
    Result.SetTransport( TRANSPORT_MC );
    return Result;
  end;

  -- Returns a reference to the internal instance
  function getConfig( Self: Participant_Class ) return OPSConfig_Class_At is
  begin
    return Self.Config;
  end;

  -- Returns a reference to the internal instance
  function getDomain( Self: Participant_Class ) return Domain_Class_At is
  begin
    return Self.Domain;
  end;

  -- Get the name that this participant has set in its ParticipantInfoData
  function getPartInfoName( Self : Participant_Class ) return string is
  begin
    return Self.PartInfoData.name.all;
  end;

  task body Participant_Pr_T is
    Events : Ops_Pa.Signal_Pa.Event_T;
  begin
    select
      accept Start;
      while not Self.TerminateFlag loop
        begin
          Self.EventsToTask.WaitForAny(Events);
          exit when (Events and TerminateEvent_C) /= Ops_Pa.Signal_Pa.NoEvent_C;
          if (Events and StartEvent_C) /= Ops_Pa.Signal_Pa.NoEvent_C then
            Self.Run;
          end if;
        exception
          when others =>
            Self.ErrorService.Report( "Participant", "Part_Pr", "Got exception from Run()" );
        end;
      end loop;
      accept Finish;
    or
      accept Finish;
    end select;
  end Participant_Pr_T;

  procedure Run( Self : in out Participant_Class ) is
    partInfoTopic : Topic_Class_At := null;
    partInfoPub : Publisher_Class_At := null;

    function errMessage return String is
    begin
      if partInfoPub = null then
        return "Failed to create publisher for ParticipantInfoTopic. Check localInterface and metaDataMcPort in configuration file.";
      else
        return "Failed to publish ParticipantInfoTopic data.";
      end if;
    end;

  begin
    while not Self.TerminateFlag loop
      delay 1.0;
      exit when Self.TerminateFlag;

      -- Handle periodic publishing of metadata, i.e. Self.PartInfoData
      begin
        -- Create the meta data publisher if user hasn't disabled it for the domain.
        -- The meta data publisher is only necessary if we have topics using transport UDP.
        if partInfoPub = null and Self.Domain.MetaDataMcPort > 0 then
          partInfoTopic := Self.createParticipantInfoTopic;
          partInfoPub := Create(partInfoTopic);
        end if;
        if partInfoPub /= null then
          declare
            S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.PartInfoDataMutex'Access);
          begin
            partInfoPub.WriteOPSObject( Ops_Pa.OpsObject_Pa.OpsObject_Class_At(Self.PartInfoData) );
          end;
        end if;
      exception
        when others =>
          StaticErrorService.Report("Participant", "Run", errMessage);
      end;
    end loop;

    if partInfoPub /= null then
      Free(partInfoPub);
    end if;
    if partInfoTopic /= null then
      Free(partInfoTopic);
    end if;
  end;

  -- Called from ReceiveDataHandlerFactory when an UDP Reciver is created/deleted, so
  -- we can send the correct UDP transport info in the participant info data
  procedure OnUdpTransport( Self : in out Participant_Class; ipaddress : String; port : Int32 ) is
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.PartInfoDataMutex'Access);
  begin
    Replace(Self.PartInfoData.ip, ipaddress);
    Self.PartInfoData.mc_udp_port := port;
  end;

  -- Method prototype to call when we want to register a TCP RDH for the participant info data
  -- Override this to react on the TCP setup callback
  procedure OnTcpTransport( Self : in out Participant_Class;
                            topicName : String;
                            rdh : ReceiveDataHandler_Class_At;
                            register : Boolean ) is
  begin
    if Self.PartInfoListener /= null then
      if register then
        Self.PartInfoListener.connectTcp( topicName, rdh );
      else
        Self.PartInfoListener.disconnectTcp( topicName, rdh );
      end if;
    end if;
  end;

  -- Method prototype to call when we connect/disconnect UDP topics with the participant info data listener
  -- Override this to react on the UDP setup callback
  procedure OnUdpTransport( Self : in out Participant_Class;
                            topic : Topic_Class_At;
                            sdh : SendDataHandler_Class_At;
                            Connect : Boolean ) is
  begin
    -- FPartInfoListener is deleted before sendDataHandlerFactory (and it must be in that order
    -- since FPartInfoListener uses a subscriber that uses the sendDataHandlerFactory).
    -- Therefore we must check that it is assigned here
    if Self.PartInfoListener = null then
      return;
    end if;

    if Connect then
      Self.PartInfoListener.connectUdp(topic, sdh);
    else
      Self.PartInfoListener.disconnectUdp(topic, sdh);
    end if;
  end;

  procedure InitInstance( Self : in out Participant_Class;
                          SelfAt : Participant_Class_At;
                          domainID : String;
                          participantID : String;
                          configFile : String ) is
  begin
    Self.ParticipantID := Copy(participantID);
    Self.DomainID := Copy(domainID);
    Self.SelfAt := SelfAt;

    Self.ErrorService := Create;

    -- Read configFile
    if configFile = "" then
      -- This gets a reference to a singleton instance and should NOT be deleted.
      -- It may be shared between several Participants.
      Self.Config := getConfig;
    else
      -- This gets a reference to a unique instance and should eventually be deleted.
      -- Note however that the getDomain() call below returns a reference to an
      -- object internally in config.
      Self.Config := getConfig(configFile);
    end if;
    if Self.Config = null then
      StaticErrorService.Report( "Participant", "InitInstance", "getConfig() returned null" );
      raise EConfigException;
    end if;

    -- Get the domain from config. Note should not be deleted, owned by config.
    Self.Domain := Self.Config.getDomain(domainID);
    if Self.Domain = null then
      StaticErrorService.Report( "Participant", "InitInstance", "Domain '" & domainID & "' missing in config-file" );
      raise EConfigException;
    end if;

    -- Create a factory instance for each participant
    Self.ObjectFactory := OPSObjectFactory_Class_At(Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.OpsObjectFactory_Pa.Create);

    -- Initialize static data in partInfoData
    Self.PartInfoData := Create;
    Self.PartInfoData.name := Copy(Ops_Pa.Socket_Pa.GetHostName & " (" & Integer'Image(GNAT.OS_Lib.Pid_To_Integer(GNAT.OS_Lib.Current_Process_Id)) & ")");
    Self.PartInfoData.languageImplementation := Copy("Ada");
    Self.PartInfoData.id := Copy(participantID);
    Self.PartInfoData.domain := Copy(domainID);

    --
    Self.InProcDistributor := Create;

    Self.SendDataHandlerFactory := Create(Self.Domain, Ops_Pa.Transport_Pa.SendDataHandlerFactory_Pa.OnUdpTransport_Interface_At(Self.SelfAt), Self.ErrorService, Self.InProcDistributor );

    Self.ReceiveDataHandlerFactory := Create( Ops_Pa.Transport_Pa.ReceiveDataHandlerFactory_Pa.OnSetupTransport_Interface_At(Self.SelfAt), Self.ErrorService, Self.InProcDistributor );

    -- Partinfo topic / Listener
    Self.PartInfoTopic := Self.createParticipantInfoTopic;
    Self.PartInfoListener := Create(Self.Domain, Self.PartInfoTopic, Self.ErrorService);

    -- Start our thread
    Self.Part_Pr.Start;
    Self.EventsToTask.Signal(StartEvent_C);
  end;

  overriding procedure Finalize( Self : in out Participant_Class ) is
  begin
    Self.TerminateFlag := True;
    Self.EventsToTask.Signal(TerminateEvent_C);
    Self.Part_Pr.Finish;

    if Self.PartInfoListener /= null then
      Free(Self.PartInfoListener);    -- Must be done before send/receive factory delete below
    end if;

    if Self.PartInfoTopic /= null then
      Free(Self.PartInfoTopic);
    end if;

    if Self.ReceiveDataHandlerFactory /= null then
      Free(Self.ReceiveDataHandlerFactory);
    end if;

    if Self.SendDataHandlerFactory /= null then
      Free(Self.SendDataHandlerFactory);
    end if;

    if Self.InProcDistributor /= null then
      Free(Self.InProcDistributor);
    end if;

    if Self.PartInfoData /= null then
      Free(Self.PartInfoData);
    end if;

    if Self.ObjectFactory /= null then
      Free(Self.ObjectFactory);
    end if;

    if Self.ErrorService /= null then
      Free(Self.ErrorService);
    end if;

    if Self.ParticipantID /= null then
      Dispose(Self.ParticipantID);
    end if;
    if Self.DomainID /= null then
      Dispose(Self.DomainID);
    end if;
  end;

end Ops_Pa.Participant_Pa;
