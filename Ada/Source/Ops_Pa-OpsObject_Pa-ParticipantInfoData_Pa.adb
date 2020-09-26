--
-- Copyright (C) 2016-2020 Lennart Andersson.
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

package body Ops_Pa.OpsObject_Pa.ParticipantInfoData_Pa is

  -- Constructors
  function Create return ParticipantInfoData_Class_At is
    Self : ParticipantInfoData_Class_At := null;
  begin
    Self := new ParticipantInfoData_Class;
    InitInstance( Self.all );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  -- Helpers for handling [de]serializing of fixed arrays
  procedure TopicInfoData_Class_InoutDynArr is new inoutdynarr2(TopicInfoData_Class, TopicInfoData_Class_At, TopicInfoData_Class_At_Arr, TopicInfoData_Class_At_Arr_At, Create);

  overriding procedure Serialize( Self : in out ParticipantInfoData_Class; archiver : ArchiverInOut_Class_At) is
  begin
    Serialize( OpsObject_Class(Self), archiver );
    if Self.IdlVersionMask /= 0 then
      archiver.inout("ParticipantInfoData_version", Self.ParticipantInfoData_Version);
    else
      Self.ParticipantInfoData_Version := 0;
    end if;
    archiver.Inout("name", Self.name);
    archiver.Inout("domain", Self.domain);
    archiver.Inout("id", Self.id);
    archiver.Inout("ip", Self.ip);
    archiver.Inout("languageImplementation", Self.languageImplementation);
    archiver.Inout("opsVersion", Self.opsVersion);
    archiver.Inout("mc_udp_port", Self.mc_udp_port);
    archiver.Inout("mc_tcp_port", Self.mc_tcp_port);
    TopicInfoData_Class_InoutDynArr(archiver, "subscribeTopics", Self.subscribeTopics, False);
    TopicInfoData_Class_InoutDynArr(archiver, "publishTopics", Self.publishTopics, False);
    archiver.Inout("knownTypes", Self.knownTypes);
  end;

  -- Returns a newely allocated deep copy/clone of Self.
  overriding function Clone( Self : ParticipantInfoData_Class ) return OpsObject_Class_At is
    Result : ParticipantInfoData_Class_At := null;
  begin
    Result := Create;
    Self.FillClone( OpsObject_Class_At(Result) );
    return OpsObject_Class_At(Result);
  end Clone;

  -- Helpers
  procedure Dispose is new Ada.Unchecked_Deallocation( TopicInfoData_Class_Arr, TopicInfoData_Class_Arr_At );
  procedure Dispose is new Ada.Unchecked_Deallocation( TopicInfoData_Class_At_Arr, TopicInfoData_Class_At_Arr_At );

  procedure Clear( Arr : in out TopicInfoData_Class_At_Arr) is
  begin
    for i in Arr'Range loop
      if Arr(i) /= null then
        Free(Arr(i));
      end if;
    end loop;
  end;

  -- Fills the parameter obj with all values from Self.
  overriding procedure FillClone( Self : ParticipantInfoData_Class; obj : OpsObject_Class_At ) is
  begin
    FillClone( OpsObject_Class(Self), obj );
    if obj.all in ParticipantInfoData_Class'Class then
      ParticipantInfoData_Class(obj.all).ParticipantInfoData_version := Self.ParticipantInfoData_version;
      Replace(ParticipantInfoData_Class(obj.all).name, Self.name);
      Replace(ParticipantInfoData_Class(obj.all).domain, Self.domain);
      Replace(ParticipantInfoData_Class(obj.all).id, Self.id);
      Replace(ParticipantInfoData_Class(obj.all).ip, Self.ip);
      Replace(ParticipantInfoData_Class(obj.all).languageImplementation, Self.languageImplementation);
      Replace(ParticipantInfoData_Class(obj.all).opsVersion, Self.opsVersion);
      ParticipantInfoData_Class(obj.all).mc_udp_port := Self.mc_udp_port;
      ParticipantInfoData_Class(obj.all).mc_tcp_port := Self.mc_tcp_port;
      if ParticipantInfoData_Class(obj.all).subscribeTopics /= null then
        Clear(ParticipantInfoData_Class(obj.all).subscribeTopics.all);
        Dispose(ParticipantInfoData_Class(obj.all).subscribeTopics);
      end if;
      if Self.subscribeTopics /= null then
        ParticipantInfoData_Class(obj.all).subscribeTopics := new TopicInfoData_Class_At_Arr(Self.subscribeTopics'Range);
        for i in Self.subscribeTopics'Range loop
          if Self.subscribeTopics(i) /= null then
            ParticipantInfoData_Class(obj.all).subscribeTopics(i) := TopicInfoData_Class_At(Clone(Self.subscribeTopics(i).all));
          end if;
        end loop;
      end if;
      if ParticipantInfoData_Class(obj.all).publishTopics /= null then
        Clear(ParticipantInfoData_Class(obj.all).publishTopics.all);
        Dispose(ParticipantInfoData_Class(obj.all).publishTopics);
      end if;
      if Self.publishTopics /= null then
        ParticipantInfoData_Class(obj.all).publishTopics := new TopicInfoData_Class_At_Arr(Self.publishTopics'Range);
        for i in Self.publishTopics'Range loop
          if Self.publishTopics(i) /= null then
            ParticipantInfoData_Class(obj.all).publishTopics(i) := TopicInfoData_Class_At(Clone(Self.publishTopics(i).all));
          end if;
        end loop;
      end if;
      Clear(ParticipantInfoData_Class(obj.all).knownTypes);
      Dispose(ParticipantInfoData_Class(obj.all).knownTypes);
      if Self.knownTypes /= null then
        ParticipantInfoData_Class(obj.all).knownTypes := new String_Arr(Self.knownTypes'Range);
        for i in Self.knownTypes'Range loop
          ParticipantInfoData_Class(obj.all).knownTypes(i) := Copy(Self.knownTypes(i));
        end loop;
      end if;
    end if;
  end;

  procedure InitInstance( Self : in out ParticipantInfoData_Class ) is
  begin
    InitInstance( OpsObject_Class(Self) );
    AppendType( OpsObject_Class(Self), "ops.ParticipantInfoData" );
  end;

  overriding procedure Finalize( Self : in out ParticipantInfoData_Class ) is
  begin
    if Self.name /= null then
      Dispose(Self.name);
    end if;
    if Self.domain /= null then
      Dispose(Self.domain);
    end if;
    if Self.id /= null then
      Dispose(Self.id);
    end if;
    if Self.ip /= null then
      Dispose(Self.ip);
    end if;
    if Self.languageImplementation /= null then
      Dispose(Self.languageImplementation);
    end if;
    if Self.opsVersion /= null then
      Dispose(Self.opsVersion);
    end if;
    if Self.subscribeTopics /= null then
      Clear(Self.subscribeTopics.all);
      Dispose(Self.subscribeTopics);
    end if;
    if Self.publishTopics /= null then
      Clear(Self.publishTopics.all);
      Dispose(Self.publishTopics);
    end if;
    if Self.knownTypes /= null then
      Clear(Self.knownTypes.all);
      Dispose(Self.knownTypes);
    end if;

    Finalize( OpsObject_Class(Self) );
  end;

  -- Help routines for updating the dynamic arrays with TopicInfoData
  procedure addTopic( arr : in out TopicInfoData_Class_At_Arr_At; top : Topic_Class_At) is
    tmp : TopicInfoData_Class_At_Arr_At;
    tid : TopicInfoData_Class_At := Create( top );
  begin
    if arr = null then
      arr := new TopicInfoData_Class_At_Arr(0..0);
      arr(0) := tid;
    else
      ---TODO ref count if same topic??
      tmp := new TopicInfoData_Class_At_Arr(arr'First..arr'Last+1);
      tmp(arr'Range) := arr(arr'Range);
      tmp(tmp'Last) := tid;
      Dispose(arr);
      arr := tmp;
    end if;
  end;

  procedure removeTopic( arr : in out TopicInfoData_Class_At_Arr_At; top : Topic_Class_At) is
    tmp : TopicInfoData_Class_At_Arr_At;
    found : Boolean := False;
    idx : Integer;
  begin
    for i in arr'Range loop
      if arr(i).name.all = top.Name then
        idx := i;
        found := True;
        exit;
      end if;
    end loop;
    if found then
      Free(arr(idx));
      if arr'Length = 1 then
        Dispose(arr);
        arr := null;
      else
        tmp := new TopicInfoData_Class_At_Arr(arr'First..arr'Last-1);
        if idx > arr'First then
          tmp(arr'First..idx-1) := arr(arr'First..idx-1);
        end if;
        if idx < arr'Last then
          tmp(idx..arr'Last-1) := arr(idx+1..arr'Last);
        end if;
        Dispose(arr);
        arr := tmp;
      end if;
    end if;
  end;

end Ops_Pa.OpsObject_Pa.ParticipantInfoData_Pa;

