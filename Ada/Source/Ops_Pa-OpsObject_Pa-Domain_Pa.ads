--
-- Copyright (C) 2016-2020 Lennart Andersson.
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

with
  Ops_Pa.OpsObject_Pa.Channel_Pa,
  Ops_Pa.OpsObject_Pa.Transport_Pa,
  Ops_Pa.OpsObject_Pa.Topic_Pa;
use
  Ops_Pa.OpsObject_Pa.Channel_Pa,
  Ops_Pa.OpsObject_Pa.Transport_Pa,
  Ops_Pa.OpsObject_Pa.Topic_Pa;

package Ops_Pa.OpsObject_Pa.Domain_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type Domain_Class is new OpsObject_Class with private;
  type Domain_Class_At is access all Domain_Class'Class;

  -- Help types used in case other idl's has vector's of the class
  type Domain_Class_Arr is array(Integer range <>) of aliased Domain_Class;
  type Domain_Class_Arr_At is access all Domain_Class_Arr;
  type Domain_Class_At_Arr is array(Integer range <>) of Domain_Class_At;
  type Domain_Class_At_Arr_At is access all Domain_Class_At_Arr;

  -- Constructors
  function Create return Domain_Class_At;

  overriding procedure Serialize( Self : in out Domain_Class; archiver : ArchiverInOut_Class_At);

  -- Returns a newely allocated deep copy/clone of Self.
  overriding function Clone( Self : Domain_Class ) return OpsObject_Class_At;

  -- Fills the parameter obj with all values from Self.
  overriding procedure FillClone( Self : Domain_Class; obj : OpsObject_Class_At );

  -- Returns references to the internal topics
  -- NOTE: The Domain still owns them
  function getTopics( Self : Domain_Class ) return Topic_Class_At_Arr_At;

  -- Returns a reference to the internal topic
  -- NOTE: The Domain still owns it
  function getTopic( Self : Domain_Class; Name : String ) return Topic_Class_At;

  function topicExist( Self : Domain_Class; Name : String ) return Boolean;

  -- Getters
  function DomainAddress( Self : Domain_Class ) return String;
  function DomainID( Self : Domain_Class ) return String;
  function MetaDataMcPort( Self : Domain_Class ) return Int32;
  function TimeToLive( Self : Domain_Class ) return Int32;
  function LocalInterface( Self : Domain_Class ) return String;
  function InSocketBufferSize( Self : Domain_Class ) return Int32;
  function OutSocketBufferSize( Self : Domain_Class ) return Int32;
  function OptNonVirt( Self : Domain_Class ) return Boolean;
  function HeartbeatPeriod( Self : Domain_Class ) return Int32;
  function HeartbeatTimeout( Self : Domain_Class ) return Int32;

  function Domain_version( Self : Domain_Class ) return Byte;
  procedure SetDomain_version( Self : in out Domain_Class; Version : Byte );

private
-- ==========================================================================
--
-- ==========================================================================
  type Domain_Class is new OpsObject_Class with
    record
      Domain_version : Byte := 0;
      DomainAddress : String_At := null;
      TimeToLive : Int32 := 1;
      LocalInterface : String_At := new String'("0.0.0.0");
      InSocketBufferSize : Int32 := -1;
      OutSocketBufferSize : Int32 := -1;
      Topics : Topic_Class_At_Arr_At := null;
      DomainID : String_At := null;
      MetaDataMcPort : Int32 := 9494;
      Channels : Channel_Class_At_Arr_At := null;
      Transports : Transport_Class_At_Arr_At := null;
      OptNonVirt : Boolean := False;
      HeartbeatPeriod : Int32 := 1000;
      HeartbeatTimeout : Int32 := 3000;
    end record;

  procedure checkTopicValues( Self : Domain_Class; top : Topic_Class_At);
  procedure checkTransports( Self : in out Domain_Class );
  function findChannel( Self : Domain_Class; id : String ) return Channel_Class_At;
  function findTopic( Self : Domain_Class; id : String) return  Topic_Class_At;

  procedure InitInstance( Self : in out Domain_Class );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out Domain_Class );

end Ops_Pa.OpsObject_Pa.Domain_Pa;

