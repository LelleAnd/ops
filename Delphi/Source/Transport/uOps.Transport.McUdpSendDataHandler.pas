unit uOps.Transport.McUdpSendDataHandler;

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

uses System.Generics.Collections,
     System.SyncObjs,
     uOps.Types,
     uOps.Error,
     uOps.Topic,
     uOps.Transport.Sender,
     uOps.Transport.SendDataHandler;

type
  TIpPortPair = class(TObject)
  private
    ip : string;
    port : Integer;
    alwaysAlive : Boolean;
    lastTimeAlive : Int64;
    const ALIVE_TIMEOUT = 3000;
  public
    constructor Create(ip : string; port : Integer; alwaysAlive : Boolean);
    function isAlive : Boolean;
    procedure feedWatchdog(alwaysAlive : Boolean);
    function getKey : string; overload;
    class function getKey(ip : string; port : Integer) : string; overload;
  end;

  Entry_T = record
    staticRoute : Boolean;
    portMap : TDictionary<string,TIpPortPair>;
  end;

  TMcUdpSendDataHandler = class(TSendDataHandler)
  private
    FTopSinkMap : TDictionary<string,Entry_T>;

  public
    constructor Create(localInterface : string; ttl : Integer; outSocketBufferSize : Int64; Reporter : TErrorService);
    destructor Destroy; override;

		function sendData(buf : PByte; bufSize : Integer; topic : TTopic) : Boolean; override;

    procedure addSink(topic : string; ip : string; port : Integer; staticRoute : Boolean = False);
  end;

implementation

uses Classes,
     SysUtils,
     uOps.TimeHelper;

constructor TIpPortPair.Create(ip : string; port : Integer; alwaysAlive : Boolean);
begin
  inherited Create;
  Self.ip := ip;
  Self.port := port;
  Self.alwaysAlive := alwaysAlive;
  Self.lastTimeAlive := TTimeHelper.CurrentTimeMillis;
end;

function TIpPortPair.isAlive : Boolean;
begin
  Result := alwaysAlive or ((TTimeHelper.currentTimeMillis - lastTimeAlive) < ALIVE_TIMEOUT);
end;

procedure TIpPortPair.feedWatchdog(alwaysAlive : Boolean);
begin
  Self.alwaysAlive := Self.alwaysAlive or alwaysAlive;  // Once set to true always true
  lastTimeAlive := TTimeHelper.CurrentTimeMillis;
end;

function TIpPortPair.getKey : string;
begin
  Result := getKey(ip, port);
end;

class function TIpPortPair.getKey(ip : string; port : Integer) : string;
begin
  Result := ip + ':' + IntToStr(port);
end;

constructor TMcUdpSendDataHandler.Create(localInterface : string; ttl : Integer; outSocketBufferSize : Int64; Reporter : TErrorService);
begin
  inherited Create;
  FTopSinkMap := TDictionary<string,Entry_T>.Create;

  FSender := TSenderFactory.CreateUDPSender(localInterface, ttl, OutSocketBufferSize);
  FSender.ErrorService := Reporter;
  FSender.setConnectStatusListener(onConnectStatusChanged);
end;

destructor TMcUdpSendDataHandler.Destroy;
var
  Key, Key2 : string;
  portMap : TDictionary<string, TIpPortPair>;
  ipPort : TIpPortPair;
begin
  FMutex.Acquire;
  try
    // Free objects in sink map
    for Key in FTopSinkMap.Keys do begin
      portMap := FTopSinkMap.Items[Key].portMap;
      for Key2 in portMap.Keys do begin
        ipPort := portMap.Items[Key2];
        FreeAndNil(ipPort);
      end;
      FreeAndNil(portMap);
    end;
    FreeAndNil(FTopSinkMap);
    FreeandNil(FSender);
  finally
    FMutex.Release;
  end;
  inherited;
end;

function TMcUdpSendDataHandler.sendData(buf : PByte; bufSize : Integer; topic : TTopic) : Boolean;
var
  i : Integer;
  ipPort : TIpPortPair;
  tmpPair : TPair<string, TIpPortPair>;
  topicSincs : Entry_T;
  sinksToDelete : TStringList;
begin
  Result := True;
  FMutex.Acquire;
  try
    if FTopSinkMap.TryGetValue(string(topic.Name), topicSincs) then begin
      sinksToDelete := TStringList.Create;
      try
        // Loop over all sinks and send data, remove items that isn't "alive".
        for ipPort in topicSincs.portMap.Values do begin
          // Check if this sink is alive
          if ipPort.isAlive then begin
            Result := Result and FSender.sendTo(buf, bufSize, ipPort.ip, ipPort.port);
          end else begin
            // Mark it for deletion
            //std::cout << topic.getName() << " removing " << it->second.getKey() << std::endl;
            sinksToDelete.Add(ipPort.getKey);
          end;
        end;

        // Delete all IpPortPair's that is marked for delete
        for i := 0 to sinksToDelete.Count - 1 do begin
          tmpPair := topicSincs.portMap.ExtractPair(sinksToDelete[i]);
          FreeAndNil(tmpPair.Value);
        end;
      finally
        FreeAndNil(sinksToDelete);
      end;
    end;
  finally
    FMutex.Release;
  end;
end;

procedure TMcUdpSendDataHandler.addSink(topic : string; ip : string; port : Integer; staticRoute : Boolean);
var
  ipPort : TIpPortPair;
  ipPortMap : Entry_T;
begin
  FMutex.Acquire;
  try
    // check if we already have any sinks for this topic
    if not FTopSinkMap.TryGetValue(topic, ipPortMap) then begin
      // We have no sinks for this topic, so add it
      ipPortMap.staticRoute := staticRoute;
      ipPortMap.portMap := TDictionary<string, TIpPortPair>.Create;
      FTopSinkMap.Add(topic, ipPortMap);
    end;

    //If created as static route, we only add sinks that are static
    if (not ipPortMap.staticRoute) or (ipPortMap.staticRoute and staticRoute) then begin
      if not ipPortMap.portMap.TryGetValue(TIpPortPair.getKey(ip, port), ipPort) then begin
        // We have no matching sink, add it
        ipPort := TIpPortPair.Create(ip, port, staticRoute);
        ipPortMap.portMap.Add(ipPort.getKey, ipPort);
        //std::cout << topic << " added as new sink " << ipPort.getKey() << std::endl;
      end;

      ipPort.feedWatchdog(staticRoute);
    end;
  finally
    FMutex.Release;
  end;
end;

end.

