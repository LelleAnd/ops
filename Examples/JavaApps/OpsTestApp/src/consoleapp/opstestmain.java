/*
 * opstestmain.java
 */

package opstestmain;

import java.util.Vector;

import opstestapp.IOpsHelperListener;
import opstestapp.COpsHelper;

import ops.Participant;
import ops.OPSConfig;
import ops.Domain;
import ops.Topic;
import ops.KeyFilterQoSPolicy;

import pizza.PizzaData;
import pizza.PizzaDataSubscriber;
import pizza.PizzaDataPublisher;
import pizza.VessuvioData;
import pizza.VessuvioDataSubscriber;
import pizza.VessuvioDataPublisher;
import pizza.special.ExtraAllt;
import pizza.special.ExtraAlltSubscriber;
import pizza.special.ExtraAlltPublisher;
import PizzaProject.PizzaProjectTypeFactory;

import ops.ConfigurationException;
import ops.Listener;
import ops.OPSObject;

public class opstestmain implements IOpsHelperListener, ops.Listener<ops.Error> {

  private String defaultDomain = "PizzaDomain";
  private String OtherDomain = "OtherPizzaDomain";

  private Participant participant = null;
  private Participant otherParticipant = null;

  private class MyTopicInfo {
      public String domain;
      public String topName;
      public String typeName;

      public COpsHelper helper;
      public Participant part;

      public boolean selected;

      public MyTopicInfo(String dom, String top, String typ)
      {
          this.domain = dom;
          this.topName = top;
          this.typeName = typ;
          this.selected = false;
      }
  }
  Vector<MyTopicInfo> MyTopicInfoList = new Vector<MyTopicInfo>();

  private pizza.PizzaData pizzaData = new PizzaData();
  private pizza.VessuvioData vessuvioData = new VessuvioData();
  private pizza.special.ExtraAllt extraAlltData = new ExtraAllt();

  private long deadLineEventIntervall = 0;
  private int Counter = 0;

