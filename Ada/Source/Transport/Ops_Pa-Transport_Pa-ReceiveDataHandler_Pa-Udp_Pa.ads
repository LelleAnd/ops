--
-- Copyright (C) 2021 Lennart Andersson.
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

with Ops_Pa.OpsObject_Pa.Domain_Pa,
     Ops_Pa.OpsObject_Pa.Topic_Pa,
     Ops_Pa.Transport_Pa.ReceiveDataHandlerFactory_Pa,
     Ops_Pa.SerializableFactory_Pa.CompFactory_Pa,
     Ops_Pa.Error_Pa;

package Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa.Udp_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type UdpReceiveDataHandler_Class is new ReceiveDataHandler_Class with
      private;
  type UdpReceiveDataHandler_Class_At is access all UdpReceiveDataHandler_Class'Class;

  function Create( top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                   dom : Ops_Pa.OpsObject_Pa.Domain_Pa.Domain_Class_At;
                   opsObjectFactory : Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.SerializableInheritingTypeFactory_Class_At;
                   Reporter : Ops_Pa.Error_Pa.ErrorService_Class_At;
                   client : ReceiveDataHandlerFactory_Pa.OnSetupTransport_Interface_At ) return UdpReceiveDataHandler_Class_At;

private
-- ==========================================================================
--
-- ==========================================================================
  type UdpReceiveDataHandler_Class is new ReceiveDataHandler_Class with
    null record;

    -- Tell derived classes which topics that are active
  overriding procedure topicUsage( Self : in out UdpReceiveDataHandler_Class;
                                   top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                                   used : Boolean);

  procedure InitInstance( Self : in out UdpReceiveDataHandler_Class;
                          SelfAt : UdpReceiveDataHandler_Class_At;
                          top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                          dom : Ops_Pa.OpsObject_Pa.Domain_Pa.Domain_Class_At;
                          opsObjectFactory : Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.SerializableInheritingTypeFactory_Class_At;
                          Reporter : Ops_Pa.Error_Pa.ErrorService_Class_At;
                          client : ReceiveDataHandlerFactory_Pa.OnSetupTransport_Interface_At );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out UdpReceiveDataHandler_Class );

end Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa.Udp_Pa;

