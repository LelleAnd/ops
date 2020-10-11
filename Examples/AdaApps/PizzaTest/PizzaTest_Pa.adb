
with Ada.Text_IO; use Ada.Text_IO;
with Ada.Characters.Latin_1; use Ada.Characters.Latin_1;
with Ada.Directories; use Ada.Directories;
with Ada.Strings.Fixed; use Ada.Strings.Fixed;
with Ada.Integer_Text_IO; use Ada.Integer_Text_IO;

with Ops_Pa.OpsObject_Pa.OPSConfig_Pa;
with Ops_Pa.SerializableFactory_Pa.CompFactory_Pa;
with Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.OpsObjectFactory_Pa;
with Ops_Pa.OpsObject_Pa.OPSConfig_Pa; use Ops_Pa.OpsObject_Pa.OPSConfig_Pa;
with Ops_Pa.Transport_Pa.Receiver_Pa.TCPClient_Pa;
with Ops_Pa.Transport_Pa.Sender_Pa.TCPServer_Pa;
with Ops_Pa.Transport_Pa.TCPConnection_Pa;

with Ops_Pa,
     Ops_Pa.Error_Pa,
     Ops_Pa.OpsObject_Pa,
     Ops_Pa.OpsObject_Pa.OPSMessage_Pa,
     Ops_Pa.OpsObject_Pa.Domain_Pa,
     Ops_Pa.OpsObject_Pa.Topic_Pa,
     Ops_Pa.Participant_Pa,
     Ops_Pa.Subscriber_Pa,
     Ops_Pa.PublisherAbs_Pa.Publisher_Pa,
     Ops_Pa.SerializableFactory_Pa,
     Ops_Pa.SerializableFactory_Pa.PizzaProject_PizzaProjectTypeFactory,
     Ops_Pa.OpsObject_Pa.pizza_PizzaData,
     Ops_Pa.OpsObject_Pa.pizza_VessuvioData,
     Ops_Pa.OpsObject_Pa.pizza_special_ExtraAllt,
     Ops_Pa.Subscriber_Pa.pizza_PizzaData,
     Ops_Pa.Subscriber_Pa.pizza_VessuvioData,
     Ops_Pa.Subscriber_Pa.pizza_special_ExtraAllt,
     Ops_Pa.PublisherAbs_Pa.Publisher_Pa.pizza_PizzaData,
     Ops_Pa.PublisherAbs_Pa.Publisher_Pa.pizza_VessuvioData,
     Ops_Pa.PublisherAbs_Pa.Publisher_Pa.pizza_special_ExtraAllt;

use Ops_Pa,
    Ops_Pa.Error_Pa,
    Ops_Pa.OpsObject_Pa,
    Ops_Pa.OpsObject_Pa.OPSMessage_Pa,
    Ops_Pa.OpsObject_Pa.Domain_Pa,
    Ops_Pa.OpsObject_Pa.Topic_Pa,
    Ops_Pa.Participant_Pa,
    Ops_Pa.Subscriber_Pa,
    Ops_Pa.PublisherAbs_Pa.Publisher_Pa,
    Ops_Pa.SerializableFactory_Pa,
    Ops_Pa.SerializableFactory_Pa.PizzaProject_PizzaProjectTypeFactory,
    Ops_Pa.OpsObject_Pa.pizza_PizzaData,
    Ops_Pa.OpsObject_Pa.pizza_VessuvioData,
    Ops_Pa.OpsObject_Pa.pizza_special_ExtraAllt,
    Ops_Pa.Subscriber_Pa.pizza_PizzaData,
    Ops_Pa.Subscriber_Pa.pizza_VessuvioData,
    Ops_Pa.Subscriber_Pa.pizza_special_ExtraAllt,
    Ops_Pa.PublisherAbs_Pa.Publisher_Pa.pizza_PizzaData,
    Ops_Pa.PublisherAbs_Pa.Publisher_Pa.pizza_VessuvioData,
    Ops_Pa.PublisherAbs_Pa.Publisher_Pa.pizza_special_ExtraAllt;

package body PizzaTest_Pa is

  use type Int64;
  use type Ops_Class_At;

  procedure ErrorLog(Sender : in out Ops_Class_At; Item : Error_Class_At; Arg : Ops_Class_At) is
  begin
    Put_Line("Error:: " & Item.GetMessage);
  end;

  package Helper_Interfaces_Pa is

    type IHelper_Interface is limited interface;
    type IHelper_Class_At is access all IHelper_Interface'Class;

    function HasPublisher( Self : IHelper_Interface ) return Boolean is abstract;
    function HasSubscriber( Self : IHelper_Interface ) return Boolean is abstract;
    procedure CreatePublisher( Self : in out IHelper_Interface; part : Participant_Class_At; topicName : String ) is abstract;
    procedure DeletePublisher( Self : in out IHelper_Interface; doLog : Boolean := true ) is abstract;
    procedure StartPublisher( Self : in out IHelper_Interface ) is abstract;
    procedure StopPublisher( Self : in out IHelper_Interface ) is abstract;
    procedure Write( Self : in out IHelper_Interface ) is abstract;
    procedure CreateSubscriber( Self : in out IHelper_Interface; part : Participant_Class_At; topicName : String ) is abstract;
    procedure DeleteSubscriber( Self : in out IHelper_Interface; doLog : Boolean := true ) is abstract;
    procedure StartSubscriber( Self : in out IHelper_Interface ) is abstract;
    procedure StopSubscriber( Self : in out IHelper_Interface ) is abstract;
    procedure SetDeadlineQos( Self : in out IHelper_Interface; timeoutMs : Int64 ) is abstract;
    procedure SetVersionMask( Self : in out IHelper_Interface; verMask : UInt32 ) is abstract;
