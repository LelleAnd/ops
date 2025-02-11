with Text_IO; use Text_IO;

with Ops_Pa,
     Ops_Pa.OpsObject_Pa.OPSMessage_Pa,
     Ops_Pa.OpsObject_Pa.Topic_Pa,
     Ops_Pa.Subscriber_Pa,
     Ops_Pa.OpsObject_Pa.TestAll_ChildData,
     Ops_Pa.PublisherAbs_Pa.Publisher_Pa.TestAll_ChildData,
     Ops_Pa.Subscriber_Pa.TestAll_ChildData;

use Ops_Pa,
    Ops_Pa.OpsObject_Pa.OPSMessage_Pa,
    Ops_Pa.Subscriber_Pa,
    Ops_Pa.OpsObject_Pa.TestAll_ChildData,
    Ops_Pa.PublisherAbs_Pa.Publisher_Pa.TestAll_ChildData,
    Ops_Pa.Subscriber_Pa.TestAll_ChildData;

with AssertHelpers_Pa; use AssertHelpers_Pa;
with AssertGenerics_Pa; use AssertGenerics_Pa;

with ClassUnderTest_Pa;
use ClassUnderTest_Pa;

package body ClassUnderTest_Test_Pa is

  package internal is
    use type Ops_Pa.Int64;
  
    -- ==========================================================================
    --      C l a s s    D e c l a r a t i o n.
    -- ==========================================================================
    type Test_Class is new Ops_Class with
      record
        pubTopic : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At := null;
        subTopic : Ops_Pa.OpsObject_Pa.Topic_Pa.Topic_Class_At := null;
        Data : OpsObject_Pa.TestAll_ChildData.ChildData_Class_At := null;
        Pub : PublisherAbs_Pa.Publisher_Pa.TestAll_ChildData.ChildDataPublisher_Class_At := null;
        Sub : Ops_Pa.Subscriber_Pa.TestAll_ChildData.ChildDataSubscriber_Class_At := null;
      end record;
    type Test_Class_At is access all Test_Class'Class;

    function Create(participant : Ops_Pa.Participant_Pa.Participant_Class_At) return Test_Class_At;

    procedure DoTest( Self : in out Test_Class );

    -- ==========================================================================
    --
    -- ==========================================================================
    procedure InitInstance( Self : in out Test_Class;
                            participant : Ops_Pa.Participant_Pa.Participant_Class_At );

    --------------------------------------------------------------------------
    --  Finalize the object
    --  Will be called automatically when object is deleted.
    --------------------------------------------------------------------------
    overriding procedure Finalize( Self : in out Test_Class );
  end internal;

  package body internal is
    
    function Create(participant : Ops_Pa.Participant_Pa.Participant_Class_At ) return Test_Class_At is
      Self : Test_Class_At := null;
    begin
      Self := new Test_Class;
      InitInstance( Self.all, participant );
      return Self;
    exception
      when others =>
        Free(Self);
        raise;
    end;

    procedure InitInstance( Self : in out Test_Class;
                            participant : Ops_Pa.Participant_Pa.Participant_Class_At ) is
    begin
      -- Create some data to publish
      Self.Data := Ops_Pa.OpsObject_Pa.TestAll_ChildData.Create;
      Self.Data.baseText := Copy("Some Server text");
      Self.Data.l := 0;

      -- Create the topic to publish on, might throw ops::NoSuchTopicException
      -- The topic must exist in the used ops configuration file
      Self.pubTopic := participant.getTopic("AA");
      Self.subTopic := participant.getTopic("BB");

      -- Setup publisher
      Self.Pub := Ops_Pa.PublisherAbs_Pa.Publisher_Pa.TestAll_ChildData.Create(Self.subTopic);
      Self.Pub.SetName("ServerPublisher");
      Self.Pub.setKey("InstanceOne");
    
      -- Setup subscriber
      Self.Sub := Ops_Pa.Subscriber_Pa.TestAll_ChildData.Create(Self.pubTopic);
      Self.Sub.start;
    end;

    overriding procedure Finalize( Self : in out Test_Class ) is
    begin
      Free(Self.Sub);
      Free(Self.Pub);
      Free(Self.Data);
    end;

    procedure AssertNotNull is new AssertGenerics_Pa.ExceptionWhenNull(ClassUnderTest_Pa.UnderTest_Class, 
                                                                       ClassUnderTest_Pa.UnderTest_Class_At);

    procedure DoTest (Self : in out Test_Class) is
      cut : ClassUnderTest_Pa.UnderTest_Class_At := null;
      GotData : Boolean;
      Value : Int64;
      ref : Ops_Pa.OpsObject_Pa.TestAll_ChildData.ChildData_Class_At := null;
    begin
      -- Create class under test
      cut := Create(Self.subTopic, Self.pubTopic);
      AssertNotNull(cut, "cut");
      
      -- Check init values
      -- procedure GetValue( Self : UnderTest_Class; GotData : out Boolean; Value : out Int64 );
      cut.GetValue( GotData, Value );
      AssertEQ(GotData, False, "GotData Init");
      AssertEQ(Value, -1, "Value");

      -- Send data to cut via OPS
      Self.Data.l := 42;
      Self.Pub.write(Self.Data);

      -- Check received values
      cut.GetValue( GotData, Value );
      AssertEQ(GotData, True, "GotData");
      AssertEQ(Value, 42, "Value");

      -- Let cut send data via OPS
      cut.update(17);

      -- Verify what was sent
      AssertEQ(Self.Sub.NewDataExist, True, "GotData");
      ref := Ops_Pa.OpsObject_Pa.TestAll_ChildData.ChildData_Class_At(Self.Sub.getDataReference);
      Value := ref.l;
      AssertEQ(Value, 17, "Value");

      -- Let cut send data via OPS
      cut.update(99);

      -- Verify what was sent
      AssertEQ(Self.Sub.NewDataExist, True, "GotData");
      ref := Ops_Pa.OpsObject_Pa.TestAll_ChildData.ChildData_Class_At(Self.Sub.getDataReference);
      Value := ref.l;
      AssertEQ(Value, 99, "Value");
      
      Free(cut);
    end;
  
  end internal;
  
  procedure PerformTest(participant : Ops_Pa.Participant_Pa.Participant_Class_At) is
    tc : internal.Test_Class_At := internal.Create(participant);
  begin
    Put_Line("Testing class 'ClassUnderTest' ...");
    tc.DoTest;
    Put_Line("Finished with ErrorCount: " & Integer'Image(AssertHelpers_Pa.GetNumErrors));
    internal.Free(tc);
  exception
    when others =>
      Put_Line("Exception");
      internal.Free(tc);
  end;

end;

