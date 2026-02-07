// Auto generated OPS-code. DO NOT MODIFY!
unit __packageName.__className;

interface

uses
  uOps.Types,
  uOps.OpsObject,
  uOps.Topic,
__importUnits
  uOps.Publisher,
  uOps.Subscriber,
  uOps.ArchiverInOut;

type
__classComment
  __className = class(__baseClassName)
  public
    ///-----------------------------------------------------------------------

__constDeclarations
  public
    ///-----------------------------------------------------------------------

__declarations
    ///-----------------------------------------------------------------------
    constructor Create; overload;
    destructor Destroy; override;

    class function getTypeName : string;

    ///This method acceptes an ops::ArchiverInOut visitor which will serialize or deserialize an
    ///instance of this class to a format dictated by the implementation of the ArchiverInout.
    procedure Serialize(archiver : TArchiverInOut); override;

    // Validation routine for fields, incl. that contained object references are of the correct type
    function Validate : Boolean; override;

		//Returns a newely allocated deep copy/clone of this object.
		function Clone : TOPSObject; override;

		//Fills the parameter obj with all values from this object.
		procedure FillClone(var obj : TOPSObject); override;
  end;

  __classNamePublisher = class(TPublisher)
  public
    constructor Create(t : TTopic);
    destructor Destroy; override;

    // NOTE: write() only serializes the object content and sends it.
    // The object is still owned by the caller.
    procedure write(data : __className);
  end;

  __classNameSubscriber = class(TSubscriber)
  public
    constructor Create(t : TTopic);
    destructor Destroy; override;

    // Copies the latest received data into d
    // Clears the "new data" flag (see newDataExist()).
    function getData(var d : __className) : Boolean;

    // Returns a reference to the latest received data object.
    // Clears the "new data" flag (see newDataExist()).
    // NOTE: MessageLock should be hold while working with the data object, to
    // prevent a new incoming message to delete the current one while in use.
    function getTypedDataReference : __className;
  end;

var
  __classNameArchiveHelper : TArchiverInOut.TSerializableHelper<__className>;

implementation

uses SysUtils,
     uOps.Exceptions;

{ __className }

constructor __className.Create;
__constructorHead
begin
  inherited Create;
  AppendType('__packageName.__className');
__constructorBody
end;

destructor __className.Destroy;
__destructorHead
begin
__destructorBody
  inherited;
end;

class function __className.getTypeName : string;
begin
  Result := '__packageName.__className';
end;

procedure __className.Serialize(archiver : TArchiverInOut);
__serializeHead
begin
	inherited Serialize(archiver);
__serialize
end;

// Validation routine for fields, incl. that contained object references are of the correct type
function __className.Validate : Boolean;
__validateHead
begin
  Result := inherited Validate;
__validateBody
end;

///Returns a newely allocated deep copy/clone of this object.
function __className.Clone : TOPSObject;
begin
	Result := __className.Create;
  Self.FillClone(Result);
end;

///Fills the parameter obj with all values from this object.
procedure __className.FillClone(var obj : TOPSObject);
__fillCloneHead
begin
	inherited fillClone(obj);
  with obj as __className do begin
__fillCloneBody
	end;
end;

{ __classNamePublisher }

constructor __classNamePublisher.Create(t : TTopic);
begin
  inherited Create(t);
end;

destructor __classNamePublisher.Destroy;
begin
  inherited;
end;

procedure __classNamePublisher.write(data : __className);
begin
  inherited write (data);
end;

{ __classNameSubscriber }

constructor __classNameSubscriber.Create(t : TTopic);
begin
  inherited Create(t);
end;

destructor __classNameSubscriber.Destroy;
begin
  inherited;
end;

// Copies the latest received data into d
// Clears the "new data" flag (see newDataExist()).
function __classNameSubscriber.getData(var d : __className) : Boolean;
begin
  Result := False;
  if Assigned(FData) then begin
    aquireMessageLock();
    inherited getData().FillClone(TOPSObject(d));
    releaseMessageLock();
    Result := True;
  end;
end;

// Returns a reference to the latest received data object.
// Clears the "new data" flag (see newDataExist()).
// NOTE: MessageLock should be hold while working with the data object, to
// prevent a new incoming message to delete the current one while in use.
function __classNameSubscriber.getTypedDataReference : __className;
begin
  Result := (getDataReference() as __className);
end;

initialization
  __classNameArchiveHelper := TArchiverInOut.TSerializableHelper<__className>.Create;

finalization
  FreeAndNil(__classNameArchiveHelper);

end.
