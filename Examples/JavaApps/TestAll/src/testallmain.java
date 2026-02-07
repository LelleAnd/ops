/*
 * testallmain.java
 */

package testallmain;

import java.io.File;
import java.io.IOException;
import java.util.Observer;
import java.util.Observable;
import java.util.Vector;
import java.nio.ByteBuffer;
import java.nio.file.Paths;
import java.util.HashMap;

import ops.Participant;
import ops.OPSConfig;
import ops.OPSConfigRepository;
import ops.Domain;
import ops.Topic;
import ops.KeyFilterQoSPolicy;
import ops.ConfigurationException;
import ops.Listener;
import ops.OPSObject;
import ops.archiver.OPSArchiverOut;
import ops.protocol.OPSMessage;
import ops.WriteByteBuffer;
import ops.StaticManager;
import ops.DeadlineNotifier;

import TestAll.Definitions;
import TestAll.BaseData;
import TestAll.ChildData;
import TestAll.ChildDataSubscriber;
import TestAll.ChildDataPublisher;
import TestAll.Fruit;
import TestAll.NoData;
import TestAll.TestData;
import TestAll.TestAllTypeFactory;

public class testallmain implements ops.Listener<ops.Error> {

  private String defaultDomain = "TestAllDomain";
  private ChildData cd1 = null;

  private Participant participant = null;
  private boolean gTestFailed = false;

  public Definitions.Command enumTest = Definitions.Command.PAUSE;

  private HashMap<String, Integer> pubCounters = new HashMap<String, Integer>();

  public void OnLog(final String str) {
    System.out.println (str);
  }

  public void onNewEvent(ops.Notifier<ops.Error> notifier, ops.Error arg) {
      OnLog("###!!! " + arg.getErrorMessage());
  }

  public void LogError(String str)
  {
      gTestFailed = true;
      OnLog("###### " + str);
  }

  // ===========================================================

  public boolean AssertTRUE(boolean val, String mess)
  {
      if (!val) LogError("Failed: " + mess + ", value= " + val);
      return val;
  }
  public boolean AssertTRUE(boolean val)
  {
      return AssertTRUE(val, "");
  }

  public void AssertEQ(String val, String exp, String mess)
  {
      if (!val.equals(exp)) LogError("Failed: " + mess + ", value= " + val + ", expected= " + exp);
  }
  public void AssertEQ(String val, String exp)
  {
      AssertEQ(val, exp, "");
  }

  public void AssertEQ(boolean val, boolean exp, String mess)
  {
      if (val != exp) LogError("Failed: " + mess + ", value= " + val + ", expected= " + exp);
  }
  public void AssertEQ(boolean val, boolean exp)
  {
      AssertEQ(val, exp, "");
  }

  public boolean AssertEQ(long val, long exp, String mess)
  {
      if (val != exp) LogError("Failed: " + mess + ", value= " + val + ", expected= " + exp);
      return val == exp;
  }
  public boolean AssertEQ(long val, long exp)
  {
      return AssertEQ(val, exp, "");
  }

  public void AssertEQ(double val, double exp, String mess)
  {
      if (val != exp) LogError("Failed: " + mess + ", value= " + val + ", expected= " + exp);
  }
  public void AssertEQ(double val, double exp)
  {
      AssertEQ(val, exp, "");
  }

  public <T extends java.lang.Enum<T>> void AssertEQ(T val, T exp, String mess)
  {
      if (val != exp) LogError("Failed: " + mess + ", value= " + val + ", expected= " + exp);
  }
  public <T extends java.lang.Enum<T>> void AssertEQ(T val, T exp)
  {
      AssertEQ(val, exp, "");
  }

  public <T> boolean AssertCountEQ(java.util.Vector<T> val, int exp, String mess)
  {
      if (AssertTRUE(val != null, mess)) {
          return AssertEQ(val.size(), exp, mess);
      }
      return false;
  }
  public <T> boolean AssertCountEQ(java.util.Vector<T> val, int exp)
  {
      return AssertCountEQ(val, exp, "");
  }

  public <T> boolean AssertCountEQ(java.util.Vector<T> val, java.util.Vector<T> exp, String mess)
  {
      if ( (AssertTRUE(val != null, mess)) && (AssertTRUE(exp != null, mess)) ) {
          return AssertEQ(val.size(), exp.size(), mess);
      }
      return false;
  }
  public <T> boolean AssertCountEQ(java.util.Vector<T> val, java.util.Vector<T> exp)
  {
      return AssertCountEQ(val, exp, "");
  }