  public opstestmain() {

    MyTopicInfoList.add(new MyTopicInfo("PizzaDomain",      "PizzaTopic",         "pizza.PizzaData"));
    MyTopicInfoList.add(new MyTopicInfo("PizzaDomain",      "VessuvioTopic",      "pizza.VessuvioData"));
    MyTopicInfoList.add(new MyTopicInfo("PizzaDomain",      "PizzaTopic2",        "pizza.PizzaData"));
    MyTopicInfoList.add(new MyTopicInfo("PizzaDomain",      "VessuvioTopic2",     "pizza.VessuvioData"));
    MyTopicInfoList.add(new MyTopicInfo("PizzaDomain",      "TcpPizzaTopic",      "pizza.PizzaData"));
    MyTopicInfoList.add(new MyTopicInfo("PizzaDomain",      "TcpVessuvioTopic",   "pizza.VessuvioData"));
    MyTopicInfoList.add(new MyTopicInfo("PizzaDomain",      "TcpPizzaTopic2",     "pizza.PizzaData"));
    MyTopicInfoList.add(new MyTopicInfo("PizzaDomain",      "TcpVessuvioTopic2",  "pizza.VessuvioData"));
    MyTopicInfoList.add(new MyTopicInfo("PizzaDomain",      "UdpPizzaTopic",      "pizza.PizzaData"));
    MyTopicInfoList.add(new MyTopicInfo("PizzaDomain",      "UdpVessuvioTopic",   "pizza.VessuvioData"));
    MyTopicInfoList.add(new MyTopicInfo("OtherPizzaDomain", "OtherPizzaTopic",    "pizza.PizzaData"));
    MyTopicInfoList.add(new MyTopicInfo("OtherPizzaDomain", "OtherVessuvioTopic", "pizza.VessuvioData"));
    MyTopicInfoList.add(new MyTopicInfo("PizzaDomain",      "ExtraAlltTopic",     "pizza.special.ExtraAllt"));

    for(int i = 0; i < MyTopicInfoList.size(); i++) {
        MyTopicInfo info = MyTopicInfoList.elementAt(i);
        info.helper = new COpsHelper(this);
    }

    try
    {
      //Create Participant
      participant = Participant.getInstance(defaultDomain, defaultDomain);
      participant.addTypeSupport(new PizzaProjectTypeFactory());
      participant.addListener(this);
      otherParticipant = Participant.getInstance(OtherDomain, OtherDomain);
      otherParticipant.addTypeSupport(new PizzaProjectTypeFactory());
      otherParticipant.addListener(this);

      OnLog("Created Participants\n");
      OnLog("DomainID: " + participant.getDomain().getDomainID());
      OnLog("DomainAddress: " + participant.getDomain().getDomainAddress());
      OnLog("InSocketBufferSize: " + participant.getDomain().GetInSocketBufferSize());
      OnLog("OutSocketBufferSize: " + participant.getDomain().GetOutSocketBufferSize());
      OnLog("MetaDataMcPort: " + participant.getDomain().GetMetaDataMcPort());
      OnLog("LocalInterface: " + participant.getDomain().getLocalInterface());
      OnLog("TimeToLive: " + participant.getDomain().getTimeToLive() + "\n");

      OnLog("DomainID: " + otherParticipant.getDomain().getDomainID());
      OnLog("DomainAddress: " + otherParticipant.getDomain().getDomainAddress());
      OnLog("InSocketBufferSize: " + otherParticipant.getDomain().GetInSocketBufferSize());
      OnLog("OutSocketBufferSize: " + otherParticipant.getDomain().GetOutSocketBufferSize());
      OnLog("MetaDataMcPort: " + otherParticipant.getDomain().GetMetaDataMcPort());
      OnLog("LocalInterface: " + otherParticipant.getDomain().getLocalInterface());
      OnLog("TimeToLive: " + otherParticipant.getDomain().getTimeToLive() + "\n");

      for(int i = 0; i < MyTopicInfoList.size(); i++) {
        MyTopicInfo info = MyTopicInfoList.elementAt(i);
        if (info.domain.equals(defaultDomain)) {
          info.part = participant;
        } else if (info.domain.equals(OtherDomain)) {
          info.part = otherParticipant;
        }
      }
    }
    catch (ConfigurationException e)
    {
      OnLog("Exception: " + e.getMessage());
    }
  }

  public void OnLog(final String str) {
    System.out.println (str);
  }

  public void onNewEvent(ops.Notifier<ops.Error> notifier, ops.Error arg) {
      OnLog("###!!! " + arg.getErrorMessage());
  }

  public void OnData(final String topName, final pizza.PizzaData Data) {
      OnLog("[ " + topName + " ] New PizzaData: " + Data.cheese + "\n");
  }

  public void OnData(final String topName, final pizza.VessuvioData Data) {
      OnLog("[ " + topName + " ] New VessuvioData: " + Data.cheese + ", Ham length: " + Data.ham.length() + "\n");
  }

  public void OnData(final String topName, final pizza.special.ExtraAllt Data) {
      String str = "";

      if (Data.shs.size() > 1) str = ", shs[1]: " + Data.shs.elementAt(1);

      OnLog("[ " + topName + " ] New ExtraAllt: " + Data.cheese +
            str +
            ", Num strings: " + Data.strings.size() + "\n");
  }

  public void OnData(final String topName, final OPSObject Data) {
      if (Data instanceof ExtraAllt) {
          OnData(topName, (ExtraAllt)Data);
      } else if (Data instanceof VessuvioData) {
          OnData(topName, (VessuvioData)Data);
      } else if (Data instanceof PizzaData) {
          OnData(topName, (PizzaData)Data);
      }
  }

  public boolean isSelected(int index) {
    MyTopicInfo info = MyTopicInfoList.elementAt(index);
    return info.selected;
  }

  public void ChangeSelect(int index, boolean value) {
    MyTopicInfo info = MyTopicInfoList.elementAt(index);
    info.selected = value;
  }

