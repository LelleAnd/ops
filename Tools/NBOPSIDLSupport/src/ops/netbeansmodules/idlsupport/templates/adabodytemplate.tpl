-- Auto generated OPS-code. DO NOT MODIFY!
with Ada.Tags; use Ada.Tags;

package body __unitName is

  -- Constructors
  function Create return __className_Class_At is
    Self : __className_Class_At := null;
  begin
    Self := new __className_Class;
    InitInstance( Self.all );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  overriding procedure Serialize( Self : in out __className_Class; archiver : ArchiverInOut_Class_At) is
  begin
    Serialize( __baseClassName_Class(Self), archiver );
__serialize
  end;

  -- Returns a newely allocated deep copy/clone of Self.
  overriding function Clone( Self : __className_Class ) return OpsObject_Class_At is
    Result : __className_Class_At := null;
  begin
    Result := Create;
    Self.FillClone( OpsObject_Class_At(Result) );
    return OpsObject_Class_At(Result);
  end Clone;

  -- Fills the parameter obj with all values from Self.
  overriding procedure FillClone( Self : __className_Class; obj : OpsObject_Class_At ) is
  begin
    FillClone( __baseClassName_Class(Self), obj );
    if obj.all in __className_Class'Class then
__fillCloneBody
    end if;
  end;

  procedure InitInstance( Self : in out __className_Class ) is
  begin
    InitInstance( __baseClassName_Class(Self) );
    AppendType( OpsObject_Class(Self), TypeName_C );
__constructorBody
  end;

  overriding procedure Finalize( Self : in out __className_Class ) is
  begin
__destructorBody
    Finalize( __baseClassName_Class(Self) );
  end;

  procedure Clear( Arr : in out __className_Class_At_Arr) is
  begin
    for i in Arr'Range loop
      if Arr(i) /= null then
        Free(Arr(i));
      end if;
    end loop;
  end;

  -- Validation routine for fields
  overriding function IsValid( Self : __className_Class ) return Boolean is
    Result : Boolean := IsValid( __baseClassName_Class(Self) );
  begin
__validateBody
    return Result;
  end;

end __unitName;
