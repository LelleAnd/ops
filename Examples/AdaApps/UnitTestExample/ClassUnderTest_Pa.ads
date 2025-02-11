
with Ops_Pa,
     Ops_Pa.OpsObject_Pa.OPSMessage_Pa,
     Ops_Pa.OpsObject_Pa.Topic_Pa,
     Ops_Pa.OpsObject_Pa.TestAll_ChildData,
     Ops_Pa.PublisherAbs_Pa.Publisher_Pa.TestAll_ChildData,
     Ops_Pa.Subscriber_Pa.TestAll_ChildData;

use Ops_Pa,
    Ops_Pa.OpsObject_Pa.OPSMessage_Pa,
    Ops_Pa.OpsObject_Pa.Topic_Pa,
    Ops_Pa.OpsObject_Pa.TestAll_ChildData,
    Ops_Pa.PublisherAbs_Pa.Publisher_Pa.TestAll_ChildData,
    Ops_Pa.Subscriber_Pa.TestAll_ChildData;

package ClassUnderTest_Pa is
  
  use type Ops_Pa.Int64;
  
-- ==========================================================================
--      C l a s s    D e c l a r a t i o n.
-- ==========================================================================
  type UnderTest_Class    is new Ops_Class and
      Ops_Pa.Subscriber_Pa.MessageNotifier_Pa.Listener_Interface with private;
  type UnderTest_Class_At is access all UnderTest_Class'Class;

  function Create(subTopic : OpsObject_Pa.Topic_Pa.Topic_Class_At;
                  pubTopic : OpsObject_Pa.Topic_Pa.Topic_Class_At ) return UnderTest_Class_At;

  procedure GetValue( Self : UnderTest_Class; GotData : out Boolean; Value : out Int64 );
  procedure Update( Self : in out UnderTest_Class; value : Int64);

private
  procedure OnNotify( Self : in out UnderTest_Class; Sender : in Ops_Class_At; Item : in OPSMessage_Class_At );

-- ==========================================================================
--
-- ==========================================================================
  type UnderTest_Class is new Ops_Class and
      Ops_Pa.Subscriber_Pa.MessageNotifier_Pa.Listener_Interface with
    record
      SelfAt : UnderTest_Class_At := null;
      Value : Int64 := -1;
      GotData : Boolean := False;
      Data : OpsObject_Pa.TestAll_ChildData.ChildData_Class_At := null;
      Pub : PublisherAbs_Pa.Publisher_Pa.TestAll_ChildData.ChildDataPublisher_Class_At := null;
      Sub : Ops_Pa.Subscriber_Pa.TestAll_ChildData.ChildDataSubscriber_Class_At := null;
    end record;

  procedure InitInstance( Self : in out UnderTest_Class;
                          SelfAt : UnderTest_Class_At;
                          subTopic : OpsObject_Pa.Topic_Pa.Topic_Class_At;
                          pubTopic : OpsObject_Pa.Topic_Pa.Topic_Class_At );

  --------------------------------------------------------------------------
  --  Finalize the object
  --  Will be called automatically when object is deleted.
  --------------------------------------------------------------------------
  overriding procedure Finalize( Self : in out UnderTest_Class );

end;

