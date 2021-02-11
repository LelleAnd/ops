unit uOps.Transport.TCPSendDataHandler;

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
  TTCPSendDataHandler = class(TSendDataHandler)
  private
  public
    constructor Create(topic : TTopic; Reporter : TErrorService);
    destructor Destroy; override;

    function sendData(buf : PByte; bufSize : Integer; topic : TTopic) : Boolean; override;
  end;

implementation

uses SysUtils;

constructor TTCPSendDataHandler.Create(topic : TTopic; Reporter : TErrorService);
begin
  inherited Create;
  FSender := TSenderFactory.createTCPServer(string(topic.DomainAddress), topic.Port, topic.OutSocketBufferSize);
  FSender.ErrorService := Reporter;
  FSender.setConnectStatusListener(onConnectStatusChanged);
end;

destructor TTCPSendDataHandler.Destroy;
begin
  FMutex.Acquire;
  try
    FreeandNil(FSender);
  finally
    FMutex.Release;
  end;
  inherited;
end;

function TTCPSendDataHandler.sendData(buf : PByte; bufSize : Integer; topic : TTopic) : Boolean;
begin
  // We don't "sendTo" but rather lets the server (sender) send to all conncted clients.
  Result := FSender.sendTo(buf, bufSize, '', 0);
end;

end.

