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

with Ops_Pa.ArchiverInOut_Pa;
use  Ops_Pa.ArchiverInOut_Pa;

package Ops_Pa.OpsObject_Pa.Topic_Pa is

  Topic_idlVersion : constant Byte := 0;

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type Topic_Class is new OpsObject_Class with private;
  type Topic_Class_At is access all Topic_Class'Class;

  -- Help types used in case other has vector's of the class
  type Topic_Class_Arr is array(Integer range <>) of aliased Topic_Class;
  type Topic_Class_Arr_At is access all Topic_Class_Arr;
  type Topic_Class_At_Arr is array(Integer range <>) of Topic_Class_At;
  type Topic_Class_At_Arr_At is access all Topic_Class_At_Arr;

  TRANSPORT_MC  : constant String := "multicast";
  TRANSPORT_TCP : constant String := "tcp";
  TRANSPORT_UDP : constant String := "udp";

  -- Constructors
  function Create(namee : String; portt : Int32; typeIDd : String; domainAddresss : String) return Topic_Class_At;
  function Create return Topic_Class_At;

  overriding procedure Serialize( Self : in out Topic_Class; archiver : ArchiverInOut_Class_At);

  -- Returns a newely allocated deep copy/clone of this object.
  overriding function Clone( Self : Topic_Class ) return OpsObject_Class_At;

  -- Fills the parameter obj with all values from this object.
  overriding procedure FillClone( Self : Topic_Class; obj : OpsObject_Class_At );

  -- Getters/Setters
  function Name( Self : Topic_Class ) return String;
  function TypeID( Self : Topic_Class ) return String;

  function DomainID( Self : Topic_Class ) return String;
  procedure SetDomainID( Self : in out Topic_Class; Value : String );

  function ParticipantID( Self : Topic_Class ) return String;
  procedure SetParticipantID( Self : in out Topic_Class; Value : String );

  function Transport( Self : Topic_Class ) return String;
  procedure SetTransport( Self : in out Topic_Class; Value : String );

  function DomainAddress( Self : Topic_Class ) return String;
  procedure SetDomainAddress( Self : in out Topic_Class; Value : String );

  function LocalInterface( Self : Topic_Class ) return String;
  procedure SetLocalInterface( Self : in out Topic_Class; Value : String );

  function SampleMaxSize( Self : Topic_Class ) return Int32;
  procedure SetSampleMaxSize( Self : in out Topic_Class; Value : Int32);

  function Port( Self : Topic_Class ) return Int32;
  procedure SetPort( Self : in out Topic_Class; Value : Int32);

  function TimeToLive( Self : Topic_Class ) return Int32;
  procedure SetTimeToLive( Self : in out Topic_Class; Value : Int32);

  function OutSocketBufferSize( Self : Topic_Class ) return Int64;
  procedure SetOutSocketBufferSize( Self : in out Topic_Class; Value : Int64);

  function InSocketBufferSize( Self : Topic_Class ) return Int64;
  procedure SetInSocketBufferSize( Self : in out Topic_Class; Value : Int64);

  function OptNonVirt( Self : Topic_Class ) return Boolean;
  procedure SetOptNonVirt( Self : in out Topic_Class; Value : Boolean);

  function HeartbeatPeriod( Self : Topic_Class ) return Int32;
  procedure SetHeartbeatPeriod( Self : in out Topic_Class; Value : Int32);

  function HeartbeatTimeout( Self : Topic_Class ) return Int32;
  procedure SetHeartbeatTimeout( Self : in out Topic_Class; Value : Int32);

  function ChannelId( Self : Topic_Class ) return String;
  procedure SetChannelId( Self : in out Topic_Class; Value : String );

  function Topic_version( Self : Topic_Class ) return Byte;
  procedure SetTopic_version( Self : in out Topic_Class; Version : Byte );

private
-- ==========================================================================
--
-- ==========================================================================
  type Topic_Class is new OpsObject_Class with
    record
      Topic_version : Byte := Topic_idlVersion;
      Name : String_At := null;
      Port : Int32 := 0;
      TimeToLive : Int32 := -1;
      TypeID : String_At := null;
      DomainAddress : String_At := null;
      LocalInterface : String_At := null;
      ParticipantID : String_At := new String'("DEFAULT_PARTICIPANT");
      DomainID : String_At := null;
      SampleMaxSize : Int32 := PACKET_MAX_SIZE;
      Deadline : Int64 := MAX_DEADLINE_TIMEOUT;
      MinSeparation : Int64 := 0;
      Transport : String_At := null;
      OutSocketBufferSize : Int64 := -1;
      InSocketBufferSize : Int64 := -1;
      OptNonVirt : Boolean := False;
      HeartbeatPeriod : Int32 := 0;
      HeartbeatTimeout : Int32 := 0;
      ChannelId : String_At := null;
    end record;

  procedure InitInstance( Self : in out Topic_Class; namee : String; portt : Int32; typeIDd : String; domainAddresss : String);
  procedure InitInstance( Self : in out Topic_Class );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out Topic_Class );

end Ops_Pa.OpsObject_Pa.Topic_Pa;

