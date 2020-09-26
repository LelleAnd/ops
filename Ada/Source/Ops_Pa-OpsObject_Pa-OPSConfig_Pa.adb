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

with Ada.Text_IO;
with Ada.IO_Exceptions;

with Ops_Pa.Mutex_Pa,
     Ops_Pa.Error_Pa,
     Ops_Pa.ArchiverInOut_Pa.XMLArchiverIn_Pa,
     Ops_Pa.OpsObject_Pa.Domain_Pa,
     Ops_Pa.SerializableFactory_Pa.CompFactory_Pa,
     Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.OpsObjectFactory_Pa;

use  Ops_Pa.Error_Pa,
     Ops_Pa.ArchiverInOut_Pa.XMLArchiverIn_Pa,
     Ops_Pa.OpsObject_Pa.Domain_Pa,
     Ops_Pa.SerializableFactory_Pa.CompFactory_Pa,
     Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.OpsObjectFactory_Pa;

package body Ops_Pa.OpsObject_Pa.OPSConfig_Pa is

  -- Constructors
  function Create return OPSConfig_Class_At is
    Self : OPSConfig_Class_At := null;
  begin
    Self := new OPSConfig_Class;
    InitInstance( Self.all );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  -- Helpers for handling [de]serializing of fixed arrays
  procedure Domain_Class_InoutDynArr is new inoutdynarr2(Domain_Class, Domain_Class_At, Domain_Class_At_Arr, Domain_Class_At_Arr_At, Create);

  overriding procedure Serialize( Self : in out OPSConfig_Class; archiver : ArchiverInOut_Class_At) is
  begin
    Serialize( OpsObject_Class(Self), archiver );
    if Self.IdlVersionMask /= 0 then
      archiver.inout("OPSConfig_version", Self.OPSConfig_Version);
    else
      Self.OPSConfig_Version := 0;
    end if;
    Domain_Class_InoutDynArr(archiver, "domains", Self.domains, False);
  end;

  function OPSConfig_version( Self : OPSConfig_Class ) return Byte is
  begin
    return Self.OPSConfig_version;
  end;

  procedure SetOPSConfig_version( Self : in out OPSConfig_Class; Version : Byte ) is
  begin
    Self.OPSConfig_version := Version;
  end;

  -- Returns a newely allocated deep copy/clone of Self.
  overriding function Clone( Self : OPSConfig_Class ) return OpsObject_Class_At is
    Result : OPSConfig_Class_At := null;
  begin
    Result := Create;
    Self.FillClone( OpsObject_Class_At(Result) );
    return OpsObject_Class_At(Result);
  end Clone;

  -- Helpers
  procedure Dispose is new Ada.Unchecked_Deallocation( Domain_Class_Arr, Domain_Class_Arr_At );
  procedure Dispose is new Ada.Unchecked_Deallocation( Domain_Class_At_Arr, Domain_Class_At_Arr_At );

  procedure Clear( Arr : in out Domain_Class_At_Arr) is
  begin
    for i in Arr'Range loop
      if Arr(i) /= null then
        Free(Arr(i));
      end if;
    end loop;
  end;

  -- Fills the parameter obj with all values from Self.
  overriding procedure FillClone( Self : OPSConfig_Class; obj : OpsObject_Class_At ) is
  begin
    FillClone( OpsObject_Class(Self), obj );
    if obj.all in OPSConfig_Class'Class then
      OPSConfig_Class(obj.all).OPSConfig_version := Self.OPSConfig_version;
      if OPSConfig_Class(obj.all).domains /= null then
        Clear(OPSConfig_Class(obj.all).domains.all);
        Dispose(OPSConfig_Class(obj.all).domains);
      end if;
      if Self.domains /= null then
        OPSConfig_Class(obj.all).domains := new Domain_Class_At_Arr(Self.domains'Range);
        for i in Self.domains'Range loop
          if Self.domains(i) /= null then
            OPSConfig_Class(obj.all).domains(i) := Domain_Class_At(Clone(Self.domains(i).all));
          end if;
        end loop;
      end if;
    end if;
  end;

  procedure InitInstance( Self : in out OPSConfig_Class ) is
  begin
    InitInstance( OpsObject_Class(Self) );
  end;

  overriding procedure Finalize( Self : in out OPSConfig_Class ) is
  begin
    if Self.domains /= null then
      Clear(Self.domains.all);
      Dispose(Self.domains);
    end if;

    Finalize( OpsObject_Class(Self) );
  end;

  -- Returns a reference to the given Domain
  -- NOTE: The OPSConfig still owns it
  function getDomain( Self : OPSConfig_Class; domainID : string) return Domain_Class_At is
  begin
    for i in Self.domains.all'Range loop
      if Self.domains(i).all.DomainID = domainID then
        return Self.domains(i);
      end if;
    end loop;
    return null;
  end;

  -- Returns references to the internal Domains
  -- NOTE: The OPSConfig still owns them
  function getDomains( Self : OPSConfig_Class ) return Domain_Class_At_Arr_At is
  begin
    return Self.Domains;
  end;

-- ---------------------------------------------------------------------------

  function Create return DefaultOPSConfigImpl_Class_At is
    Self : DefaultOPSConfigImpl_Class_At := null;
  begin
    Self := new DefaultOPSConfigImpl_Class;
    InitInstance( Self.all );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  overriding procedure Serialize( Self : in out DefaultOPSConfigImpl_Class; archiver : ArchiverInOut_Class_At) is
  begin
    Serialize( OPSConfig_Class(Self), archiver );
  end;

  procedure InitInstance( Self : in out DefaultOPSConfigImpl_Class ) is
  begin
    InitInstance( OPSConfig_Class(Self) );
    AppendType( OPSConfig_Class(Self), "DefaultOPSConfigImpl" );
  end;

  overriding procedure Finalize( Self : in out DefaultOPSConfigImpl_Class ) is
  begin
    Finalize( OPSConfig_Class(Self) );
  end;

  -- ---------------------------------------------------------------------------

  function Create return ExtendedOPSConfig_Class_At is
    Self : ExtendedOPSConfig_Class_At := null;
  begin
    Self := new ExtendedOPSConfig_Class;
    InitInstance( Self.all );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  procedure InitInstance( Self : in out ExtendedOPSConfig_Class ) is
  begin
    InitInstance( DefaultOPSConfigImpl_Class(Self) );
  end;

  overriding procedure Finalize( Self : in out ExtendedOPSConfig_Class ) is
  begin
    Finalize( DefaultOPSConfigImpl_Class(Self) );
  end;

  procedure Add( Self : in out ExtendedOPSConfig_Class; domain : Domain_Class_At) is
    old : Domain_Class_At_Arr_At := Self.domains;
  begin
    if Self.Domains = null then
      Self.Domains := new Domain_Class_At_Arr(0..0);
    else
      Self.Domains := new Domain_Class_At_Arr(old'First..old'Last+1);
      Self.Domains(old'Range) := old.all;
      Dispose(old);
    end if;
    Self.Domains(Self.Domains'Last) := domain;
  end;

  procedure Remove( Self : in out ExtendedOPSConfig_Class; domain : Domain_Class_At) is
    Idx : Integer := -1;
    Found : Boolean := False;
  begin
    if Self.domains /= null then
      for i in Self.domains'Range loop
        if Self.domains(i) = domain then
          Idx := i;
          Found := True;
          exit;
        end if;
      end loop;
      if Found then
        if Self.domains'Length = 1 then
          Dispose(Self.domains);
          Self.domains := null;
        else
          if Idx < Self.domains'Last then
            for i in Idx+1 .. Self.domains'Last loop
              Self.domains(i-1) := Self.domains(i);
            end loop;
          end if;
          declare
            old : Domain_Class_At_Arr_At := Self.domains;
          begin
            Self.Domains := new Domain_Class_At_Arr(old'First..old'Last-1);
            Self.Domains(Self.domains'Range) := old.all(old'First..old'Last-1);
            Dispose(old);
          end;
        end if;
      end if;
    end if;
  end;

  procedure Clear( Self : in out ExtendedOPSConfig_Class ) is
  begin
    -- We don't own the domains, so just clear array
    if Self.domains /= null then
      Dispose(Self.domains);
      Self.domains := null;
    end if;
  end;

-- ---------------------------------------------------------------------------

  gConfiguration : OPSConfig_Class_At := null;
  gMutex : aliased Ops_Pa.Mutex_Pa.Mutex;


  function ReadFile(Name : String) return String_At is
    Result : String_At := new String'("");
    File : Ada.Text_IO.File_Type;

    procedure Append(str : String) is
      Old : String_At := Result;
    begin
      Result := new String'(Old.all & str);
      Dispose(old);
    end;

  begin
    -- Open file
    Ada.Text_IO.Open(File, Ada.Text_IO.In_File, Name);
    begin
      -- Read complete file
      while not Ada.Text_IO.End_Of_File(File) loop
        Append(Ada.Text_IO.Get_Line(File));
      end loop;
      -- Close file
      Ada.Text_IO.Close(File);
      return Result;
    exception
      when others =>
        Ada.Text_IO.Close(File);
        raise;
    end;
  exception
    when others =>
      Dispose(Result);
      raise;
  end;

  -- Returns a reference to a unique instance and should be deleted.
  -- See also releaseConfig below
  function getConfig(configFile : String) return OPSConfig_Class_At is
    archiver : XMLArchiverIn_Class_At := null;
    Result : OPSConfig_Class_At := null;
    list : String_At := null;
  begin
    begin
      list := ReadFile(configFile);
      archiver := XMLArchiverIn_Pa.Create(list.all, "root", SerializableInheritingTypeFactory_Class_At(getInstance));
      begin
        Result := OPSConfig_Class_At(archiver.inout2("ops_config", Serializable_Class_At(Result)));
      exception
        when others =>
          StaticErrorService.Report( "OPSConfig", "getConfig", "Unknown exception during XML parsing" );
      end;
      Free(archiver);
    exception
      when Ada.IO_Exceptions.Name_Error =>
        StaticErrorService.Report( "OPSConfig", "getConfig", "File '" & configFile & "' not found" );
      when others =>
        StaticErrorService.Report( "OPSConfig", "getConfig", "Unknown exception during file reading" );
    end;
    if list /= null then
      Dispose(list);
    end if;
    return Result;
  end;

  -- Returns a reference to a singleton instance and should NOT be deleted.
  -- See also releaseConfig below
  function getConfig return OPSConfig_Class_At is
    S : Ops_Pa.Mutex_Pa.Scope_Lock(gMutex'Access);
  begin
    if gConfiguration = null then
      gConfiguration := RepositoryInstance.getConfig;
    end if;
    return gConfiguration;
  end;

  -- Help method that deletes the config if it isn't the singleton instance
  -- The variable used in the call will always be set to null
  procedure releaseConfig(cfg : in out OPSConfig_Class_At) is
  begin
    -- We don't want to delete the singleton instance
    if cfg /= gConfiguration and cfg /= null then
      Free(cfg);
    end if;
    cfg := null;
  end;

  -----------------------------------------------------------------------------

  use type MyMap.cursor;

  function Less (Left, Right : String) return Boolean is
  begin
    return Left < Right;
  end;

  function Equal (Left, Right : OPSConfig_Class_At) return Boolean is
  begin
    return Left = Right;
  end;

  procedure InitInstance( Self : in out OPSConfigRepository_Class ) is
    cfg : ExtendedOPSConfig_Class_At := Create;
  begin
    Self.Config := OPSConfig_Class_At(cfg);
  end;

  function Create return OPSConfigRepository_Class_At is
    Self : OPSConfigRepository_Class_At := null;
  begin
    Self := new OPSConfigRepository_Class;
    InitInstance( Self.all );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  overriding procedure Finalize( Self : in out OPSConfigRepository_Class ) is

    procedure Process( Pos : in MyMap.Cursor ) is
      Value : OPSConfig_Class_At := MyMap.Element(Pos);
    begin
      if Value /= null then
        Free(Value);
      end if;
    end;

  begin
    -- Clear the internal list since it doesn't own the objects
    ExtendedOPSConfig_Class_At(Self.Config).Clear;
    Free(Self.Config);

    -- Need to free all objects in the dictionary
    Self.ConfigFiles.Iterate(Process'Access);
  end;


  -- Add domains from OPS configuration file "filename"
  -- if "domain" == '', all domains will be added otherwise only the specified "domain"
  -- Returns true if at least one domain added
  function Add( Self : in out OPSConfigRepository_Class; filename : string; domainID : string := "" ) return Boolean is
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.Mutex'Access);
  begin
    return Self.LocalAdd( filename, domainID );
  end;

  -- Must only be called while holding the Self.Mutex
  function LocalAdd( Self : in out OPSConfigRepository_Class; filename : string; domainID : string := "" ) return Boolean is
    config : OPSConfig_Class_At := null;
    domains : Domain_Class_At_Arr_At := null;
    pos : MyMap.Cursor;
    Result : Boolean := False;
  begin
    if domainID /= "" then
      -- Check if domain already exist
      if Self.LocalDomainExist( domainID ) then
        StaticErrorService.Report( "OPSConfigRepository", "Add", "domain '" & domainID & "' already exist" );
        return False;
      end if;
    end if;

    begin
      -- Check if file already read
      pos := Self.ConfigFiles.Find( filename );
      if pos /= MyMap.No_Element then
        -- Get the earlier read config
        config := MyMap.Element(pos);

      else
        -- Need to read file
        config := getConfig( filename );
        if config = null then
          StaticErrorService.Report( "TOPSConfigRepository", "Add", "Failed to parse file '" & filename & "'" );
          return False;
        end if;
        Self.ConfigFiles.Insert(filename, config);
      end if;
    exception
      when others =>
        StaticErrorService.Report( "TOPSConfigRepository", "Add", "Failed to parse file '" & filename & "'" );
        return False;
    end;

    -- Get all domains read from file
    domains := config.getDomains;

    -- Add the choosen one(s) to our list if not already there
    if domains /= null then
      for i in domains.all'Range loop
        if (domainID = "") or (domains(i).DomainID = domainID) then
          if Self.LocalDomainExist( domains(i).DomainID ) then
            StaticErrorService.Report( "TOPSConfigRepository", "Add", "domain '" & domains(i).DomainID & "' already exist" );
          else
            -- Add unique domains to our list
            ExtendedOPSConfig_Class_At(Self.Config).Add(domains(i));
            Result := True;
          end if;
        end if;
      end loop;
    end if;
    return Result;
  end;

  procedure Clear( Self : in out OPSConfigRepository_Class ) is
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.Mutex'Access);
  begin
    -- Since we just borrow the references to domains (they are owned by file cache)
    -- we can just clear the domain list in our OPSConfig object
    ExtendedOPSConfig_Class_At(Self.Config).Clear;
  end;

  function domainExist( Self : in out OPSConfigRepository_Class; domainID : string ) return Boolean is
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.Mutex'Access);
  begin
    return Self.LocalDomainExist( domainID );
  end;

  -- Must only be called while holding the Self.Mutex
  function LocalDomainExist( Self : in out OPSConfigRepository_Class; domainID : string ) return Boolean is
    domains : Domain_Class_At_Arr_At := null;
    Result : Boolean := False;
  begin
    domains := Self.Config.getDomains;
    if domains /= null then
      for i in domains.all'Range loop
        if domains(i).DomainID = domainID then
          Result := True;
        end if;
      end loop;
    end if;
    return Result;
  end;

  function getConfig( Self : in out OPSConfigRepository_Class; domainID : string := "" ) return OPSConfig_Class_At is
    domains : Domain_Class_At_Arr_At;
    S : Ops_Pa.Mutex_Pa.Scope_Lock(Self.Mutex'Access);
  begin
    -- If no domain have been added, we try to add the default file
    -- This is for backward compatibility
    domains := Self.Config.getDomains;
    if domains = null or else domains.all'Length = 0 then
      if not Self.LocalAdd("ops_config.xml") then
        return null;
      end if;
    end if;

    if domainID /= "" then
      if not Self.LocalDomainExist( domainID ) then
        return null;
      end if;
    end if;

    return Self.Config;
  end;

  gInstance : OPSConfigRepository_Class_At := Create;

  function RepositoryInstance return OPSConfigRepository_Class_At is
  begin
    return gInstance;
  end;

  procedure Debug_TotalClear is
  begin
    if gInstance /= null then
      Free(gInstance);
      gInstance := null;
    end if;
  end;

end Ops_Pa.OpsObject_Pa.OPSConfig_Pa;