  public <T> boolean AssertCountEQ(java.util.Vector<T> val, java.util.Vector<T> exp, int size, String mess)
  {
      if ( (AssertTRUE(val != null, mess)) && (AssertTRUE(exp != null, mess)) ) {
          return AssertEQ(val.size(), exp.size(), mess) && AssertEQ(val.size(), size, mess);
      }
      return false;
  }
  public <T> boolean AssertCountEQ(java.util.Vector<T> val, java.util.Vector<T> exp, int size)
  {
      return AssertCountEQ(val, exp, size, "");
  }

  // ===========================================================

  public void CheckEmpty(TestData data) {
      if (AssertTRUE(data != null, "TestData ref is null")) {
          AssertEQ(data.text, "");
          AssertEQ(data.value, 0.0);
      }
  }

  public void CheckEmpty(Fruit data) {
      if (AssertTRUE(data != null, "Fruit ref is null")) {
          AssertEQ(data.value, Fruit.Value.APPLE);
      }
  }

  public void CheckEmpty(ChildData data)
  {
      OnLog("Checking empty object...");

      if (!AssertTRUE(data != null)) return;

      // BaseData
      //   std::string baseText;
      AssertEQ(data.baseText, "");
      //   std::vector<std::string> stringOpenArr;
      AssertCountEQ(data.stringOpenArr, 0);
      //   std::string stringFixArr[5];
      if (AssertCountEQ(data.stringFixArr, 5)) {
          for (int i = 0; i < 5; i++) AssertEQ(data.stringFixArr.elementAt(i), "");
      }
      //   ops::strings::fixed_string<23> fixLengthString;
      AssertEQ(data.fixLengthString, "");
      //   std::vector<ops::strings::fixed_string<16>> fixLengthStringOpenArr;
      AssertCountEQ(data.fixLengthStringOpenArr, 0);
      //   ops::strings::fixed_string<16> fixLengthStringFixArr[10];
      if (AssertCountEQ(data.fixLengthStringFixArr, 10)) {
          for (int i = 0; i < 10; i++) AssertEQ(data.fixLengthStringFixArr.elementAt(i), "");
      }

      // ChildData
      //  enums
      AssertEQ(data.enu1, ChildData.Order.ABC);
      AssertCountEQ(data.enuVec, 0);
      if (AssertCountEQ(data.enuFixArr, 6)) {
          for (int i = 0; i < 6; i++) AssertEQ(data.enuFixArr.elementAt(i), ChildData.Order.ABC);
      }
      AssertEQ(data.cmd, Definitions.Command.START);
      if (AssertCountEQ(data.cmds, 2)) {
          for (int i = 0; i < 2; i++) AssertEQ(data.cmds.elementAt(i), Definitions.Command.START);
      }

      //  core types
      AssertEQ(data.bo, false, "data.bo");
      AssertEQ(data.b, 0, "data.b");
      AssertEQ(data.sh, 0);
      AssertEQ(data.i, 0);
      AssertEQ(data.l, 0);
      AssertEQ(data.f, 0.0);
      AssertEQ(data.d, 0.0);
      AssertEQ(data.s, "");

      CheckEmpty(data.test2);
      CheckEmpty(data.testPointer);

      CheckEmpty(data.fruit);

      AssertCountEQ(data.bos, 0);
      //	bool fbos[11];
      if (AssertCountEQ(data.fbos, 11)) {
          for (int i = 0; i < 11; i++) AssertEQ(data.fbos.elementAt(i), false);
      }

      AssertCountEQ(data.bs, 0);
      //	char fbs[256];
      if (AssertCountEQ(data.fbs, 256)) {
          for (int i = 0; i < 256; i++) AssertEQ(data.fbs.elementAt(i), 0);
      }

      AssertCountEQ(data.shs, 0);
      //	short fshs[4];
      if (AssertCountEQ(data.fshs, 4)) {
          for (int i = 0; i < 4; i++) AssertEQ(data.fshs.elementAt(i), 0);
      }

      AssertCountEQ(data.is_, 0);
      //	int fis_[3];
      if (AssertCountEQ(data.fis_, 3)) {
          for (int i = 0; i < 3; i++) AssertEQ(data.fis_.elementAt(i), 0);
      }

      AssertCountEQ(data.ls, 0);
      //	int64_t fls[6];
      if (AssertCountEQ(data.fls, 6)) {
          for (int i = 0; i < 6; i++) AssertEQ(data.fls.elementAt(i), 0);
      }

      AssertCountEQ(data.fs, 0);
      //    float ffs[77];
      if (AssertCountEQ(data.ffs, 77)) {
          for (int i = 0; i < 77; i++) AssertEQ(data.ffs.elementAt(i), 0.0);
      }

      AssertCountEQ(data.ds, 0);
      //    double fds[5];
      if (AssertCountEQ(data.fds, 5)) {
          for (int i = 0; i < 5; i++) AssertEQ(data.fds.elementAt(i), 0.0);
      }

      AssertCountEQ(data.ss, 0);
      //    std::string fss[10];
      if (AssertCountEQ(data.fss, 10)) {
          for (int i = 0; i < 10; i++) AssertEQ(data.fss.elementAt(i), "");
      }

      AssertCountEQ(data.test2s, 0);
      //    TestData* ftest2s[5];
      if (AssertCountEQ(data.ftest2s, 5)) {
          for (int i = 0; i < 5; i++) CheckEmpty(data.ftest2s.elementAt(i));
      }

      AssertCountEQ(data.secondVirtArray, 0);
      //    TestData* fsecondVirtArray[7];
      if (AssertCountEQ(data.fsecondVirtArray, 7)) {
          for (int i = 0; i < 7; i++) CheckEmpty(data.fsecondVirtArray.elementAt(i));
      }

      AssertCountEQ(data.test2s2, 0);
      //    TestData ftest2s2[4];
      if (AssertCountEQ(data.ftest2s2, 4)) {
          for (int i = 0; i < 4; i++) CheckEmpty(data.ftest2s2.elementAt(i));
      }

      AssertCountEQ(data.fruitarr, 0);
      //    Fruit ffruitarr[15];
      if (AssertCountEQ(data.ffruitarr, 15)) {
          for (int i = 0; i < 15; i++) CheckEmpty(data.ffruitarr.elementAt(i));
      }
  }

