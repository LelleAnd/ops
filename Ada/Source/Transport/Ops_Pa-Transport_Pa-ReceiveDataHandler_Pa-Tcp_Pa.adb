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
    Ops_Pa.OpsObject_Pa.Topic_Pa,
     Ops_Pa.OpsObject_Pa.OPSMessage_Pa;

use Ops_Pa.Error_Pa,
    Ops_Pa.OpsObject_Pa,
    Ops_Pa.OpsObject_Pa.Topic_Pa,
    Ops_Pa.OpsObject_Pa.OPSMessage_Pa;

package body Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa.Tcp_Pa is

  use type ReceiveDataChannel_Pa.ReceiveDataChannel_Class_At;
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
                   client : ReceiveDataHandlerFactory_Pa.OnSetupTransport_Interface_At ) return TcpReceiveDataHandler_Class_At is
    Self : TcpReceiveDataHandler_Class_At := null;
  begin
    Self := new TcpReceiveDataHandler_Class;
    InitInstance( Self.all, Self, top, dom, opsObjectFactory, Reporter, client );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure InitInstance( Self : in out TcpReceiveDataHandler_Class;
                          SelfAt : TcpReceiveDataHandler_Class_At;
                          top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                          dom : Ops_Pa.OpsObject_Pa.Domain_Pa.Domain_Class_At;
                          opsObjectFactory : Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.SerializableInheritingTypeFactory_Class_At;
                          Reporter : Ops_Pa.Error_Pa.ErrorService_Class_At;
                          client : ReceiveDataHandlerFactory_Pa.OnSetupTransport_Interface_At ) is
    Rdc : ReceiveDataChannel_Pa.ReceiveDataChannel_Class_At := null;
    key : String := top.DomainAddress & "::" & Int32'Image(top.Port);
  begin
    -- Make a local copy of the topic
    Self.Topic := Topic_Class_At(top.all.Clone);

    Self.Domain := dom;
    Self.opsObjectFactory := opsObjectFactory;
    Self.client := client;

    -- Handle TCP channels specified with an address and port
    if (top.Transport = TRANSPORT_TCP) and (top.Port /= 0) then
      Self.usingPartInfo := False;
      Rdc := ReceiveDataChannel_Pa.Create( top, dom, opsObjectFactory, Reporter );
      Rdc.SetKey( Key );
    else
      -- Since we use the same "topic" parameters for all created RDC's, we can set the sampleMaxSize here
      Self.SampleMaxSize := ReceiveDataChannel_Pa.calcSampleMaxSize(top);
    end if;

    InitInstance( ReceiveDataHandler_Class(Self), ReceiveDataHandler_Class_At(SelfAt), top, dom, opsObjectFactory, Reporter, Rdc );
  end;

  procedure AddReceiveChannel( Self : in out TcpReceiveDataHandler_Class; topicName : String; ip : String; port : Int32 ) is
    key : String := ip & "::" & Int32'Image(port);
    found : Boolean := False;
    Rdc : ReceiveDataChannel_Pa.ReceiveDataChannel_Class_At := null;
  begin
    -- We need to check if a new publisher has emerged that we need to connect to
    -- Look for it in rdc, if not there, create one
    for i in Self.Rdc.First_Index .. Self.Rdc.Last_Index loop
      if Self.Rdc.Element(i) /= null then
        if Self.Rdc.Element(i).Key = key then
          found := True;
          Exit;
        end if;
      end if;
    end loop;
    if not found then
      Self.Topic.setDomainAddress(ip);
      Self.Topic.setPort(port);
      Rdc := ReceiveDataChannel_Pa.Create( Self.Topic, Self.Domain, Self.opsObjectFactory, Self.ErrorService );
      Rdc.SetKey( Key );
      Self.AddRdc( Rdc );
    end if;
  end;

  -- Tell derived classes which topics that are active
  overriding procedure topicUsage( Self : in out TcpReceiveDataHandler_Class;
                                   top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                                   used : Boolean) is
  begin
    if Self.usingPartInfo then
      declare
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
            Self.client.OnTcpTransport(top.Name, rdh => Self.SelfAt, register => True);
          end if;
        else
          info.Num := info.Num - 1;
          if info.Num = 0 then
            Self.client.OnTcpTransport(top.Name, rdh => Self.SelfAt, register => False);
          end if;
        end if;
        if pos /= MyMap.No_Element then
          Self.TopicInfoMap.Replace_Element( pos, info );
        else
          Self.TopicInfoMap.Insert( top.Name, info );
        end if;
      end;
    end if;
  end;

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out TcpReceiveDataHandler_Class ) is
  begin
    Free(Self.Topic);
    Finalize( ReceiveDataHandler_Class(Self) );
  end;

end Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa.Tcp_Pa;

