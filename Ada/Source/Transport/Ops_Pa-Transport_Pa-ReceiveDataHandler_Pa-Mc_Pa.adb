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

package body Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa.Mc_Pa is

  function Create( top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                   dom : Ops_Pa.OpsObject_Pa.Domain_Pa.Domain_Class_At;
                   opsObjectFactory : Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.SerializableInheritingTypeFactory_Class_At;
                   Reporter : Ops_Pa.Error_Pa.ErrorService_Class_At ) return McReceiveDataHandler_Class_At is
    Self : McReceiveDataHandler_Class_At := null;
  begin
    Self := new McReceiveDataHandler_Class;
    InitInstance( Self.all, Self, top, dom, opsObjectFactory, Reporter );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out McReceiveDataHandler_Class;
                          SelfAt : McReceiveDataHandler_Class_At;
                          top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                          dom : Ops_Pa.OpsObject_Pa.Domain_Pa.Domain_Class_At;
                          opsObjectFactory : Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.SerializableInheritingTypeFactory_Class_At;
                          Reporter : Ops_Pa.Error_Pa.ErrorService_Class_At ) is
  begin
    InitInstance( ReceiveDataHandler_Class(Self), ReceiveDataHandler_Class_At(SelfAt), top, dom, opsObjectFactory, Reporter,
                  ReceiveDataChannel_Pa.Create( top, dom, opsObjectFactory, Reporter ));
  end;

  -- Tell derived classes which topics that are active
  overriding procedure topicUsage( Self : in out McReceiveDataHandler_Class;
                                   top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                                   used : Boolean) is
  begin
    null;
  end;

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out McReceiveDataHandler_Class ) is
  begin
    Finalize( ReceiveDataHandler_Class(Self) );
  end;

end Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa.Mc_Pa;