  // ===========================================================

  public void CheckObjects(TestData data, TestData exp, String mess)
  {
      AssertTRUE(data != exp, mess + " 'data' end 'exp' is equal");
      if (AssertTRUE(data != null, "TestData 'data' ref is null") && AssertTRUE(exp != null, "TestData 'exp' ref is null")) {
          AssertEQ(data.text, exp.text);
          AssertEQ(data.value, exp.value);
      }
  }
  public void CheckObjects(TestData data, TestData exp)
  {
      CheckObjects(data, exp, "");
  }

  public void CheckObjects(Fruit data, Fruit exp, String mess)
  {
      AssertTRUE(data != exp, mess + " 'data' end 'exp' is equal");
      if (AssertTRUE(data != null, "Fruit 'data' ref is null") && AssertTRUE(exp != null, "Fruit 'exp' ref is null")) {
          AssertEQ(data.value, exp.value);
      }
  }
  public void CheckObjects(Fruit data, Fruit exp)
  {
      CheckObjects(data, exp, "");
  }

  public void CheckObjects(ChildData data, ChildData exp)
  {
      OnLog("Comparing objects object...");

      AssertTRUE(data != exp, "'data' end 'exp' is equal");
      if (!AssertTRUE(data != null) || !AssertTRUE(exp != null)) return;

      // BaseData
      //   std::string baseText;
      AssertEQ(data.baseText, exp.baseText);

      //   std::vector<std::string> stringOpenArr;
      if (AssertCountEQ(data.stringOpenArr, exp.stringOpenArr)) {
          for (int i = 0; i < data.stringOpenArr.size(); i++) AssertEQ(data.stringOpenArr.elementAt(i), exp.stringOpenArr.elementAt(i), "stringOpenArr");
      }

      //   std::string stringFixArr[5];
      if (AssertCountEQ(data.stringFixArr, exp.stringFixArr, 5)) {
          for (int i = 0; i < 5; i++) AssertEQ(data.stringFixArr.elementAt(i), exp.stringFixArr.elementAt(i));
      }

      //   ops::strings::fixed_string<23> fixLengthString;
      AssertEQ(data.fixLengthString, exp.fixLengthString);

      //   std::vector<ops::strings::fixed_string<16>> fixLengthStringOpenArr;
      if (AssertCountEQ(data.fixLengthStringOpenArr, exp.fixLengthStringOpenArr)) {
          for (int i = 0; i < data.fixLengthStringOpenArr.size(); i++) AssertEQ(data.fixLengthStringOpenArr.elementAt(i), exp.fixLengthStringOpenArr.elementAt(i), "fixLengthStringOpenArr");
      }

      //   ops::strings::fixed_string<16> fixLengthStringFixArr[10];
      if (AssertCountEQ(data.fixLengthStringFixArr, exp.fixLengthStringFixArr, 10)) {
          for (int i = 0; i < 10; i++) AssertEQ(data.fixLengthStringFixArr.elementAt(i), exp.fixLengthStringFixArr.elementAt(i));
      }

      // ChildData
      //  enums
      AssertEQ(data.enu1, exp.enu1);
      if (AssertCountEQ(data.enuVec, exp.enuVec, "enuVec")) {
          for (int i = 0; i < data.enuVec.size(); i++) AssertEQ(data.enuVec.elementAt(i), exp.enuVec.elementAt(i), "enuVec");
      }
      if (AssertCountEQ(data.enuFixArr, exp.enuFixArr, 6, "enuFixArr")) {
          for (int i = 0; i < 6; i++) AssertEQ(data.enuFixArr.elementAt(i), exp.enuFixArr.elementAt(i), "enuFixArr");
      }
      AssertEQ(data.cmd, exp.cmd);
      if (AssertCountEQ(data.cmds, exp.cmds, 2)) {
          for (int i = 0; i < 2; i++) AssertEQ(data.cmds.elementAt(i), exp.cmds.elementAt(i));
      }

      //  core types
      AssertEQ(data.bo, exp.bo, "data.bo");
      AssertEQ(data.b, exp.b, "data.b");
      AssertEQ(data.sh, exp.sh);
      AssertEQ(data.i, exp.i);
      AssertEQ(data.l, exp.l);
      AssertEQ(data.f, exp.f);
      AssertEQ(data.d, exp.d);
      AssertEQ(data.s, exp.s);

      CheckObjects(data.test2, exp.test2);

      AssertTRUE(data.testPointer != exp.testPointer, "data.testPointer & exp.testPointer is equal");
      CheckObjects(data.testPointer, exp.testPointer);

      CheckObjects(data.fruit, exp.fruit);

      if (AssertCountEQ(data.bos, exp.bos)) {
          for (int i = 0; i < data.bos.size(); i++) AssertEQ(data.bos.elementAt(i), exp.bos.elementAt(i));
      }
      //	bool fbos[11];
      if (AssertCountEQ(data.fbos, exp.fbos, 11)) {
          for (int i = 0; i < 11; i++) AssertEQ(data.fbos.elementAt(i), exp.fbos.elementAt(i));
      }

      if (AssertCountEQ(data.bs, exp.bs)) {
          for (int i = 0; i < data.bs.size(); i++) AssertEQ(data.bs.elementAt(i), exp.bs.elementAt(i));
      }
      //	char fbs[256];
      if (AssertCountEQ(data.fbs, exp.fbs, 256)) {
          for (int i = 0; i < 256; i++) AssertEQ(data.fbs.elementAt(i), exp.fbs.elementAt(i));
      }

      if (AssertCountEQ(data.shs, exp.shs)) {
          for (int i = 0; i < data.shs.size(); i++) AssertEQ(data.shs.elementAt(i), exp.shs.elementAt(i));
      }
      //	short fshs[4];
      if (AssertCountEQ(data.fshs, exp.fshs, 4)) {
          for (int i = 0; i < 4; i++) AssertEQ(data.fshs.elementAt(i), exp.fshs.elementAt(i));
      }

      if (AssertCountEQ(data.is_, exp.is_)) {
          for (int i = 0; i < data.is_.size(); i++) AssertEQ(data.is_.elementAt(i), exp.is_.elementAt(i));
      }
      //	int fis_[3];
      if (AssertCountEQ(data.fis_, exp.fis_, 3)) {
          for (int i = 0; i < 3; i++) AssertEQ(data.fis_.elementAt(i), exp.fis_.elementAt(i));
      }

      if (AssertCountEQ(data.ls, exp.ls)) {
          for (int i = 0; i < data.ls.size(); i++) AssertEQ(data.ls.elementAt(i), exp.ls.elementAt(i));
      }
      //	int64_t fls[6];
      if (AssertCountEQ(data.fls, exp.fls, 6)) {
          for (int i = 0; i < 6; i++) AssertEQ(data.fls.elementAt(i), exp.fls.elementAt(i));
      }

      if (AssertCountEQ(data.fs, exp.fs)) {
          for (int i = 0; i < data.fs.size(); i++) AssertEQ(data.fs.elementAt(i), exp.fs.elementAt(i));
      }
      //    float ffs[77];
      if (AssertCountEQ(data.ffs, exp.ffs, 77)) {
          for (int i = 0; i < 77; i++) AssertEQ(data.ffs.elementAt(i), exp.ffs.elementAt(i));
      }

      if (AssertCountEQ(data.ds, exp.ds)) {
          for (int i = 0; i < data.ds.size(); i++) AssertEQ(data.ds.elementAt(i), exp.ds.elementAt(i));
      }
      //    double fds[5];
      if (AssertCountEQ(data.fds, exp.fds, 5)) {
          for (int i = 0; i < 5; i++) AssertEQ(data.fds.elementAt(i), exp.fds.elementAt(i));
      }

      if (AssertCountEQ(data.ss, exp.ss)) {
          for (int i = 0; i < data.ss.size(); i++) AssertEQ(data.ss.elementAt(i), exp.ss.elementAt(i));
      }
      //    std::string fss[10];
      if (AssertCountEQ(data.fss, exp.fss, 10)) {
          for (int i = 0; i < 10; i++) AssertEQ(data.fss.elementAt(i), exp.fss.elementAt(i));
      }

      if (AssertCountEQ(data.test2s, exp.test2s)) {
          for (int i = 0; i < data.test2s.size(); i++) CheckObjects(data.test2s.elementAt(i), exp.test2s.elementAt(i), "test2s");
      }
      //    TestData* ftest2s[5];
      if (AssertCountEQ(data.ftest2s, exp.ftest2s, 5)) {
          for (int i = 0; i < 5; i++) CheckObjects(data.ftest2s.elementAt(i), exp.ftest2s.elementAt(i));
      }

      if (AssertCountEQ(data.secondVirtArray, exp.secondVirtArray)) {
          for (int i = 0; i < data.secondVirtArray.size(); i++) CheckObjects(data.secondVirtArray.elementAt(i), exp.secondVirtArray.elementAt(i));
      }
      //    TestData* fsecondVirtArray[7];
      if (AssertCountEQ(data.fsecondVirtArray, exp.fsecondVirtArray, 7)) {
          for (int i = 0; i < 7; i++) CheckObjects(data.fsecondVirtArray.elementAt(i), exp.fsecondVirtArray.elementAt(i));
      }

      if (AssertCountEQ(data.test2s2, exp.test2s2)) {
          for (int i = 0; i < data.test2s2.size(); i++) CheckObjects(data.test2s2.elementAt(i), exp.test2s2.elementAt(i));
      }
      //    TestData ftest2s2[4];
      if (AssertCountEQ(data.ftest2s2, exp.ftest2s2, 4)) {
          for (int i = 0; i < 4; i++) CheckObjects(data.ftest2s2.elementAt(i), exp.ftest2s2.elementAt(i));
      }

      if (AssertCountEQ(data.fruitarr, exp.fruitarr)) {
          for (int i = 0; i < data.fruitarr.size(); i++) CheckObjects(data.fruitarr.elementAt(i), exp.fruitarr.elementAt(i));
      }
      //    Fruit ffruitarr[15];
      if (AssertCountEQ(data.ffruitarr, exp.ffruitarr, 15)) {
          for (int i = 0; i < 15; i++) CheckObjects(data.ffruitarr.elementAt(i), exp.ffruitarr.elementAt(i));
      }
  }

