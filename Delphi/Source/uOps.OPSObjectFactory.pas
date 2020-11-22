unit uOps.OPSObjectFactory;

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

uses uOps.SerializableInheritingTypeFactory;

type
  TOPSObjectFactory = class(TSerializableInheritingTypeFactory)
  public
    // Return singelton instance of OPSObjectFactory.
    class function getInstance : TOPSObjectFactory;
  end;

  TOPSObjectFactoryImpl = class(TOPSObjectFactory)
  public
    constructor Create;
  end;

implementation

uses SysUtils,
     uOps.OPSConfig,
     uOps.OPSMessage,
     uOPs.Topic,
     uOps.Domain,
     uOps.Channel,
     uOps.Transport,
     uOps.ParticipantInfoData,
     uOps.TopicInfoData,
     uOps.ArchiverInOut,
     opsidls.SendAckPatternData,
     uOps.SerializableFactory;

type
  TBuiltInFactory = class(TSerializableFactory)
  public
    // Named 'Make' instead of 'Create' which in Delphi normaly is used for constructors
    function Make(types : string) : TSerializable; override;
  end;

function TBuiltInFactory.Make(types : string) : TSerializable;
begin
  Result := nil;
  if types = 'ops.protocol.OPSMessage' then begin
    Result := TOPSMessage.Create;
  end else if types = 'ops.ParticipantInfoData' then begin
    Result := TParticipantInfoData.Create;
  end else if types = 'opsidls.SendAckPatternData' then begin
    Result := opsidls.SendAckPatternData.SendAckPatternData.Create;
  end else if types = 'Topic' then begin
    Result := TTopic.Create;
  end else if types = 'Channel' then begin
    Result := TChannel.Create;
  end else if types = 'Transport' then begin
    Result := TTransport.Create;
  end else if types = 'DefaultOPSConfigImpl' then begin
    Result := TDefaultOPSConfigImpl.Create;
  end else if types = 'MulticastDomain' then begin
    Result := TDomain.Create;
  end else if types = 'Domain' then begin
    Result := TDomain.Create;
  end else if types = 'TopicInfoData' then begin
    Result := TTopicInfoData.Create;
  end;
end;

constructor TOPSObjectFactoryImpl.Create;
begin
  inherited Create;
  Add(TBuiltInFactory.Create);
end;

var
  gInstance : TOPSObjectFactory;

// Return singelton instance of OPSObjectFactory.
class function TOPSObjectFactory.getInstance : TOPSObjectFactory;
begin
  Result := gInstance;
end;

initialization
  // Create singelton instance of OPSObjectFactoryImpl.
  gInstance := TOPSObjectFactoryImpl.Create;

finalization
  FreeAndNil(gInstance);

end.

