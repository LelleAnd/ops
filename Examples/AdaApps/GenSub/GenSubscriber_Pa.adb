
with Ada.Streams;
with Text_IO; use Text_IO;

with Ops_Pa,
     Ops_Pa.Error_Pa,
     Ops_Pa.Notifier_Pa,
     Ops_Pa.OpsObject_Pa,
     Ops_Pa.OpsObject_Pa.Topic_Pa,
     Ops_Pa.OpsObject_Pa.OPSMessage_Pa,
     Ops_Pa.Participant_Pa,
     Ops_Pa.Subscriber_Pa,
     Ops_Pa.FilterQoSPolicy_Pa,
     Ops_Pa.FilterQoSPolicy_Pa.KeyFilterQoSPolicy_Pa,
     Ops_Pa.SerializableFactory_Pa,
     Ops_Pa.SerializableFactory_Pa.GenericTypeFactory;

use  Ops_Pa,
     Ops_Pa.Error_Pa,
     Ops_Pa.OpsObject_Pa,
     Ops_Pa.OpsObject_Pa.Topic_Pa,
     Ops_Pa.OpsObject_Pa.OPSMessage_Pa,
     Ops_Pa.Participant_Pa,
     Ops_Pa.Subscriber_Pa,
     Ops_Pa.FilterQoSPolicy_Pa,
     Ops_Pa.FilterQoSPolicy_Pa.KeyFilterQoSPolicy_Pa,
     Ops_Pa.SerializableFactory_Pa,
     Ops_Pa.SerializableFactory_Pa.GenericTypeFactory;

package body GenSubscriber_Pa is

  use type Ada.Streams.Stream_Element;

  -- ===========================================================================
  --
  -- ===========================================================================
  procedure ErrorLog(Sender : in out Ops_Class_At; Item : Ops_Pa.Error_Pa.Error_Class_At; Arg : Ops_Class_At) is
  begin
    Put_Line("Error:: " & Item.GetMessage);
  end;

  procedure SetUpErrorLog is
  begin
    -- Setup the OPS static error service (common for all participants, reports
    -- errors during participant creation)
    StaticErrorService.addListener(ErrorLog'Access, null);
  end;

  -- ===========================================================================
  --
  -- ===========================================================================
  HexDigits : array (Byte range 0..15) of Character :=
    (
      0 => '0',
      1 => '1',
      2 => '2',
      3 => '3',
      4 => '4',
      5 => '5',
      6 => '6',
      7 => '7',
      8 => '8',
      9 => '9',
     10 => 'A',
     11 => 'B',
     12 => 'C',
     13 => 'D',
     14 => 'E',
     15 => 'F'
     );

  function ToHexNibble(value : Byte) return Character is
  begin
    return HexDigits(value and Byte(16#0F#));
  end;

  function ToHex(value : Byte) return String is
  begin
    return ToHexNibble(value / 16) & ToHexNibble(value);
  end;

  procedure ToHex(value : Byte_Arr) is
    str : String(1..16) := (others => ' ');
    num : Integer := 0;
  begin
    Put("   ");
    for i in value'Range loop
      Put(ToHex(value(i)) & " ");
      num := num + 1;
      if value(i) < 32 or value(i) > 127 then
        str(num) := '.';
      else
        str(num) := Character'Val(value(i));
      end if;
      if num = 8 then
        Put(" ");
      end if;
      if num > 15 then
        num := 0;
        Put_Line("   " & str);
        str := (others => ' ');
        Put("   ");
      end if;
    end loop;
    if num > 0 then
      if num < 8 then
        Put(" ");
      end if;
      for i in num .. 15 loop
        Put("   ");
      end loop;
      Put_Line("   " & str);
    end if;
  end;

  -- ===========================================================================
  --
  -- ===========================================================================
  procedure Listener(cfgfile : String; domainName : String; topicName : String) is
    participant : Participant_Class_At := null;
    topic : Topic_Class_At := null;
    sub : Subscriber_Class_At := null;
    mess : OPSMessage_Class_At := null;
    data : OpsObject_Class_At := null;
    spare : Byte_Arr_At := null;
    spareLen : Integer := 0;
  begin
    -- Create OPS participant to access a domain in the default configuration file
    -- "ops_config.xml" in current working directory. There are other overloads to
    -- create a participant for a specific configuration file.
    participant := getInstance(domainName, domainName, cfgfile);
    if participant = null then
      Put_Line("participant == NULL");
      return;
    end if;

    -- Setup the participant errorService to catch ev. internal OPS errors
    participant.getErrorService.addListener(ErrorLog'Access, null);

    -- Add our generated factory so OPS can create our data objects
    declare
      fact : GenericTypeFactory_Class_At := Create;
    begin
      participant.addTypeSupport( SerializableFactory_Class_At(fact) );
    end;

    -- Create the topic to publish on, might throw ops::NoSuchTopicException
    -- The topic must exist in the used ops configuration file
    topic := participant.getTopic(topicName);

    Put_Line("Subscribing on " & topic.Name &
               " [" & topic.Transport &
               "::" & topic.DomainAddress &
               "::" & Int32'Image(topic.Port) &
               "] ");


    -- Create a subscriber for ChildData
    sub := Create(topic);

    -- Setup any filters ...

    -- Finally start the subscriber (tell it to start listening for data)
    sub.start;

    while True loop
      if sub.waitForNewData(100) then
        -- Need to lock message while using it's data via the reference
        sub.acquireMessageLock;
        begin
          mess := sub.getMessage;
          if mess /= null then
            mess.Reserve;
          end if;
          sub.releaseMessageLock;
        exception
          when others =>
            sub.releaseMessageLock;
        end;
        begin
          if mess /= null then
            spare := mess.SpareBytes;
            if spare /= null then
              spareLen := Integer(spare.all'Length);
            else
              spareLen := 0;
            end if;
            -- Print OPSMessage fields
            Put_Line("Topic: " & mess.TopicName &
                       ", PubName: " & mess.PublisherName &
                       ", Key: " & mess.Key &
                       ", PubId:" & Int64'Image(mess.PublicationID) &
                       ", Ver:" & Byte'Image(mess.OPSMessage_version) &
                       ", IP: " & mess.getSourceIP &
                       "::" & Integer'Image(mess.getSourcePort));

            data := mess.Data;
            if data /= null then
              -- Print OPSObject fields
              Put_Line("  Key: " & data.Key &
                         ", TypeString: " & data.TypesString &
                         ", Ver:" & Byte'Image(data.OPSObject_version) &
                         ", VerMask:" & UInt32'Image(data.IdlVersionMask) &
                         ", # SpareBytes:" & Integer'Image(spareLen));

              ToHex(spare.all);

            else
              Put_Line("  Data = null !!!!" );
            end if;
            mess.UnReserve;
          end if;
        exception
          when others =>
            Put_Line("Exception");
        end;
      end if;
    end loop;

    -- TBD if loop is exited we need to do cleanup
  end Listener;

end GenSubscriber_Pa;