  public void FillChildData(ChildData data)
  {
      // Data for fields in BaseData
      //   std::string baseText;
      data.baseText = "dynamic string";
      //   std::vector<std::string> stringOpenArr;
      data.stringOpenArr.add("dyn str 1");
      data.stringOpenArr.add("dyn str 2");
      //   std::string stringFixArr[5];
      data.stringFixArr.set(0, "dsf 0");
      data.stringFixArr.set(1, "dsf 1");
      data.stringFixArr.set(2, "dsf 2");
      data.stringFixArr.set(3, "dsf 3");
      data.stringFixArr.set(4, "dsf 4");
      //   ops::strings::fixed_string<23> fixLengthString;
      data.fixLengthString = "fixed length string";
      //   std::vector<ops::strings::fixed_string<16>> fixLengthStringOpenArr;
      data.fixLengthStringOpenArr.add("fix len str 1");
      data.fixLengthStringOpenArr.add("fix len str 2");
      //   ops::strings::fixed_string<16> fixLengthStringFixArr[10];
      data.fixLengthStringFixArr.set(0, "fsf 0");
      data.fixLengthStringFixArr.set(1, "fsf 1");
      data.fixLengthStringFixArr.set(2, "fsf 2");
      data.fixLengthStringFixArr.set(3, "fsf 3");
      data.fixLengthStringFixArr.set(4, "fsf 4");
      data.fixLengthStringFixArr.set(5, "fsf 5");
      data.fixLengthStringFixArr.set(6, "fsf 6");
      data.fixLengthStringFixArr.set(7, "fsf 7");
      data.fixLengthStringFixArr.set(8, "fsf 8");
      data.fixLengthStringFixArr.set(9, "fsf 9");

      // Data for fields in ChildData
      //  enums
      data.enu1 = ChildData.Order.GHI;

      data.enuVec.add(ChildData.Order.GHI);
      data.enuVec.add(ChildData.Order.JKL);
      data.enuVec.add(ChildData.Order.JKL);
      data.enuVec.add(ChildData.Order.ABC);
      data.enuVec.add(ChildData.Order.DEF);

      data.enuFixArr.set(0, ChildData.Order.DEF);
      data.enuFixArr.set(4, ChildData.Order.JKL);
      data.enuFixArr.set(5, ChildData.Order.DEF);

      data.cmd = Definitions.Command.CONTINUE;
    	data.cmds.set(0, Definitions.Command.PAUSE);
    	data.cmds.set(1, Definitions.Command.STOP);

      //  core types
      data.bo = true;
      data.b = 7;
      data.sh = -99;
      data.i = 19;
      data.l = 3456789;
      data.f = 123.4567f;
      data.d = 987.12345678901;
      data.s = "Test of [de]serializing";

      data.test2.text = "TestData";
      data.test2.value = 555.5;

      data.testPointer.text = "TestPtr";
      data.testPointer.value = 777.7;

      data.fruit.value = Fruit.Value.PEAR;

      data.bos.add(false);
      data.bos.add(true);
      data.bos.add(false);

      //	bool fbos[11];
      data.fbos.set(5, true);
      data.fbos.set(10, true);

      data.bs.add((byte)10);
      data.bs.add((byte)20);
      data.bs.add((byte)30);

      //	char fbs[256];
      for (int i = 0; i < 256; i++) data.fbs.set(i, (byte)i);

      data.shs.add((short)1111);
      data.shs.add((short)2222);

      //	short fshs[4];
      data.fshs.set(0, (short)21);
      data.fshs.set(1, (short)121);
      data.fshs.set(2, (short)221);
      data.fshs.set(3, (short)321);

      data.is_.add(100000);
      data.is_.add(101010);
      data.is_.add(110101);
      data.is_.add(111111);

      //	int fis_[3];
      data.fis_.set(0, -1);
      data.fis_.set(2, -3);
      data.fis_.set(1, -2);

      data.ls.add((long)9);
      data.ls.add((long)8);
      data.ls.add((long)7);
      data.ls.add((long)6);

      //	int64_t fls[6];
      data.fls.set(0, (long)9999);
      data.fls.set(1, (long)9998);
      data.fls.set(2, (long)9997);
      data.fls.set(3, (long)9996);
      data.fls.set(4, (long)9995);
      data.fls.set(5, (long)9994);

      data.fs.add(3.1f);
      data.fs.add(31.14f);
      data.fs.add(4.56f);
      data.fs.add(987.0f);

      //    float ffs[77];
      for (int i = 0; i < 77; i++) data.ffs.set(i, 0.0f);
      data.ffs.set(21, 3.1415f);

      data.ds.add(1.987654321);
      data.ds.add(2.3456789);

      //    double fds[5];
      data.fds.set(0, 1.1);
      data.fds.set(1, 2.1);
      data.fds.set(2, 3.1);
      data.fds.set(4, 5.1);
      data.fds.set(3, 4.1);

      data.ss.add("Index 0");
      data.ss.add("Index 1");
      data.ss.add("Index 2");

      //    std::string fss[10];
      data.fss.set(4, "4 string");
      data.fss.set(9, "9 string");
      data.fss.set(7, "7 string");

      //    std::vector<TestData*> test2s;
      data.test2s.add(new TestData());
      data.test2s.add(new TestData());
      data.test2s.add(new TestData());
      data.test2s.add(new TestData());
      //    TestData* ftest2s[5];
      data.ftest2s.get(2).text = "Index 2";
      data.ftest2s.get(2).value = 7.7;

      //    std::vector<TestData*> secondVirtArray;
      data.secondVirtArray.add(new TestData());
      data.secondVirtArray.add(new TestData());
      //    TestData* fsecondVirtArray[7];
      data.fsecondVirtArray.get(5).text = "Index 5";
      data.fsecondVirtArray.get(5).value = -9.99;

      //    std::vector<TestData> test2s2;
      for (int i = 0; i < 11; i++) data.test2s2.add(new TestData());
      //    TestData ftest2s2[4];
      data.ftest2s2.get(3).text = "";
      data.ftest2s2.get(1).value = 710.6;

      data.fruitarr.add(new Fruit());
      data.fruitarr.add(new Fruit());
      data.fruitarr.get(0).value = Fruit.Value.PEAR;
      data.fruitarr.get(1).value = Fruit.Value.BANANA;

      //    Fruit ffruitarr[15];
      data.ffruitarr.get(0).value = Fruit.Value.PEAR;
      data.ffruitarr.get(14).value = Fruit.Value.PEAR;
  }

