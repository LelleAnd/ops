unit uOps.Transport.ReceiveDataChannel;

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
  TReceiveDataChannel = class(TObject)
  private
    // Borrowed references
    FErrorService : TErrorService;

    // Used for notifications to users of the ReceiveDataHandler
    FDataNotifier : TSingleNotifier<TOPSMessage>;
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

    // The accumulated size in bytes of the current message being received
    FCurrentMessageSize : Integer;

    // Status variables for the reception
    FExpectedSegment : Uint32;
    FFirstReceived : Boolean;

    // Key used to identify the channel when kept in a container
    FKey : string;

    procedure onConnectStatusChanged(Sender : TObject; arg : TConnectStatus);

  public
    constructor Create(top : TTopic;
                       dom : TDomain;
                       opsObjectFactory : TSerializableInheritingTypeFactory;
                       Reporter : TErrorService);
    destructor Destroy; override;

    procedure Start;
    procedure Stop;

		procedure addListener(Proc : TOnNotifyEvent<TOPSMessage>); overload;
		procedure removeListener(Proc : TOnNotifyEvent<TOPSMessage>); overload;

		procedure addListener(Proc : TOnNotifyEvent<TConnectStatus>); overload;
		procedure removeListener(Proc : TOnNotifyEvent<TConnectStatus>); overload;

    function getReceiver : TReceiver;

    property SampleMaxSize : Integer read FSampleMaxSize;
    property Key : string read FKey write FKey;

	protected
    // Called whenever the receiver has new data.
    procedure onNewEvent(Sender : TObject; arg : TBytesSizePair);

    // Handles spare bytes, i.e. extra bytes in buffer not consumed by created message
    procedure calculateAndSetSpareBytes(mess : TOPSMessage; segmentPaddingSize : Integer);
	end;

  function calcSampleMaxSize(top : TTopic) : Integer;

implementation

uses SysUtils,
     uOps.Exceptions;

function calcSampleMaxSize(top : TTopic) : Integer;
begin
  if top.SampleMaxSize > PACKET_MAX_SIZE then begin
    Result := top.SampleMaxSize;
  end else begin
    Result := PACKET_MAX_SIZE;
  end;
end;

constructor TReceiveDataChannel.Create(
              top : TTopic;
              dom : TDomain;
              opsObjectFactory : TSerializableInheritingTypeFactory;
              Reporter : TErrorService);
begin
  inherited Create;
  FErrorService := Reporter;
  FDataNotifier := TSingleNotifier<TOPSMessage>.Create(Self);
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

destructor TReceiveDataChannel.Destroy;
begin
  Stop;
  FreeAndNil(FReceiver);
  FreeAndNil(FTmpBuffer);
  FreeAndNil(FTmpMemMap);
  FreeAndNil(FArchiver);
  FreeAndNil(FBuffer);
  FreeAndNil(FMemMap);
  FreeAndNil(FCsNotifier);
  FreeAndNil(FDataNotifier);
  inherited;
end;

procedure TReceiveDataChannel.Start;
begin
  FExpectedSegment := 0;
  FCurrentMessageSize := 0;
  FReceiver.Start(FMemMap.getSegment(FExpectedSegment), FMemMap.SegmentSize);
end;

procedure TReceiveDataChannel.Stop;
begin
  FReceiver.Stop;
end;

procedure TReceiveDataChannel.addListener(Proc : TOnNotifyEvent<TOPSMessage>);
begin
  FDataNotifier.connectListener(Proc);
end;

procedure TReceiveDataChannel.removeListener(Proc : TOnNotifyEvent<TOPSMessage>);
begin
  FDataNotifier.disconnectListener(Proc);
end;

function TReceiveDataChannel.getReceiver : TReceiver;
begin
  Result := FReceiver;
end;

procedure TReceiveDataChannel.addListener(Proc : TOnNotifyEvent<TConnectStatus>);
begin
  FCsNotifier.addListener(Proc);
end;

procedure TReceiveDataChannel.removeListener(Proc : TOnNotifyEvent<TConnectStatus>);
begin
  FCsNotifier.removeListener(Proc);
end;

procedure TReceiveDataChannel.onConnectStatusChanged(Sender : TObject; arg : TConnectStatus);
begin
  FCsNotifier.doNotify(arg);
end;

// Called whenever the receiver has new data.
procedure TReceiveDataChannel.onNewEvent(Sender : TObject; arg : TBytesSizePair);

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
  newMessage : TOPSMessage;
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
        // Send it to Subscribers
        FDataNotifier.doNotify(newMessage);
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

procedure TReceiveDataChannel.calculateAndSetSpareBytes(mess : TOPSMessage; segmentPaddingSize : Integer);
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

