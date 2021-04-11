using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Timers;
using Ops;

namespace TestAll
{

    public delegate void SafeLog(string str);

    public class TestCode
    {
        private static System.Timers.Timer aTimer;
        public bool gTestFailed = false;
        Participant participant = null;
        ChildDataSubscriber sub = null;
        ChildDataPublisher pub = null;
        ChildData cd1 = null;

        SafeLog safeLog = null;

        public void Log(string str)
        {
            safeLog(str);
        }

        public void LogNL(string str)
        {
            safeLog(str + "\r\n");
        }

        public void LogError(string str)
        {
            gTestFailed = true;
            LogNL("###### " + str);
        }

        class MyLogger : LoggerImplementation
        {
            TestCode owner = null;
            public MyLogger(TestCode own)
            {
                owner = own;
            }
            override public void LogError(string error)
            {
                if (owner != null)
                {
                    owner.LogError(error);
                }
            }
        }

        MyLogger logger = null;

        public TestCode(SafeLog log)
        {
            safeLog = log;
            logger = new MyLogger(this);
        }

        // ===========================================================

        public bool AssertTRUE(bool val, string mess = "")
        {
            if (!val) LogError("Failed: " + mess + ", value= " + val);
            return val;
        }

        public void AssertEQ(string val, string exp, string mess = "")
        {
            if (String.IsNullOrEmpty(val) && String.IsNullOrEmpty(exp)) return;
            if (val != exp) LogError("Failed: " + mess + ", value= " + val + ", expected= " + exp);
        }

        public void AssertEQ(bool val, bool exp, string mess = "")
        {
            if (val != exp) LogError("Failed: " + mess + ", value= " + val + ", expected= " + exp);
        }

        public bool AssertEQ(long val, long exp, string mess = "")
        {
            if (val != exp) LogError("Failed: " + mess + ", value= " + val + ", expected= " + exp);
            return val == exp;
        }

        public void AssertEQ(double val, double exp, string mess = "")
        {
            if (val != exp) LogError("Failed: " + mess + ", value= " + val + ", expected= " + exp);
        }

        public void AssertEQ(ChildData.Order val, ChildData.Order exp, string mess = "")
        {
            if (!val.Equals(exp)) LogError("Failed: " + mess + ", value= " + val + ", expected= " + exp);
        }

        public void AssertEQ(Definitions.Command val, Definitions.Command exp, string mess = "")
        {
            if (!val.Equals(exp)) LogError("Failed: " + mess + ", value= " + val + ", expected= " + exp);
        }

        public void AssertEQ(Fruit.Value val, Fruit.Value exp, string mess = "")
        {
            if (!val.Equals(exp)) LogError("Failed: " + mess + ", value= " + val + ", expected= " + exp);
        }

        public bool AssertCountEQ<T>(List<T> val, int exp, string mess = "")
        {
            if (AssertTRUE(val != null, mess))
            {
                return AssertEQ(val.Count(), exp, mess);
            }
            return false;
        }

        public bool AssertCountEQ<T>(List<T> val, List<T> exp, string mess = "")
        {
            if (AssertTRUE(val != null, mess) && AssertTRUE(exp != null, mess))
            {
                return AssertEQ(val.Count(), exp.Count(), mess);
            }
            return false;
        }

        public bool AssertCountEQ<T>(List<T> val, List<T> exp, int cnt, string mess = "")
        {
            if (AssertTRUE(val != null, mess) && AssertTRUE(exp != null, mess))
            {
                if (AssertEQ(val.Count(), cnt, mess))
                {
                    return AssertEQ(val.Count(), exp.Count(), mess);
                }
            }
            return false;
        }

        // ===========================================================

        public void CheckEmpty(TestData data)
        {
            if (AssertTRUE(data != null, "TestData ref is null"))
            {
                AssertEQ(data.text, "");
                AssertEQ(data.value, 0.0);
            }
        }

        public void CheckEmpty(Fruit data)
        {
            if (AssertTRUE(data != null, "Fruit ref is null"))
            {
                AssertEQ(data.value, Fruit.Value.APPLE);
            }
        }

