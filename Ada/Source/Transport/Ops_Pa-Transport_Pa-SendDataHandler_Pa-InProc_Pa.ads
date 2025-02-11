--
-- Copyright (C) 2024-2025 Lennart Andersson.
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

with Ops_Pa.OpsObject_Pa.OPSMessage_Pa;
use Ops_Pa.OpsObject_Pa.OPSMessage_Pa;

with Ops_Pa.Transport_Pa.InProcDistributor_Pa;
use Ops_Pa.Transport_Pa.InProcDistributor_Pa;

package Ops_Pa.Transport_Pa.SendDataHandler_Pa.InProc_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type InProcSendDataHandler_Class    is new SendDataHandler_Class with private;
  type InProcSendDataHandler_Class_At is access all InProcSendDataHandler_Class'Class;

  function Create(topic : Topic_Class_At; distributor : InProcDistributor_Class_At ) return InProcSendDataHandler_Class_At;

  overriding function sendData( Self : in out InProcSendDataHandler_Class; buf : Byte_Arr_At; bufSize : Integer; topic : Topic_Class_At ) return Boolean;

  -- Used for inprocess transport
  overriding function sendMessage( Self : in out InProcSendDataHandler_Class; topic : Topic_Class_At; message : OPSMessage_Class_At ) return Boolean;

private
-- ==========================================================================
--
-- ==========================================================================
  type InProcSendDataHandler_Class is new SendDataHandler_Class with record
    distributor : InProcDistributor_Class_At;
  end record;

  procedure InitInstance( Self : in out InProcSendDataHandler_Class;
                          SelfAt : InProcSendDataHandler_Class_At;
                          topic : Topic_Class_At;
                          distributor : InProcDistributor_Class_At );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out InProcSendDataHandler_Class );


end Ops_Pa.Transport_Pa.SendDataHandler_Pa.InProc_Pa;
