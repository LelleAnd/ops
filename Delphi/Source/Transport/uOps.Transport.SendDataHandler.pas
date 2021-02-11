unit uOps.Transport.SendDataHandler;

(**
*
* Copyright (C) 2016-2021 Lennart Andersson.
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

uses Contnrs,
     System.SyncObjs,
     uNotifier,
     uOps.Types,
     uOps.Topic,
     uOps.Transport.Sender;

type
  TSendDataHandler = class(TObject)
  private
    FUsers : TObjectList;

    FCsNotifier : TNotifierValue<TConnectStatus>;

  protected
    FSender : TSender;
    FMutex : TMutex;

    procedure onConnectStatusChanged(Sender : TObject; arg : TConnectStatus);

  public
    constructor Create;
    destructor Destroy; override;

		procedure addListener(Proc : TOnNotifyEvent<TConnectStatus>);
		procedure removeListener(Proc : TOnNotifyEvent<TConnectStatus>);

		function sendData(buf : PByte; bufSize : Integer; topic : TTopic) : Boolean; virtual; abstract;

		procedure addUser(client : TObject); virtual;
		procedure removeUser(client : TObject); virtual;
  end;

implementation

uses Classes, SysUtils, System.Types;

constructor TSendDataHandler.Create;
begin
  inherited Create;
  FUsers := TObjectList.Create(False);    // We don't own objects in list
  FCsNotifier := TNotifierValue<TConnectStatus>.Create(Self, True);
  FMutex := TMutex.Create;
end;

destructor TSendDataHandler.Destroy;
begin
  FreeAndNil(FMutex);
  FreeAndNil(FCsNotifier);
  FreeAndNil(FUsers);
  inherited;
end;

procedure TSendDataHandler.addListener(Proc : TOnNotifyEvent<TConnectStatus>);
begin
  FCsNotifier.addListener(Proc);
end;

procedure TSendDataHandler.removeListener(Proc : TOnNotifyEvent<TConnectStatus>);
begin
  FCsNotifier.removeListener(Proc);
end;

procedure TSendDataHandler.onConnectStatusChanged(Sender : TObject; arg : TConnectStatus);
begin
  FCsNotifier.doNotify(arg);
end;

procedure TSendDataHandler.addUser(client : TObject);
begin
  FMutex.Acquire;
  try
    // Check that it isn't already in the list
    if FUsers.IndexOf(client) >= 0 then Exit;

    // Save client in the list
    FUsers.Add(client);

    // For the first client, we open the sender
    if FUsers.Count = 1 then FSender.Open;
  finally
    FMutex.Release;
  end;
end;

procedure TSendDataHandler.removeUser(client : TObject);
begin
  FMutex.Acquire;
  try
    // Remove it from the list
    FUsers.Extract(client);

    // For the last client, we close the sender
    if FUsers.Count = 0 then FSender.Close;
  finally
    FMutex.Release;
  end;
end;

end.

