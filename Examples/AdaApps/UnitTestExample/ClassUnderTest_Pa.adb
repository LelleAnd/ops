
package body ClassUnderTest_Pa is
  
  function Create(subTopic : OpsObject_Pa.Topic_Pa.Topic_Class_At;
                  pubTopic : OpsObject_Pa.Topic_Pa.Topic_Class_At ) return UnderTest_Class_At is
    Self : UnderTest_Class_At := null;
  begin
    Self := new UnderTest_Class;
    InitInstance( Self.all, Self, subTopic, pubTopic );
    return Self;
  exception
    when others =>
      Free(Self);
      raise;
  end;

  procedure GetValue( Self : UnderTest_Class; GotData : out Boolean; Value : out Int64 ) is
  begin
    GotData := Self.GotData;
    Value := Self.Value;
  end;
  
  procedure Update( Self : in out UnderTest_Class; value : Int64) is
  begin
    Self.Data.all.l := value;
    Self.Pub.write(Self.Data);
  end;

  procedure OnNotify( Self : in out UnderTest_Class; Sender : in Ops_Class_At; Item : in OPSMessage_Class_At ) is
  begin
    Self.Value := ChildData_Class_At(Item.Data).l;
    Self.GotData := True;
  end;

  procedure InitInstance( Self : in out UnderTest_Class;
                          SelfAt : UnderTest_Class_At;
                          subTopic : OpsObject_Pa.Topic_Pa.Topic_Class_At;
                          pubTopic : OpsObject_Pa.Topic_Pa.Topic_Class_At ) is
  begin
    Self.SelfAt := SelfAt;

    -- Create some data to publish
    Self.Data := Create;
    Self.Data.baseText := Copy("Some Server text");
    Self.Data.l := 0;

    -- Setup publisher
    Self.Pub := Create(pubTopic);
    Self.Pub.SetName("ServerPublisher");
    Self.Pub.setKey("InstanceOne");
    
    -- Setup subscriber
    Self.Sub := Create(subTopic);
    -- Add this class instance as listener for data, the given method will be called
    Self.Sub.AddListener( Ops_Pa.Subscriber_Pa.MessageNotifier_Pa.Listener_Interface_At(Self.SelfAt) );
    Self.Sub.start;
  end;

  overriding procedure Finalize( Self : in out UnderTest_Class ) is
  begin
    Free(Self.Sub);
    Free(Self.Pub);
    Free(Self.Data);
  end;

end;

