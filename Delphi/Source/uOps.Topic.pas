unit uOps.Topic;

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

uses uOps.Types,
     uOps.ArchiverInOut,
     uOps.OpsObject;

type
	TTopic = class(TOPSObject)
  public
    const
      TRANSPORT_MC = 'multicast';
      TRANSPORT_TCP = 'tcp';
      TRANSPORT_UDP = 'udp';

  private
    FTopic_version : Byte;
    FName : AnsiString;
		FPort : Integer;
		FTimeToLive : Integer;
		FTypeID : AnsiString;
    FDomainAddress : AnsiString;
		FLocalInterface : AnsiString;
		FParticipantID : string;
		FDomainID : string;
		//bool reliable;
		FSampleMaxSize : Integer;
		FDeadline : Int64;
		FMinSeparation : Int64;
		FTransport : AnsiString;
		FOutSocketBufferSize : Int64;
		FInSocketBufferSize : Int64;
		FOptNonVirt : Boolean;

		procedure SetSampleMaxSize(Value : Integer);

	public
    constructor Create(namee : AnsiString; portt : Integer; typeIDd : AnsiString; domainAddresss : AnsiString); overload;
		constructor Create; overload;

    procedure Serialize(archiver : TArchiverInOut); override;

		// Returns a newely allocated deep copy/clone of this object.
		function Clone : TOPSObject; override;

		// Fills the parameter obj with all values from this object.
		procedure FillClone(var obj : TOPSObject); override;

    property Topic_version : Byte read FTopic_version write FTopic_version;
    property Name : AnsiString read FName;
    property TypeID : AnsiString read FTypeID;
    property DomainID : string read FDomainID write FDomainID;
    property ParticipantID : string read FParticipantID write FParticipantID;
    property Transport : AnsiString read FTransport write FTransport;
    property DomainAddress : AnsiString read FDomainAddress write FDomainAddress;
    property LocalInterface : AnsiString read FLocalInterface write FLocalInterface;
    property SampleMaxSize : Integer read FSampleMaxSize write SetSampleMaxSize;
    property Port : Integer read FPort write FPort;
    property TimeToLive : Integer read FTimeToLive write FTimeToLive;
    property OutSocketBufferSize : Int64 read FOutSocketBufferSize write FOutSocketBufferSize;
    property InSocketBufferSize : Int64 read FInSocketBufferSize write FInSocketBufferSize;
    property OptNonVirt : Boolean read FOptNonVirt write FOptNonVirt;
  end;

implementation

uses uOps.Exceptions;

constructor TTopic.Create(namee : AnsiString; portt : Integer; typeIDd : AnsiString; domainAddresss : AnsiString);
begin
  inherited Create;
  FTopic_version := 0;
  FName := namee;
  FPort := portt;
  FTypeID := typeIDd;
  FDomainAddress := domainAddresss;
  FParticipantID := 'DEFAULT_PARTICIPANT';
  //reliable(false),
  FSampleMaxSize := uOps.Types.PACKET_MAX_SIZE;
  FDeadline := uOps.Types.MAX_DEADLINE_TIMEOUT;
  FMinSeparation := 0;
  FOutSocketBufferSize := -1;
  FInSocketBufferSize := -1;
  FTimeToLive := -1;

  AppendType('Topic');
end;

constructor TTopic.Create;
begin
  inherited;
  FTopic_version := 0;
  FParticipantID := 'DEFAULT_PARTICIPANT';
	//reliable(false),
  FSampleMaxSize := uOps.Types.PACKET_MAX_SIZE;
  FDeadline := uOps.Types.MAX_DEADLINE_TIMEOUT;
  FMinSeparation := 0;
  FOutSocketBufferSize := -1;
  FInSocketBufferSize := -1;
  FTimeToLive := -1;

  AppendType('Topic');
end;

procedure TTopic.SetSampleMaxSize(value : Integer);
begin
  if value < uOps.Types.PACKET_MAX_SIZE then begin
    FSampleMaxSize := uOps.Types.PACKET_MAX_SIZE;
  end else begin
    FSampleMaxSize := value;
  end;
end;

procedure TTopic.Serialize(archiver : TArchiverInOut);
var
  tSampleMaxSize : Integer;
begin
  inherited Serialize(archiver);
  if FIdlVersionMask <> 0 then begin
    archiver.inout('Topic_version', FTopic_version);
  end else begin
    FTopic_version := 0;
  end;

  archiver.inout('name', FName);
  archiver.inout('dataType', FTypeID);
  archiver.inout('port', FPort);
  archiver.inout('address', FDomainAddress);

  archiver.inout('outSocketBufferSize', FOutSocketBufferSize);
  archiver.inout('inSocketBufferSize', FInSocketBufferSize);

  //Limit this value
  tSampleMaxSize := SampleMaxSize;
  archiver.inout('sampleMaxSize', tSampleMaxSize);
  SampleMaxSize := tSampleMaxSize;

  archiver.inout('transport', FTransport);
  if FTransport = '' then begin
    FTransport := TRANSPORT_MC;
  end else if (FTransport <> TRANSPORT_MC) and (FTransport <> TRANSPORT_TCP) and (FTransport <> TRANSPORT_UDP) then begin
    raise EConfigException('Illegal transport: "' + FTransport +
          '". Transport for topic must be either "multicast", "tcp", "udp" or left blank( = multicast)');
  end;
end;

// Returns a newely allocated deep copy/clone of this object.
function TTopic.Clone : TOPSObject;
begin
	Result := TTopic.Create;
  Self.FillClone(Result);
end;

// Fills the parameter obj with all values from this object.
procedure TTopic.FillClone(var obj : TOPSObject);
begin
	inherited FillClone(obj);
  with obj as TTopic do begin
    FTopic_version := Self.FTopic_version;
    FName := Self.FName;
		FPort := Self.FPort;
		FTypeID := Self.FTypeID;
    FDomainAddress := Self.FDomainAddress;
		FParticipantID := Self.FParticipantID;
		FDomainID := Self.FDomainID;
		FSampleMaxSize := Self.FSampleMaxSize;
		FTransport := Self.FTransport;
		FOutSocketBufferSize := Self.FOutSocketBufferSize;
		FInSocketBufferSize := Self.FInSocketBufferSize;
  end;
end;

end.

