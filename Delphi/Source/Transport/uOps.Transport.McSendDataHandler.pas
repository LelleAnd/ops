unit uOps.Transport.McSendDataHandler;

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

uses System.SyncObjs,
     uOps.Types,
     uOps.Error,
     uOps.Topic,
     uOps.Transport.Sender,
     uOps.Transport.SendDataHandler;

type
  TMcSendDataHandler = class(TSendDataHandler)
  public
    constructor Create(topic : TTopic; localInterface : string; ttl : Integer; Reporter : TErrorService);
    destructor Destroy; override;

		function sendData(buf : PByte; bufSize : Integer; topic : TTopic) : Boolean; override;
  end;

implementation

uses SysUtils;

constructor TMcSendDataHandler.Create(topic : TTopic; localInterface : string; ttl : Integer; Reporter : TErrorService);
begin
  inherited Create;
  FSender := TSenderFactory.CreateMCSender(localInterface, ttl, topic.OutSocketBufferSize);
  FSender.ErrorService := Reporter;
  FSender.setConnectStatusListener(onConnectStatusChanged);
end;

destructor TMcSendDataHandler.Destroy;
begin
  FMutex.Acquire;
  try
    FreeandNil(FSender);
  finally
    FMutex.Release;
  end;
  inherited;
end;

function TMcSendDataHandler.sendData(buf : PByte; bufSize : Integer; topic : TTopic) : Boolean;
begin
  Result := FSender.sendTo(buf, bufSize, string(topic.DomainAddress), topic.Port);
end;

end.

