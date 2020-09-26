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

with Ada.Containers.Indefinite_Ordered_Maps;

with Ops_Pa.Mutex_Pa,
     Ops_Pa.OpsObject_Pa.Domain_Pa,
     Ops_Pa.ArchiverInOut_Pa;

use  Ops_Pa.OpsObject_Pa.Domain_Pa,
     Ops_Pa.ArchiverInOut_Pa;

package Ops_Pa.OpsObject_Pa.OPSConfig_Pa is

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type OPSConfig_Class is new OpsObject_Class with private;
  type OPSConfig_Class_At is access all OPSConfig_Class'Class;

  -- Help types used in case other idl's has vector's of the class
  type OPSConfig_Class_Arr is array(Integer range <>) of aliased OPSConfig_Class;
  type OPSConfig_Class_Arr_At is access all OPSConfig_Class_Arr;
  type OPSConfig_Class_At_Arr is array(Integer range <>) of OPSConfig_Class_At;
  type OPSConfig_Class_At_Arr_At is access all OPSConfig_Class_At_Arr;

  -- Constructors
  function Create return OPSConfig_Class_At;

  overriding procedure Serialize( Self : in out OPSConfig_Class; archiver : ArchiverInOut_Class_At);

  -- Returns a newely allocated deep copy/clone of Self.
  overriding function Clone( Self : OPSConfig_Class ) return OpsObject_Class_At;

  -- Fills the parameter obj with all values from Self.
  overriding procedure FillClone( Self : OPSConfig_Class; obj : OpsObject_Class_At );

  -- Returns a reference to a singleton instance and should NOT be deleted.
  -- See also releaseConfig below
  function getConfig return OPSConfig_Class_At;

  -- Returns a reference to a unique instance and should be deleted.
  -- See also releaseConfig below
  function getConfig(configFile : String) return OPSConfig_Class_At;

  -- Help method that deletes the config if it isn't the singleton instance
  -- The variable used in the call will always be set to null
  procedure releaseConfig(cfg : in out OPSConfig_Class_At);

  -- Returns a reference to the given Domain
  -- NOTE: The OPSConfig still owns it
  function getDomain( Self : OPSConfig_Class; domainID : string) return Domain_Class_At;

  -- Returns references to the internal Domains
  -- NOTE: The OPSConfig still owns them
  function getDomains( Self : OPSConfig_Class ) return Domain_Class_At_Arr_At;

  function OPSConfig_version( Self : OPSConfig_Class ) return Byte;
  procedure SetOPSConfig_version( Self : in out OPSConfig_Class; Version : Byte );

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type DefaultOPSConfigImpl_Class is new OPSConfig_Class with private;
  type DefaultOPSConfigImpl_Class_At is access all DefaultOPSConfigImpl_Class'Class;

  function Create return DefaultOPSConfigImpl_Class_At;

  overriding procedure Serialize( Self : in out DefaultOPSConfigImpl_Class; archiver : ArchiverInOut_Class_At);


-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type ExtendedOPSConfig_Class is new DefaultOPSConfigImpl_Class with private;
  type ExtendedOPSConfig_Class_At is access all ExtendedOPSConfig_Class'Class;

  function Create return ExtendedOPSConfig_Class_At;

  procedure Add( Self : in out ExtendedOPSConfig_Class; domain : Domain_Class_At);
  procedure Remove( Self : in out ExtendedOPSConfig_Class; domain : Domain_Class_At);
  procedure Clear( Self : in out ExtendedOPSConfig_Class );


-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type OPSConfigRepository_Class is new Ops_Class with private;
  type OPSConfigRepository_Class_At is access all OPSConfigRepository_Class'Class;

  -- Access to the singleton repository object
  function RepositoryInstance return OPSConfigRepository_Class_At;

  -- ======================================================
  -- Add one or more domains from OPS configuration file "filename"
  -- if "domainID" == "", all domains will be added otherwise only the specified "domainID"
  -- Returns true if at least one domain was added
  function Add( Self : in out OPSConfigRepository_Class; filename : string; domainID : string := "" ) return Boolean;

  -- Remove all domain references from the repository (Note does not clear the file-cache)
  -- Note: Calling this while TParticipant, TPublisher or TSubscriber instances exist
  -- may have unwanted side effects
  procedure Clear( Self : in out OPSConfigRepository_Class );

  -- ======================================================
  -- Get a reference to the internal OPSConfig object
  -- if "domainID" <> "", the domain "domainID" must exist otherwise null is returned.
  function getConfig( Self : in out OPSConfigRepository_Class; domainID : string := "" ) return OPSConfig_Class_At;

  function domainExist( Self : in out OPSConfigRepository_Class; domainID : string ) return Boolean;


-- ==========================================================================
-- Debug/Test helpers
-- ==========================================================================
  -- Removes all singleton instances. Note should only be used when looking for memory leaks.
  -- Requires that all OPS usage has been stopped and all objects freed.
  procedure Debug_TotalClear;

private
-- ==========================================================================
--
-- ==========================================================================
  type OPSConfig_Class is new OpsObject_Class with
    record
      OPSConfig_version : Byte := 0;
      domains : Domain_Class_At_Arr_At := null;
    end record;

  procedure InitInstance( Self : in out OPSConfig_Class );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out OPSConfig_Class );

-- ==========================================================================
--
-- ==========================================================================
  type DefaultOPSConfigImpl_Class is new OPSConfig_Class with null record;

  procedure InitInstance( Self : in out DefaultOPSConfigImpl_Class );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out DefaultOPSConfigImpl_Class );

-- ==========================================================================
--
-- ==========================================================================
  type ExtendedOPSConfig_Class is new DefaultOPSConfigImpl_Class with null record;

  procedure InitInstance( Self : in out ExtendedOPSConfig_Class );

  overriding procedure Finalize( Self : in out ExtendedOPSConfig_Class );

-- ==========================================================================
--
-- ==========================================================================
  function Less (Left, Right : String) return Boolean;
  function Equal (Left, Right : OPSConfig_Class_At) return Boolean;

  package MyMap is new Ada.Containers.Indefinite_Ordered_Maps(String, OPSConfig_Class_At, Less, Equal);

-- ==========================================================================
--
-- ==========================================================================
  type OPSConfigRepository_Class is new Ops_Class with
    record
      -- Our OPSConfig object containing references to all selectivly added domains
      Config : OPSConfig_Class_At := null;

      -- File cache with all added config files and their domains
      ConfigFiles : MyMap.Map;

      Mutex : aliased Ops_Pa.Mutex_Pa.Mutex;
    end record;

  -- Must only be called while holding the Self.Mutex
  function LocalAdd( Self : in out OPSConfigRepository_Class; filename : string; domainID : string := "" ) return Boolean;
  function LocalDomainExist( Self : in out OPSConfigRepository_Class; domainID : string ) return Boolean;

  procedure InitInstance( Self : in out OPSConfigRepository_Class );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out OPSConfigRepository_Class );

end Ops_Pa.OpsObject_Pa.OPSConfig_Pa;

