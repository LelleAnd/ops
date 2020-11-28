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

package body Ops_Pa.OpsObject_Pa.TopicInfoData_Pa is

  -- Constructors
  function Create return TopicInfoData_Class_At is
    Self : TopicInfoData_Class_At := null;
  begin
    Self := new TopicInfoData_Class;
    InitInstance( Self.all );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  function Create(top : Topic_Class_At) return TopicInfoData_Class_At is
    Self : TopicInfoData_Class_At := null;
  begin
    Self := new TopicInfoData_Class;
    InitInstance( Self.all, top );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  overriding procedure Serialize( Self : in out TopicInfoData_Class; archiver : ArchiverInOut_Class_At) is
  begin
    Serialize( OpsObject_Class(Self), archiver );
    if Self.IdlVersionMask /= 0 then
      archiver.inout("TopicInfoData_version", Self.TopicInfoData_Version);
      ValidateVersion("TopicInfoData", Self.TopicInfoData_version, TopicInfoData_idlVersion);
    else
      Self.TopicInfoData_Version := 0;
    end if;
    archiver.Inout("name", Self.name);
    archiver.Inout("type", Self.dataType);
    archiver.Inout("transport", Self.transport);
    archiver.Inout("address", Self.address);
    archiver.Inout("port", Self.port);
    archiver.Inout("keys", Self.keys);
  end;

  -- Returns a newely allocated deep copy/clone of Self.
  overriding function Clone( Self : TopicInfoData_Class ) return OpsObject_Class_At is
    Result : TopicInfoData_Class_At := null;
  begin
    Result := Create;
    Self.FillClone( OpsObject_Class_At(Result) );
    return OpsObject_Class_At(Result);
  end Clone;

  -- Fills the parameter obj with all values from Self.
  overriding procedure FillClone( Self : TopicInfoData_Class; obj : OpsObject_Class_At ) is
  begin
    FillClone( OpsObject_Class(Self), obj );
    if obj.all in TopicInfoData_Class'Class then
      TopicInfoData_Class(obj.all).TopicInfoData_version := Self.TopicInfoData_version;
      Replace(TopicInfoData_Class(obj.all).name, Self.name);
      Replace(TopicInfoData_Class(obj.all).dataType, Self.dataType);
      Replace(TopicInfoData_Class(obj.all).transport, Self.transport);
      Replace(TopicInfoData_Class(obj.all).address, Self.address);
      TopicInfoData_Class(obj.all).port := Self.port;
      Clear(TopicInfoData_Class(obj.all).keys);
      Dispose(TopicInfoData_Class(obj.all).keys);
      if Self.keys /= null then
        TopicInfoData_Class(obj.all).keys := new String_Arr(Self.keys'Range);
        for i in Self.keys'Range loop
          TopicInfoData_Class(obj.all).keys(i) := Copy(Self.keys(i));
        end loop;
      end if;
    end if;
  end;

  procedure InitInstance( Self : in out TopicInfoData_Class ) is
  begin
    InitInstance( OpsObject_Class(Self) );
    AppendType( OpsObject_Class(Self), "TopicInfoData" );
  end;

  procedure InitInstance( Self : in out TopicInfoData_Class; top : Topic_Class_At ) is
  begin
    InitInstance( OpsObject_Class(Self) );
    AppendType( OpsObject_Class(Self), "TopicInfoData" );
    Replace(Self.Name, top.Name);
    Replace(Self.DataType, top.TypeID);
    Replace(Self.Transport, top.Transport);
    Replace(Self.Address, top.DomainAddress);
    Self.Port := top.Port;
  end;

  overriding procedure Finalize( Self : in out TopicInfoData_Class ) is
  begin
    if Self.name /= null then
      Dispose(Self.name);
    end if;
    if Self.dataType /= null then
      Dispose(Self.dataType);
    end if;
    if Self.transport /= null then
      Dispose(Self.transport);
    end if;
    if Self.address /= null then
      Dispose(Self.address);
    end if;
    if Self.keys /= null then
      Clear(Self.keys.all);
      Dispose(Self.keys);
    end if;

    Finalize( OpsObject_Class(Self) );
  end;

end Ops_Pa.OpsObject_Pa.TopicInfoData_Pa;

