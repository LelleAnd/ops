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
  Ops_Pa.OpsObject_Pa.Topic_Pa,
  Ops_Pa.OpsObject_Pa.TopicInfoData_Pa;
use
  Ops_Pa.OpsObject_Pa.Topic_Pa,
  Ops_Pa.OpsObject_Pa.TopicInfoData_Pa;

package Ops_Pa.OpsObject_Pa.ParticipantInfoData_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type ParticipantInfoData_Class is new OpsObject_Class with
    record
      ParticipantInfoData_version : Byte := 0;
      name : String_At := null;
      domain : String_At := null;
      id : String_At := null;
      ip : String_At := null;
      languageImplementation : String_At := null;
      opsVersion : String_At := null;
      mc_udp_port : Int32 := 0;
      mc_tcp_port : Int32 := 0;
      subscribeTopics : TopicInfoData_Class_At_Arr_At := null;
      publishTopics : TopicInfoData_Class_At_Arr_At := null;
      knownTypes : String_Arr_At := null;
    end record;
  type ParticipantInfoData_Class_At is access all ParticipantInfoData_Class'Class;

  -- Help types used in case other idl's has vector's of the class
  type ParticipantInfoData_Class_Arr is array(Integer range <>) of aliased ParticipantInfoData_Class;
  type ParticipantInfoData_Class_Arr_At is access all ParticipantInfoData_Class_Arr;
  type ParticipantInfoData_Class_At_Arr is array(Integer range <>) of ParticipantInfoData_Class_At;
  type ParticipantInfoData_Class_At_Arr_At is access all ParticipantInfoData_Class_At_Arr;

  -- Constructors
  function Create return ParticipantInfoData_Class_At;

  overriding procedure Serialize( Self : in out ParticipantInfoData_Class; archiver : ArchiverInOut_Class_At);

  -- Returns a newely allocated deep copy/clone of Self.
  overriding function Clone( Self : ParticipantInfoData_Class ) return OpsObject_Class_At;

  -- Fills the parameter obj with all values from Self.
  overriding procedure FillClone( Self : ParticipantInfoData_Class; obj : OpsObject_Class_At );

  -- Help routines for updating the dynamic arrays with TopicInfoData
  procedure addTopic( arr : in out TopicInfoData_Class_At_Arr_At; top : Topic_Class_At);
  procedure removeTopic( arr : in out TopicInfoData_Class_At_Arr_At; top : Topic_Class_At);

private
-- ==========================================================================
--
-- ==========================================================================

  procedure InitInstance( Self : in out ParticipantInfoData_Class );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out ParticipantInfoData_Class );

end Ops_Pa.OpsObject_Pa.ParticipantInfoData_Pa;

