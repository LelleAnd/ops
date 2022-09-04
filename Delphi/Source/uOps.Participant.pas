unit uOps.Participant;

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

uses System.Generics.Collections,
     System.SyncObjs,
     uNotifier,
     uRunner,
     uOps.Types,
     uOps.Topic,
     uOps.Domain,
     uOps.Transport.SendDataHandler,
     uOps.Transport.SendDataHandlerFactory,
     uOps.Transport.ReceiveDataHandler,
     uOps.Transport.ReceiveDataHandlerFactory,
     uOps.SerializableFactory,
     uOps.OPSObjectFactory,
     uOps.Error,
     uOps.OPSConfig,
     uOps.ParticipantInfoData,
     uOps.ParticipantInfoDataListener,
     uOps.PublisherAbs,
     uOps.OpsObject,
     uOps.OpsMessage;

type
	TParticipant = class(TObject)
  public
    class function getInstance(domainID : string) : TParticipant; overload;
    class function getInstance(domainID : string; participantID : string) : TParticipant; overload;
    class function getInstance(domainID : string; participantID : string; configFile : string) : TParticipant; overload;

    // Get the name that this participant has set in its ParticipantInfoData
    function getPartInfoName : string;

    // Add a SerializableFactory which has support for data types (i.e. OPSObject derivatives you want this Participant to understand)
    // Takes over ownership of the object and it will be deleted with the participant
    procedure addTypeSupport(typeSupport : TSerializableFactory);

    // Get a Topic from the ops config. See config below.
    // Returns a reference to the internal storage
    function getTopic(name : string) : TTopic;

    // Make this participant report an Error, which will be delivered to all ErrorService listeners
    // Takes ownership over given TError instance
    procedure ReportError(error : TError);

    // Returns a reference to the internal instance
    function getConfig : TOPSConfig;

    // Return a reference to the internal ErrorService instance
    //
    // Add a listener to the service to get error reports from the participant and its objects
    // Prototype for listener is:
    //    procedure <someobject>.OnErrorReport(Sender : TObject; Error : TError);
    //
    // Note that several threads at a time can report errors so take that into account
    function getErrorService : TErrorService;

    // Returns a reference to the internal instance
    function getDomain : TDomain;

    // Get a reference to the data type factory used by this Participant.
    function getObjectFactory : TOPSObjectFactory;

    destructor Destroy; override;

		// Should only be used by Publishers
		function getSendDataHandler(top : TTopic) : TSendDataHandler;
    procedure updateSendPartInfo(top : TTopic);
		procedure releaseSendDataHandler(top : TTopic);

    // Should only be used by Subscribers
    function getReceiveDataHandler(top : TTopic) : TReceiveDataHandler;
    procedure releaseReceiveDataHandler(top : TTopic);

  private
    // The id of this participant, must be unique in process
    FParticipantID : string;

    // The domainID for this Participant
    FDomainID : string;

    // Objects on loan from OPSConfig
    FConfig : TOPSConfig;
    FDomain : TDomain;

    // The ErrorService
    FErrorService : TErrorService;

    //------------------------------------------------------------------------
    // The ParticipantInfoData that partInfoPub will publish periodically
    FPartInfoData : TParticipantInfoData;
    FPartInfoDataMutex : TMutex;

    //------------------------------------------------------------------------
    // A listener and handler for ParticipantInfoData
    FPartInfoListener : TParticipantInfoDataListener;
    FPartInfoTopic : TTopic;

    //------------------------------------------------------------------------
    //
    FReceiveDataHandlerFactory : TReceiveDataHandlerFactory;
    FSendDataHandlerFactory : TSendDataHandlerFactory;

    // The interval with which this Participant publishes ParticipantInfoData
    FAliveTimeout : Int64;

    // The data type factory used in this Participant.
    FObjectFactory : TOPSObjectFactory;

    // Our thread running our Run() method
    FRunner : TRunner;
    FTerminated : Boolean;
    procedure Run;

    // Constructor is private, instances are acquired through getInstance()
    constructor Create(domainID : string; participantID : string; configFile : string);

    // Called from SendDataHandlerFactory when UDP topics should be connected/disconnected
    // to/from the participant info data listener
    procedure OnUdpConnectDisconnectProc(top : TTopic; sdh : TSendDataHandler; connect : Boolean);

    // Called from ReceiveDataHandlerFactory when a TCP Reciver starts/stop to use a specific topic, so
    // we can connect TCP transports
    procedure OnTcpConnectDisconnectProc(top : TTopic; rdh : TReceiveDataHandler; connect : Boolean);

    // Called from ReceiveDataHandlerFactory when an UDP Reciver is created/deleted, so
    // we can send the correct UDP transport info in the participant info data
    procedure OnUdpTransportInfoProc(ipaddress : string; port : Integer);

    function HasPublisherOn(topicName : AnsiString) : Boolean;

    // Create a Topic for subscribing or publishing on ParticipantInfoData
    function createParticipantInfoTopic : TTopic;

    // Remove this instance from the static instance map
    procedure RemoveInstance;

  end;