  public testallmain() {
    try
    {
      cd1 = new ChildData();
      ChildData cd2 = new ChildData();
      ChildData cd3 = new ChildData();

      // ==============================================================
      OnLog("Test initial state...");
      CheckEmpty(cd1);
      CheckEmpty(cd2);
      CheckEmpty(cd3);

      CheckObjects(cd1, cd2);
      OnLog("Finished ");

      OnLog("Test cloning...");
      FillChildData(cd1);
      cd1.fillClone(cd2);
      CheckObjects(cd1, cd2);

      OnLog("Finished ");

      // ==============================================================
      OnLog("Serialize filled object");
      ByteBuffer buffer = ByteBuffer.allocateDirect(StaticManager.MAX_SIZE);
      WriteByteBuffer buf = new WriteByteBuffer(buffer, StaticManager.MAX_SIZE, true);
      OPSArchiverOut ao = new OPSArchiverOut(buf, false);
      OnLog("  Position()= " + buf.position());
      try {
          ao.inout("data", cd1, ChildData.class);
      } catch (IOException ex) {
          OnLog("ao.inout() exception: ");
          ex.printStackTrace();
      }
      OnLog("  optNonVirt = false, Position()= " + buf.position());
      AssertEQ(buf.position(), 3150, "Serialization size error");

      buffer = ByteBuffer.allocateDirect(StaticManager.MAX_SIZE);
      buf = new WriteByteBuffer(buffer, StaticManager.MAX_SIZE, true);
      ao = new OPSArchiverOut(buf, true);
      OnLog("  Position()= " + buf.position());
      try {
          ao.inout("data", cd1, ChildData.class);
      } catch (IOException ex) {
          OnLog("ao.inout() exception: ");
          ex.printStackTrace();
      }
      OnLog("  optNonVirt = true,  Position()= " + buf.position());
      AssertEQ(buf.position(), 2591, "Serialization size error");
      OnLog("Serialize finished");


      // ==============================================================
      OnLog("Test publish/subscribe");

      File tmp = new File("ops_config.xml");
      if (tmp.exists()) {
        OnLog("Using config file in CWD\n");
      } else {
        String cwd = Paths.get(".").toAbsolutePath().normalize().toString();
        int idx = cwd.indexOf("Examples");
        if (idx > 0) {
          String cfg = cwd.substring(0, idx) + "Examples/OPSIdls/TestAll/ops_config.xml";
          OPSConfigRepository.add(cfg);
          OnLog("Using config file in: " + cfg);
        }
      }

      //Create Participant
      participant = Participant.getInstance(defaultDomain, defaultDomain);
      participant.addTypeSupport(new TestAllTypeFactory());
      participant.addListener(this);

      OnLog("Created Participant\n");
      OnLog("DomainID: " + participant.getDomain().getDomainID());
      OnLog("DomainAddress: " + participant.getDomain().getDomainAddress());
      OnLog("InSocketBufferSize: " + participant.getDomain().GetInSocketBufferSize());
      OnLog("OutSocketBufferSize: " + participant.getDomain().GetOutSocketBufferSize());
      OnLog("MetaDataMcPort: " + participant.getDomain().GetMetaDataMcPort());
      OnLog("LocalInterface: " + participant.getDomain().getLocalInterface());
      OnLog("TimeToLive: " + participant.getDomain().getTimeToLive() + "\n");

      // Setup & start subscriber w polling
      Topic topic = participant.createTopic("ChildTopic");
      ChildDataSubscriber sub = new ChildDataSubscriber(topic);
      sub.start();

      // Setup & start publisher
      ChildDataPublisher pub = new ChildDataPublisher(topic);
      pub.setName("Java");
      pub.start();

      try {
          Thread.sleep(100);
      } catch (InterruptedException e) {
          OnLog("Exception: " + e.getMessage());
      }

      // Publish data
      pub.write(cd1);

      // Check that sent data isn't affected by publish
      CheckObjects(cd1, cd2);

      // Check received values against sent values
      sub.waitForNextData(500);
      OPSMessage msg = sub.getMessage();
      if (msg != null) cd3 = (ChildData)msg.getData();
      AssertTRUE((msg != null) && (cd3 != null), "No received data");

      if ((msg != null) && (cd3 != null)) CheckObjects(cd1, cd3);

      OnLog("Finished ");
      OnLog("Wait for data (60 seconds)...");

      //Add a listener to the subscriber
      sub.addObserver(new Observer() {
          public void update(Observable o, Object arg)
          {
              OPSMessage msg = sub.getMessage();
              ChildData cd3 = null;
              if (msg != null) {
                  cd3 = (ChildData)msg.getData();
                  OnLog("Got message from: " + msg.getPublisherName());
                  int value = pubCounters.getOrDefault(msg.getPublisherName(), 0);
                  pubCounters.put(msg.getPublisherName(), value + 1);
              }
              AssertTRUE((msg != null) && (cd3 != null), "No received data");
              if ((msg != null) && (cd3 != null)) CheckObjects(cd1, cd3);
          }
      });

      for (int i = 0; i < 10; i++) {
          try {
              Thread.sleep(6000);
              pub.write(cd1);
          } catch (InterruptedException e) {
              OnLog("Exception: " + e.getMessage());
          }
      }

      pub.stop();
      sub.stop();
      participant.stopThread();
      DeadlineNotifier.getInstance().stopRunning();
      try {
        Thread.sleep(2000);
      } catch (InterruptedException e) {
          OnLog("Exception: " + e.getMessage());
      }

      OnLog("------------------------------------");

      for (String key : pubCounters.keySet()) {
        OnLog(pubCounters.get(key) + " messages from '" + key + "'");
      }

      OnLog("------------------------------------");

      try {
          if (gTestFailed) {
              OnLog("");
              OnLog("####  SOME TESTS FAILED  #### Sleeping for 20 seconds");
              OnLog("");
              Thread.sleep(20000);
          } else {
              OnLog("");
              OnLog(" T E S T   O K ");
              OnLog("");
              OnLog(" Sleeping for 15 seconds");
              OnLog("");
              Thread.sleep(15000);
          }
      } catch (InterruptedException e) {
          OnLog("Exception: " + e.getMessage());
      }
    }
    catch (ConfigurationException e)
    {
      OnLog("Exception: " + e.getMessage());
    }
  }

  public void doRun()
  {
  }

}
