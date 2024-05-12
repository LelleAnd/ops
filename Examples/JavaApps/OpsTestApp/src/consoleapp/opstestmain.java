/*
 * opstestmain.java
 */

package opstestmain;

import java.util.Vector;
import java.nio.file.Paths;
import java.io.File;
import java.io.IOException;

import opstestapp.IOpsHelperListener;
import opstestapp.COpsHelper;

import ops.Participant;
import ops.OPSConfig;
import ops.OPSConfigRepository;
import ops.Domain;
import ops.Topic;
import ops.KeyFilterQoSPolicy;
import ops.NetworkSupport;

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
import ops.DeadlineNotifier;

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

    File tmp = new File("ops_config.xml");
    if (tmp.exists()) {
      OnLog("Using config file in CWD\n");
    } else {
      String cwd = Paths.get(".").toAbsolutePath().normalize().toString();
      int idx = cwd.indexOf("Examples");
      if (idx > 0) {
        String cfg = cwd.substring(0, idx) + "Examples/OPSIdls/PizzaProject/ops_config.xml";
        OPSConfigRepository.add(cfg);
        OnLog("Using config file in: " + cfg);
      }
    }

    // Get topics from config file
    OPSConfig cfg = OPSConfigRepository.getConfig();
    if (cfg != null) {
      Vector<Domain> doms = cfg.getDomains();
      for (Domain dom : doms) {
        String domID = dom.getDomainID();
        Vector<Topic> tops = dom.getTopics();
        for (Topic top : tops) {
          //OnLog(domID + ", " + top.getName() + ", " + top.getTypeID());
          MyTopicInfoList.add(new MyTopicInfo(domID, top.getName(), top.getTypeID()));
        }
      }
    }

    for(int i = 0; i < MyTopicInfoList.size(); i++) {
        MyTopicInfo info = MyTopicInfoList.elementAt(i);
        info.helper = new COpsHelper(this);
    }

    try
    {
      //Create Participant
      participant = Participant.getInstance(defaultDomain, defaultDomain);
      if (participant == null) {
        OnLog("Failed to create Participant. Check ops_config.xml files\n");
        System.exit(1);
      }
      participant.addTypeSupport(new PizzaProjectTypeFactory());
      participant.addListener(this);
      otherParticipant = Participant.getInstance(OtherDomain, OtherDomain);
      if (otherParticipant == null) {
        OnLog("Failed to create Participant. Check ops_config.xml files\n");
        System.exit(1);
      }
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
      System.exit(1);
    }
  }

  public void OnLog(final String str) {
    System.out.println (str);
  }

  public void onNewEvent(ops.Notifier<ops.Error> notifier, ops.Error arg) {
      OnLog("###!!! " + arg.getErrorMessage());
  }

  public void OnData(final String topName, final pizza.PizzaData Data) {
      OnLog("[ " + topName + " ] New PizzaData(v" + Data.PizzaData_version + "): " + Data.cheese + "\n");
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

  public void WriteToAll(int NumExtraBytes, int pd_version) {
      Counter++;
      pizzaData.cheese = "From Java " + Counter;
      vessuvioData.cheese = "From Java " + Counter;
      extraAlltData.cheese = "From Java " + Counter;

      if (pd_version < 0) {
          pizzaData.idlVersionMask = 0;
      } else {
          pizzaData.idlVersionMask = 1;
          pizzaData.PizzaData_version = (byte)pd_version;
      }

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
  int pd_version = pizza.PizzaData.PizzaData_idlVersion;

  public void menu()
	{
		for (int i = 0; i < MyTopicInfoList.size(); i++) {
      MyTopicInfo info = MyTopicInfoList.elementAt(i);
			System.out.format("\t %2d", i);
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
  	System.out.println("\t PC      Create Publishers");
  	System.out.println("\t PD      Delete Publishers");
  	//System.out.println("\t PS      Start Publishers");
  	//System.out.println("\t PT      Stop Publishers");
  	System.out.println("\t SC      Create Subscriber");
  	System.out.println("\t SD      Delete Subscriber");
  	System.out.println("\t SS      Start Subscriber");
  	System.out.println("\t ST      Stop Subscriber");
  	System.out.println("\t L num   Set num Vessuvio Bytes [" + NumVessuvioBytes + "]");
  	System.out.println("\t T ms    Set deadline timeout [ms]");
  	System.out.println("\t V ms    Set send period [ms] [" + sendPeriod + "]");
    System.out.println("\t M ver   Set PizzaData version ["+ pd_version + "]");
  	System.out.println("\t A       Start/Stop periodical Write with set period");
  	System.out.println("\t W       Write data");
    System.out.println("\t I name  Lookup host name");
  	//System.out.println("\t Q       Quite (minimize program output)");
  	System.out.println("\t X       Exit program");
	}

  public void test(String addr)
  {
      System.out.println("IsValidNodeAddress(" + addr + ") = " + NetworkSupport.IsValidNodeAddress(addr));
      System.out.println("IsMyNodeAddress(" + addr + ") = " + NetworkSupport.IsMyNodeAddress(addr));
  }

  public void doRun()
  {
    // set item 0 as default
    ChangeSelect(0, true);
    menu();

    /*
    test("");
    test("0.0.0.0");
    test("223.255.255.255");
    test("224.0.0.0");
    test("127.0.0.1");
    test("192.168.0.24");
    test("192.168.0.25");
    test("192.168.0.26");
    test("172.17.205.209");
    */

    boolean doPeriodicalSends = false;

    for (; ; ) {
			System.out.print(" command > ");

      if (doPeriodicalSends) {
        try {
          while (!System.console().reader().ready()) {
            WriteToAll(NumVessuvioBytes, pd_version);
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
      if (input.startsWith("m", 0)) {
        try {
          pd_version = Integer.parseInt(input.substring(2));
          menu();
        } catch (NumberFormatException nfe) {
          System.err.println("Invalid Format!");
        }
      }
      if (input.startsWith("a", 0)) {
        doPeriodicalSends = !doPeriodicalSends;
      }

      if (input.startsWith("i", 0)) {
        String name = input.substring(2);
        String[] ads = NetworkSupport.GetHostAddresses(name);
        System.out.println("IP Addresses for host: " + name);
        for (String s : ads) {
          System.out.println("  " + s);
        }
        System.out.println("Selected: " + name + " --> " + NetworkSupport.GetHostAddressEx(name));
      }

			if (input.startsWith("w", 0)) { WriteToAll(NumVessuvioBytes, pd_version); }
			if (input.startsWith("x", 0)) break;
		}

    // Clean up
    for (int i = 0; i < MyTopicInfoList.size(); i++) {
      MyTopicInfo info = MyTopicInfoList.elementAt(i);
      if (info.helper.HasSubscriber()) info.helper.DeleteSubscriber();
      if (info.helper.HasPublisher()) info.helper.DeletePublisher();
      info.helper = null;
		}

    participant.stopThread();
    otherParticipant.stopThread();
    participant = null;
    otherParticipant = null;
    DeadlineNotifier.getInstance().stopRunning();
  }

}