implementation

uses
  Windows,
  SysUtils,
  WinSock,
  uOps.Exceptions,
  uOps.Publisher;

var
  // By Singelton, one Participant per 'domainId + participantID'
  gInstances : TDictionary<string,TParticipant>;
  gMutex : TMutex;

// -----------------------------------------------------------------------------

class function TParticipant.getInstance(domainID : string) : TParticipant;
begin
	Result := TParticipant.getInstance(domainID, 'DEFAULT_PARTICIPANT');
end;

class function TParticipant.getInstance(domainID : string; participantID : string) : TParticipant;
begin
	Result := TParticipant.getInstance(domainID, participantID, '');
end;

class function TParticipant.getInstance(domainID : string; participantID : string; configFile : string) : TParticipant;
var
  key : string;
begin
  Result := nil;
  gMutex.Acquire;
  try
    key := domainID + '::' + participantID;
    if not gInstances.ContainsKey(key) then begin
      try
        Result := TParticipant.Create(domainID, participantID, configFile);
        gInstances.Add(key, Result);
      except
        on ex : Exception do begin
          uOps.Error.gStaticErrorService.Report(
            TBasicError.Create('Participant', 'Participant', ex.ToString));
        end;
        else begin
          uOps.Error.gStaticErrorService.Report(
            TBasicError.Create('Participant', 'Participant', 'Unknown Exception'));
        end;
      end;
    end else begin
      Result := gInstances.Items[key];
    end;
  finally
    gMutex.Release;
  end;
end;

// Remove this instance from the static instance map
procedure TParticipant.RemoveInstance;
var
  key : string;
begin
  gMutex.Acquire;
  try
    key := FDomainID + '::' + FParticipantID;
    gInstances.ExtractPair(key);
  finally
    gMutex.Release;
  end;
end;

// -----------------------------------------------------------------------------

// Make this participant report an Error, which will be delivered to all ErrorService listeners
// Takes ownership over given TError instance
procedure TParticipant.ReportError(error : TError);
begin
  // Gives ownership to error service
  FErrorService.Report(error);
end;

// -----------------------------------------------------------------------------

// Constructor is private, instances are acquired through getInstance()
constructor TParticipant.Create(domainID : string; participantID : string; configFile : string);
var
  hname: array[0..1023] of AnsiChar;
begin
  inherited Create;
  FDomainID := domainID;
  FParticipantID := participantID;
  FAliveTimeout := 1000;

  FErrorService := TErrorService.Create;

  // Read configFile
  if configFile = '' then begin
    // This gets a reference to a singleton instance and should NOT be deleted.
    // It may be shared between several Participants.
    FConfig := TOPSConfig.getConfig;
  end else begin
    // This gets a reference to a unique instance and should eventually be deleted.
    // Note however that the getDomain() call below returns a reference to an
    // object internally in config.
    FConfig := TOPSConfig.getConfig(configFile);
  end;
  if not Assigned(FConfig) then begin
    raise ECommException.Create('No config on rundirectory');
  end;

  // Get the domain from config. Note should not be deleted, owned by config.
  FDomain := FConfig.getDomain(domainID);
  if not Assigned(FDomain) then begin
    raise ECommException.Create('Domain "' + domainID + '" missing in config-file');
  end;

  // Create a factory instance for each participant
  FObjectFactory := TOPSObjectFactoryImpl.Create;

  // Initialize static data in partInfoData
  FPartInfoData := TParticipantInfoData.Create;
  FPartInfoDataMutex := TMutex.Create;

  hname[0] := #0;
  gethostname(hname, sizeof(hname));
  FPartInfoData.name := AnsiString(hname) + '(' + AnsiString(IntToStr(GetCurrentProcessId)) + ')';
  FPartInfoData.languageImplementation := 'Delphi';
  FPartInfoData.id := AnsiString(FParticipantID);
  FPartInfoData.domain := AnsiString(FDomainID);

  FSendDataHandlerFactory := TSendDataHandlerFactory.Create(FDomain, OnUdpConnectDisconnectProc, FErrorService);
  FReceiveDataHandlerFactory := TReceiveDataHandlerFactory.Create(OnUdpTransportInfoProc, OnTcpConnectDisconnectProc, FErrorService);

  FPartInfoTopic := createParticipantInfoTopic;
  FPartInfoListener := TParticipantInfoDataListener.Create(FDomain, FPartInfoTopic, HasPublisherOn, FErrorService);

  // Start a thread running our run() method
  FRunner := TRunner.Create(Run);
