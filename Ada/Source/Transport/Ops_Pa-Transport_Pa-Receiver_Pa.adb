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

with Ops_Pa.Transport_Pa.Receiver_Pa.MulticastReceiver_Pa,
     Ops_Pa.Transport_Pa.Receiver_Pa.UDPReceiver_Pa,
     Ops_Pa.Transport_Pa.Receiver_Pa.TCPClient_Pa;

use Ops_Pa.Transport_Pa.Receiver_Pa.MulticastReceiver_Pa,
    Ops_Pa.Transport_Pa.Receiver_Pa.UDPReceiver_Pa,
    Ops_Pa.Transport_Pa.Receiver_Pa.TCPClient_Pa;

with Ops_Pa.Socket_Pa;

package body Ops_Pa.Transport_Pa.Receiver_Pa is

  use type Ops_Pa.Signal_Pa.Event_T;

  procedure InitInstance( Self : in out Receiver_Class; SelfAt : Receiver_Class_At ) is
  begin
    Self.DataNotifier := ReceiverNotifier_Pa.Create( Ops_Class_At(SelfAt) );
    Self.Receiver_Pr.Start;
  end;

  overriding procedure Finalize( Self : in out Receiver_Class ) is
  begin
    Self.TerminateFlag := True;
    Self.EventsToTask.Signal(TerminateEvent_C);
    Self.Receiver_Pr.Finish;
    ReceiverNotifier_Pa.Free(Self.DataNotifier);
  end;

  procedure addListener( Self : in out Receiver_Class; Client : ReceiverNotifier_Pa.Listener_Interface_At ) is
  begin
    Self.DataNotifier.connectListener( Client );
  end;

  procedure removeListener( Self : in out Receiver_Class; Client : ReceiverNotifier_Pa.Listener_Interface_At ) is
  begin
    Self.DataNotifier.disconnectListener;
  end;

  task body Receiver_Pr_T is
    Events : Ops_Pa.Signal_Pa.Event_T;
  begin
    select
      accept Start;
      while not Self.TerminateFlag loop
        begin
          Self.EventsToTask.WaitForAny(Events);
          exit when (Events and TerminateEvent_C) /= Ops_Pa.Signal_Pa.NoEvent_C;
          if (Events and StartEvent_C) /= Ops_Pa.Signal_Pa.NoEvent_C then
            Self.Run;
          end if;
        exception
          when others =>
            Self.ErrorService.Report( "Receiver", "Receiver_Pr", "Got exception from Receiver.Run()" );
        end;
      end loop;
      accept Finish;
    or
      accept Finish;
    end select;
  end Receiver_Pr_T;

  procedure Run( Self : in out Receiver_Class ) is
  begin
    null;
  end;

  -- --------------------------------------------------------------------------

  function getReceiver(top : Topic_Class_At; dom : Domain_Class_At; Reporter : ErrorService_Class_At) return Receiver_Class_At is
    Result : Receiver_Class_At := null;
    localif : String := Ops_Pa.Socket_Pa.doSubnetTranslation(top.LocalInterface);
  begin
    if top.Transport = TRANSPORT_MC then
      Result := Receiver_Class_At(Ops_Pa.Transport_Pa.Receiver_Pa.MulticastReceiver_Pa.
        Create( top.DomainAddress,
                Integer(top.Port),
                localIf,
                top.InSocketBufferSize));

    elsif top.Transport = TRANSPORT_TCP then
      Result := Receiver_Class_At(Ops_Pa.Transport_Pa.Receiver_Pa.TCPClient_Pa.
                                    Create( ServerIP => top.DomainAddress,
                                            ServerPort => Integer(top.Port),
                                            HeartbeatPeriod => top.HeartbeatPeriod,
                                            HeartbeatTimeout => top.HeartbeatTimeout,
                                            InSocketBufferSize => top.InSocketBufferSize) );

    elsif top.Transport = TRANSPORT_UDP then
      -- If UDP topic is configured with my node address, we use the configured port, otherwise
      -- we use port 0 which will force the OS to create a unique port that we listen to.
      if Ops_Pa.Socket_Pa.isMyNodeAddress(top.DomainAddress) then
        Result := Receiver_Class_At(Ops_Pa.Transport_Pa.Receiver_Pa.UDPReceiver_Pa.
                                      Create( bindPort => Integer(top.Port),
                                              localInterface => top.DomainAddress,
                                              inSocketBufferSize => top.InSocketBufferSize));

      else
        Result := Receiver_Class_At(Ops_Pa.Transport_Pa.Receiver_Pa.UDPReceiver_Pa.
                                      Create( bindPort => 0,
                                              localInterface => localIf,
                                              inSocketBufferSize => top.InSocketBufferSize));
      end if;

    end if;

    if Result /= null then
      Result.SetErrorService( Reporter );
    end if;

    return Result;
  end;

end Ops_Pa.Transport_Pa.Receiver_Pa;

