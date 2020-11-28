unit uOps.Exceptions;

(**
*
* Copyright (C) 2016-2020 Lennart Andersson.
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

uses SysUtils;

type
  EArchiverException = class(Exception)
    constructor Create(const Msg: string = '');
  end;

  ECommException = class(Exception)
    constructor Create(const Msg: string = '');
  end;

  EConfigException = class(Exception)
    constructor Create(const Msg: string = '');
  end;

  ENoSuchTopicException = class(Exception)
    constructor Create(const Msg: string = '');
  end;

  EReceiveTimedOutException = class(Exception)
    constructor Create(const Msg: string = '');
  end;

  EPublisherException = class(Exception)
    constructor Create(const Msg: string = '');
  end;

  EIdlVersionException = class(Exception)
    constructor Create(const Msg: string; gotVer : Byte; maxVer : Byte);
  end;

implementation

{ CommException }

constructor ECommException.Create(const Msg: string);
begin
  inherited Create('CommException: ' + Msg);
end;

{ ConfigException }

constructor EConfigException.Create(const Msg: string);
begin
  inherited Create('ConfigException: ' + Msg);
end;

{ NoSuchTopicException }

constructor ENoSuchTopicException.Create(const Msg: string);
begin
  inherited Create('NoSuchTopicException: ' + Msg);
end;

{ EReceiveTimedOutException }

constructor EReceiveTimedOutException.Create(const Msg: string);
begin
  inherited Create('ReceiveTimedOutException: ' + Msg);
end;

{ EPublisherException }

constructor EPublisherException.Create(const Msg: string);
begin
  inherited Create('PublisherException: ' + Msg);
end;

{ EArchiverException }

constructor EArchiverException.Create(const Msg: string);
begin
  inherited Create('ArchiverException: ' + Msg);
end;

constructor EIdlVersionException.Create(const Msg: string; gotVer : Byte; maxVer : Byte);
begin
  inherited Create('IdlVersionException: ' +
                   Msg + ' received version "' + IntToStr(gotVer) +
                   '" > known version "' + IntToStr(maxVer) + '"');
end;

end.

