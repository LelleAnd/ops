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

with Ops_Pa.ByteBuffer_Pa,
     Ops_Pa.MemoryMap_Pa,
     Ops_Pa.ArchiverInOut_Pa.ArchiverOut_Pa,
     Ops_Pa.Participant_Pa,
     Ops_Pa.OpsObject_Pa,
     Ops_Pa.OpsObject_Pa.Topic_Pa,
     OPs_Pa.OpsObject_Pa.OPSMessage_Pa,
     Ops_Pa.Transport_Pa.SendDataHandler_Pa;

use  Ops_Pa.ByteBuffer_Pa,
     Ops_Pa.MemoryMap_Pa,
     Ops_Pa.ArchiverInOut_Pa.ArchiverOut_Pa,
     Ops_Pa.Participant_Pa,
     Ops_Pa.OpsObject_Pa,
     Ops_Pa.OpsObject_Pa.Topic_Pa,
     OPs_Pa.OpsObject_Pa.OPSMessage_Pa,
     Ops_Pa.Transport_Pa.SendDataHandler_Pa;

package Ops_Pa.PublisherAbs_Pa.Publisher_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type Publisher_Class is new PublisherAbs_Class and
    Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface with
      private;
  type Publisher_Class_At is access all Publisher_Class'Class;

  -- Constructors
  function Create( t : Topic_Class_At ) return Publisher_Class_At;

  overriding procedure Start( Self : in out Publisher_Class );
  overriding procedure Stop( Self : in out Publisher_Class );

  -- Return connect status (currently only valid for TCP)
  function isConnected( Self : Publisher_Class ) return Boolean;

  procedure addListener( Self : in out Publisher_Class; Client : Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface_At );
  procedure removeListener( Self : in out Publisher_Class; Client : Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface_At );

  overriding procedure WriteOPSObject( Self : in out Publisher_Class; obj : OpsObject_Class_At );

  -- Methods to store and retrieve user parameters.
  -- These are not used by OPS in any way, the usage is up to the user of OPS.
  procedure SetUserParam( Self : in out Publisher_Class; Value : UInt64 );
  procedure SetUserRef( Self : in out Publisher_Class; Value : Ops_Class_At );
  function GetUserParam( Self : Publisher_Class ) return UInt64;
  function GetUserRef( Self : Publisher_Class ) return Ops_Class_At;

private
-- ==========================================================================
--
-- ==========================================================================
  type Publisher_Class is new PublisherAbs_Class and
    Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface with
    record
      SelfAt : Publisher_Class_At := null;
      CurrentPublicationID : Int64 := 0;

      MemMap : MemoryMap_Class_At := null;
      Buf : ByteBuffer_Class_At := null;
      Archive : ArchiverOut_Class_At := null;
      Message : OPSMessage_Class_At := null;

      CsNotifier : Transport_Pa.ConnectStatusNotifier_Pa.Notifier_Class_At := null;

      -- Keep status for current ConnectStatus
      Connected : Boolean := False;

      -- The Participant to which this Publisher belongs (NOTE: we don't own the object)
      Participant : Participant_Class_At := null;

      -- The SendDataHandler used by the Publisher (NOTE: we don't own the object)
      SendDataHandler : SendDataHandler_Class_At := null;
      UseInProc : Boolean := False;

      -- User variables
      UserParam : UInt64 := 0;
      UserRef : Ops_Class_At := null;
    end record;

  procedure Write( Self : in out Publisher_Class; data : OpsObject_Class_At);

  procedure OnNotify( Self : in out Publisher_Class; Sender : in Ops_Class_At; Item : in Transport_Pa.ConnectStatus_T );

  procedure InitInstance( Self : in out Publisher_Class;
                          SelfAt : Publisher_Class_At;
                          t : Topic_Class_At);

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out Publisher_Class );

end Ops_Pa.PublisherAbs_Pa.Publisher_Pa;

