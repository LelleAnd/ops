using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;

namespace TestAll
{
    class MyLogger
    {
        public void MyLog(string str)
        {
            Console.Write(str);
        }
    }

    static class VerifySerDes
    {
        [STAThread]
        static void Main()
        {
            MyLogger myl = new MyLogger();
            TestCode testCode = new TestCode(new SafeLog(myl.MyLog));
            testCode.DoTest();

            Thread.Sleep(60 * 1000);

            Console.WriteLine();
            if (testCode.gTestFailed) {
                Console.WriteLine("T e s t i n g   F A I L E D  !!!");
            } else {
                Console.WriteLine("T e s t i n g   O K");
            }
            Console.WriteLine();
            Console.WriteLine("Sleeping for 5 seconds ...");
            Thread.Sleep(5 * 1000);

        }
    }
}