end;

destructor TParticipant.Destroy;
begin
  // Make sure we are removed from gInstances
  RemoveInstance;

  // Tell thread to terminate
  FTerminated := True;
  if Assigned(FRunner) then FRunner.Terminate;
  // If thread exist, wait for thread to terminate and then delete the object
  FreeAndNil(FRunner);

  FreeAndNil(FPartInfoListener);    // Must be done before send/receive factory delete below
  FreeAndNil(FPartInfoTopic);
  FreeAndNil(FReceiveDataHandlerFactory);
  FreeAndNil(FSendDataHandlerFactory);

  FreeAndNil(FPartInfoData);
  FreeAndNil(FPartInfoDataMutex);

  FreeAndNil(FObjectFactory);

  TOPSConfig.releaseConfig(FConfig);   // Free's both FConfig and FDomain
  FDomain := nil;

  FreeAndNil(FErrorService);
  inherited;
end;

// -----------------------------------------------------------------------------

function TParticipant.HasPublisherOn(topicName : AnsiString) : Boolean;
begin
  FPartInfoDataMutex.Acquire;
  try
    Result := FPartInfoData.existTopic(FPartInfoData.publishTopics, topicName);
  finally
    FPartInfoDataMutex.Release;
  end;
end;

function TParticipant.getSendDataHandler(top : TTopic) : TSendDataHandler;
begin
  Result := FSendDataHandlerFactory.getSendDataHandler(top);
  // We can't update Participant Info here, delayed until updateSendPartInfo()
end;

procedure TParticipant.updateSendPartInfo(top : TTopic);
begin
  FPartInfoDataMutex.Acquire;
  try
    // Need to add topic to partInfoData.publishTopics
    FPartInfoData.addTopic(FPartInfoData.publishTopics, top);
  finally
    FPartInfoDataMutex.Release;
  end;
end;

procedure TParticipant.releaseSendDataHandler(top : TTopic);
begin
  FSendDataHandlerFactory.releaseSendDataHandler(top);

  FPartInfoDataMutex.Acquire;
  try
    // Remove topic from partInfoData.publishTopics
    FPartInfoData.removeTopic(FPartInfoData.publishTopics, top);
  finally
    FPartInfoDataMutex.Release;
  end;
end;

function TParticipant.getReceiveDataHandler(top : TTopic) : TReceiveDataHandler;
begin
  Result := FReceiveDataHandlerFactory.getReceiveDataHandler(top, FDomain, FObjectFactory);

  if Assigned(Result) then begin
    FPartInfoDataMutex.Acquire;
    try
      // Need to add topic to partInfoData.subscribeTopics
      FPartInfoData.addTopic(FPartInfoData.subscribeTopics, top);
    finally
      FPartInfoDataMutex.Release;
    end;
  end;
end;

procedure TParticipant.releaseReceiveDataHandler(top : TTopic);
begin
  FReceiveDataHandlerFactory.releaseReceiveDataHandler(top);

  FPartInfoDataMutex.Acquire;
  try
    // Remove topic from partInfoData.subscribeTopics
    FPartInfoData.removeTopic(FPartInfoData.subscribeTopics, top);
  finally
    FPartInfoDataMutex.Release;
  end;
end;

// -----------------------------------------------------------------------------

function TParticipant.getObjectFactory : TOPSObjectFactory;
begin
  Result := FObjectFactory;
end;

// Add a SerializableFactory which has support for data types (i.e. OPSObject
// derivatives you want this Participant to understand)
// Takes over ownership of the object and it will be deleted with the participant
procedure TParticipant.addTypeSupport(typeSupport : TSerializableFactory);
begin
  FObjectFactory.Add(typeSupport);
end;

// Get a Topic from the ops config
function TParticipant.getTopic(name : string) : TTopic;
begin
  Result := FDomain.getTopic(AnsiString(name));
  Result.ParticipantID := FParticipantID;
  Result.DomainID := FDomainID;
end;

