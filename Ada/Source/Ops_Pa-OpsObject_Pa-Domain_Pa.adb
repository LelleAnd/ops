--
-- Copyright (C) 2016 Lennart Andersson.
--
-- This file is part of OPS (Open Publish Subscribe).
--
-- OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Lesser General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public License
-- along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.

with Ops_Pa.Error_Pa;
use Ops_Pa.Error_Pa;

package body Ops_Pa.OpsObject_Pa.Domain_Pa is

  -- Constructors
  function Create return Domain_Class_At is
    Self : Domain_Class_At := null;
  begin
    Self := new Domain_Class;
    InitInstance( Self.all );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end Create;

  -- Getters
  function DomainAddress( Self : Domain_Class ) return String is
  begin
    return Self.DomainAddress.all;
  end;

  function DomainID( Self : Domain_Class ) return String is
  begin
    return Self.DomainID.all;
  end;

  function MetaDataMcPort( Self : Domain_Class ) return Int32 is
  begin
    return Self.MetaDataMcPort;
  end;

  function TimeToLive( Self : Domain_Class ) return Int32 is
  begin
    return Self.TimeToLive;
  end;

  function LocalInterface( Self : Domain_Class ) return String is
  begin
    return Self.LocalInterface.all;
  end;

  function InSocketBufferSize( Self : Domain_Class ) return Int32 is
  begin
    return Self.InSocketBufferSize;
  end;

  function OutSocketBufferSize( Self : Domain_Class ) return Int32 is
  begin
    return Self.OutSocketBufferSize;
  end;

  -- Helpers for handling [de]serializing of fixed arrays
  procedure Topic_Class_InoutDynArr is new inoutdynarr2(Topic_Class, Topic_Class_At, Topic_Class_At_Arr, Topic_Class_At_Arr_At);
  procedure Channel_Class_InoutDynArr is new inoutdynarr2(Channel_Class, Channel_Class_At, Channel_Class_At_Arr, Channel_Class_At_Arr_At);
  procedure Transport_Class_InoutDynArr is new inoutdynarr2(Transport_Class, Transport_Class_At, Transport_Class_At_Arr, Transport_Class_At_Arr_At);

  procedure Serialize( Self : in out Domain_Class; archiver : ArchiverInOut_Class_At) is
  begin
    Serialize( OpsObject_Class(Self), archiver );
    archiver.Inout("domainID", Self.domainID);
    Topic_Class_InoutDynArr(archiver, "topics", Self.topics);
    archiver.Inout("domainAddress", Self.domainAddress);
    archiver.Inout("localInterface", Self.localInterface);
    archiver.Inout("timeToLive", Self.timeToLive);
    archiver.Inout("inSocketBufferSize", Self.inSocketBufferSize);
    archiver.Inout("outSocketBufferSize", Self.outSocketBufferSize);
    archiver.Inout("metaDataMcPort", Self.metaDataMcPort);

    -- To not break binary compatibility we only do this when we know we are
    -- reading from an XML-file
    raise Not_Yet_Implemented;
