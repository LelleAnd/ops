unit uOps.Transport.ReceiveDataHandler;

(**
*
* Copyright (C) 2016-2021 Lennart Andersson.
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
     uOps.Types,
     uOps.Error,
     uOps.Topic,
     uOps.MemoryMap,
     uOps.ByteBuffer,
     uOps.OPSMessage,
     uOps.ArchiverInOut,
     uOps.OPSArchiverIn,
     uOps.SerializableInheritingTypeFactory,
     uOps.Domain,
     uOps.Transport.Receiver;

type
  TReceiveDataHandler = class(TObject)
  private
    // Borrowed references
    FErrorService : TErrorService;

    // Used for notifications to users of the ReceiveDataHandler
    FDataNotifier : TNotifier<TOPSMessage>;
    FCsNotifier : TNotifierValue<TConnectStatus>;

    // The receiver used for this ReceiveHandler.
    FReceiver : TReceiver;

    // Preallocated MemoryMap and buffer for receiving data
    FMemMap : TMemoryMap;
    FBuffer : TByteBuffer;
    FSampleMaxSize : Integer;

    // Archiver used for deserializing byte buffers into messages
    FArchiver : TOPSArchiverIn;

    // Temporary MemoryMap and buffer used during basic validation of a segment
    FTmpMemMap : TMemoryMap;
    FTmpBuffer : TByteBuffer;

		// Current OPSMessage, valid until next sample arrives.
    FMessage : TOPSMessage;

    // The accumulated size in bytes of the current message being received
    FCurrentMessageSize : Integer;

    //
    FMessageLock : TMutex;

    // Status variables for the reception
    FExpectedSegment : Uint32;
    FFirstReceived : Boolean;

    function GetNumListeners : Integer;
    procedure onConnectStatusChanged(Sender : TObject; arg : TConnectStatus);

  public
    constructor Create(top : TTopic;
                       dom : TDomain;
                       opsObjectFactory : TSerializableInheritingTypeFactory;
                       Reporter : TErrorService);
    destructor Destroy; override;

    function aquireMessageLock : Boolean;
    procedure releaseMessageLock;

    procedure Stop;

		procedure addListener(Proc : TOnNotifyEvent<TOPSMessage>); overload;
		procedure removeListener(Proc : TOnNotifyEvent<TOPSMessage>); overload;

		procedure addListener(Proc : TOnNotifyEvent<TConnectStatus>); overload;
		procedure removeListener(Proc : TOnNotifyEvent<TConnectStatus>); overload;

    function getReceiver : TReceiver;

    property SampleMaxSize : Integer read FSampleMaxSize;
    property NumListeners : Integer read GetNumListeners;

	protected
    // Called whenever the receiver has new data.
    procedure onNewEvent(Sender : TObject; arg : TBytesSizePair);

    // Handles spare bytes, i.e. extra bytes in buffer not consumed by created message
    procedure calculateAndSetSpareBytes(mess : TOPSMessage; segmentPaddingSize : Integer);
	end;

implementation

uses SysUtils,
     uOps.Exceptions;

constructor TReceiveDataHandler.Create(
              top : TTopic;
              dom : TDomain;
              opsObjectFactory : TSerializableInheritingTypeFactory;
              Reporter : TErrorService);
begin
  inherited Create;
  FErrorService := Reporter;
  FDataNotifier := TNotifier<TOPSMessage>.Create(Self);
  FMessageLock := TMutex.Create;
  FCsNotifier := TNotifierValue<TConnectStatus>.Create(Self, True);

  FMemMap := TMemoryMap.Create(top.SampleMaxSize div PACKET_MAX_SIZE + 1, PACKET_MAX_SIZE);
  FBuffer := TByteBuffer.Create(FMemMap);
	FSampleMaxSize := top.SampleMaxSize;

  FArchiver := TOPSArchiverIn.Create(FBuffer, opsObjectFactory);

  // Temporary MemoryMap and buffer
  FTmpMemMap := TMemoryMap.Create(nil, 0);
  FTmpBuffer := TByteBuffer.Create(FTmpMemMap);

  FReceiver := TReceiverFactory.getReceiver(top, dom, FErrorService);
  if not Assigned(FReceiver) then begin
    raise ECommException.Create('Could not create receiver');
  end;
  FReceiver.addListener(onNewEvent);
  FReceiver.setConnectStatusListener(onConnectStatusChanged);
end;

destructor TReceiveDataHandler.Destroy;
begin
  Stop;
  FreeAndNil(FReceiver);
  FreeAndNil(FTmpBuffer);
  FreeAndNil(FTmpMemMap);
  FreeAndNil(FArchiver);
  FreeAndNil(FBuffer);
  FreeAndNil(FMemMap);
  FreeAndNil(FCsNotifier);
  FreeAndNil(FMessageLock);
  FreeAndNil(FDataNotifier);
  inherited;
end;

function TReceiveDataHandler.aquireMessageLock : Boolean;
begin
  Result := True;
  FMessageLock.Acquire;
end;

procedure TReceiveDataHandler.releaseMessageLock;
begin
  FMessageLock.Release;
end;

procedure TReceiveDataHandler.addListener(Proc : TOnNotifyEvent<TOPSMessage>);
begin
  FMessageLock.Acquire;
  try
    FDataNotifier.addListener(Proc);
    if FDataNotifier.numListeners = 1 then begin
      FExpectedSegment := 0;
      FCurrentMessageSize := 0;
      FReceiver.Start(FMemMap.getSegment(FExpectedSegment), FMemMap.SegmentSize);
    end;
  finally
    FMessageLock.Release;
  end;
end;

procedure TReceiveDataHandler.removeListener(Proc : TOnNotifyEvent<TOPSMessage>);
begin
  FMessageLock.Acquire;
  try
  FDataNotifier.removeListener(Proc);
  if FDataNotifier.numListeners = 0 then begin
    FReceiver.Stop;
  end;
  finally
    FMessageLock.Release;
  end;
end;

function TReceiveDataHandler.GetNumListeners : Integer;
begin
  Result := FDataNotifier.numListeners;
end;

function TReceiveDataHandler.getReceiver : TReceiver;
begin
  Result := FReceiver;
end;

procedure TReceiveDataHandler.addListener(Proc : TOnNotifyEvent<TConnectStatus>);
begin
  FCsNotifier.addListener(Proc);
end;

procedure TReceiveDataHandler.removeListener(Proc : TOnNotifyEvent<TConnectStatus>);
begin
  FCsNotifier.removeListener(Proc);
end;

procedure TReceiveDataHandler.onConnectStatusChanged(Sender : TObject; arg : TConnectStatus);
begin
  FCsNotifier.doNotify(arg);
end;

// Called whenever the receiver has new data.
procedure TReceiveDataHandler.onNewEvent(Sender : TObject; arg : TBytesSizePair);

  procedure Report(msg : string); overload;
  begin
    if Assigned(FErrorService) then begin
      FErrorService.Report(TBasicError.Create('ReceiveDataHandler', 'onNewEvent', msg));
    end;
  end;

  procedure Report(msg : string; addr : string; port : Integer); overload;
  begin
    Report(msg + ' [' + addr + '::' + IntToStr(port) + ']');
  end;

var
  nrOfFragments : UInt32;
  currentFragment : UInt32;
  segmentPaddingSize : Integer;
  oldMessage, newMessage : TOPSMessage;
  srcAddr : string;
  srcPort : Integer;
begin
  if arg.size <= 0 then begin
//            //Inform participant that we had an error waiting for data,
//            //this means the underlying socket is down but hopefully it will reconnect, so no need to do anything.
//            //Only happens with tcp connections so far.
//
//            if (byteSizePair.size == -5)
//            {
//                Report('Connection was lost but is now reconnected.');
//            }
//            else
//            {
//                Report('Empty message or error.');
//            }

    // Continue with the same buffer, so just exit
    Exit;
  end;

  // TODO Check that all segments come from the same source (IP and port)
  FReceiver.GetSource(srcAddr, srcPort);

  // Use a temporary map and buf to peek data before putting it in to FMemMap
  FTmpMemMap.ChangeBuffer(FMemMap.getSegment(FExpectedSegment), FMemMap.SegmentSize);
  FTmpBuffer.Reset;   // Reset buffer to use changed memmap values

  // Check protocol
  if FTmpBuffer.CheckProtocol then begin
    //Read of message ID and fragmentation info, this is ignored so far.
    //std::string messageID = tBuf.ReadString();
    nrOfFragments := FTmpBuffer.ReadInt;
    currentFragment := FTmpBuffer.ReadInt;

    if (currentFragment <> (nrOfFragments - 1)) and (arg.size <> PACKET_MAX_SIZE) then begin
      Report('Debug: Received broken package.', srcAddr, srcPort);
    end;

    Inc(FCurrentMessageSize, arg.size);

    if currentFragment <> FExpectedSegment then begin
      // Error
      if FFirstReceived then begin
        Report('Segment Error, sample will be lost.', srcAddr, srcPort);
        FFirstReceived := False;
      end;
      FExpectedSegment := 0;
      FCurrentMessageSize := 0;
      FReceiver.SetReceiveBuffer(FMemMap.getSegment(FExpectedSegment), FMemMap.SegmentSize);
      Exit;
    end;

    if currentFragment = (nrOfFragments - 1) then begin
      // We have got all segments
      FFirstReceived := True;
      FExpectedSegment := 0;
      FBuffer.Reset;

      // Skip some protocol parts already checked
      FBuffer.checkProtocol;
      FBuffer.ReadInt;
      FBuffer.ReadInt;
      segmentPaddingSize := FBuffer.GetSize;

      // Read of the actual OPSMessage
      newMessage := nil;
      try
        newMessage := TOPSMessage(FArchiver.inout2('message', TSerializable(newMessage)));
        if Assigned(newMessage) then begin
          // Check that we succeded in creating the actual data message
          if Assigned(newMessage.Data) then begin
            // Put spare bytes in data-field of message
            calculateAndSetSpareBytes(newMessage, segmentPaddingSize);

            // Add IP and port for source as meta data into OPSMessage
            newMessage.setSource(srcAddr, srcPort);
          end else begin
            Report('Failed to deserialize message. Check added Factories.', srcAddr, srcPort);
            FreeAndNil(newMessage);
          end;
        end else begin
          // Inform participant that invalid data is on the network.
          Report('Unexpected type received. Type creation failed.', srcAddr, srcPort);
        end;
      except
        on E : Exception do begin
          FreeAndNil(newMessage);
          Report('Invalid data on network. Exception: ' + e.ToString, srcAddr, srcPort);
        end;
      end;

      FCurrentMessageSize := 0;

      if Assigned(newMessage) then begin
        FMessageLock.Acquire;
        try
          oldMessage := FMessage;
          FMessage := newMessage;
          newMessage := nil;

          // Increment ref count for message
          FMessage.Reserve;

          // Send it to Subscribers
          FDataNotifier.doNotify(FMessage);

          // This will delete the old message if no one has reserved it in the application layer.
          if Assigned(oldMessage) then oldMessage.Unreserve;
        finally
          FMessageLock.Release;
        end;
      end;

    end else begin
      Inc(FExpectedSegment);

  		if FExpectedSegment >= FMemMap.NrOfSegments then begin
        Report('Buffer too small for received message.', srcAddr, srcPort);
        FExpectedSegment := 0;
        FCurrentMessageSize := 0;
			end;
    end;
    FReceiver.SetReceiveBuffer(FMemMap.getSegment(FExpectedSegment), FMemMap.SegmentSize);
  end else begin
    //Inform participant that invalid data is on the network.
    Report('Protocol ERROR.', srcAddr, srcPort);
    FReceiver.SetReceiveBuffer(FMemMap.getSegment(FExpectedSegment), FMemMap.SegmentSize);
  end;
end;

procedure TReceiveDataHandler.Stop;
begin
  FReceiver.removeListener(onNewEvent);

  // Need to release the last message we received, if any.
  // (We always keep a reference to the last message received)
  if Assigned(FMessage) then FMessage.Unreserve;
  FMessage := nil;

///TODO  receiver->stop(); ???

end;

procedure TReceiveDataHandler.calculateAndSetSpareBytes(mess : TOPSMessage; segmentPaddingSize : Integer);
var
  nrOfSerializedBytes : Integer;
  totalNrOfSegments : Integer;
  nrOfSerializedSegements : Integer;
  nrOfUnserializedSegments : Integer;
  nrOfSpareBytes : Integer;
begin
  // We must calculate how many unserialized segment headers we have and substract
  // that total header size from the size of spareBytes.
  nrOfSerializedBytes := FBuffer.GetSize;
  totalNrOfSegments := FCurrentMessageSize div Integer(FMemMap.SegmentSize);
  nrOfSerializedSegements := nrOfSerializedBytes div Integer(FMemMap.SegmentSize);
  nrOfUnserializedSegments := totalNrOfSegments - nrOfSerializedSegements;

  nrOfSpareBytes := FCurrentMessageSize - FBuffer.GetSize - (nrOfUnserializedSegments * segmentPaddingSize);

  if nrOfSpareBytes > 0 then begin
    SetLength(mess.Data.spareBytes, nrOfSpareBytes);
    // This will read the rest of the bytes as raw bytes and put them into spareBytes field of data.
    FBuffer.ReadChars(@mess.Data.spareBytes[0], nrOfSpareBytes);
  end;
end;

end.

