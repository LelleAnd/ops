unit uOps.Types;

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

uses uLogger;

const
  PACKET_MAX_SIZE = 60000;
  MAX_DEADLINE_TIMEOUT = High(Int64);

type
  TDynSingleArray = array of Single;
  TDynInt16Array = array of Int16;
  TDynInt32Array = array of Int32;
  TDynInt64Array = array of Int64;
  TDynDoubleArray = array of Double;
  TDynByteArray = array of Byte;
  TDynBooleanArray = array of Boolean;
  TDynAnsiStringArray = array of AnsiString;

  // Used for reporting Connect Status (only for TCP transports for now)
  TConnectStatus = record
    Address : String;
    Port : Integer;
    Connected : Boolean;
    TotalNo : Integer;
  end;

var
  Logger : TLogger;

implementation

initialization
  Logger := TLogger.Create;
  Logger.AddTime := True;
  Logger.LogType := ltNone;   // Only used for debug, so set default to no logging

finalization
  Logger.Free;

end.

