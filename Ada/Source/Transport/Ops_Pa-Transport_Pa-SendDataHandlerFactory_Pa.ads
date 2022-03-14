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

with Ada.Containers.Indefinite_Ordered_Maps;

with Ops_Pa.Mutex_Pa;

with Ops_Pa.Error_Pa;
with Ops_Pa.Transport_Pa.SendDataHandler_Pa;
with Ops_Pa.OpsObject_Pa.Domain_Pa;
with Ops_Pa.OpsObject_Pa.Topic_Pa;

use Ops_Pa.Error_Pa;
use Ops_Pa.Transport_Pa.SendDataHandler_Pa;
use Ops_Pa.OpsObject_Pa.Domain_Pa;
use Ops_Pa.OpsObject_Pa.Topic_Pa;

package Ops_Pa.Transport_Pa.SendDataHandlerFactory_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type OnUdpTransport_Interface is limited interface;
  type OnUdpTransport_Interface_At is access all OnUdpTransport_Interface'Class;

  -- Method prototype to call when we connect/disconnect UDP topics with the participant info data listener
  -- Override this to react on the UDP setup callback
  procedure OnUdpTransport( Self : in out OnUdpTransport_Interface;
                            topic : Topic_Class_At;
                            sdh : SendDataHandler_Class_At;
                            Connect : Boolean ) is abstract;

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type SendDataHandlerFactory_Class    is new Ops_Class with private;
  type SendDataHandlerFactory_Class_At is access all SendDataHandlerFactory_Class'Class;

  -- Constructors
  function Create(Dom : Domain_Class_At;
                  Client : OnUdpTransport_Interface_At;
                  Reporter : ErrorService_Class_At) return SendDataHandlerFactory_Class_At;

  function getSendDataHandler( Self : in out SendDataHandlerFactory_Class; top : Topic_Class_At) return SendDataHandler_Class_At;
  procedure releaseSendDataHandler( Self : in out SendDataHandlerFactory_Class; top : Topic_Class_At);

  TraceEnabled : Boolean := False;

private
-- ==========================================================================
--
-- ==========================================================================
  type HandlerInfo is
    record
      handler : SendDataHandler_Class_At := null;
      numUsers : Integer := 0;
    end record;

  function Less (Left, Right : String) return Boolean;
  function Equal (Left, Right : HandlerInfo) return Boolean;

  package MyMap is new Ada.Containers.Indefinite_Ordered_Maps(String, HandlerInfo, Less, Equal);

-- ==========================================================================
--
-- ==========================================================================
  type SendDataHandlerFactory_Class is new Ops_Class with
    record
      -- Borrowed reference
      ErrorService : ErrorService_Class_At := null;

      -- The Domain to which this Factory belongs (NOTE: we don't own the object)
      Domain : Domain_Class_At := null;

      -- Client to call when we connect/disconnect UDP topics with the participant info data listener
      -- normally handled by the participant
      OnUdpConnectDisconnectClient : OnUdpTransport_Interface_At := null;

      -- There is only one McUdpSendDataHandler for each participant
      UdpSendDataHandler : SendDataHandler_Class_At := null;
      UdpUsers : Integer := 0;

      -- Dictionary with all SendDataHandlers except for UDP-transport
      SendDataHandlers : MyMap.Map;
      Mutex : aliased Ops_Pa.Mutex_Pa.Mutex;
    end record;

  -- Generate the key used in the dictionary
  function getKey( top : Topic_Class_At ) return String;

  procedure InitInstance( Self : in out SendDataHandlerFactory_Class;
                          Dom : Domain_Class_At;
                          Client : OnUdpTransport_Interface_At;
                          Reporter : ErrorService_Class_At );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out SendDataHandlerFactory_Class );

end Ops_Pa.Transport_Pa.SendDataHandlerFactory_Pa;

