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

with Ops_Pa.Error_Pa,
     Ops_Pa.OpsObject_Pa,
     Ops_Pa.OpsObject_Pa.OPSMessage_Pa;

use Ops_Pa.Error_Pa,
    Ops_Pa.OpsObject_Pa,
    Ops_Pa.OpsObject_Pa.OPSMessage_Pa;

package body Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa.InProc_Pa is

  use type ReceiveDataHandlerFactory_Pa.OnSetupTransport_Interface_At;
  use type MyMap.cursor;

  function Less (Left, Right : String) return Boolean is
  begin
    return Left < Right;
  end;

  function Equal (Left, Right : TopicInfo) return Boolean is
  begin
    return Left.Num = Right.Num;
  end;

  function Create( top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                   dom : Ops_Pa.OpsObject_Pa.Domain_Pa.Domain_Class_At;
                   opsObjectFactory : Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.SerializableInheritingTypeFactory_Class_At;
                   Reporter : Ops_Pa.Error_Pa.ErrorService_Class_At;
                   InProcDistributor : Ops_Pa.Transport_Pa.InProcDistributor_Pa.InProcDistributor_Class_At) return InProcReceiveDataHandler_Class_At is
    Self : InProcReceiveDataHandler_Class_At := null;
  begin
    Self := new InProcReceiveDataHandler_Class;
    InitInstance( Self.all, Self, top, dom, opsObjectFactory, Reporter, InProcDistributor );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out InProcReceiveDataHandler_Class;
                          SelfAt : InProcReceiveDataHandler_Class_At;
                          top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                          dom : Ops_Pa.OpsObject_Pa.Domain_Pa.Domain_Class_At;
                          opsObjectFactory : Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.SerializableInheritingTypeFactory_Class_At;
                          Reporter : Ops_Pa.Error_Pa.ErrorService_Class_At;
                          distributor : InProcDistributor_Class_At) is
  begin
    InitInstance( ReceiveDataHandler_Class(Self), ReceiveDataHandler_Class_At(SelfAt), top, dom, opsObjectFactory, Reporter,
                  null );
    Self.distributor := distributor;
  end;

  --  type OnNotifyEvent_T is access procedure( Sender : in InProcDistributor_Class_At; Rcv : Ops_Class_At; msg : OPSMessage_Class_At );
  procedure HandleMessage( Sender : in InProcDistributor_Class_At; Rcv : Ops_Class_At; msg : OPSMessage_Class_At ) is
    mess : OPSMessage_Class_At := OPSMessage_Class_At(msg.all.Clone);
  begin
    mess.all.SetDataOwner( True );

    -- procedure OnNotify( Self : in out ReceiveDataHandler_Class; Sender : in Ops_Class_At; Item : in OPSMessage_Class_At ) is
    ReceiveDataHandler_Class_At(Rcv).all.OnNotify( null, mess );
  end;

   -- Tell derived classes which topics that are active
  overriding procedure topicUsage( Self : in out InProcReceiveDataHandler_Class;
                                   top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                                   used : Boolean) is
    pos : MyMap.Cursor;
    info : TopicInfo;
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.TopicLock'Access);
  begin
    -- We should only register unique topics, so need to count
    pos := Self.TopicInfoMap.Find( top.Name );
    if pos /= MyMap.No_Element then
      info := MyMap.Element(pos);
    end if;
    if used then
      info.Num := info.Num + 1;
      if info.Num = 1 then
        Self.distributor.regInProcReceiver( top, Ops_Class_At(Self.SelfAt), HandleMessage'Access );
      end if;
    else
      info.Num := info.Num - 1;
      if info.Num = 0 then
        Self.distributor.unregInProcReceiver( top, Ops_Class_At(Self.SelfAt), HandleMessage'Access );
      end if;
    end if;
    if pos /= MyMap.No_Element then
      Self.TopicInfoMap.Replace_Element( pos, info );
    else
      Self.TopicInfoMap.Insert( top.Name, info );
    end if;
  end;

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out InProcReceiveDataHandler_Class ) is
  begin
    Finalize( ReceiveDataHandler_Class(Self) );
  end;

end Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa.InProc_Pa;

