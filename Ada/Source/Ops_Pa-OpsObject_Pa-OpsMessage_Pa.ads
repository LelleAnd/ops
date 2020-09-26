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

with System.Atomic_Counters;

with Ops_Pa.ArchiverInOut_Pa;
use  Ops_Pa.ArchiverInOut_Pa;

package Ops_Pa.OpsObject_Pa.OPSMessage_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type OPSMessage_Class is new OpsObject_Class with private;
  type OPSMessage_Class_At is access all OPSMessage_Class'Class;

  -- Help types used in case other idl's has vector's of the class
  type OPSMessage_Class_Arr is array(Integer range <>) of aliased OPSMessage_Class;
  type OPSMessage_Class_Arr_At is access all OPSMessage_Class_Arr;
  type OPSMessage_Class_At_Arr is array(Integer range <>) of OPSMessage_Class_At;
  type OPSMessage_Class_At_Arr_At is access all OPSMessage_Class_At_Arr;

  -- Constructors
  function Create return OPSMessage_Class_At;

  overriding procedure Serialize( Self : in out OPSMessage_Class; archiver : ArchiverInOut_Class_At);

  -- Returns a newely allocated deep copy/clone of Self.
  overriding function Clone( Self : OPSMessage_Class ) return OpsObject_Class_At;

  -- Fills the parameter obj with all values from Self.
  overriding procedure FillClone( Self : OPSMessage_Class; obj : OpsObject_Class_At );

  --
  procedure setSource( Self : in out OPSMessage_Class; addr : string; port : Integer);

  function getSourceIP( Self : OPSMessage_Class ) return String;
  function getSourcePort( Self : OPSMessage_Class ) return Integer;

  -- Reservation handling
  -- --------------------
  -- Reserve()    increments the reservation counter
  -- Unreserve()  decrements the reservation counter and when it becomes 0 the
  --              message is deleted
  -- NOTE: If these calls are used, the object shouldn't be freed explicitly,
  -- the Unreserve() call will do that for you.
  procedure Reserve( Self : in out OPSMessage_Class );
  procedure UnReserve( Self : in out OPSMessage_Class );
  function NrOfReservations( Self : OPSMessage_Class ) return System.Atomic_Counters.Atomic_Unsigned;

  -- Getters/Setters
  function DataOwner( Self : OPSMessage_Class ) return Boolean;
  procedure SetDataOwner( Self : in out OPSMessage_Class; value : Boolean );
  function PublicationID( Self : OPSMessage_Class ) return Int64;
  procedure SetPublicationID( Self : in out OPSMessage_Class; value : Int64 );
  function PublisherName( Self : OPSMessage_Class ) return String;
  procedure SetPublisherName( Self : in out OPSMessage_Class; value : String );
  function TopicName( Self : OPSMessage_Class ) return String;
  procedure SetTopicName( Self : in out OPSMessage_Class; value : String );
  function Data( Self : OPSMessage_Class ) return OPSObject_Class_At;
  procedure SetData( Self : in out OPSMessage_Class; value : OPSObject_Class_At );

  function OPSMessage_version( Self : OpsMessage_Class ) return Byte;
  procedure SetOPSMessage_version( Self : in out OpsMessage_Class; Version : Byte );

private
-- ==========================================================================
--
-- ==========================================================================
  type OPSMessage_Class is new OpsObject_Class with
    record
      SelfAt : OPSMessage_Class_At := null;

      -- Serialized members
      OPSMessage_version : Byte := 0;
      messageType : Byte := 0;
      publisherPriority : Byte := 0;
      publicationID : Int64 := 0;
      publisherName : String_At := null;
      topicName : String_At := null;
      topLevelKey : String_At := null;
      address : String_At := null;
      data : OPSObject_Class_At := null;

      -- Non-Serialized members
      DataOwner : Boolean := False;
      SourcePort : Integer := 0;
      SourceIP : String_At := new String'("");

      -- Reservation handling
      NrOfReservations : aliased System.Atomic_Counters.Atomic_Unsigned := 0;
    end record;

  procedure InitInstance( Self : in out OPSMessage_Class; SelfAt : OPSMessage_Class_At );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out OPSMessage_Class );

end Ops_Pa.OpsObject_Pa.OPSMessage_Pa;

