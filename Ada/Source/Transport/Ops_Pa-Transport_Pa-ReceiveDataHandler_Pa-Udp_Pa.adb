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

with Ops_Pa.Error_Pa,
     Ops_Pa.OpsObject_Pa,
     Ops_Pa.OpsObject_Pa.OPSMessage_Pa;

use Ops_Pa.Error_Pa,
    Ops_Pa.OpsObject_Pa,
    Ops_Pa.OpsObject_Pa.OPSMessage_Pa;

package body Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa.Udp_Pa is

  use type ReceiveDataHandlerFactory_Pa.OnSetupTransport_Interface_At;

  function Create( top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                   dom : Ops_Pa.OpsObject_Pa.Domain_Pa.Domain_Class_At;
                   opsObjectFactory : Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.SerializableInheritingTypeFactory_Class_At;
                   Reporter : Ops_Pa.Error_Pa.ErrorService_Class_At;
                   client : ReceiveDataHandlerFactory_Pa.OnSetupTransport_Interface_At ) return UdpReceiveDataHandler_Class_At is
    Self : UdpReceiveDataHandler_Class_At := null;
  begin
    Self := new UdpReceiveDataHandler_Class;
    InitInstance( Self.all, Self, top, dom, opsObjectFactory, Reporter, client );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out UdpReceiveDataHandler_Class;
                          SelfAt : UdpReceiveDataHandler_Class_At;
                          top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                          dom : Ops_Pa.OpsObject_Pa.Domain_Pa.Domain_Class_At;
                          opsObjectFactory : Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.SerializableInheritingTypeFactory_Class_At;
                          Reporter : Ops_Pa.Error_Pa.ErrorService_Class_At;
                          client : ReceiveDataHandlerFactory_Pa.OnSetupTransport_Interface_At ) is
  begin
    InitInstance( ReceiveDataHandler_Class(Self), ReceiveDataHandler_Class_At(SelfAt), top, dom, opsObjectFactory, Reporter,
                  ReceiveDataChannel_Pa.Create( top, dom, opsObjectFactory, Reporter ));

    if client /= null then
      client.OnUdpTransport( Self.Rdc.Element(0).getReceiver.Address,
                             Int32(Self.Rdc.Element(0).getReceiver.Port) );
    end if;
  end;

   -- Tell derived classes which topics that are active
  overriding procedure topicUsage( Self : in out UdpReceiveDataHandler_Class;
                                   top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                                   used : Boolean) is
  begin
    null;
  end;

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out UdpReceiveDataHandler_Class ) is
  begin
    Finalize( ReceiveDataHandler_Class(Self) );
  end;

end Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa.Udp_Pa;