--    if archiver is TXMLArchiverIn then
--      Channel_Class_InoutDynArr(archiver, "channels", Self.channels);
--      Transport_Class_InoutDynArr(archiver, "transports", Self.transports);
--      CheckTransports();
--    end if;
  end;

  -- Returns a newely allocated deep copy/clone of Self.
  function Clone( Self : Domain_Class ) return OpsObject_Class_At is
    Result : Domain_Class_At := null;
  begin
    Result := Create;
    Self.FillClone( OpsObject_Class_At(Result) );
    return OpsObject_Class_At(Result);
  end Clone;

    -- Helpers
  procedure Dispose is new Ada.Unchecked_Deallocation( Topic_Class_Arr, Topic_Class_Arr_At );
  procedure Dispose is new Ada.Unchecked_Deallocation( Topic_Class_At_Arr, Topic_Class_At_Arr_At );

  procedure Clear( Arr : in out Topic_Class_At_Arr) is
  begin
    for i in Arr'Range loop
      if Arr(i) /= null then
        Free(Arr(i));
      end if;
    end loop;
  end;

  -- Helpers
  procedure Dispose is new Ada.Unchecked_Deallocation( Channel_Class_Arr, Channel_Class_Arr_At );
  procedure Dispose is new Ada.Unchecked_Deallocation( Channel_Class_At_Arr, Channel_Class_At_Arr_At );

  procedure Clear( Arr : in out Channel_Class_At_Arr) is
  begin
    for i in Arr'Range loop
      if Arr(i) /= null then
        Free(Arr(i));
      end if;
    end loop;
  end;

  -- Helpers
  procedure Dispose is new Ada.Unchecked_Deallocation( Transport_Class_Arr, Transport_Class_Arr_At );
  procedure Dispose is new Ada.Unchecked_Deallocation( Transport_Class_At_Arr, Transport_Class_At_Arr_At );

  procedure Clear( Arr : in out Transport_Class_At_Arr) is
  begin
    for i in Arr'Range loop
      if Arr(i) /= null then
        Free(Arr(i));
      end if;
    end loop;
  end;

  -- Fills the parameter obj with all values from Self.
  procedure FillClone( Self : Domain_Class; obj : OpsObject_Class_At ) is
  begin
    FillClone( OpsObject_Class(Self), obj );
    if obj.all in Domain_Class'Class then
      Replace(Domain_Class(obj.all).domainAddress, Self.domainAddress);
      Domain_Class(obj.all).timeToLive := Self.timeToLive;
      Replace(Domain_Class(obj.all).localInterface, Self.localInterface);
      Domain_Class(obj.all).inSocketBufferSize := Self.inSocketBufferSize;
      Domain_Class(obj.all).outSocketBufferSize := Self.outSocketBufferSize;
      if Domain_Class(obj.all).topics /= null then
        Clear(Domain_Class(obj.all).topics.all);
        Dispose(Domain_Class(obj.all).topics);
      end if;
      if Self.topics /= null then
        Domain_Class(obj.all).topics := new Topic_Class_At_Arr(Self.topics'Range);
        for i in Self.topics'Range loop
          if Self.topics(i) /= null then
            Domain_Class(obj.all).topics(i) := Topic_Class_At(Clone(Self.topics(i).all));
          end if;
        end loop;
      end if;
      Replace(Domain_Class(obj.all).domainID, Self.domainID);
      Domain_Class(obj.all).metaDataMcPort := Self.metaDataMcPort;
      if Domain_Class(obj.all).channels /= null then
        Clear(Domain_Class(obj.all).channels.all);
        Dispose(Domain_Class(obj.all).channels);
      end if;
      if Self.channels /= null then
        Domain_Class(obj.all).channels := new Channel_Class_At_Arr(Self.channels'Range);
        for i in Self.channels'Range loop
          if Self.channels(i) /= null then
            Domain_Class(obj.all).channels(i) := Channel_Class_At(Clone(Self.channels(i).all));
          end if;
        end loop;
      end if;
      if Domain_Class(obj.all).transports /= null then
        Clear(Domain_Class(obj.all).transports.all);
        Dispose(Domain_Class(obj.all).transports);
      end if;
      if Self.transports /= null then
        Domain_Class(obj.all).transports := new Transport_Class_At_Arr(Self.transports'Range);
        for i in Self.transports'Range loop
          if Self.transports(i) /= null then
            Domain_Class(obj.all).transports(i) := Transport_Class_At(Clone(Self.transports(i).all));
          end if;
        end loop;
      end if;
    end if;
  end;

  procedure checkTopicValues( Self : Domain_Class; top : Topic_Class_At) is
  begin
    if top.DomainAddress = "" then
      top.SetDomainAddress( Self.DomainAddress.all );
    end if;
    if top.LocalInterface = "" then
      top.SetLocalInterface( Self.LocalInterface.all );
    end if;
    if top.TimeToLive < 0 then
      top.SetTimeToLive( Self.TimeToLive );
    end if;
    if top.InSocketBufferSize < 0 then
      top.SetInSocketBufferSize( Int64(Self.InSocketBufferSize) );
    end if;
    if top.OutSocketBufferSize < 0 then
      top.SetOutSocketBufferSize( Int64(Self.OutSocketBufferSize) );
    end if;
  end;

  -- Returns references to the internal topics
  -- NOTE: The Domain still owns them
  function getTopics( Self : Domain_Class ) return Topic_Class_At_Arr_At is
  begin
    if Self.Topics /= null then
      for i in Self.Topics.all'Range loop
        checkTopicValues(Self, Self.Topics(i));
      end loop;
    end if;
    return Self.Topics;
  end;

  -- Returns a reference to the internal topic
  -- NOTE: The Domain still owns it
  function getTopic( Self : Domain_Class; Name : String ) return Topic_Class_At is
  begin
    if Self.Topics /= null then
      for i in Self.Topics.all'Range loop
        if Self.Topics(i).Name = Name then
          checkTopicValues(Self, Self.Topics(i));
          return Self.Topics(i);
        end if;
      end loop;
    end if;
    StaticErrorService.
      Report( Error_Class_At(Create("Domain", "getTopic",
              "Topic '" & Name & "' does not exist in ops config file." )));
    raise ENoSuchTopicException;
  end;

  function topicExist( Self : Domain_Class; Name : String ) return Boolean is
  begin
    if Self.Topics /= null then
      for i in Self.Topics.all'Range loop
        if Self.Topics(i).Name = Name then
          return True;
        end if;
      end loop;
    end if;
    return False;
  end;

  function findChannel( Self : Domain_Class; id : String ) return Channel_Class_At is
  begin
    if id /= "" then
      for i in Self.Channels.all'Range loop
        if id = Self.Channels(i).channelID.all then
          return Self.Channels(i);
        end if;
      end loop;
    end if;
    return null;
  end;

  function findTopic( Self : Domain_Class; id : String) return Topic_Class_At is
  begin
    if id /= "" then
      for i in Self.Topics.all'Range loop
        if id = Self.Topics(i).Name then
          return Self.Topics(i);
        end if;
      end loop;
    end if;
    return null;
  end;

  procedure checkTransports( Self : in out Domain_Class ) is
    trp : TRansport_Class_At := null;
    channel : Channel_Class_At := null;
    top : Topic_Class_At := null;
  begin
    -- Now update topics with values from the transports and channels
    -- Loop over all transports and for each topic, see if it needs parameters from the channel
    for i in Self.Transports.all'Range loop
      trp := Self.Transports(i);
      -- Get channel
      channel := findChannel( Self, trp.channelID.all );
      if channel = null then
        StaticErrorService.
          Report( Error_Class_At(Create("Domain", "CheckTransport",
                  "Non existing channelID: '" & trp.channelID.all &
                    "' used in transport spcification." )));
        raise EConfigException;
      else
        for j in trp.topics.all'Range loop
          top := findTopic( Self, trp.Topics(j).all );
          if top = null then
            StaticErrorService.
              Report( Error_Class_At(Create("Domain", "CheckTransport",
                      "Non existing topicID: '" & trp.Topics(j).all &
                        "' used in transport specification." )));
            raise EConfigException;
          else
            channel.PopulateTopic(top);
          end if;
        end loop;
      end if;
    end loop;
  end;

--  /// ------------------------------------------
--  /// Helper to get all IP interfaces
--
--  procedure VVGetIpAddrTable(var p: PMibIpAddrTable; var Size: Cardinal; const bOrder: BOOL);
--  var
--    Res: DWORD;
--  begin
--    p := nil;
--    Size := 0;
--    if @GetIpAddrTable = nil then Exit;   //Not implemented in this windows version
--    Res := GetIpAddrTable(p,Size,bOrder);
--    if Res=ERROR_INSUFFICIENT_BUFFER then begin
--      Getmem(p,Size);
--      // Caller must free this buffer when it is no longer used
--      FillChar(p^,Size,#0);
--      Res := GetIpAddrTable(p,Size,bOrder);
--    end;
--    if Res <> NO_ERROR then begin
--      if Assigned(p) then FreeMem(p);
--      p := nil;
--      Size := 0;
--    end;
--  end;
--
--  function IpAddressToString(Addr: DWORD): AnsiString;
--  var
--    InAddr: TInAddr;
--  begin
--    InAddr.S_addr := Addr;
--    Result := AnsiString(inet_ntoa(InAddr));
--  end;

-- If argument contains a "/" we assume it is on the form:  subnet-address/subnet-mask
-- e.g "192.168.10.0/255.255.255.0" or "192.168.10.0/24"
-- In that case we loop over all interfaces and take the first one that matches
-- i.e. the one whos interface address is on the subnet
  function doSubnetTranslation(addr : String) return String is
--      Idx : Integer;
--      subnet, mask : AnsiString;
--      subnetIp, subnetMask : DWORD;
--      Size: ULONG;
--      p: PMibIpAddrTable;
--      i: integer;
  begin
    return addr;
--    Result := addr;
--
--      -- If no '/' we just return the given address
--      Idx := Pos('/', string(addr));
--      if Idx = 0 then Exit;
--
--        subnet := Copy(addr, 1, Idx-1);
--        mask   := Copy(addr, Idx+1, MaxInt);
--
--    subnetIp := inet_addr(PAnsiChar(subnet));
--    if Length(mask) <= 2 then begin
--      // Expand to the number of bits given
--      subnetMask := StrToInt(string(mask));
--      subnetMask := (((1 shl subnetMask)-1) shl (32 - subnetMask)) and $FFFFFFFF;
--      subnetMask := ntohl(subnetMask);
--    end else begin
--      subnetMask := inet_addr(PAnsiChar(mask));
--    end;
--
--    VVGetIpAddrTable(p, Size, False);
--    if Assigned(p) then begin
--      try
--        with p^ do begin
--          for i := 0 to dwNumEntries - 1 do begin
--            with table[i] do begin
--              if (dwAddr and subnetMask) = (subnetIp and subnetMask) then begin
--                Result := IpAddressToString(dwAddr);
--                Break;
--              end;
--            end;
--          end;
--        end;
--      finally
--        FreeMem(p);
--      end;
--    end;
  end;

  procedure InitInstance( Self : in out Domain_Class ) is
  begin
    InitInstance( OpsObject_Class(Self) );
    AppendType( OpsObject_Class(Self), "Domain" );
    Self.DomainAddress := Copy("");
    Self.DomainID := Copy("");
  end;

  procedure Finalize( Self : in out Domain_Class ) is
  begin
    if Self.domainAddress /= null then
      Dispose(Self.domainAddress);
    end if;
    if Self.localInterface /= null then
      Dispose(Self.localInterface);
    end if;
    if Self.topics /= null then
      Clear(Self.topics.all);
      Dispose(Self.topics);
    end if;
    if Self.domainID /= null then
      Dispose(Self.domainID);
    end if;
    if Self.channels /= null then
      Clear(Self.channels.all);
      Dispose(Self.channels);
    end if;
    if Self.transports /= null then
      Clear(Self.transports.all);
      Dispose(Self.transports);
    end if;

    Finalize( OpsObject_Class(Self) );
  end;

end Ops_Pa.OpsObject_Pa.Domain_Pa;