// Create a Topic for subscribing or publishing on ParticipantInfoData
function TParticipant.createParticipantInfoTopic : TTopic;
begin
  // ops::Topic infoTopic("ops.bit.ParticipantInfoTopic", 9494, "ops.ParticipantInfoData", domain->getDomainAddress());
  Result := TTopic.Create('ops.bit.ParticipantInfoTopic', FDomain.MetaDataMcPort, 'ops.ParticipantInfoData', FDomain.DomainAddress);
  Result.LocalInterface := FDomain.LocalInterface;
  Result.TimeToLive := FDomain.TimeToLive;
  Result.DomainID := FDomainID;
  Result.ParticipantID := FParticipantID;
  Result.Transport := TTopic.TRANSPORT_MC;
end;

// Get the name that this participant has set in its ParticipantInfoData
function TParticipant.getPartInfoName : string;
begin
  Result := string(FPartInfoData.name);
end;

// -----------------------------------------------------------------------------

// Called from SendDataHandlerFactory when UDP topics should be connected/disconnected
// to/from the participant info data listener
procedure TParticipant.OnUdpConnectDisconnectProc(top : TTopic; sdh : TSendDataHandler; connect : Boolean);
begin
  // FPartInfoListener is deleted before sendDataHandlerFactory (and it must be in that order
  // since FPartInfoListener uses a subscriber that uses the sendDataHandlerFactory).
  // Therefore we must check that it is assigned here
  if not Assigned(FPartInfoListener) then Exit;

  if connect then begin
    FPartInfoListener.connectUdp(top, sdh);
  end else begin
    FPartInfoListener.disconnectUdp(top, sdh);
  end;
end;

procedure TParticipant.OnTcpConnectDisconnectProc(top : TTopic; rdh : TReceiveDataHandler; connect : Boolean);
begin
  if not Assigned(FPartInfoListener) then Exit;

  if connect then begin
    FPartInfoListener.connectTcp(top, rdh);
  end else begin
    FPartInfoListener.disconnectTcp(top, rdh);
  end;
end;

// Called from ReceiveDataHandlerFactory when an UDP Reciver is created/deleted, so
// we can send the correct UDP transport info in the participant info data
procedure TParticipant.OnUdpTransportInfoProc(ipaddress : string; port : Integer);
begin
  FPartInfoDataMutex.Acquire;
  try
    FPartInfoData.ip := AnsiString(ipaddress);
    FPartInfoData.mc_udp_port := port;
  finally
    FPartInfoDataMutex.Release;
  end;
end;

// -----------------------------------------------------------------------------

function TParticipant.getDomain : TDomain;
begin
  Result := FDomain;
end;

function TParticipant.getConfig : TOPSConfig;
begin
  Result := FConfig;
end;

function TParticipant.getErrorService : TErrorService;
begin
  Result := FErrorService;
end;

(**************************************************************************
*
**************************************************************************)
procedure TParticipant.Run;
var
  errMessage : string;
  partInfoTopic : TTopic;
  // A publisher of ParticipantInfoData
  partInfoPub : TPublisher;
begin
  partInfoTopic := nil;
  partInfoPub := nil;
  while not FTerminated do begin
    Sleep(FAliveTimeout);
    if FTerminated then Break;

    // Handle periodic publishing of metadata, i.e. FPartInfoData
    try
      // Create the meta data publisher if user hasn't disabled it for the domain.
      // The meta data publisher is only necessary if we have topics using transport UDP.
      if (not Assigned(partInfoPub)) and (FDomain.MetaDataMcPort > 0) then begin
        partInfoTopic := createParticipantInfoTopic;
        partInfoPub := TPublisher.Create(partInfoTopic);
      end;
      if Assigned(partInfoPub) then begin
        FPartInfoDataMutex.Acquire;
        try
          partInfoPub.WriteOPSObject(FPartInfoData);
        finally
          FPartInfoDataMutex.Release;
        end;
      end;

    except
      on E: Exception do begin
				if not Assigned(partInfoPub) then begin
					errMessage := 'Failed to create publisher for ParticipantInfoTopic. Check localInterface and metaDataMcPort in configuration file.';
				end else begin
					errMessage := 'Failed to publish ParticipantInfoTopic data.';
				end;
				uOps.Error.gStaticErrorService.Report(TBasicError.Create('Participant', 'Run', errMessage));
			end;
    end;
  end;

  FreeAndNil(partInfoPub);
  FreeAndNil(partInfoTopic);
end;

initialization
  gInstances := TDictionary<string,TParticipant>.Create;
  gMutex := TMutex.Create;

finalization
  FreeAndNil(gMutex);
  FreeAndNil(gInstances);

end.

