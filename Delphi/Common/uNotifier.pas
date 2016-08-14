unit uNotifier;

(**
*
* Copyright (C) 2016 Lennart Andersson.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*)

interface

uses Classes, System.SyncObjs;

type
  // Sender is the object owning the TNotifier (given in the Constructor)
  TOnNotifyEvent<T> = procedure(Sender : TObject; arg : T) of object;

  // Interface that used in conjunction with Notifier
  // forms an implmentation of the Observer GoF-pattern.
  IListener<T> = interface
    // If this interface is registred with a Notifier, this method will be
    // called when the Notifier wants to inform its Listeners.
    // Sender is the object owning the TNotifier (given in the Constructor)
		procedure onNotify(Sender : TObject; arg : T);
  end;

  // Interface for a Notifier
  INotifier<T> = interface
    // Called by subclasses that wishes to notify its listeners.
		procedure doNotify(arg : T);
    // Register a Listener via Interface
    procedure addListener(Listener : IListener<T>); overload;
    // Register a Listener via callback event
    procedure addListener(Proc : TOnNotifyEvent<T>); overload;
  end;

  // class which in the conjunction with Listener forms an implementation of the
  // observer GoF-pattern. classes extending this class extends an interface to which
  // Listeners can register their interest to be notified.
  TNotifier<T> = class(TObject)
  private
    FOwner : TObject;
    FMutex : TMutex;
    // Vector that holds pointers to the IListeners
    FListerners : TInterfaceList;
    FEvents : array of TOnNotifyEvent<T>;

    function SameMethod(AMethod1, AMethod2: TOnNotifyEvent<T>): Boolean;
    function GetNumListeners : Integer;

  public
    constructor Create(Owner : TObject);
    destructor Destroy; override;

    // Called by subclasses that wishes to notify its listeners.
		procedure doNotify(arg : T);

    // Register a Listener via Interface
    procedure addListener(listener : IListener<T>); overload;

    // Register a Listener via callback event
    procedure addListener(proc : TOnNotifyEvent<T>); overload;

    // Remove a Listener via Interface
    procedure removeListener(listener : IListener<T>); overload;

    // Remove a Listener via callback event
    procedure removeListener(proc : TOnNotifyEvent<T>); overload;

    property numListeners : Integer read GetNumListeners;
  end;

implementation

uses SysUtils;

{ TNotifier }

constructor TNotifier<T>.Create(Owner : TObject);
begin
  FOwner := Owner;
  FMutex := TMutex.Create;
  FListerners := TInterfaceList.Create;
  SetLength(FEvents, 0);
end;

destructor TNotifier<T>.Destroy;
begin
  FListerners.Free;
  SetLength(FEvents, 0);
  FreeAndNIl(FMutex);
  inherited;
end;

function TNotifier<T>.GetNumListeners : Integer;
begin
  Result := Length(FEvents) + FListerners.Count;
end;

procedure TNotifier<T>.addListener(Listener: IListener<T>);
begin
  FMutex.Acquire;
  try
    FListerners.Add(Listener);
  finally
    FMutex.Release;
  end;
end;

procedure TNotifier<T>.addListener(Proc: TOnNotifyEvent<T>);
var
  Len : Integer;
begin
  FMutex.Acquire;
  try
    Len := Length(FEvents);
    SetLength(FEvents, Len + 1);
    FEvents[Len] := Proc;
  finally
    FMutex.Release;
  end;
end;

procedure TNotifier<T>.removeListener(listener : IListener<T>);
begin
  FMutex.Acquire;
  try
    FListerners.Remove(Listener);
  finally
    FMutex.Release;
  end;
end;

function TNotifier<T>.SameMethod(AMethod1, AMethod2: TOnNotifyEvent<T>): Boolean;
begin
  Result := (TMethod(AMethod1).Code = TMethod(AMethod2).Code)
            and (TMethod(AMethod1).Data = TMethod(AMethod2).Data);
end;

procedure TNotifier<T>.removeListener(proc : TOnNotifyEvent<T>);
var
  Len, i, Found : Integer;
begin
  Found := -1;
  FMutex.Acquire;
  try
    Len := Length(FEvents);
    for i := 0 to Len - 1 do begin
      if SameMethod(FEvents[i], proc) then begin
        Found := i;
        Break;
      end;
    end;
    if Found >= 0 then begin
      for i := Found+1 to Len - 1 do begin
        FEvents[i-1] := FEvents[i];
      end;
      SetLength(FEvents, Len - 1);
    end;
  finally
    FMutex.Release;
  end;
end;

procedure TNotifier<T>.doNotify(arg : T);
var
  i : Integer;
begin
  FMutex.Acquire;
  try
    for i := 0 to FListerners.Count - 1 do begin
      IListener<T>(FListerners.Items[i]).onNotify(FOwner, arg);
    end;
    for i := 0 to Length(FEvents) - 1 do begin
      FEvents[i](FOwner, arg);
    end;
  finally
    FMutex.Release;
  end;
end;

end.