  public void CreateSubscriber() {
      for(int i = 0; i < MyTopicInfoList.size(); i++) {
          MyTopicInfo info = MyTopicInfoList.elementAt(i);
          if (info.selected) {
              info.helper.CreateSubscriber(info.part, info.topName);
          }
      }
  }

  public void CreatePublisher() {
      for(int i = 0; i < MyTopicInfoList.size(); i++) {
          MyTopicInfo info = MyTopicInfoList.elementAt(i);
          if (info.selected) {
              info.helper.CreatePublisher(info.part, info.topName);
          }
      }
  }

  public void DeleteSubscriber() {
      for(int i = 0; i < MyTopicInfoList.size(); i++) {
          MyTopicInfo info = MyTopicInfoList.elementAt(i);
          if (info.selected) {
              info.helper.DeleteSubscriber();
          }
      }
  }

  public void DeletePublisher() {
      for(int i = 0; i < MyTopicInfoList.size(); i++) {
          MyTopicInfo info = MyTopicInfoList.elementAt(i);
          if (info.selected) {
              info.helper.DeletePublisher();
          }
      }
  }

  public void StartSubscriber() {
      for(int i = 0; i < MyTopicInfoList.size(); i++) {
          MyTopicInfo info = MyTopicInfoList.elementAt(i);
          if (info.selected) {
              info.helper.StartSubscriber(deadLineEventIntervall);
          }
      }
  }

  public void StopSubscriber() {
      for(int i = 0; i < MyTopicInfoList.size(); i++) {
          MyTopicInfo info = MyTopicInfoList.elementAt(i);
          if (info.selected) {
              info.helper.StopSubscriber(true);
          }
      }
  }

  public void SetDeadLineInterval(long timeoutMs) {
    for(int i = 0; i < MyTopicInfoList.size(); i++) {
        MyTopicInfo info = MyTopicInfoList.elementAt(i);
        if (info.selected) {
            info.helper.SetDeadLineInterval(timeoutMs);
        }
    }
  }

  public void WriteToAll(int NumExtraBytes) {
      Counter++;
      pizzaData.cheese = "From Java " + Counter;
      vessuvioData.cheese = "From Java " + Counter;
      extraAlltData.cheese = "From Java " + Counter;

      StringBuilder sb = new StringBuilder();
      sb.setLength(NumExtraBytes);
      vessuvioData.ham = sb.toString();

      extraAlltData.sh = -7;
      if (extraAlltData.shs.size() == 0) {
        extraAlltData.shs.add((short)11);
        extraAlltData.shs.add((short)22);
        extraAlltData.shs.add((short)33);
      }
      if (extraAlltData.strings.size() == 0) {
        extraAlltData.strings.add("aaa");
        extraAlltData.strings.add("bbbbbb");
        extraAlltData.strings.add("cc");
        extraAlltData.strings.add("ddddddddd");
      }
      for(int i = 0; i < MyTopicInfoList.size(); i++) {
          MyTopicInfo info = MyTopicInfoList.elementAt(i);
          if (info.selected) {
              if (info.typeName.equals(PizzaData.getTypeName())) {
                  info.helper.Write(pizzaData);
              }
              if (info.typeName.equals(VessuvioData.getTypeName())) {
                  info.helper.Write(vessuvioData);
              }
              if (info.typeName.equals(ExtraAllt.getTypeName())) {
                  info.helper.Write(extraAlltData);
              }
          }
      }
  }

  int NumVessuvioBytes = 0;
  int deadlineTimeout = 1000;
  int sendPeriod = 1000;

