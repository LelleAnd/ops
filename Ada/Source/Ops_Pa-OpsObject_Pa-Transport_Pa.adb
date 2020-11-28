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

package body Ops_Pa.OpsObject_Pa.Transport_Pa is

  -- Constructors
  function Create return Transport_Class_At is
    Self : Transport_Class_At := null;
  begin
    Self := new Transport_Class;
    InitInstance( Self.all );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  overriding procedure Serialize( Self : in out Transport_Class; archiver : ArchiverInOut_Class_At) is
  begin
    Serialize( OpsObject_Class(Self), archiver );
    if Self.IdlVersionMask /= 0 then
      archiver.inout("Transport_version", Self.Transport_Version);
      ValidateVersion("Transport", Self.Transport_version, Transport_idlVersion);
    else
      Self.Transport_Version := 0;
    end if;
    archiver.Inout("channelID", Self.ChannelID);
    archiver.Inout("topics", Self.Topics);
  end;

  -- Returns a newely allocated deep copy/clone of Self.
  overriding function Clone( Self : Transport_Class ) return OpsObject_Class_At is
    Result : Transport_Class_At := null;
  begin
    Result := Create;
    Self.FillClone( OpsObject_Class_At(Result) );
    return OpsObject_Class_At(Result);
  end Clone;

  -- Fills the parameter obj with all values from Self.
  overriding procedure FillClone( Self : Transport_Class; obj : OpsObject_Class_At ) is
  begin
    FillClone( OpsObject_Class(Self), obj );
    if obj.all in Transport_Class'Class then
      Transport_Class(obj.all).Transport_version := Self.Transport_version;
      Replace(Transport_Class(obj.all).ChannelID, Self.ChannelID);

      if Transport_Class(obj.all).Topics /= null then
        Clear(Transport_Class(obj.all).Topics);
        Dispose(Transport_Class(obj.all).Topics);
      end if;
      if Self.Topics /= null then
        Transport_Class(obj.all).Topics := new String_Arr(Self.Topics'Range);
        for i in Self.Topics'range loop
          Transport_Class(obj.all).Topics(i) := Copy(Self.Topics(i));
        end loop;
      end if;
    end if;
  end;

  procedure InitInstance( Self : in out Transport_Class ) is
  begin
    InitInstance( OpsObject_Class(Self) );
    AppendType( OpsObject_Class(Self), "Transport" );
  end;

  overriding procedure Finalize( Self : in out Transport_Class ) is
  begin
    if Self.ChannelID /= null then
      Dispose(Self.ChannelID);
    end if;
    if Self.Topics /= null then
      Clear(Self.Topics);
      Dispose(Self.Topics);
    end if;

    Finalize( OpsObject_Class(Self) );
  end;

end Ops_Pa.OpsObject_Pa.Transport_Pa;

