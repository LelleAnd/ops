--
-- Copyright (C) 2017-2021 Lennart Andersson.
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

with Ops_Pa.Error_Pa;
use  Ops_Pa.Error_Pa;

package Ops_Pa.Transport_Pa.SendDataHandler_Pa.TCP_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type TcpSendDataHandler_Class    is new SendDataHandler_Class with private;
  type TcpSendDataHandler_Class_At is access all TcpSendDataHandler_Class'Class;

  function Create( topic : Topic_Class_At; Reporter : ErrorService_Class_At ) return TcpSendDataHandler_Class_At;

  overriding function sendData( Self : in out TcpSendDataHandler_Class; buf : Byte_Arr_At; bufSize : Integer; topic : Topic_Class_At) return Boolean;

  -- At least one publisher must be added to us for this call to work correct
  -- Updates topic with the used transport info
  overriding procedure updateTransportInfo( Self : in out TcpSendDataHandler_Class; topic : Topic_Class_At );

private
-- ==========================================================================
--
-- ==========================================================================
  type TcpSendDataHandler_Class is new SendDataHandler_Class with
    record
      null;
    end record;

  procedure InitInstance( Self : in out TcpSendDataHandler_Class;
                          SelfAt : in TcpSendDataHandler_Class_At;
                          topic : Topic_Class_At;
                          Reporter : ErrorService_Class_At );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out TcpSendDataHandler_Class );

end Ops_Pa.Transport_Pa.SendDataHandler_Pa.TCP_Pa;