  public void menu()
	{
		for (int i = 0; i < MyTopicInfoList.size(); i++) {
      MyTopicInfo info = MyTopicInfoList.elementAt(i);
			System.out.print("\t " + i);
      if (info.helper.HasPublisher()) {
        System.out.print(" P");
      } else {
        System.out.print("  ");
      }
      if (info.helper.HasSubscriber()) {
        System.out.print("S");
      } else {
        System.out.print(" ");
      }
      if (info.selected) {
        System.out.print("*");
      } else {
        System.out.print(" ");
      }
			System.out.println(info.domain + "::" + info.topName);
		}

    System.out.println("");
  	System.out.println("\t PC    Create Publishers");
  	System.out.println("\t PD    Delete Publishers");
  	//System.out.println("\t PS    Start Publishers");
  	//System.out.println("\t PT    Stop Publishers");
  	System.out.println("\t SC    Create Subscriber");
  	System.out.println("\t SD    Delete Subscriber");
  	System.out.println("\t SS    Start Subscriber");
  	System.out.println("\t ST    Stop Subscriber");
  	System.out.println("\t L num Set num Vessuvio Bytes [" + NumVessuvioBytes + "]");
  	System.out.println("\t T ms  Set deadline timeout [ms]");
  	System.out.println("\t V ms  Set send period [ms] [" + sendPeriod + "]");
  	System.out.println("\t A     Start/Stop periodical Write with set period");
  	System.out.println("\t W     Write data");
  	//System.out.println("\t Q     Quite (minimize program output)");
  	System.out.println("\t X     Exit program");
	}

  public void doRun()
  {
    // set item 0 as default
    ChangeSelect(0, true);
    menu();

    boolean doPeriodicalSends = false;

    for (; ; ) {
			System.out.print(" command > ");

      if (doPeriodicalSends) {
        try {
          while (!System.console().reader().ready()) {
            WriteToAll(NumVessuvioBytes);
            try {
              Thread.sleep(sendPeriod);
            } catch (InterruptedException e) {
            }
          }
        } catch (java.io.IOException e) {
        }
      }

			String input = System.console().readLine().toLowerCase();

			if (input.length() == 0) continue;

			if ((input.charAt(0) >= '0') && (input.charAt(0) <= '9')) {
				try {
					int i = Integer.parseInt(input);
					//System.out.printf("Integer was: %d\n", i);
					ChangeSelect(i, !isSelected(i));
          menu();
				} catch (NumberFormatException nfe) {
					System.err.println("Invalid Format!");
				}
			}

      if (input.startsWith("?", 0)) { menu(); }

			if (input.startsWith("pc", 0)) { CreatePublisher(); }
			if (input.startsWith("pd", 0)) { DeletePublisher(); }
			//if (input == "ps") { app.StartPublisher(); }
			//if (input == "pt") { app.StopPublisher(); }

			if (input.startsWith("sc", 0)) { CreateSubscriber(); }
			if (input.startsWith("sd", 0)) { DeleteSubscriber(); }
			if (input.startsWith("ss", 0)) { StartSubscriber(); }
			if (input.startsWith("st", 0)) { StopSubscriber(); }

      if (input.startsWith("l", 0)) {
        try {
          NumVessuvioBytes = Integer.parseInt(input.substring(2));
        } catch (NumberFormatException nfe) {
          System.err.println("Invalid Format!");
        }
        menu();
      }
      if (input.startsWith("t", 0)) {
        try {
          long t = Long.parseLong(input.substring(2));
          SetDeadLineInterval(t);
        } catch (NumberFormatException nfe) {
          System.err.println("Invalid Format!");
        }
      }
      if (input.startsWith("v", 0)) {
        try {
          sendPeriod = Integer.parseInt(input.substring(2));
          menu();
        } catch (NumberFormatException nfe) {
          System.err.println("Invalid Format!");
        }
      }
      if (input.startsWith("a", 0)) {
        doPeriodicalSends = !doPeriodicalSends;
      }

			if (input.startsWith("w", 0)) { WriteToAll(NumVessuvioBytes); }
			if (input.startsWith("x", 0)) break;
		}

    // Clean up
    for (int i = 0; i < MyTopicInfoList.size(); i++) {
      MyTopicInfo info = MyTopicInfoList.elementAt(i);
      if (info.helper.HasSubscriber()) info.helper.DeleteSubscriber();
      if (info.helper.HasPublisher()) info.helper.DeletePublisher();
      info.helper = null;
		}

    participant = null;
    otherParticipant = null;
  }

}
