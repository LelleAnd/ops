using System;
using System.IO;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using Ops;

namespace TestAll
{
    public partial class Form1 : Form
    {
        TestCode testCode = null;

        private ConcurrentQueue<string> LogList = new ConcurrentQueue<string>();

        public void safeLog(string str)
        {
            LogList.Enqueue(str);
        }

        public Form1()
        {
            testCode = new TestCode(new SafeLog(safeLog));
            InitializeComponent();
        }

        private void timerLog_Tick_1(object sender, EventArgs e)
        {
            string str;
            int counter = 0;
            while (LogList.TryDequeue(out str))
            {
                textBox1.AppendText(str);
                if (++counter > 100) break;         // Exit to let other events have their chance
            }
            if (testCode.gTestFailed) textBox1.BackColor = Color.LightSalmon;
        }

        private void Form1_Shown(object sender, EventArgs e)
        {
            testCode.DoTest();
        }

    }
}
