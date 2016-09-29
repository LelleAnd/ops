unit uOps.Publisher;

(**
*
* Copyright (C) 2016 Lennart Andersson.
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

uses uOps.Types,
     uOps.Topic,
     uOps.ByteBuffer,
     uOps.MemoryMap,
     uOps.OPSArchiverOut,
     uOps.Participant,
     uOps.Transport.SendDataHandler,
     uOps.OpsObject,
     uOps.OpsMessage,
     uOps.PublisherAbs;

type
  TPublisher = class(TPublisherAbs)
  protected
    FCurrentPublicationID : UInt64;

    FMemMap : TMemoryMap;
    FBuf : TByteBuffer;
    FArchive : TOPSArchiverOut;
    FMessage : TOPSMessage;

    // The Participant to which this Publisher belongs (NOTE: we don't own the object)
    FParticipant : TParticipant;

    // The SendDataHandler used by the Publisher (NOTE: we don't own the object)
    FSendDataHandler : TSendDataHandler;

  public
    constructor Create(t : TTopic);
    destructor Destroy; override;

    procedure Start; override;
    procedure Stop; override;

    procedure WriteOPSObject(obj : TOPSObject); override;

  protected
  	procedure Write(data : TOPSObject);
  end;

implementation

uses SysUtils,
     uOps.Exceptions,
     uOps.ArchiverInOut;

constructor TPublisher.Create(t : TTopic);
begin
  inherited Create;
  FTopic := t;

  FMemMap := TMemoryMap.Create(UInt32(t.SampleMaxSize div PACKET_MAX_SIZE) + 1, PACKET_MAX_SIZE);
  FBuf := TByteBuffer.Create(FMemMap);
  FArchive := TOPSArchiverOut.Create(FBuf);

  FSendSleepTime := 1;
  FSleepEverySendPacket := 100000;
  FSleepOnSendFailed := True;

  FParticipant := TParticipant.getInstance(topic.DomainID, topic.ParticipantID);
  FSendDataHandler := FParticipant.getSendDataHandler(FTopic);

  FMessage := TOPSMessage.Create;
  FMessage.PublisherName := FName;
  FMessage.TopicName := FTopic.Name;
  FMessage.DataOwner := False;

	Start();
end;

destructor TPublisher.Destroy;
begin
  Stop();

  FParticipant.releaseSendDataHandler(FTopic);

  FreeAndNil(FMessage);
  FreeAndNil(FArchive);
  FreeAndNil(FBuf);
  FreeAndNil(FMemMap);
  inherited;
end;

procedure TPublisher.Start;
begin
  FSendDataHandler.addUser(Self);
end;

procedure TPublisher.Stop;
begin
  FSendDataHandler.removeUser(Self);
end;

procedure TPublisher.WriteOPSObject(obj : TOPSObject);
begin
  Write(obj);
end;

procedure TPublisher.Write(data : TOPSObject);
var
  i, segSize : Integer;
  sendOK : Boolean;
begin
  // Validate data that is going to be written.
  // We need this since IDL fields with or without 'virtual', in Delphi
  // are represented in the same way, ie. we could change the object in a
  // field that is supposed to be 'static' in the containing object.
  // So we need to check that the non-virtual fields contain the correct objects.
  if not data.Validate then begin
    raise EPublisherException.Create('Data object contains invalid object references');
  end;

  if FKey <> '' then begin
    data.Key := FKey;
  end;

  FBuf.Reset;
  FBuf.writeNewSegment;

  FMessage.Data := data;
  FMessage.PublicationID := FCurrentPublicationID;
  FMessage.PublisherName := FName;

  FArchive.inout('message', TSerializable(FMessage));

  // If data has spare bytes, write them to the end of the buffer
  if Length(FMessage.Data.spareBytes) > 0 then begin
    FBuf.WriteChars(@FMessage.Data.spareBytes[0], Length(FMessage.Data.spareBytes));
  end;

  FBuf.Finish;

  for i := 0 to FBuf.getNrOfSegments-1 do begin
    segSize := FBuf.getSegmentSize(i);
    sendOK := FSendDataHandler.sendData(FBuf.getSegment(i), segSize, FTopic);
    if not sendOK then begin
      Sleep(sendSleepTime);
      sendOK := FSendDataHandler.sendData(FBuf.getSegment(i), segSize, FTopic);
      if not sendOK then Break;   // Exit loop. No meaning to send the rest if a packet is lost??
    end else if (i > 0) and ((i mod sleepEverySendPacket) = 0) then begin
      Sleep(sendSleepTime);
    end;
  end;

  Inc(FCurrentPublicationID);
end;

end.

