unit uOps.OpsMessage;

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
  TOPSMessage = class(TOPSObject)
  private
    FOPSMessage_version : Byte;
    FMessageType : Byte;          // Serialized (not used, always 0)
    //FEndianness : Byte;         //            (not used)
    FPublisherPriority : Byte;    // Serialized (not used, always 0)
    FDataOwner : Boolean;
    FSourcePort : Integer;
		FSourceIP : string;
    //FQosMask : Int64;
    FPublicationID : Int64;       // Serialized
    FPublisherName : AnsiString;  // Serialized
    FTopicName : AnsiString;      // Serialized
    FTopLevelKey : AnsiString;    // Serialized (not used, empty string)
    FAddress : AnsiString;        // Serialized (not used, empty string)
    FData : TOPSObject;           // Serialized

    // Reservation handling
    FNrOfReservations : Integer;

    procedure SetData(Value : TOPSObject);

  public
    constructor Create;
    destructor Destroy; override;

    procedure Serialize(archiver : TArchiverInOut); override;

		// Returns a newely allocated deep copy/clone of this object.
		function Clone : TOPSObject; override;

		// Fills the parameter obj with all values from this object.
		procedure FillClone(var obj : TOPSObject); override;

    //
    procedure setSource(addr : string; port : Integer);
    procedure getSource(var addr : string; var port : Integer);

    // Reservation handling
    // --------------------
    // Reserve()    increments the reservation counter
    // Unreserve()  decrements the reservation counter and when it becomes 0 the
    //              message is deleted
    // NOTE: If these calls are used, the object shouldn't be freed explicitly,
    // the Unreserve() call will do that for you.
    procedure Reserve;
    procedure UnReserve;
    property NrOfReservations : Integer read FNrOfReservations;

    //
    property OPSMessage_Version : Byte read FOPSMessage_version write FOPSMessage_version;
    property DataOwner : Boolean read FDataOwner write FDataOwner;
    property PublicationID : Int64 read FPublicationID write FPublicationID;
    property PublisherName : AnsiString read FPublisherName write FPublisherName;
    property TopicName : AnsiString read FTopicName write FTopicName;
    property Data : TOPSObject read FData write SetData;
  end;

var
  gNumObjects : Int64;

implementation

uses SysUtils,
     SyncObjs,
     uOps.Error;

constructor TOPSMessage.Create;
begin
  inherited;
  TInterlocked.Increment(gNumObjects);
  FOPSMessage_version := 0;
  FDataOwner := True;
  AppendType('ops.protocol.OPSMessage');
end;

destructor TOPSMessage.Destroy;
begin
  // Validation
  if FNrOfReservations <> 0 then begin
    gStaticErrorService.Report(TBasicError.Create(
      'TOPSMessage', 'Destroy', 'Invalid reserve value @ delete. Mismatched Reserve/Unreserve calls!'));
  end;
  if FDataOwner then begin
    FreeAndNil(FData);
  end;
  TInterlocked.Decrement(gNumObjects);
  inherited;
end;

procedure TOPSMessage.Reserve;
begin
  TInterlocked.Increment(FNrOfReservations);
end;

procedure TOPSMessage.UnReserve;
begin
  if TInterlocked.Decrement(FNrOfReservations) = 0 then
    Destroy;
end;

procedure TOPSMessage.SetData(Value : TOPSObject);
begin
  if FDataOwner then begin
    FreeAndNil(FData);
  end;
  FData := Value;
end;

procedure TOPSMessage.setSource(addr : string; port : Integer);
begin
  FSourceIP := addr;
  FSourcePort := port;
end;

procedure TOPSMessage.getSource(var addr : string; var port : Integer);
begin
  addr := FSourceIP;
  port := FSourcePort;
end;

procedure TOPSMessage.Serialize(archiver : TArchiverInOut);
begin
  inherited Serialize(archiver);
  if FIdlVersionMask <> 0 then begin
    archiver.inout('OPSMessage_version', FOPSMessage_version);
  end else begin
    FOPSMessage_version := 0;
  end;

  archiver.inout('messageType', FMessageType);
  archiver.inout('publisherPriority', FPublisherPriority);
  archiver.inout('publicationID', FPublicationID);
  archiver.inout('publisherName', FPublisherName);
  archiver.inout('topicName', FTopicName);
  archiver.inout('topLevelKey', FTopLevelKey);
  archiver.inout('address', FAddress);
  FData := TOPSObject(archiver.inout2('data', TSerializable(FData)));
end;

// Returns a newely allocated deep copy/clone of this object.
function TOPSMessage.Clone : TOPSObject;
begin
	Result := TOPSMessage.Create;
  Self.FillClone(Result);
end;

// Fills the parameter obj with all values from this object.
procedure TOPSMessage.FillClone(var obj : TOPSObject);
begin
	inherited FillClone(obj);
  with obj as TOPSMessage do begin
    FOPSMessage_version := Self.FOPSMessage_version;
    FMessageType := Self.FMessageType;
    FPublisherPriority := Self.FPublisherPriority;
    FDataOwner := True;
    FSourcePort := Self.FSourcePort;
		FSourceIP := Self.FSourceIP;
    FPublicationID := Self.FPublicationID;
    FPublisherName := Self.FPublisherName;
    FTopicName := Self.FTopicName;
    FTopLevelKey := Self.FTopLevelKey;
    FAddress := Self.FAddress;
    FData := Self.FData.Clone;
  end;
end;


initialization
  gNumObjects := 0;

end.

