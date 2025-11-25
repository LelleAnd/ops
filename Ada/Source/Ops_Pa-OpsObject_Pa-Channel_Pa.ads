--
-- Copyright (C) 2016-2025 Lennart Andersson.
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
     Ops_Pa.OpsObject_Pa.Topic_Pa;
use  Ops_Pa.ArchiverInOut_Pa,
     Ops_Pa.OpsObject_Pa.Topic_Pa;

package Ops_Pa.OpsObject_Pa.Channel_Pa is

  Channel_idlVersion : constant Byte := 0;

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type Channel_Class is new OpsObject_Class with
    record
      Channel_version : Byte := Channel_idlVersion;
      ChannelID : String_At := new String'("");
      Linktype : String_At := new String'("");
      LocalInterface : String_At := new String'(""); -- If multicast, this specifies interface to use
      DomainAddress : String_At := new String'("");
      TimeToLive : Int32 := -1;                      -- If multicast, this specifies the ttl parameter
      Port : Int32 := 0;
      OutSocketBufferSize : Int64 := -1;
      InSocketBufferSize : Int64 := -1;
    end record;
  type Channel_Class_At is access all Channel_Class'Class;

  -- Help types used in case other has vector's of the class
  type Channel_Class_Arr is array(Integer range <>) of aliased Channel_Class;
  type Channel_Class_Arr_At is access all Channel_Class_Arr;
  type Channel_Class_At_Arr is array(Integer range <>) of Channel_Class_At;
  type Channel_Class_At_Arr_At is access all Channel_Class_At_Arr;

  LINKTYPE_INPROC : constant String := "inprocess";
  LINKTYPE_MC     : constant String := "multicast";
  LINKTYPE_TCP    : constant String := "tcp";
  LINKTYPE_UDP    : constant String := "udp";
  LINKTYPE_SHMEM  : constant String := "shmem";

  -- Constructors
  function Create return Channel_Class_At;

  overriding procedure Serialize( Self : in out Channel_Class; archiver : ArchiverInOut_Class_At);

  -- Returns a newely allocated deep copy/clone of this object.
  overriding function Clone( Self : Channel_Class ) return OpsObject_Class_At;

  -- Fills the parameter obj with all values from this object.
  overriding procedure FillClone( Self : Channel_Class; obj : OpsObject_Class_At );

  procedure PopulateTopic( Self : in out Channel_Class; top : in out Topic_Class_At );

private
-- ==========================================================================
--
-- ==========================================================================

  procedure InitInstance( Self : in out Channel_Class );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out Channel_Class );


end Ops_Pa.OpsObject_Pa.Channel_Pa;