--    function Data( Self : IHelper_Interface ) return OpsObject_Class_At is abstract;

    generic
      type DataType_T is private;
    package Listener_Interfaces_Pa is
      type Listener_Interface is limited interface;
      type Listener_Interface_At is access all Listener_Interface'Class;

      procedure onData( Self : in out Listener_Interface; sub : Subscriber_Class_At; data : DataType_T ) is abstract;
    end Listener_Interfaces_Pa;
  end Helper_Interfaces_Pa;

  generic
    type DataType is new OpsObject_Class with private;
    type DataType_At is access all DataType'Class;
    type DataType_Publisher is new Publisher_Class with private;
    type DataType_Publisher_At is access all DataType_Publisher'Class;
    type DataType_Subscriber is new Subscriber_Class with private;
    type DataType_Subscriber_At is access all DataType_Subscriber'Class;
    with function CreateObject return DataType_At;
  package Helper_Pa is
    package z is new Helper_Interfaces_Pa.Listener_Interfaces_Pa(DataType_At);

    type Helper_Class is new Ops_Class and
      Helper_Interfaces_Pa.IHelper_Interface and
      Ops_Pa.Subscriber_Pa.MessageNotifier_Pa.Listener_Interface and
      Ops_Pa.Subscriber_Pa.Deadline_Pa.DeadlineListener_Interface and
      Ops_Pa.Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface with
        private;
    type Helper_Class_At is access all Helper_Class'Class;

    function Create(client : z.Listener_Interface_At) return Helper_Class_At;
    function HasPublisher( Self : Helper_Class ) return Boolean;
    function HasSubscriber( Self : Helper_Class ) return Boolean;
    procedure CreatePublisher( Self : in out Helper_Class; part : Participant_Class_At; topicName : String );
    procedure DeletePublisher( Self : in out Helper_Class; doLog : Boolean := true );
    procedure StartPublisher( Self : in out Helper_Class );
    procedure StopPublisher( Self : in out Helper_Class );
    procedure Write( Self : in out Helper_Class );
    procedure CreateSubscriber( Self : in out Helper_Class; part : Participant_Class_At; topicName : String );
    procedure DeleteSubscriber( Self : in out Helper_Class; doLog : Boolean := true );
    procedure StartSubscriber( Self : in out Helper_Class );
    procedure StopSubscriber( Self : in out Helper_Class );
    procedure SetDeadlineQos( Self : in out Helper_Class; timeoutMs : Int64 );
    procedure SetVersionMask( Self : in out Helper_Class; verMask : UInt32 );
    function Data( Self : Helper_Class ) return DataType_At;

  private
    type Helper_Class is new Ops_Class and
      Helper_Interfaces_Pa.IHelper_Interface and
      Ops_Pa.Subscriber_Pa.MessageNotifier_Pa.Listener_Interface and
      Ops_Pa.Subscriber_Pa.Deadline_Pa.DeadlineListener_Interface and
      Ops_Pa.Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface with
       record
         SelfAt : Helper_Class_At := null;
         data : DataType_At := null;
         client : z.Listener_Interface_At := null;
         pub : Publisher_Class_At := null; -- DataType_Publisher_At := null;
         sub : Subscriber_Class_At := null; -- DataType_Subscriber_At := null;
         expectedPubId : Int64 := -1;
       end record;
    procedure OnNotify( Self : in out Helper_Class;
                        Sender : in Ops_Class_At;
                        Item : in OPSMessage_Class_At );
    procedure OnDeadlineMissed( Self : in out Helper_Class; Sender : in Ops_Class_At );
    procedure OnNotify( Self : in out Helper_Class; Sender : in Ops_Class_At; Item : in Ops_Pa.Transport_Pa.ConnectStatus_T );

    procedure InitInstance( Self : in out Helper_Class; SelfAt : Helper_Class_At; client : z.Listener_Interface_At );
    procedure Finalize( Self : in out Helper_Class );
  end;

  package body Helper_Pa is

    function Create(client : z.Listener_Interface_At) return Helper_Class_At is
      Self : Helper_Class_At := null;
    begin
      Self := new Helper_Class;
      InitInstance( Self.all, Self, client );
      return Self;
    exception
      when others =>
        Free(Self);
        raise;
    end;

    procedure InitInstance( Self : in out Helper_Class; SelfAt : Helper_Class_At; client : z.Listener_Interface_At ) is
    begin
      Self.SelfAt := SelfAt;
      Self.client := client;
      Self.Data := CreateObject;
    end;

    procedure Finalize( Self : in out Helper_Class ) is
    begin
      DeletePublisher(Self, false);
      DeleteSubscriber(Self, false);
      Free(Self.Data);
      Self.Data := null;
    end;

    function Data( Self : Helper_Class ) return DataType_At is
    begin
      return DataType_At(Self.Data);
    end;

    function HasPublisher( Self : Helper_Class ) return Boolean is
    begin
      return Self.pub /= null;
    end;

    function HasSubscriber( Self : Helper_Class ) return Boolean is
    begin
      return Self.sub /= null;
    end;

    procedure CreatePublisher( Self : in out Helper_Class; part : Participant_Class_At; topicName : String ) is
    begin
      if Self.pub /= null then
        Put_Line("Publisher already exist for topic " & Self.pub.Topic.Name);
      else
        declare
          --Create topic, might throw ops::NoSuchTopicException
          topic : Topic_Class_At := part.getTopic(topicName);
        begin
          Put_Line("Created topic " & topic.Name &
                     " [" & topic.Transport &
                     "::" & topic.DomainAddress &
                     "::" & Int32'Image(topic.Port) &
                     "] ");

          Put_Line("  InSocketBufferSize: " & Int64'Image(topic.InSocketBufferSize));
          Put_Line("  OutSocketBufferSize: " & Int64'Image(topic.OutSocketBufferSize));
          Put_Line("  SampleMaxSize: " & Int32'Image(topic.SampleMaxSize));

          --Create a publisher on that topic
          Self.pub := Create(topic);

          --          myStream << " Win(" << _getpid() << ")" << std::ends;
          Self.pub.SetName("Ada Testprog ( TBD )");
          Self.pub.addListener( Ops_Pa.Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface_At(Self.SelfAt) );
        end;
      end if;
    exception
      when others =>
        Put_LIne("Requested topic '" & topicName & "' does not exist!!");
    end;

    procedure DeletePublisher( Self : in out Helper_Class; doLog : Boolean := true ) is
    begin
      if Self.pub /= null then
        Put_Line("Deleting publisher for topic " & Self.pub.Topic.Name);
        --pub->stop();
        Free(Self.pub);
        Self.pub := NULL;
      else
        if doLog then
          Put_Line("Publisher must be created first!!");
        end if;
      end if;
    end;

    procedure StartPublisher( Self : in out Helper_Class ) is
    begin
      if Self.pub /= null then
        Self.pub.start;
      else
        Put_Line("Publisher must be created first!!");
      end if;
    end;

    procedure StopPublisher( Self : in out Helper_Class ) is
    begin
      if Self.pub /= null then
        Self.pub.stop;
      else
        Put_Line("Publisher must be created first!!");
      end if;
    end;

    procedure Write( Self : in out Helper_Class ) is
    begin
      if Self.pub /= null then
        begin
          Self.pub.writeOPSObject(OpsObject_Class_At(Self.data)); -- Write using pointer
          --Self.pub.write(Self.data);			-- Write using ref
        exception
          when others =>
            Put_Line("Write(): Got exception ");
        end;
      else
        Put_Line("Publisher must be created first!!");
      end if;
    end;

    procedure CreateSubscriber( Self : in out Helper_Class; part : Participant_Class_At; topicName : String ) is
    begin
      if Self.sub /= null then
        Put_Line("Subscriber already exist for topic " & Self.sub.Topic.Name);
      else
        declare
          --Create topic, might throw ops::NoSuchTopicException
          topic : Topic_Class_At := part.getTopic(topicName);
        begin
          Put_Line("Created topic " & topic.Name &
					" [" & topic.Transport &
					"::" & topic.DomainAddress &
					"::" & Int32'Image(topic.Port) &
					"] ");

          Put_Line("  InSocketBufferSize: " & Int64'Image(topic.InSocketBufferSize));
          Put_Line("  OutSocketBufferSize: " & Int64'Image(topic.OutSocketBufferSize));
          Put_Line("  SampleMaxSize: " & Int32'Image(topic.SampleMaxSize));

          --Create a subscriber on that topic.
          Self.sub := Create(topic);
          Self.sub.addListener( MessageNotifier_Pa.Listener_Interface_At(Self.SelfAt) );
          Self.sub.addDeadlineListener( Deadline_Pa.DeadlineListener_Interface_At(Self.SelfAt) );
          Self.sub.addListener( Ops_Pa.Transport_Pa.ConnectStatusNotifier_Pa.Listener_Interface_At(Self.SelfAt) );

--          -- Add a publication ID Checker
--          sub->pubIdChecker = new ops::PublicationIdChecker;
--          sub->pubIdChecker->addListener(this);

          Self.sub.start;
        end;
      end if;
    exception
      when others =>
        Put_Line("Exception: Requested topic '" & topicName & "' does not exist!!");
    end;

    procedure DeleteSubscriber( Self : in out Helper_Class; doLog : Boolean := true ) is
    begin
      if Self.sub /= null then
        Put_Line("Deleting Subscriber for topic " & Self.sub.Topic.Name);
        Self.sub.stop;
        Free(Self.sub);
        Self.sub := null;
      else
        if doLog then
          Put_Line("Subscriber must be created first!!");
        end if;
      end if;
    end;

    procedure StartSubscriber( Self : in out Helper_Class ) is
    begin
      if Self.sub /= null then
        Put_Line("Starting subscriber for topic " & Self.sub.Topic.Name);
        Self.sub.start;
      else
        Put_Line("Subscriber must be created first!!");
      end if;
    end;

    procedure StopSubscriber( Self : in out Helper_Class ) is
    begin
      if Self.sub /= null then
        Put_Line("Stoping subscriber for topic " & Self.sub.Topic.Name);
        Self.sub.stop;
      else
        Put_Line("Subscriber must be created first!!");
      end if;
    end;

    procedure SetDeadlineQos( Self : in out Helper_Class; timeoutMs : Int64 ) is
    begin
      if Self.sub /= null then
        Put_Line("Setting deadlineQos to " & Int64'Image(timeoutMs) & " [ms] for topic " & self.sub.Topic.Name);
        Self.sub.setDeadlineQoS(TimeMs_T(timeoutMs));
      else
        Put_Line("Subscriber must be created first!!");
      end if;
    end;

    procedure SetVersionMask( Self : in out Helper_Class; verMask : UInt32 ) is
    begin
      Self.data.SetIdlVersionMask( verMask );
    end;

    procedure OnNotify( Self : in out Helper_Class;
                        Sender : in Ops_Class_At;
                        Item : in OPSMessage_Class_At ) is
    begin
      if Sender = Ops_Class_At(Self.sub) then
        -- Check if we have lost any messages. We use the publicationID and that works as long as
        -- it is the same publisher sending us messages.
        if Self.expectedPubId >= 0 then
          if Self.expectedPubId /= Item.PublicationID then
            Put_Line(">>>>> Lost message for topic " & Self.sub.Topic.Name &
                       ". Exp.pubid: " & Int64'Image(Self.expectedPubId) & " got: " & Int64'Image(Item.PublicationID));
          end if;
        end if;
        Self.expectedPubId := Item.PublicationID + 1;
--      procedure onData( Self : in out Listener_Interface; sub : Subscriber_Class_At; data : DataType_T ) is abstract;
        Self.client.onData(Self.sub, DataType_At(Item.Data));
      end if;
    end;

    procedure OnNotify( Self : in out Helper_Class;
                        Sender : in Ops_Class_At;
                        Item : in Ops_Pa.Transport_Pa.ConnectStatus_T ) is
    begin
      if Sender.all in Ops_Pa.Subscriber_Pa.Subscriber_Class'Class then
        declare
          sub : Subscriber_Class_At := Subscriber_Class_At(Sender);
        begin
          Ada.Text_IO.Put("[Topic: " & sub.Topic.Name & "] Subscriber ");
          if Item.Connected then
            Ada.Text_IO.Put_Line("OnConnect, IP: " & Item.Address & "::" & Integer'Image(Item.Port));
          else
            Ada.Text_IO.Put_Line("OnDisconnect, IP: " & Item.Address & "::" & Integer'Image(Item.Port));
          end if;
        end;
      elsif Sender.all in Ops_Pa.PublisherAbs_Pa.Publisher_Pa.Publisher_Class'Class then
        declare
          pub : Publisher_Class_At := Publisher_Class_At(Sender);
        begin
          Ada.Text_IO.Put("[Topic: " & pub.Topic.Name & "] Publisher ");
          if Item.Connected then
            Ada.Text_IO.Put("OnConnect, IP: " & Item.Address & "::" & Integer'Image(Item.Port));
          else
            Ada.Text_IO.Put("OnDisconnect, IP: " & Item.Address & "::" & Integer'Image(Item.Port));
          end if;
          Ada.Text_IO.Put_Line(", TotalNo: " & Integer'Image(Item.TotalNo));
        end;
      else
        Ada.Text_IO.Put_Line(" >>>>>>> OnNotify() UNKNOWN Sender ");
      end if;
    end;

    procedure OnDeadlineMissed( Self : in out Helper_Class; Sender : in Ops_Class_At ) is
    begin
      Put_Line(">>> Deadline timeout for topic " & Self.sub.Topic.Name);
    end;

  end Helper_Pa;

  function Create_ExtraAllt return ExtraAllt_Class_At is
  begin
    return Ops_Pa.OpsObject_Pa.pizza_special_ExtraAllt.Create;
  end;

  function Create_PizzaData return PizzaData_Class_At is
  begin
    return Ops_Pa.OpsObject_Pa.pizza_PizzaData.Create;
  end;

  function Create_VessuvioData return VessuvioData_Class_At is
  begin
    return Ops_Pa.OpsObject_Pa.pizza_VessuvioData.Create;
  end;

  package ExtraAlltDataHelper is new Helper_Pa(ExtraAllt_Class,
                                               ExtraAllt_Class_At,
                                               ExtraAlltPublisher_Class,
                                               ExtraAlltPublisher_Class_At,
                                               ExtraAlltSubscriber_Class,
                                               ExtraAlltSubscriber_Class_At,
                                               Create_ExtraAllt
                                              );
  package PizzaDataHelper is new Helper_Pa(PizzaData_Class,
                                           PizzaData_Class_At,
                                           PizzaDataPublisher_Class,
                                           PizzaDataPublisher_Class_At,
                                           PizzaDataSubscriber_Class,
                                           PizzaDataSubscriber_Class_At,
                                           Create_PizzaData
                                          );
  package VessuvioDataHelper is new Helper_Pa(VessuvioData_Class,
                                              VessuvioData_Class_At,
                                              VessuvioDataPublisher_Class,
                                              VessuvioDataPublisher_Class_At,
                                              VessuvioDataSubscriber_Class,
                                              VessuvioDataSubscriber_Class_At,
                                              Create_VessuvioData
                                             );

  type ItemInfo_T is
     record
       Domain : String_At := null;
       TopicName : String_At := null;
       TypeName : String_At := null;
       Selected : Boolean := false;
       Helper : Helper_Interfaces_Pa.IHelper_Class_At := null;
       Part : Participant_Class_At := null;
     end record;

  type ItemInfoList_T is array(Integer range 0..17) of ItemInfo_T;

  ItemInfoList : ItemInfoList_T;
  ItemInfoNum : Integer := 0;

  procedure SetUp(dom, Topic, Typ : String) is
  begin
    ItemInfoList(ItemInfoNum).Domain := Copy(dom);
    ItemInfoList(ItemInfoNum).TopicName := Copy(Topic);
    ItemInfoList(ItemInfoNum).TypeName := Copy(Typ);
    ItemInfoNum := ItemInfoNum + 1;
  end;

  type MyListener_Class is new Ops_Class and
    PizzaDataHelper.z.Listener_Interface and
    VessuvioDataHelper.z.Listener_Interface and
    ExtraAlltDataHelper.z.Listener_Interface with null record;
  type MyListener_Class_At is access all MyListener_Class'Class;

  procedure onData( Self : in out MyListener_Class; sub : Subscriber_Class_At; data : PizzaData_Class_At );
  procedure onData( Self : in out MyListener_Class; sub : Subscriber_Class_At; data : VessuvioData_Class_At );
  procedure onData( Self : in out MyListener_Class; sub : Subscriber_Class_At; data : ExtraAllt_Class_At );
  procedure Finalize( Self : in out MyListener_Class );

  function Create return MyListener_Class_At is
  begin
    return new MyListener_Class;
  end;

  procedure Finalize( Self : in out MyListener_Class ) is
  begin
    null;
  end;

  beQuite : Boolean := False;

  function X(Str : String_At) return String is
  begin
    if Str = null then
      return "";
    else
      return Str.all;
    end if;
  end;

  function XL(Str : String_At) return String is
  begin
    if Str = null then
      return "0";
    else
      return Integer'Image(Str'Length);
    end if;
  end;

  function XL(Str : String_Arr_At) return String is
  begin
    if Str = null then
      return "0";
    else
      return Integer'Image(Str'Length);
    end if;
  end;

  function XL( arr : Byte_Arr_At ) return String is
  begin
    if arr = null then
      return "0";
    else
      return Integer'Image(arr'Length);
    end if;
  end;

  procedure onData( Self : in out MyListener_Class; sub : Subscriber_Class_At; data : PizzaData_Class_At ) is
  begin
    -- test for sending derived objects on same topic
--      if (dynamic_cast<pizza::VessuvioData*>(data) != NULL)  then
--        onData(sub, dynamic_cast<pizza::VessuvioData*>(data));
--        return;
--      end if;

    declare
      addr : String := sub.getMessage.getSourceIP;
      port : Integer := sub.getMessage.getSourcePort;
    begin
      if not beQuite then
        Put_Line(
                 "[Topic: " & sub.Topic.Name &
                   "] (From " & addr & ":" & Integer'Image(port) &
                   ") PizzaData(v" & Byte'Image(data.PizzaData_version) &
                   "):: Cheese: " & X(data.cheese) &
                   ", Tomato sauce: " & X(data.tomatoSauce) &
                   ", spareBytes: " & XL(data.SpareBytes)
                );
      end if;
    end;
  end;

  procedure onData( Self : in out MyListener_Class; sub : Subscriber_Class_At; data : VessuvioData_Class_At ) is
  begin
    -- test for sending derived objects on same topic
--      if (dynamic_cast<pizza::special::ExtraAllt*>(data) != NULL) then
--        onData(sub, dynamic_cast<pizza::special::ExtraAllt*>(data));
--        return;
--      end if;

    declare
      addr : String := sub.getMessage.getSourceIP;
      port : Integer := sub.getMessage.getSourcePort;
    begin
      if not beQuite then
        Put_Line(
                 "[Topic: " & sub.Topic.Name &
                 "] (From " & addr & ":" & Integer'Image(port) &
                 ") VessuvioData:: Cheese: " & X(data.cheese) &
                 ", Tomato sauce: " & X(data.tomatoSauce) &
                 ", Ham length: " & XL(data.ham)
                 );
      end if;
    end;
  end;

  procedure onData( Self : in out MyListener_Class; sub : Subscriber_Class_At; data : ExtraAllt_Class_At ) is

    function ss return String is
    begin
      if data.shs = null then
        return "";
      elsif data.shs'Length > 1 then
        return ", shs[1]: " & Int16'Image(data.shs(1));
      else
        return "";
      end if;
    end;

  begin
    declare
      addr : String := sub.getMessage.getSourceIP;
      port : Integer := sub.getMessage.getSourcePort;
    begin
      if not beQuite then

        Put_Line(
                 "[Topic: " & sub.Topic.Name &
                 "] (From " & addr & ":" & Integer'Image(port) &
                 ") ExtraAllt:: Cheese: " & X(data.cheese) &
                 ss &
                 ", Tomato sauce: " & X(data.tomatoSauce) &
                 ", Num strings: " & XL(data.strings)
                 );
      end if;
    end;
  end;

  MyListener : MyListener_Class_At := Create;


  procedure printDomainInfo(part : Participant_Class_At) is
    dom : Domain_Class_At := part.getDomain;
  begin
    Put_Line("");
    Put_Line("  DomainID: " & dom.DomainID);
    Put_Line("  DomainAddress: " & dom.DomainAddress);
    Put_Line("  InSocketBufferSize: " & Int32'Image(dom.InSocketBufferSize));
    Put_Line("  OutSocketBufferSize: " & Int32'Image(dom.OutSocketBufferSize));
    Put_Line("  MetaDataMcPort: " & Int32'Image(dom.MetaDataMcPort));
    Put_Line("  LocalInterface: " & dom.LocalInterface);
    Put_Line("  TimeToLive: " &	Int32'Image(dom.TimeToLive));

    declare
      top : Topic_Class_At := part.createParticipantInfoTopic;
    begin
      Put_Line("");
      Put_Line("  TopicName: " & top.Name);
      Put_Line("  TypeID: " & top.TypeID);
      Put_Line("  ParticipantID: " & top.ParticipantID);
      Put_Line("  DomainID: " & top.DomainID);
      Put_Line("  DomainAddress: " & top.DomainAddress);
      Put_Line("  LocalInterface: " & top.LocalInterface);
      Put_Line("  Transport: " & top.Transport);
      Put_Line("  TimeToLive: " & Int32'Image(top.TimeToLive));
      Put_Line("  SampleMaxSize: " & Int32'Image(top.SampleMaxSize));
      Put_Line("  Port: " & Int32'Image(top.Port));
      Put_Line("  InSocketBufferSize: " & Int64'Image(top.InSocketBufferSize));
      Put_Line("  OutSocketBufferSize: " & Int64'Image(top.OutSocketBufferSize));
      Free(top);
    end;
  end;

  NumVessuvioBytes : Integer := 0;
  FillerStr : String_At := Copy("");
  sendPeriod : Int64 := 1000;
  Counter : Int64 := 0;
  PD_Version : Integer := Integer(Ops_Pa.OpsObject_Pa.pizza_PizzaData.PizzaData_idlVersion);

  procedure WriteToAllSelected is
  begin
    for i in ItemInfoList'Range loop
      if ItemInfoList(i).selected then

        if ItemInfoList(i).TypeName.all = Ops_Pa.OpsObject_Pa.pizza_PizzaData.TypeName_C then
          declare
            data : PizzaData_Class_At :=
              PizzaDataHelper.Data(PizzaDataHelper.Helper_Class_At(ItemInfoList(i).Helper).all);
          begin
            if PD_Version < 0 then
              data.all.SetIdlVersionMask(0);
            else
              data.all.SetIdlVersionMask(1);
              data.all.PizzaData_version := Byte(PD_Version);
            end if;
            Replace(data.cheese, "Pizza from Ada: " & Int64'Image(Counter));
            Replace(data.tomatoSauce, "Tomato");
          end;

        elsif ItemInfoList(i).TypeName.all = Ops_Pa.OpsObject_Pa.pizza_VessuvioData.TypeName_C then
          declare
            data : VessuvioData_Class_At :=
              VessuvioDataHelper.Data(VessuvioDataHelper.Helper_Class_At(ItemInfoList(i).Helper).all);
          begin
            Replace(data.cheese, "Vessuvio from Ada: " & Int64'Image(Counter));
            Replace(data.ham, FillerStr);
          end;

--  	elsif ItemInfoList(i).TypeName.all = Ops_Pa.OpsObject_Pa.pizza_special_ExtraAllt.TypeName_C then
--            TExtraAlltHelper* hlp = (TExtraAlltHelper*)info->helper;
--            hlp->data.cheese = "ExtraAllt from C++: " + CounterStr;
--            if (hlp->data.strings.size() == 0) {
--            for (int k = 0; k < 1000; k++) hlp->data.strings.push_back("hej");
--            }
--            hlp->data.sh = -7;
--            if (hlp->data.shs.size() == 0) {
--              hlp->data.shs.push_back(17);
--            hlp->data.shs.push_back(42);
--            hlp->data.shs.push_back(-63);
--            }
        end if;

        ItemInfoList(i).helper.Write;

        Counter := Counter + 1;
      end if;
    end loop;
  end;

  procedure Menu is
    PubStr : array(Boolean) of Character := (' ', 'P');
    SubStr : array(Boolean) of Character := (' ', 'S');
    SelStr : array(Boolean) of Character := (' ', '*');
  begin
    New_Line;
    for i in ItemInfoList'Range loop
      Put_Line(HT & Integer'Image(i) & " " &
                 PubStr(ItemInfoList(i).helper.HasPublisher) &
                 SubStr(ItemInfoList(i).helper.HasSubscriber) &
                 SelStr(ItemInfoList(i).selected) &
                 " " &
                 ItemInfoList(i).Domain.all & "::" & ItemInfoList(i).TopicName.all);
    end loop;

    New_Line;
    Put_Line(HT & " PC    Create Publishers");
    Put_Line(HT & " PD    Delete Publishers");
    Put_Line(HT & " PS    Start Publishers");
    Put_Line(HT & " PT    Stop Publishers");
    Put_Line(HT & " SC    Create Subscriber");
    Put_Line(HT & " SD    Delete Subscriber");
    Put_Line(HT & " SS    Start Subscriber");
    Put_Line(HT & " ST    Stop Subscriber");
    Put_Line(HT & " L num Set num Vessuvio Bytes [" & Integer'Image(NumVessuvioBytes) & "]");
    Put_Line(HT & " T ms  Set deadline timeout [ms]");
    Put_Line(HT & " V ms  Set send period [ms] [" & Int64'Image(sendPeriod) & "]");
    Put_Line(HT & " A     Start/Stop periodical Write with set period");
    Put_Line(HT & " M ver Set Pizzadata version [" & Integer'Image(PD_Version) & "]");
    Put_Line(HT & " W     Write data");
    Put_Line(HT & " Q     Quite (minimize program output)");
    Put_Line(HT & " X     Exit program");
  end;

  procedure MyTraceProc( NameStr : String; ValueStr : String ) is
  begin
    Ada.Text_IO.Put_Line( ">>>>>> [ " & NameStr & " ]  " & ValueStr);
  end;

  procedure setup_alt_config(cfg_rel_ops4 : String) is
  begin
    -- Check if we have an ops_config.xml in CWD
    if Exists("ops_config.xml") then
      Put_Line("Using config file in CWD");

    else
      declare
        cwd : String := Current_Directory;
        pos : Natural := Index(cwd, "Examples", 1);
      begin
        if pos > 1 then
          if RepositoryInstance.add(Head(cwd, pos - 1) & cfg_rel_ops4) then
            Put_Line("Using config file: " & Head(cwd, pos - 1) & cfg_rel_ops4);
          end if;
        end if;
      end;
    end if;
  end;

  procedure PizzaTest is
    participant : Participant_Class_At := null;
    otherParticipant : Participant_Class_At := null;
    dummy : Boolean;
  begin
    -- Enable TCP Client trace
--        Ops_Pa.InstallTrace(MyTraceProc'Access);
--        Ops_Pa.Transport_Pa.Receiver_Pa.TCPClient_Pa.TraceEnabled := True;
--        Ops_Pa.Transport_Pa.TCPConnection_Pa.TraceEnabled := True;
--        Ops_Pa.Transport_Pa.Sender_Pa.TCPServer_Pa.TraceEnabled := True;

    -- Setup the OPS static error service (common for all participants, reports errors during participant creation)
    StaticErrorService.addListener(ErrorLog'Access, null);

    -- Add all Domain's from given file(s)
    setup_alt_config("Examples/OPSIdls/PizzaProject/ops_config.xml");

    -- Setup Topic list
    -- Setup the InfoItem list (TODO take from ops_config.xml)
    Setup("PizzaDomain", "PizzaTopic", "pizza.PizzaData");
    Setup("PizzaDomain", "VessuvioTopic", "pizza.VessuvioData");
    Setup("PizzaDomain", "PizzaTopic2", "pizza.PizzaData");
    Setup("PizzaDomain", "VessuvioTopic2", "pizza.VessuvioData");
    Setup("PizzaDomain", "TcpPizzaTopic", "pizza.PizzaData");
    Setup("PizzaDomain", "TcpVessuvioTopic", "pizza.VessuvioData");
    Setup("PizzaDomain", "TcpPizzaTopic2", "pizza.PizzaData");
    Setup("PizzaDomain", "TcpVessuvioTopic2", "pizza.VessuvioData");
    Setup("PizzaDomain", "UdpPizzaTopic", "pizza.PizzaData");
    Setup("PizzaDomain", "UdpVessuvioTopic", "pizza.VessuvioData");
    Setup("PizzaDomain", "UdpPizzaTopic2", "pizza.PizzaData");
    Setup("PizzaDomain", "UdpVessuvioTopic2", "pizza.VessuvioData");
    Setup("OtherPizzaDomain", "OtherPizzaTopic", "pizza.PizzaData");
    Setup("OtherPizzaDomain", "OtherVessuvioTopic", "pizza.VessuvioData");
    Setup("PizzaDomain", "ExtraAlltTopic", "pizza.special.ExtraAllt");
    Setup("PizzaDomain", "PizzaTopic", "pizza.PizzaData");
    Setup("PizzaDomain", "TcpPizzaTopic", "pizza.PizzaData");
    Setup("PizzaDomain", "TcpPizzaTopic", "pizza.PizzaData");

    -- Create participants
    participant := getInstance("PizzaDomain");
    if participant = null then
      Put_Line("Failed to create Participant. Missing ops_config.xml ??");
      return;
    end if;
    -- Add our generated factory so OPS can create our data objects
    declare
      fact : PizzaProjectTypeFactory_Class_At := Create;
    begin
      participant.addTypeSupport( SerializableFactory_Class_At(fact) );
    end;
    printDomainInfo(participant);

    otherParticipant := getInstance("OtherPizzaDomain");
    if otherParticipant = null then
      Put_Line("Failed to create otherParticipant. Missing ops_config.xml ??");
      return;
    end if;
    -- Add our generated factory so OPS can create our data objects
    declare
      fact : PizzaProjectTypeFactory_Class_At := Create;
    begin
      otherParticipant.addTypeSupport( SerializableFactory_Class_At(fact) );
    end;
    printDomainInfo(otherParticipant);

    -- Add error writers to catch internal ops errors
    participant.getErrorService.addListener(ErrorLog'Access, null);
    otherParticipant.getErrorService.addListener(ErrorLog'Access, null);

    -- finish setup
    for idx in ItemInfoList'Range loop
      -- Create helper
      if ItemInfoList(idx).TypeName.all = Ops_Pa.OpsObject_Pa.pizza_special_ExtraAllt.TypeName_C then
        ItemInfoList(idx).helper :=
          Helper_Interfaces_Pa.IHelper_Class_At(ExtraAlltDataHelper.Create(ExtraAlltDataHelper.z.Listener_Interface_At(MyListener)));
      end if;
      if ItemInfoList(idx).TypeName.all = Ops_Pa.OpsObject_Pa.pizza_PizzaData.TypeName_C then
        ItemInfoList(idx).helper :=
          Helper_Interfaces_Pa.IHelper_Class_At(PizzaDataHelper.Create(PizzaDataHelper.z.Listener_Interface_At(MyListener)));
      end if;
      if ItemInfoList(idx).TypeName.all = Ops_Pa.OpsObject_Pa.pizza_VessuvioData.TypeName_C then
        ItemInfoList(idx).helper :=
          Helper_Interfaces_Pa.IHelper_Class_At(VessuvioDataHelper.Create(VessuvioDataHelper.z.Listener_Interface_At(MyListener)));
      end if;

      -- Set participant
      if ItemInfoList(idx).Domain.all = "PizzaDomain" then
        ItemInfoList(idx).part := participant;
      end if;
      if ItemInfoList(idx).Domain.all = "OtherPizzaDomain" then
        ItemInfoList(idx).part := otherParticipant;
       end if;
    end loop;

    ItemInfoList(0).selected := true;

    Menu;

    declare
      doPeriodicSend : Boolean := false;
      --      nextSendTime : Int64 := (__int64)getNow() + sendPeriod;
    begin
      while true loop
        New_Line;
        Put(" (? = menu) > ");
        declare
          line : String := Ada.Strings.Fixed.Trim(Get_line, Ada.Strings.Both);
          SelNo : Integer := 0;
          Last : Positive := 1;
        begin
          -- Check if a number is entered and if so update selection
          if line'Length > 0 then
            if line(1) >= '0' and line(1) <= '9' then
              Get(line, SelNo, Last);
              if SelNo >= ItemInfoList'First and SelNo <= ItemInfoList'Last then
                ItemInfoList(SelNo).Selected := not ItemInfoList(SelNo).Selected;
              end if;

            elsif Ada.Strings.Fixed.Index(line, "pc") = 1 then
              for i in ItemInfoList'Range loop
                if ItemInfoList(i).selected then
                  ItemInfoList(i).helper.CreatePublisher(ItemInfoList(i).part, ItemInfoList(i).TopicName.all);
                end if;
              end loop;

            elsif Ada.Strings.Fixed.Index(line, "pd") = 1 then
              for i in ItemInfoList'Range loop
                if ItemInfoList(i).selected then
                  ItemInfoList(i).helper.DeletePublisher;
                end if;
              end loop;

            elsif Ada.Strings.Fixed.Index(line, "ps") = 1 then
              for i in ItemInfoList'Range loop
                if ItemInfoList(i).selected then
                  ItemInfoList(i).helper.StartPublisher;
                end if;
              end loop;

            elsif Ada.Strings.Fixed.Index(line, "pt") = 1 then
              for i in ItemInfoList'Range loop
                if ItemInfoList(i).selected then
                  ItemInfoList(i).helper.StopPublisher;
                end if;
              end loop;

            elsif Ada.Strings.Fixed.Index(line, "sc") = 1 then
              for i in ItemInfoList'Range loop
                if ItemInfoList(i).selected then
                  ItemInfoList(i).helper.CreateSubscriber(ItemInfoList(i).part, ItemInfoList(i).TopicName.all);
                end if;
              end loop;

            elsif Ada.Strings.Fixed.Index(line, "sd") = 1 then
              for i in ItemInfoList'Range loop
                if ItemInfoList(i).selected then
                  ItemInfoList(i).helper.DeleteSubscriber;
                end if;
              end loop;

            elsif Ada.Strings.Fixed.Index(line, "ss") = 1 then
              for i in ItemInfoList'Range loop
                if ItemInfoList(i).selected then
                  ItemInfoList(i).helper.StartSubscriber;
                end if;
              end loop;

            elsif Ada.Strings.Fixed.Index(line, "st") = 1 then
              for i in ItemInfoList'Range loop
                if ItemInfoList(i).selected then
                  ItemInfoList(i).helper.StopSubscriber;
                end if;
              end loop;

            elsif Ada.Strings.Fixed.Index(line, "l") = 1 then
              line(1) := ' ';
              declare
                ll : String := Ada.Strings.Fixed.Trim(line, Ada.Strings.Both);
              begin
                -- get length
                if ll(1) >= '0' and ll(1) <= '9' then
                  Get(ll, NumVessuvioBytes, Last);
                  Dispose(FillerStr);
                  if NumVessuvioBytes > 0 then
                    FillerStr := new String(1..NumVessuvioBytes);
                  else
                    FillerStr := new String'("");
                  end if;
                end if;
              end;

            elsif Ada.Strings.Fixed.Index(line, "t") = 1 then
              line(1) := ' ';
              declare
                ll : String := Ada.Strings.Fixed.Trim(line, Ada.Strings.Both);
                ms : Integer := 0;
              begin
                -- get ms
                if ll(1) >= '0' and ll(1) <= '9' then
                  Get(ll, ms, Last);
                  for i in ItemInfoList'Range loop
                    if ItemInfoList(i).selected then
                      ItemInfoList(i).helper.SetDeadlineQos(Int64(ms));
                    end if;
                  end loop;
                end if;
              end;

            elsif Ada.Strings.Fixed.Index(line, "m") = 1 then
              line(1) := ' ';
              declare
                ll : String := Ada.Strings.Fixed.Trim(line, Ada.Strings.Both);
              begin
                if (ll(1) >= '0' and ll(1) <= '9') or (ll(1) = '-' or ll(1) = '+') then
                  Get(ll, PD_Version, Last);
                end if;
              end;
              Menu;

            elsif Ada.Strings.Fixed.Index(line, "w") = 1 then
              WriteToAllSelected;

            elsif Ada.Strings.Fixed.Index(line, "q") = 1 then
              beQuite := not beQuite;

            elsif Ada.Strings.Fixed.Index(line, "x") = 1 then
              exit;

            elsif Ada.Strings.Fixed.Index(line, "?") = 1 then
              Menu;
            else
              Put_Line("Unknown command");
            end if;
          end if;
        end;
      end loop;
    end;

    Put_Line("Terminating...");

    for idx in ItemInfoList'Range loop
      -- Delete helper
      if ItemInfoList(idx).TypeName.all = Ops_Pa.OpsObject_Pa.pizza_special_ExtraAllt.TypeName_C then
        ExtraAlltDataHelper.Free(ExtraAlltDataHelper.Helper_Class_At(ItemInfoList(idx).helper));
      end if;
      if ItemInfoList(idx).TypeName.all = Ops_Pa.OpsObject_Pa.pizza_PizzaData.TypeName_C then
        PizzaDataHelper.Free(PizzaDataHelper.Helper_Class_At(ItemInfoList(idx).helper));
      end if;
      if ItemInfoList(idx).TypeName.all = Ops_Pa.OpsObject_Pa.pizza_VessuvioData.TypeName_C then
        VessuvioDataHelper.Free(VessuvioDataHelper.Helper_Class_At(ItemInfoList(idx).helper));
      end if;
      ItemInfoList(idx).helper := null;
      ItemInfoList(idx).part := null;

      Dispose(ItemInfoList(idx).Domain);
      Dispose(ItemInfoList(idx).TopicName);
      Dispose(ItemInfoList(idx).TypeName);
    end loop;

    releaseInstance(otherParticipant);
    releaseInstance(participant);

    Free(MyListener);
    Dispose(FillerStr);

    Ops_Pa.Error_Pa.Debug_TotalClear;
    Ops_Pa.OpsObject_Pa.OPSConfig_Pa.Debug_TotalClear;
    Ops_Pa.SerializableFactory_Pa.CompFactory_Pa.OpsObjectFactory_Pa.Debug_TotalClear;

    Put_Line("Finished");
  exception
    when others =>
      Put_Line("Exception terminated procedure PizzaTest()");

  end;

end PizzaTest_Pa;

