--
-- Copyright (C) 2016-2017 Lennart Andersson.
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

with Ops_Pa.OpsObject_Pa.Topic_Pa,
     Ops_Pa.Error_Pa;

use Ops_Pa.OpsObject_Pa.Topic_Pa,
    Ops_Pa.Error_Pa;

package body Ops_Pa.Transport_Pa.ReceiveDataHandlerFactory_Pa is

  use type MyMap.cursor;

  function Create( Proc : TOnUdpTransportInfoProc;
                   Reporter : Ops_Pa.Error_Pa.ErrorService_Class_At )
                  return ReceiveDataHandlerFactory_Class_At is
    Self : ReceiveDataHandlerFactory_Class_At := null;
  begin
    Self := new ReceiveDataHandlerFactory_Class;
    InitInstance( Self.all, Proc, Reporter );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  function Less (Left, Right : String) return Boolean is
  begin
    return Left < Right;
  end;

  function Equal (Left, Right : HandlerInfo) return Boolean is
  begin
    return Left.handler = Right.handler; -- and Left.numUsers = Right.numUsers;
  end;

  procedure InitInstance( Self : in out ReceiveDataHandlerFactory_Class;
                          Proc : TOnUdpTransportInfoProc;
                          Reporter : Ops_Pa.Error_Pa.ErrorService_Class_At ) is
  begin
    Self.OnUdpTransportInfoProc := Proc;
    Self.ErrorService := Reporter;
  end;

  procedure Finalize( Self : in out ReceiveDataHandlerFactory_Class ) is

    procedure Process( Pos : in MyMap.Cursor ) is
      Value : HandlerInfo := MyMap.Element(Pos);
    begin
      if Value.handler /= null then
        Free(Value.handler);
      end if;
    end;

  begin
    -- Cleanup/Free all receivedatahandlers under protection
    Self.Lock.Acquire;
    begin
      Self.ReceiveDataHandlerInstances.Iterate(Process'Access);
      Self.Lock.Release;
    exception
      when others =>
        Self.Lock.Release;
        raise;
    end;
  end;


  function makeKey( top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At ) return String is
  begin
    -- Since topics can use the same port for transports multicast & tcp, or
    -- use transport udp which always use a single ReceiveDataHandler,
    -- we need to return the same ReceiveDataHandler in these cases.
    -- Make a key with the transport info that uniquely defines the receiver.
    if top.Transport = TRANSPORT_UDP then
      return top.Transport;
    else
      return top.Transport & "::" & top.DomainAddress & "::" & Int32'Image(top.Port);
    end if;
  end;

  function getReceiveDataHandler( Self : in out ReceiveDataHandlerFactory_Class;
                                  top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At;
                                  dom : Ops_Pa.OpsObject_Pa.Domain_Pa.Domain_Class_At;
                                  opsObjectFactory : Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.SerializableInheritingTypeFactory_Class_At )
                                 return ReceiveDataHandler_Class_At is

    procedure Report(msg : string) is
    begin
      if Self.ErrorService /= null then
        Self.ErrorService.Report( "ReceiveDataHandlerFactory", "getReceiveDataHandler", msg );
      end if;
    end;

    -- Make a key with the transport info that uniquely defines the receiver.
    key : String := makeKey(top);
    Result : ReceiveDataHandler_Class_At := null;

    pos : MyMap.Cursor;
    info : HandlerInfo;

  begin
    Self.Lock.Acquire;
    begin
      pos := Self.ReceiveDataHandlerInstances.Find( key );

      if pos /= MyMap.No_Element then
        -- If we already have a ReceiveDataHandler for this topic, use it.
        info := MyMap.Element(pos);
        Result := info.handler;

        -- Check if any of the topics have a sample size larger than MAX_SEGMENT_SIZE
        -- This will lead to a problem when using the same port or using UDP, if samples becomes > MAX_SEGMENT_SIZE
        if ((Result.GetSampleMaxSize > PACKET_MAX_SIZE) or (top.SampleMaxSize > PACKET_MAX_SIZE)) then
          if top.Transport = TRANSPORT_UDP then
            Report("Warning: UDP Transport is used with Topics with 'sampleMaxSize' > " & Integer'Image(PACKET_MAX_SIZE));
          else
            Report("Warning: Same port (" & Int32'Image(top.Port) &
                     ") is used with Topics with 'sampleMaxSize' > " & Integer'Image(PACKET_MAX_SIZE));
          end if;
        end if;

      elsif (top.Transport = TRANSPORT_MC) or (top.Transport = TRANSPORT_TCP) then
        Result := Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa.Create(top, dom, opsObjectFactory, Self.ErrorService);
        info.handler := Result;
        --info.numUsers := 1;
        Self.ReceiveDataHandlerInstances.Insert(key, info);

      elsif top.Transport = TRANSPORT_UDP then
        Result := Ops_Pa.Transport_Pa.ReceiveDataHandler_Pa.Create(top, dom, opsObjectFactory, Self.ErrorService);

--///TODO        if Assigned(FOnUdpTransportInfoProc) then
--          FOnUdpTransportInfoProc(
--                                  (Result.getReceiver as TUDPReceiver).Address,
--                                  (Result.getReceiver as TUDPReceiver).Port);
--        end if;

        info.handler := Result;
        --info.numUsers := 1;
        Self.ReceiveDataHandlerInstances.Insert(key, info);

      else  -- For now we can not handle more transports
        Report("Unknown transport for Topic: " & top.Name);
        -- Signal an error by returning nil.

      end if;
      Self.Lock.Release;
    exception
      when others =>
        Self.Lock.Release;
    end;
    return Result;
  end;

  procedure releaseReceiveDataHandler( Self : in out ReceiveDataHandlerFactory_Class;
                                       top : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At ) is

    -- Make a key with the transport info that uniquely defines the receiver.
    key : String := makeKey(top);
    rdh : ReceiveDataHandler_Class_At := null;

    pos : MyMap.Cursor;
    info : HandlerInfo;

  begin
    Self.Lock.Acquire;
    begin
      pos := Self.ReceiveDataHandlerInstances.Find( key );

      if pos /= MyMap.No_Element then
        info := MyMap.Element(pos);
        rdh := info.handler;
        if rdh.GetNumListeners = 0 then
          -- Start by removing it from the active list
          -- Remove it from the dictionary and then delete it
          Self.ReceiveDataHandlerInstances.Delete(pos);

          Free(rdh);

--///TODO          if top.Transport = TRANSPORT_UDP then
--            if FOnUdpTransportInfoProc /= null then
--              FOnUdpTransportInfoProc("", 0);
--            end if;
--          end if;
        end if;
      end if;
      Self.Lock.Release;
    exception
      when others =>
        Self.Lock.Release;
    end;
  end;

end Ops_Pa.Transport_Pa.ReceiveDataHandlerFactory_Pa;
