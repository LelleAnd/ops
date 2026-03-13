-- Auto generated OPS-code. DO NOT MODIFY!
with
__importUnits
  System;
use
__importUnits
  System;

package __unitName is

__classComment
__constDeclarations

-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type __className_Class is new __baseClassName_Class with
    record
__declarations
    end record;
  type __className_Class_At is access all __className_Class'Class;

  -- Help types used in case other idl's has vector's of the class
  type __className_Class_Arr is array(Integer range <>) of aliased __className_Class;
  type __className_Class_Arr_At is access all __className_Class_Arr;
  type __className_Class_At_Arr is array(Integer range <>) of __className_Class_At;
  type __className_Class_At_Arr_At is access all __className_Class_At_Arr;

  TypeName_C : constant string := "__packageName.__className";

  -- Constructors
  function Create return __className_Class_At;

  overriding procedure Serialize( Self : in out __className_Class; archiver : ArchiverInOut_Class_At);

  -- Returns a newely allocated deep copy/clone of Self.
  overriding function Clone( Self : __className_Class ) return OpsObject_Class_At;

  -- Fills the parameter obj with all values from Self.
  overriding procedure FillClone( Self : __className_Class; obj : OpsObject_Class_At );

  -- Validation routine for fields
  overriding function IsValid( Self : __className_Class ) return Boolean;

--private

  procedure InitInstance( Self : in out __className_Class );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out __className_Class );

  -- Helpers
  procedure Dispose is new Ada.Unchecked_Deallocation( __className_Class_Arr, __className_Class_Arr_At );
  procedure Dispose is new Ada.Unchecked_Deallocation( __className_Class_At_Arr, __className_Class_At_Arr_At );

  procedure Clear( Arr : in out __className_Class_At_Arr);

  -- Helpers for handling [de]serializing of fixed arrays
  procedure __className_Class_InoutFixArr is new inoutfixarr(__className_Class, __className_Class_At, __className_Class_Arr);
  procedure __className_Class_InoutFixArr is new inoutfixarr2(__className_Class, __className_Class_At, __className_Class_At_Arr);

  procedure __className_Class_InoutDynArr is new inoutdynarr(__className_Class, __className_Class_At, __className_Class_Arr, __className_Class_Arr_At);
  procedure __className_Class_InoutDynArr is new inoutdynarr2(__className_Class, __className_Class_At, __className_Class_At_Arr, __className_Class_At_Arr_At, Create);

end __unitName;
