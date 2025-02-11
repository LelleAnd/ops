--
-- Copyright (C) 2016-2025 Lennart Andersson.
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

with Ops_Pa.Transport_Pa.SendDataHandler_Pa.Mc_Pa,
     Ops_Pa.Transport_Pa.SendDataHandler_Pa.McUdp_Pa,
     Ops_Pa.Transport_Pa.SendDataHandler_Pa.TCP_Pa,
     Ops_Pa.Transport_Pa.SendDataHandler_Pa.InProc_Pa;

with Ops_Pa.Socket_Pa;

package body Ops_Pa.Transport_Pa.SendDataHandlerFactory_Pa is

  use type MyMap.cursor;

  procedure Trace(Self : SendDataHandlerFactory_Class; Msg : String) is
    NameStr : String := "SendDataHandlerFactory";
  begin
    Trace(NameStr, Msg);
  end;

  -- Constructors
  function Create(Dom : Domain_Class_At;
                  Client : OnUdpTransport_Interface_At;
                  Reporter : ErrorService_Class_At;
                  InProcDistributor : Transport_Pa.InProcDistributor_Pa.InProcDistributor_Class_At ) return SendDataHandlerFactory_Class_At is
     Self : SendDataHandlerFactory_Class_At := null;
  begin
    Self := new SendDataHandlerFactory_Class;
    InitInstance( Self.all, Dom, Client, Reporter, InProcDistributor );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  function Less (Left, Right : String) return Boolean is
  begin
    return Left < Right;
  end;

  function Equal (Left, Right : HandlerInfo) return Boolean is
  begin
    return Left.handler = Right.handler and Left.numUsers = Right.numUsers;
  end;

  -- Generate the key used in the dictionary
  function getKey(top : Topic_Class_At) return String is
  begin
    if top.Transport = TRANSPORT_INPROC then
      return top.Transport;

    elsif top.Transport = TRANSPORT_TCP and top.Port = 0 then
      -- We add the channel name so different channels get different TCP Servers
      return top.Transport & "::" & top.ChannelId & "::" & top.DomainAddress & "::" & Int32'Image(top.Port);

    else
      return top.Transport & "::" & top.DomainAddress & "::" & Int32'Image(top.Port);
    end if;
  end;

  function getSendDataHandler( Self : in out SendDataHandlerFactory_Class; top : Topic_Class_At) return SendDataHandler_Class_At is

    procedure Inc( key : in String; element : in out HandlerInfo ) is
    begin
      element.numUsers := element.numUsers + 1;
    end;

    localIf : string := Ops_Pa.Socket_Pa.doSubnetTranslation(top.LocalInterface);
    ttl : Integer := Integer(top.TimeToLive);
    result : SendDataHandler_Class_At := null;
    pos : MyMap.Cursor;
    info : HandlerInfo;

    -- We need to store SendDataHandlers with more than just the name as key.
    -- Since topics can use the same port, we need to return the same SendDataHandler.
    -- Make a key with the transport info that uniquely defines the receiver.
    key : String := getKey(top);

    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.Mutex'Access);
  begin
    if TraceEnabled then Self.Trace("getSendDataHandler(), Key: " & key); end if;
    pos := Self.SendDataHandlers.Find( key );

    if pos /= MyMap.No_Element then
      -- Increment usage count
      Self.SendDataHandlers.Update_Element(pos, Process => Inc'Access);
      -- Return found handler
      info := MyMap.Element(pos);
      return info.handler;
    end if;

    if top.Transport = TRANSPORT_MC then
      Result := SendDataHandler_Class_At(Ops_Pa.Transport_Pa.SendDataHandler_Pa.Mc_Pa.Create(top, localIf, ttl, Self.ErrorService));
      info.handler := Result;
      info.numUsers := 1;
      Self.SendDataHandlers.Insert(key, info);

    elsif top.Transport = TRANSPORT_UDP then
      if Self.UdpSendDataHandler = null then
        -- We have only one sender for all topics, so use the domain value for buffer size
        Self.UdpSendDataHandler :=
          SendDataHandler_Class_At(Ops_Pa.Transport_Pa.SendDataHandler_Pa.McUdp_Pa.Create(
                                   localIf,
                                   ttl,
                                   Int64(Self.Domain.OutSocketBufferSize),
                                   Self.ErrorService));
      end if;

      -- If topic specifies a valid node address, add that as a static destination address for topic
      if Ops_Pa.Socket_Pa.isValidNodeAddress(top.DomainAddress) then
        declare
          topName : String := top.Name;
          destAddress : String := top.DomainAddress;
          destPort : Int32 := top.Port;
        begin
          Ops_Pa.Transport_Pa.SendDataHandler_Pa.McUdp_Pa.McUdpSendDataHandler_Class_At(Self.UdpSendDataHandler).
            addSink(topName, destAddress, destPort, True);
        end;

      else
        -- Setup a listener on the participant info data published by participants on our domain.
        -- We use the information for topics with UDP as transport, to know the destination for UDP sends
        -- ie. we extract ip and port from the information and add it to our McUdpSendDataHandler
        if Self.OnUdpConnectDisconnectClient /= null then
          Self.OnUdpConnectDisconnectClient.OnUdpTransport(top, Self.UdpSendDataHandler, True);
        end if;
      end if;
      Self.UdpUsers := Self.UdpUsers + 1;

      Result := Self.UdpSendDataHandler;

    elsif top.Transport = TRANSPORT_TCP then
      Result := SendDataHandler_Class_At(Ops_Pa.Transport_Pa.SendDataHandler_Pa.TCP_Pa.Create(top, Self.ErrorService));
      info.handler := Result;
      info.numUsers := 1;
      Self.SendDataHandlers.Insert(key, info);

    elsif top.Transport = TRANSPORT_INPROC then
      Result := SendDataHandler_Class_At(Ops_Pa.Transport_Pa.SendDataHandler_Pa.InProc_Pa.Create(top, Self.InProcDistributor));
      info.handler := Result;
      info.numUsers := 1;
      Self.SendDataHandlers.Insert(key, info);

    end if;
    return Result;
  end;

  procedure releaseSendDataHandler( Self : in out SendDataHandlerFactory_Class; top : Topic_Class_At) is

    procedure Dec( key : in String; element : in out HandlerInfo ) is
    begin
      element.numUsers := element.numUsers - 1;
    end;

    key : String := getKey(top);
    pos : MyMap.Cursor;
    info : HandlerInfo;

    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.Mutex'Access);
  begin
    if TraceEnabled then Self.Trace("releaseSendDataHandler(), Key: " & key); end if;
    if top.Transport = TRANSPORT_UDP then
      if self.OnUdpConnectDisconnectClient /= null then
        Self.OnUdpConnectDisconnectClient.OnUdpTransport(top, Self.UdpSendDataHandler, False);
      end if;
      Self.UdpUsers := Self.UdpUsers - 1;
      if Self.UdpUsers = 0 then
        -- This is the last matching call, so now no one is using the SendDataHandler
        -- Delete it
        Free(Self.UdpSendDataHandler);
        Self.UdpSendDataHandler := null;
      end if;

    else
      pos := Self.SendDataHandlers.Find( key );

      if pos /= MyMap.No_Element then
        -- Decrement usage count
        Self.SendDataHandlers.Update_Element(pos, Dec'Access);

        if MyMap.Element(pos).numUsers = 0 then
          -- This is the last matching call, so now no one is using the SendDataHandler
          -- Remove it from the dictionary and then delete it
          info := MyMap.Element(pos);
          Self.SendDataHandlers.Delete(pos);
          Free(info.handler);
        end if;
      end if;
    end if;
  end;

  procedure InitInstance( Self : in out SendDataHandlerFactory_Class;
                          Dom : Domain_Class_At;
                          Client : OnUdpTransport_Interface_At;
                          Reporter : ErrorService_Class_At;
                          InProcDistributor : Transport_Pa.InProcDistributor_Pa.InProcDistributor_Class_At ) is
  begin
    Self.ErrorService := Reporter;
    Self.Domain := dom;
    Self.InProcDistributor := InProcDistributor;
    Self.OnUdpConnectDisconnectClient := Client;
  end;

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out SendDataHandlerFactory_Class ) is

    handlerExist : Boolean := False;

    procedure Process( Pos : in MyMap.Cursor ) is
      Value : HandlerInfo := MyMap.Element(Pos);
    begin
      if Value.numUsers /= 0 then
        handlerExist := True;
      end if;
      if Value.handler /= null then
        Free(Value.handler);
      end if;
    end;

  begin
    declare
      -- Cleanup/Free all senddatahandlers under protection
      S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.Mutex'Access);
    begin
      if Self.UdpUsers /= 0 then
        handlerExist := True;
      end if;

      if Self.UdpSendDataHandler /= null then
        Free(Self.UdpSendDataHandler);
      end if;

      Self.SendDataHandlers.Iterate(Process'Access);
    end;

    if handlerExist then
      if Self.ErrorService /= null then
        Self.ErrorService.Report(
          "SendDataHandlerFactory", "Finalize", "Publishers still alive when deleting factory!!!" );
      end if;
    end if;
  end;

end Ops_Pa.Transport_Pa.SendDataHandlerFactory_Pa;