        public void CheckEmpty(ChildData data)
        {
            LogNL("Checking empty object...");

            if (!AssertTRUE(data != null)) return;

            // BaseData
            //   std::string baseText;
            AssertEQ(data.baseText, "");
            //   std::vector<std::string> stringOpenArr;
            AssertCountEQ(data.stringOpenArr, 0);
            //   std::string stringFixArr[5];
            if (AssertCountEQ(data.stringFixArr, 5))
            {
                for (int i = 0; i < 5; i++) AssertEQ(data.stringFixArr[i], "");
            }
            //   ops::strings::fixed_string<23> fixLengthString;
            AssertEQ(data.fixLengthString, "");
            //   std::vector<ops::strings::fixed_string<16>> fixLengthStringOpenArr;
            AssertCountEQ(data.fixLengthStringOpenArr, 0);
            //   ops::strings::fixed_string<16> fixLengthStringFixArr[10];
            if (AssertCountEQ(data.fixLengthStringFixArr, 10))
            {
                for (int i = 0; i < 10; i++) AssertEQ(data.fixLengthStringFixArr[i], "");
            }

            // ChildData
            //  enums
            AssertEQ(data.enu1, ChildData.Order.ABC);
            AssertCountEQ(data.enuVec, 0);
            if (AssertCountEQ(data.enuFixArr, 6))
            {
                for (int i = 0; i < 6; i++) AssertEQ(data.enuFixArr[i], ChildData.Order.ABC);
            }
            AssertEQ(data.cmd, Definitions.Command.START);
            if (AssertCountEQ(data.cmds, 2))
            {
                for (int i = 0; i < 2; i++) AssertEQ(data.cmds[i], Definitions.Command.START);
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
            if (AssertCountEQ(data.fbos, 11))
            {
                for (int i = 0; i < 11; i++) AssertEQ(data.fbos[i], false);
            }

            AssertCountEQ(data.bs, 0);
            //	char fbs[256];
            if (AssertCountEQ(data.fbs, 256))
            {
                for (int i = 0; i < 256; i++) AssertEQ(data.fbs[i], 0);
            }

            AssertCountEQ(data.shs, 0);
            //	short fshs[4];
            if (AssertCountEQ(data.fshs, 4))
            {
                for (int i = 0; i < 4; i++) AssertEQ(data.fshs[i], 0);
            }

            AssertCountEQ(data.is_, 0);
            //	int fis_[3];
            if (AssertCountEQ(data.fis_, 3))
            {
                for (int i = 0; i < 3; i++) AssertEQ(data.fis_[i], 0);
            }

            AssertCountEQ(data.ls, 0);
            //	int64_t fls[6];
            if (AssertCountEQ(data.fls, 6))
            {
                for (int i = 0; i < 6; i++) AssertEQ(data.fls[i], 0);
            }

            AssertCountEQ(data.fs, 0);
            //    float ffs[77];
            if (AssertCountEQ(data.ffs, 77))
            {
                for (int i = 0; i < 77; i++) AssertEQ(data.ffs[i], 0.0);
            }

            AssertCountEQ(data.ds, 0);
            //    double fds[5];
            if (AssertCountEQ(data.fds, 5))
            {
                for (int i = 0; i < 5; i++) AssertEQ(data.fds[i], 0.0);
            }

            AssertCountEQ(data.ss, 0);
            //    std::string fss[10];
            if (AssertCountEQ(data.fss, 10))
            {
                for (int i = 0; i < 10; i++) AssertEQ(data.fss[i], "");
            }

            AssertCountEQ(data.test2s, 0);
            //    TestData* ftest2s[5];
            if (AssertCountEQ(data.ftest2s, 5))
            {
                for (int i = 0; i < 5; i++) CheckEmpty(data.ftest2s[i]);
            }

            AssertCountEQ(data.secondVirtArray, 0);
            //    TestData* fsecondVirtArray[7];
            if (AssertCountEQ(data.fsecondVirtArray, 7))
            {
                for (int i = 0; i < 7; i++) CheckEmpty(data.fsecondVirtArray[i]);
            }

            AssertCountEQ(data.test2s2, 0);
            //    TestData ftest2s2[4];
            if (AssertCountEQ(data.ftest2s2, 4))
            {
                for (int i = 0; i < 4; i++) CheckEmpty(data.ftest2s2[i]);
            }

            AssertCountEQ(data.fruitarr, 0);
            //    Fruit ffruitarr[15];
            if (AssertCountEQ(data.ffruitarr, 15))
            {
                for (int i = 0; i < 15; i++) CheckEmpty(data.ffruitarr[i]);
            }
        }

        public void CheckObjects(TestData data, TestData exp)
        {
            if (AssertTRUE(data != null, "TestData 'data' ref is null") && AssertTRUE(exp != null, "TestData 'exp' ref is null"))
            {
                AssertEQ(data.text, exp.text);
                AssertEQ(data.value, exp.value);
            }
        }

        public void CheckObjects(Fruit data, Fruit exp)
        {
            if (AssertTRUE(data != null, "Fruit 'data' ref is null") && AssertTRUE(exp != null, "Fruit 'exp' ref is null"))
            {
                AssertEQ(data.value, exp.value);
            }
        }

        public void CheckObjects(ChildData data, ChildData exp)
        {
            LogNL("Comparing objects object...");

            if (!AssertTRUE(data != null) || !AssertTRUE(exp != null)) return;

            // BaseData
            //   std::string baseText;
            AssertEQ(data.baseText, exp.baseText);

            //   std::vector<std::string> stringOpenArr;
            if (AssertCountEQ(data.stringOpenArr, exp.stringOpenArr))
            {
                for (int i = 0; i < data.stringOpenArr.Count(); i++) AssertEQ(data.stringOpenArr[i], exp.stringOpenArr[i], "stringOpenArr");
            }

            //   std::string stringFixArr[5];
            if (AssertCountEQ(data.stringFixArr, exp.stringFixArr, 5))
            {
                for (int i = 0; i < 5; i++) AssertEQ(data.stringFixArr[i], exp.stringFixArr[i]);
            }

            //   ops::strings::fixed_string<23> fixLengthString;
            AssertEQ(data.fixLengthString, exp.fixLengthString);

            //   std::vector<ops::strings::fixed_string<16>> fixLengthStringOpenArr;
            if (AssertCountEQ(data.fixLengthStringOpenArr, exp.fixLengthStringOpenArr))
            {
                for (int i = 0; i < data.fixLengthStringOpenArr.Count(); i++) AssertEQ(data.fixLengthStringOpenArr[i], exp.fixLengthStringOpenArr[i], "fixLengthStringOpenArr");
            }

            //   ops::strings::fixed_string<16> fixLengthStringFixArr[10];
            if (AssertCountEQ(data.fixLengthStringFixArr, exp.fixLengthStringFixArr, 10))
            {
                for (int i = 0; i < 10; i++) AssertEQ(data.fixLengthStringFixArr[i], exp.fixLengthStringFixArr[i]);
            }

            // ChildData
            //  enums
            AssertEQ(data.enu1, exp.enu1);
            if (AssertCountEQ(data.enuVec, exp.enuVec, "enuVec"))
            {
                for (int i = 0; i < data.enuVec.Count(); i++) AssertEQ(data.enuVec[i], exp.enuVec[i], "enuVec");
            }
            if (AssertCountEQ(data.enuFixArr, exp.enuFixArr, 6, "enuFixArr"))
            {
                for (int i = 0; i < 6; i++) AssertEQ(data.enuFixArr[i], exp.enuFixArr[i], "enuFixArr");
            }
            AssertEQ(data.cmd, exp.cmd);
            if (AssertCountEQ(data.cmds, 2))
            {
                for (int i = 0; i < 2; i++) AssertEQ(data.cmds[i], exp.cmds[i]);
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

            if (AssertCountEQ(data.bos, exp.bos))
            {
                for (int i = 0; i < data.bos.Count(); i++) AssertEQ(data.bos[i], exp.bos[i]);
            }
            //	bool fbos[11];
            if (AssertCountEQ(data.fbos, exp.fbos, 11))
            {
                for (int i = 0; i < 11; i++) AssertEQ(data.fbos[i], exp.fbos[i]);
            }

            if (AssertCountEQ(data.bs, exp.bs))
            {
                for (int i = 0; i < data.bs.Count(); i++) AssertEQ(data.bs[i], exp.bs[i]);
            }
            //	char fbs[256];
            if (AssertCountEQ(data.fbs, exp.fbs, 256))
            {
                for (int i = 0; i < 256; i++) AssertEQ(data.fbs[i], exp.fbs[i]);
            }

            if (AssertCountEQ(data.shs, exp.shs))
            {
                for (int i = 0; i < data.shs.Count(); i++) AssertEQ(data.shs[i], exp.shs[i]);
            }
            //	short fshs[4];
            if (AssertCountEQ(data.fshs, exp.fshs, 4))
            {
                for (int i = 0; i < 4; i++) AssertEQ(data.fshs[i], exp.fshs[i]);
            }

            if (AssertCountEQ(data.is_, exp.is_))
            {
                for (int i = 0; i < data.is_.Count(); i++) AssertEQ(data.is_[i], exp.is_[i]);
            }
            //	int fis_[3];
            if (AssertCountEQ(data.fis_, exp.fis_, 3))
            {
                for (int i = 0; i < 3; i++) AssertEQ(data.fis_[i], exp.fis_[i]);
            }

            if (AssertCountEQ(data.ls, exp.ls))
            {
                for (int i = 0; i < data.ls.Count(); i++) AssertEQ(data.ls[i], exp.ls[i]);
            }
            //	int64_t fls[6];
            if (AssertCountEQ(data.fls, exp.fls, 6))
            {
                for (int i = 0; i < 6; i++) AssertEQ(data.fls[i], exp.fls[i]);
            }

            if (AssertCountEQ(data.fs, exp.fs))
            {
                for (int i = 0; i < data.fs.Count(); i++) AssertEQ(data.fs[i], exp.fs[i]);
            }
            //    float ffs[77];
            if (AssertCountEQ(data.ffs, exp.ffs, 77))
            {
                for (int i = 0; i < 77; i++) AssertEQ(data.ffs[i], exp.ffs[i]);
            }

            if (AssertCountEQ(data.ds, exp.ds))
            {
                for (int i = 0; i < data.ds.Count(); i++) AssertEQ(data.ds[i], exp.ds[i]);
            }
            //    double fds[5];
            if (AssertCountEQ(data.fds, exp.fds, 5))
            {
                for (int i = 0; i < 5; i++) AssertEQ(data.fds[i], exp.fds[i]);
            }

            if (AssertCountEQ(data.ss, exp.ss))
            {
                for (int i = 0; i < data.ss.Count(); i++) AssertEQ(data.ss[i], exp.ss[i]);
            }
            //    std::string fss[10];
            if (AssertCountEQ(data.fss, exp.fss, 10))
            {
                for (int i = 0; i < 10; i++) AssertEQ(data.fss[i], exp.fss[i]);
            }

            if (AssertCountEQ(data.test2s, exp.test2s))
            {
                for (int i = 0; i < data.test2s.Count(); i++) CheckObjects(data.test2s[i], exp.test2s[i]);
            }
            //    TestData* ftest2s[5];
            if (AssertCountEQ(data.ftest2s, exp.ftest2s, 5))
            {
                for (int i = 0; i < 5; i++) CheckObjects(data.ftest2s[i], exp.ftest2s[i]);
            }

            if (AssertCountEQ(data.secondVirtArray, exp.secondVirtArray))
            {
                for (int i = 0; i < data.secondVirtArray.Count(); i++) CheckObjects(data.secondVirtArray[i], exp.secondVirtArray[i]);
            }
            //    TestData* fsecondVirtArray[7];
            if (AssertCountEQ(data.fsecondVirtArray, exp.fsecondVirtArray, 7))
            {
                for (int i = 0; i < 7; i++) CheckObjects(data.fsecondVirtArray[i], exp.fsecondVirtArray[i]);
            }

            if (AssertCountEQ(data.test2s2, exp.test2s2))
            {
                for (int i = 0; i < data.test2s2.Count(); i++) CheckObjects(data.test2s2[i], exp.test2s2[i]);
            }
            //    TestData ftest2s2[4];
            if (AssertCountEQ(data.ftest2s2, exp.ftest2s2, 4))
            {
                for (int i = 0; i < 4; i++) CheckObjects(data.ftest2s2[i], exp.ftest2s2[i]);
            }

            if (AssertCountEQ(data.fruitarr, exp.fruitarr))
            {
                for (int i = 0; i < data.fruitarr.Count(); i++) AssertEQ(data.fruitarr[i].value, exp.fruitarr[i].value);
            }
            //    Fruit ffruitarr[15];
            if (AssertCountEQ(data.ffruitarr, exp.ffruitarr, 15))
            {
                for (int i = 0; i < 15; i++) AssertEQ(data.ffruitarr[i].value, exp.ffruitarr[i].value);
            }
        }


        public void FillChildData(ChildData data)
        {
            // Data for fields in BaseData
            //   std::string baseText;
            data.baseText = "dynamic string";
            //   std::vector<std::string> stringOpenArr;
            data.stringOpenArr.Add("dyn str 1");
            data.stringOpenArr.Add("dyn str 2");
            //   std::string stringFixArr[5];
            data.stringFixArr[0] = "dsf 0";
            data.stringFixArr[1] = "dsf 1";
            data.stringFixArr[2] = "dsf 2";
            data.stringFixArr[3] = "dsf 3";
            data.stringFixArr[4] = "dsf 4";
            //   ops::strings::fixed_string<23> fixLengthString;
            data.fixLengthString = "fixed length string";
            //   std::vector<ops::strings::fixed_string<16>> fixLengthStringOpenArr;
            data.fixLengthStringOpenArr.Add("fix len str 1");
            data.fixLengthStringOpenArr.Add("fix len str 2");
            //   ops::strings::fixed_string<16> fixLengthStringFixArr[10];
            data.fixLengthStringFixArr[0] = "fsf 0";
            data.fixLengthStringFixArr[1] = "fsf 1";
            data.fixLengthStringFixArr[2] = "fsf 2";
            data.fixLengthStringFixArr[3] = "fsf 3";
            data.fixLengthStringFixArr[4] = "fsf 4";
            data.fixLengthStringFixArr[5] = "fsf 5";
            data.fixLengthStringFixArr[6] = "fsf 6";
            data.fixLengthStringFixArr[7] = "fsf 7";
            data.fixLengthStringFixArr[8] = "fsf 8";
            data.fixLengthStringFixArr[9] = "fsf 9";

            // Data for fields in ChildData
            //  enums
            data.enu1 = ChildData.Order.GHI;

            data.enuVec.Add(ChildData.Order.GHI);
            data.enuVec.Add(ChildData.Order.JKL);
            data.enuVec.Add(ChildData.Order.JKL);
            data.enuVec.Add(ChildData.Order.ABC);
            data.enuVec.Add(ChildData.Order.DEF);

            data.enuFixArr[0] = ChildData.Order.DEF;
            data.enuFixArr[4] = ChildData.Order.JKL;
            data.enuFixArr[5] = ChildData.Order.DEF;

            data.cmd = Definitions.Command.CONTINUE;
            data.cmds[0] = Definitions.Command.PAUSE;
            data.cmds[1] = Definitions.Command.STOP;

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

            data.bos.Add(false);
            data.bos.Add(true);
            data.bos.Add(false);

            //	bool fbos[11];
            data.fbos[5] = true;
            data.fbos[10] = true;

            data.bs.Add(10);
            data.bs.Add(20);
            data.bs.Add(30);

            //	char fbs[256];
            for (int i = 0; i < 256; i++) data.fbs[i] = (byte)i;

            data.shs.Add(1111);
            data.shs.Add(2222);

            //	short fshs[4];
            data.fshs[0] = 21;
            data.fshs[1] = 121;
            data.fshs[2] = 221;
            data.fshs[3] = 321;

            data.is_.Add(100000);
            data.is_.Add(101010);
            data.is_.Add(110101);
            data.is_.Add(111111);

            //	int fis_[3];
            data.fis_[0] = -1;
            data.fis_[1] = -2;
            data.fis_[2] = -3;

            data.ls.Add(9);
            data.ls.Add(8);
            data.ls.Add(7);
            data.ls.Add(6);

            //	int64_t fls[6];
            data.fls[0] = 9999;
            data.fls[1] = 9998;
            data.fls[2] = 9997;
            data.fls[3] = 9996;
            data.fls[4] = 9995;
            data.fls[5] = 9994;

            data.fs.Add(3.1f);
            data.fs.Add(31.14f);
            data.fs.Add(4.56f);
            data.fs.Add(987.0f);

            //    float ffs[77];
            for (int i = 0; i < 77; i++) data.ffs[i] = 0.0f;
            data.ffs[21] = 3.1415f;

            data.ds.Add(1.987654321);
            data.ds.Add(2.3456789);

            //    double fds[5];
            data.fds[0] = 1.1;
            data.fds[1] = 2.1;
            data.fds[2] = 3.1;
            data.fds[3] = 4.1;
            data.fds[4] = 5.1;

            data.ss.Add("Index 0");
            data.ss.Add("Index 1");
            data.ss.Add("Index 2");

            //    std::string fss[10];
            data.fss[4] = "4 string";
            data.fss[7] = "7 string";
            data.fss[9] = "9 string";

            //    std::vector<TestData*> test2s;
            data.test2s.Add(new TestData());
            data.test2s.Add(new TestData());
            data.test2s.Add(new TestData());
            data.test2s.Add(new TestData());
            //    TestData* ftest2s[5];
            data.ftest2s[2].text = "Index 2";
            data.ftest2s[2].value = 7.7;

            //    std::vector<TestData*> secondVirtArray;
            data.secondVirtArray.Add(new TestData());
            data.secondVirtArray.Add(new TestData());
            //    TestData* fsecondVirtArray[7];
            data.fsecondVirtArray[5].text = "Index 5";
            data.fsecondVirtArray[5].value = -9.99;

            //    std::vector<TestData> test2s2;
            for (int i = 0; i < 11; i++) data.test2s2.Add(new TestData());
            //    TestData ftest2s2[4];
            data.ftest2s2[3].text = "";
            data.ftest2s2[1].value = 710.6;

            data.fruitarr.Add(new Fruit());
            data.fruitarr.Add(new Fruit());
            data.fruitarr[0].value = Fruit.Value.PEAR;
            data.fruitarr[1].value = Fruit.Value.BANANA;

            //    Fruit ffruitarr[15];
            data.ffruitarr[0].value = Fruit.Value.PEAR;
            data.ffruitarr[14].value = Fruit.Value.PEAR;
        }


        public void DoTest()
        {
            // ===========================================
            cd1 = new ChildData();
            ChildData cd2 = new ChildData();
            ChildData cd3 = new ChildData();

            // ==============================================================
            LogNL("Test initial state...");
            CheckEmpty(cd1);
            CheckEmpty(cd2);
            CheckEmpty(cd3);

            CheckObjects(cd1, cd2);
            LogNL("Finished ");

            LogNL("Test cloning...");
            FillChildData(cd1);
            cd1.FillClone(cd2);
            CheckObjects(cd1, cd2);

            LogNL("Finished ");

            // ==============================================================
            LogNL("Serialize filled object");
            byte[] bytes = new byte[Globals.MAX_SEGMENT_SIZE];
            WriteByteBuffer buf = new WriteByteBuffer(bytes, Globals.MAX_SEGMENT_SIZE, true);
            OPSArchiverOut ao = new OPSArchiverOut(buf, false);
            LogNL("  Position()= " + buf.Position());
            ao.Inout<ChildData>("data", cd1);
            LogNL("  optNonVirt = false, Position()= " + buf.Position());
            AssertEQ(buf.Position(), 3150, "Serialized size error");

            buf = new WriteByteBuffer(bytes, Globals.MAX_SEGMENT_SIZE, true);
            ao = new OPSArchiverOut(buf, true);
            LogNL("  Position()= " + buf.Position());
            ao.Inout<ChildData>("data", cd1);
            LogNL("  optNonVirt = true,  Position()= " + buf.Position());
            AssertEQ(buf.Position(), 2591, "Serialized size error");
            LogNL("Serialize finished");

            // Create a new stream to write to the file
            //BinaryWriter Writer = new BinaryWriter(File.OpenWrite(@"csharp-dump-opt.bin"));
            //Writer.Write(bytes, 0, buf.Position());
            //Writer.Flush();
            //Writer.Close();

            // ==============================================================
            LogNL("Test publish/subscribe");

            Logger.ExceptionLogger.AddLogger(logger);

            if (File.Exists("ops_config.xml"))
            {
                LogNL("Using config file in CWD");
            }
            else
            {
                string cwd = Environment.CurrentDirectory;
                int pos = cwd.IndexOf("Example");
                if (pos > 0)
                {
                    cwd = cwd.Substring(0, pos) + "Examples/OPSIdls/TestAll/ops_config.xml";
                    LogNL("Using config file: " + cwd);
                    OPSConfigRepository.Add(cwd);
                }
            }

            participant = Participant.GetInstance("TestAllDomain");
            if (participant == null)
            {
                LogNL("Create participant failed. do you have ops_config.xml on your rundirectory?");
                return;
            }
            participant.AddTypeSupport(new TestAllTypeFactory());

            {
                // Setup & start subscriber w polling
                Topic topic = participant.CreateTopic("ChildTopic");
                sub = new ChildDataSubscriber(topic);
                sub.Start();

                // Setup & start publisher
                pub = new ChildDataPublisher(topic);
                pub.SetName("C#");
                pub.Start();

                Thread.Sleep(100);

                // Publish data
                pub.Write(cd1);

                // Check that sent data isn't affected by publish
                CheckObjects(cd1, cd2);

                // Check received values against sent values
                sub.WaitForNextData(500);
                OPSMessage msg = sub.GetMessage();
                if (msg != null) cd3 = (ChildData)msg.GetData();
                AssertTRUE((msg != null) && (cd3 != null), "No received data");

                if ((msg != null) && (cd3 != null)) CheckObjects(cd1, cd3);

                LogNL("Finished ");

                sub.newData += new ChildDataEventHandler(SubscriberNewData);

                // Create a timer with a 5 second interval.
                aTimer = new System.Timers.Timer(5000);
                // Hook up the Elapsed event for the timer. 
                aTimer.Elapsed += OnTimedEvent;
                aTimer.AutoReset = true;
                aTimer.Enabled = true;
            }
        }

        private void OnTimedEvent(Object source, ElapsedEventArgs e)
        {
            // Publish data
            pub.Write(cd1);
        }

        private void SubscriberNewData(ChildDataSubscriber sender, ChildData data)
        {
            OPSMessage msg = sender.GetMessage();
            if (msg != null)
            {
                ChildData cd3 = (ChildData)msg.GetData();
                AssertTRUE(cd3 != null, "No received data (" + msg.GetPublisherName() + ")");
                if (cd3 != null)
                {
                    Log("Received Data from " + msg.GetPublisherName() + ": ");
                    CheckObjects(cd1, cd3);
                }
            }
            else
            {
                LogError("Callback without message");
            }
        }

    }

}
