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
     Ops_Pa.Participant_Pa;

use  Ops_Pa.ArchiverInOut_Pa,
     Ops_Pa.Participant_Pa;

package body Ops_Pa.PublisherAbs_Pa.Publisher_Pa is

  --------------------------------------------------------------------------
  --
  --------------------------------------------------------------------------
  function Create( t : Topic_Class_At ) return Publisher_Class_At is
    Self : Publisher_Class_At := null;
  begin
    Self := new Publisher_Class;
    InitInstance( Self.all, Self, t );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  -- ---------------------------------------------------------------------------

  procedure SetUserParam( Self : in out Publisher_Class; Value : UInt64 ) is
  begin
    Self.UserParam := Value;
  end;

  procedure SetUserRef( Self : in out Publisher_Class; Value : Ops_Class_At ) is
  begin
    Self.UserRef := Value;
  end;

  function GetUserParam( Self : Publisher_Class ) return UInt64 is
  begin
    return Self.UserParam;
  end;

  function GetUserRef( Self : Publisher_Class ) return Ops_Class_At is
  begin
    return Self.UserRef;
  end;

  -- ---------------------------------------------------------------------------

  overriding procedure Start( Self : in out Publisher_Class ) is
  begin
    Self.SendDataHandler.addUser( Ops_Class_At(Self.SelfAt) );
    Self.SendDataHandler.addListener( Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface_At(Self.SelfAt) );
  end;

  overriding procedure Stop( Self : in out Publisher_Class ) is
  begin
    Self.SendDataHandler.removeListener( Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface_At(Self.SelfAt) );
    Self.SendDataHandler.removeUser( Ops_Class_At(Self.SelfAt) );
    Self.Connected := False;
  end;

  -- Return connect status (currently only valid for TCP)
  function isConnected( Self : Publisher_Class ) return Boolean is
  begin
    return Self.Connected;
  end;

  procedure addListener( Self : in out Publisher_Class; Client : Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface_At ) is
  begin
    Self.CsNotifier.addListener( Client );
  end;

  procedure removeListener( Self : in out Publisher_Class; Client : Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface_At ) is
  begin
    Self.CsNotifier.removeListener( Client );
  end;

  procedure OnNotify( Self : in out Publisher_Class; Sender : in Ops_Class_At; Item : in Transport_Pa.ConnectStatus_T ) is
  begin
    Self.Connected := Item.Connected;
    Self.CsNotifier.doNotify( Item );
  end;

  overriding procedure WriteOPSObject( Self : in out Publisher_Class; obj : OpsObject_Class_At) is
  begin
    Write(Self, obj);
  end;

  procedure Write( Self : in out Publisher_Class; data : OpsObject_Class_At) is
    durMS : constant Duration := 0.001;
    segSize : Integer;
    sendOK : Boolean;
  begin
    -- Validate data that is going to be written.
    -- We need this since IDL fields with or without 'virtual', in Delphi
    -- are represented in the same way, ie. we could change the object in a
    -- field that is supposed to be 'static' in the containing object.
    -- So we need to check that the non-virtual fields contain the correct objects.
    --if not data.Validate then
    --  raise EPublisherException.Create('Data object contains invalid object references');
    --end;

    if Self.Key /= null and then Self.Key.all /= "" then
      data.all.Key(Self.Key.all);
    end if;

    Self.Buf.Reset;
    Self.Buf.WriteNewSegment;

    Self.Message.SetData( data );
    Self.Message.SetPublicationID( Self.CurrentPublicationID );
    if Self.Name /= null then
      Self.Message.SetPublisherName( Self.Name.all );
    end if;

    declare
      dummy : Serializable_Class_At;
    begin
      dummy := Self.Archive.inout2("message", Serializable_Class_At(Self.Message));
    end;

    -- If data has spare bytes, write them to the end of the buffer
    if Self.Message.Data.SpareBytes /= null then
      Self.Buf.WriteChars(Self.Message.Data.SpareBytes.all);
    end if;

    Self.Buf.Finish;

    for i in 0 .. Self.Buf.getNrOfSegments-1 loop
      segSize := Integer(Self.Buf.getSegmentSize(i));
      sendOK := Self.SendDataHandler.sendData(Self.Buf.getSegment(i), segSize, Self.Topic);
      if not sendOK then
        delay DurMs * Duration(Self.SendSleepTime);
        sendOK := Self.SendDataHandler.sendData(Self.Buf.getSegment(i), segSize, Self.Topic);
        exit when not sendOK; -- Exit loop. No meaning to send the rest if a packet is lost??
      elsif (i > 0) and ((i mod UInt32(Self.sleepEverySendPacket)) = 0) then
        delay DurMs * Duration(Self.SendSleepTime);
      end if;
    end loop;

    Self.CurrentPublicationID := Self.CurrentPublicationID + 1;
  end;

  procedure InitInstance( Self : in out Publisher_Class; SelfAt : Publisher_Class_At; t : Topic_Class_At) is
  begin
    Self.SelfAt := SelfAt;
    Self.Topic := t;

    Self.MemMap := Create(UInt32(t.SampleMaxSize / PACKET_MAX_SIZE) + 1, PACKET_MAX_SIZE);
    Self.Buf := Create(Self.MemMap);
    Self.Archive := Create(Self.Buf, Self.Topic.OptNonVirt);

    Self.Participant := getInstance(t.DomainID, t.ParticipantID);
    Self.SendDataHandler := Self.Participant.getSendDataHandler(Self.Topic);

    Self.CsNotifier := Transport_Pa.ConnectStatusNotifier_Pa.Create( Ops_Class_At(SelfAt) );

    Self.Message := Create;
    if Self.Name /= null then
      Self.Message.SetPublisherName( Self.Name.all );
    end if;
    Self.Message.SetTopicName( Self.Topic.Name );
    Self.Message.SetDataOwner( False );

    Start( Self );
  end;

  overriding procedure Finalize( Self : in out Publisher_Class ) is
  begin
    Stop( Self );

    Self.Participant.releaseSendDataHandler(Self.Topic);

    Transport_Pa.ConnectStatusNotifier_Pa.Free(Self.CsNotifier);

    Free( Self.Message );
    Free( Self.Archive );
    Free( Self.Buf );
    Free( self.MemMap );

    Finalize( PublisherAbs_Class(Self) );
  end;

end Ops_Pa.PublisherAbs_Pa.Publisher_Pa;

