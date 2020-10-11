namespace OPSTest
{
    partial class Form_TestOps
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.buttonCreateParticipant = new System.Windows.Forms.Button();
            this.buttonSaveParticipantConfiguration = new System.Windows.Forms.Button();
            this.saveFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.buttonStopTransport = new System.Windows.Forms.Button();
            this.buttonCreateSubscriber = new System.Windows.Forms.Button();
            this.buttonCreatePublisher = new System.Windows.Forms.Button();
            this.textBoxMessages = new System.Windows.Forms.TextBox();
            this.buttonSendMessage = new System.Windows.Forms.Button();
            this.buttonCreateAll = new System.Windows.Forms.Button();
            this.buttonStartSubscriber = new System.Windows.Forms.Button();
            this.buttonStopSubscriber = new System.Windows.Forms.Button();
            this.groupBoxSetup = new System.Windows.Forms.GroupBox();
            this.buttonDeleteParticipant = new System.Windows.Forms.Button();
            this.buttonDeletePublisher = new System.Windows.Forms.Button();
            this.buttonDeleteSubscriber = new System.Windows.Forms.Button();
            this.groupBoxMisc = new System.Windows.Forms.GroupBox();
            this.label3 = new System.Windows.Forms.Label();
            this.button2 = new System.Windows.Forms.Button();
            this.button1 = new System.Windows.Forms.Button();
            this.checkBoxEnableTrace = new System.Windows.Forms.CheckBox();
            this.buttonGCCollect = new System.Windows.Forms.Button();
            this.groupBoxSubscriberSettings = new System.Windows.Forms.GroupBox();
            this.buttonSetDeadLineInterval = new System.Windows.Forms.Button();
            this.textBoxDeadLineInterval = new System.Windows.Forms.TextBox();
            this.labelDeadLineInterval = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.checkedListBoxTopics = new System.Windows.Forms.CheckedListBox();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.textBoxNumBytes = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.timerCyclicWrite = new System.Windows.Forms.Timer(this.components);
            this.checkBox1 = new System.Windows.Forms.CheckBox();
            this.textBoxSendPeriod = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.textBoxPizzaDataVersion = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.groupBoxSetup.SuspendLayout();
            this.groupBoxMisc.SuspendLayout();
            this.groupBoxSubscriberSettings.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // buttonCreateParticipant
            // 
            this.buttonCreateParticipant.Location = new System.Drawing.Point(6, 19);
            this.buttonCreateParticipant.Name = "buttonCreateParticipant";
            this.buttonCreateParticipant.Size = new System.Drawing.Size(111, 23);
            this.buttonCreateParticipant.TabIndex = 1;
            this.buttonCreateParticipant.Text = "Create Participant";
            this.buttonCreateParticipant.UseVisualStyleBackColor = true;
            this.buttonCreateParticipant.Click += new System.EventHandler(this.buttonCreateParticipant_Click);
            // 
            // buttonSaveParticipantConfiguration
            // 
            this.buttonSaveParticipantConfiguration.Location = new System.Drawing.Point(6, 48);
            this.buttonSaveParticipantConfiguration.Name = "buttonSaveParticipantConfiguration";
            this.buttonSaveParticipantConfiguration.Size = new System.Drawing.Size(176, 23);
            this.buttonSaveParticipantConfiguration.TabIndex = 2;
            this.buttonSaveParticipantConfiguration.Text = "Save Participant Configuration...";
            this.buttonSaveParticipantConfiguration.UseVisualStyleBackColor = true;
            this.buttonSaveParticipantConfiguration.Click += new System.EventHandler(this.buttonSaveParticipantConfiguration_Click);
            // 
            // buttonStopTransport
            // 
            this.buttonStopTransport.Location = new System.Drawing.Point(6, 19);
            this.buttonStopTransport.Name = "buttonStopTransport";
            this.buttonStopTransport.Size = new System.Drawing.Size(98, 23);
            this.buttonStopTransport.TabIndex = 3;
            this.buttonStopTransport.Text = "Stop Transport";
            this.buttonStopTransport.UseVisualStyleBackColor = true;
            this.buttonStopTransport.Click += new System.EventHandler(this.buttonStopTransport_Click);
            // 
            // buttonCreateSubscriber
            // 
            this.buttonCreateSubscriber.Location = new System.Drawing.Point(6, 48);
            this.buttonCreateSubscriber.Name = "buttonCreateSubscriber";
            this.buttonCreateSubscriber.Size = new System.Drawing.Size(111, 23);
            this.buttonCreateSubscriber.TabIndex = 4;
            this.buttonCreateSubscriber.Text = "Create Subscriber";
            this.buttonCreateSubscriber.UseVisualStyleBackColor = true;
            this.buttonCreateSubscriber.Click += new System.EventHandler(this.buttonCreateSubscriber_Click);
            // 
            // buttonCreatePublisher
            // 
            this.buttonCreatePublisher.Location = new System.Drawing.Point(6, 78);
            this.buttonCreatePublisher.Name = "buttonCreatePublisher";
            this.buttonCreatePublisher.Size = new System.Drawing.Size(111, 23);
            this.buttonCreatePublisher.TabIndex = 5;
            this.buttonCreatePublisher.Text = "Create Publisher";
            this.buttonCreatePublisher.UseVisualStyleBackColor = true;
            this.buttonCreatePublisher.Click += new System.EventHandler(this.buttonCreatePublisher_Click);
            // 
            // textBoxMessages
            // 
            this.textBoxMessages.Location = new System.Drawing.Point(12, 277);
            this.textBoxMessages.Multiline = true;
            this.textBoxMessages.Name = "textBoxMessages";
            this.textBoxMessages.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBoxMessages.Size = new System.Drawing.Size(710, 220);
            this.textBoxMessages.TabIndex = 6;
            this.textBoxMessages.WordWrap = false;
            // 
            // buttonSendMessage
            // 
            this.buttonSendMessage.Location = new System.Drawing.Point(535, 238);
            this.buttonSendMessage.Name = "buttonSendMessage";
            this.buttonSendMessage.Size = new System.Drawing.Size(109, 23);
            this.buttonSendMessage.TabIndex = 7;
            this.buttonSendMessage.Text = "Send Message";
            this.buttonSendMessage.UseVisualStyleBackColor = true;
            this.buttonSendMessage.Click += new System.EventHandler(this.buttonSendMessage_Click);
            // 
            // buttonCreateAll
            // 
            this.buttonCreateAll.Location = new System.Drawing.Point(6, 107);
            this.buttonCreateAll.Name = "buttonCreateAll";
            this.buttonCreateAll.Size = new System.Drawing.Size(228, 23);
            this.buttonCreateAll.TabIndex = 8;
            this.buttonCreateAll.Text = "Create Participant/Subscriber/Publisher";
            this.buttonCreateAll.UseVisualStyleBackColor = true;
            this.buttonCreateAll.Click += new System.EventHandler(this.buttonCreateAll_Click);
            // 
            // buttonStartSubscriber
            // 
            this.buttonStartSubscriber.Location = new System.Drawing.Point(6, 19);
            this.buttonStartSubscriber.Name = "buttonStartSubscriber";
            this.buttonStartSubscriber.Size = new System.Drawing.Size(111, 23);
            this.buttonStartSubscriber.TabIndex = 9;
            this.buttonStartSubscriber.Text = "Start Subscriber";
            this.buttonStartSubscriber.UseVisualStyleBackColor = true;
            this.buttonStartSubscriber.Click += new System.EventHandler(this.buttonStartSubscriber_Click);
            // 
            // buttonStopSubscriber
            // 
            this.buttonStopSubscriber.Location = new System.Drawing.Point(6, 47);
            this.buttonStopSubscriber.Name = "buttonStopSubscriber";
            this.buttonStopSubscriber.Size = new System.Drawing.Size(111, 23);
            this.buttonStopSubscriber.TabIndex = 10;
            this.buttonStopSubscriber.Text = "Stop Subscriber";
            this.buttonStopSubscriber.UseVisualStyleBackColor = true;
            this.buttonStopSubscriber.Click += new System.EventHandler(this.buttonStopSubscriber_Click);
            // 
            // groupBoxSetup
            // 
            this.groupBoxSetup.Controls.Add(this.buttonDeleteParticipant);
            this.groupBoxSetup.Controls.Add(this.buttonDeletePublisher);
            this.groupBoxSetup.Controls.Add(this.buttonDeleteSubscriber);
            this.groupBoxSetup.Controls.Add(this.buttonCreateParticipant);
            this.groupBoxSetup.Controls.Add(this.buttonCreateSubscriber);
            this.groupBoxSetup.Controls.Add(this.buttonCreatePublisher);
            this.groupBoxSetup.Controls.Add(this.buttonCreateAll);
            this.groupBoxSetup.Location = new System.Drawing.Point(12, 11);
            this.groupBoxSetup.Name = "groupBoxSetup";
            this.groupBoxSetup.Size = new System.Drawing.Size(240, 140);
            this.groupBoxSetup.TabIndex = 11;
            this.groupBoxSetup.TabStop = false;
            this.groupBoxSetup.Text = "Setup Participant/Subscriber/Publisher";
            // 
            // buttonDeleteParticipant
            // 
            this.buttonDeleteParticipant.Location = new System.Drawing.Point(128, 19);
            this.buttonDeleteParticipant.Name = "buttonDeleteParticipant";
            this.buttonDeleteParticipant.Size = new System.Drawing.Size(106, 23);
            this.buttonDeleteParticipant.TabIndex = 11;
            this.buttonDeleteParticipant.Text = "Delete Participant";
            this.buttonDeleteParticipant.UseVisualStyleBackColor = true;
            this.buttonDeleteParticipant.Click += new System.EventHandler(this.buttonDeleteParticipant_Click);
            // 
            // buttonDeletePublisher
            // 
            this.buttonDeletePublisher.Location = new System.Drawing.Point(128, 78);
            this.buttonDeletePublisher.Name = "buttonDeletePublisher";
            this.buttonDeletePublisher.Size = new System.Drawing.Size(106, 23);
            this.buttonDeletePublisher.TabIndex = 10;
            this.buttonDeletePublisher.Text = "Delete Publisher";
            this.buttonDeletePublisher.UseVisualStyleBackColor = true;
            this.buttonDeletePublisher.Click += new System.EventHandler(this.buttonDeletePublisher_Click);
            // 
            // buttonDeleteSubscriber
            // 
            this.buttonDeleteSubscriber.Location = new System.Drawing.Point(128, 48);
            this.buttonDeleteSubscriber.Name = "buttonDeleteSubscriber";
            this.buttonDeleteSubscriber.Size = new System.Drawing.Size(106, 23);
            this.buttonDeleteSubscriber.TabIndex = 9;
            this.buttonDeleteSubscriber.Text = "Delete Subscriber";
            this.buttonDeleteSubscriber.UseVisualStyleBackColor = true;
            this.buttonDeleteSubscriber.Click += new System.EventHandler(this.buttonDeleteSubscriber_Click);
            // 
            // groupBoxMisc
            // 
            this.groupBoxMisc.Controls.Add(this.label3);
            this.groupBoxMisc.Controls.Add(this.button2);
            this.groupBoxMisc.Controls.Add(this.button1);
            this.groupBoxMisc.Controls.Add(this.checkBoxEnableTrace);
            this.groupBoxMisc.Controls.Add(this.buttonGCCollect);
            this.groupBoxMisc.Controls.Add(this.buttonStopTransport);
            this.groupBoxMisc.Controls.Add(this.buttonSaveParticipantConfiguration);
            this.groupBoxMisc.Location = new System.Drawing.Point(529, 12);
            this.groupBoxMisc.Name = "groupBoxMisc";
            this.groupBoxMisc.Size = new System.Drawing.Size(193, 139);
            this.groupBoxMisc.TabIndex = 12;
            this.groupBoxMisc.TabStop = false;
            this.groupBoxMisc.Text = "Miscellaneous";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(131, 112);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(35, 13);
            this.label3.TabIndex = 20;
            this.label3.Text = "label3";
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(118, 77);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(64, 23);
            this.button2.TabIndex = 19;
            this.button2.Text = "button2";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(125, 18);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(57, 23);
            this.button1.TabIndex = 18;
            this.button1.Text = "Test IP";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // checkBoxEnableTrace
            // 
            this.checkBoxEnableTrace.AutoSize = true;
            this.checkBoxEnableTrace.Location = new System.Drawing.Point(10, 111);
            this.checkBoxEnableTrace.Name = "checkBoxEnableTrace";
            this.checkBoxEnableTrace.Size = new System.Drawing.Size(90, 17);
            this.checkBoxEnableTrace.TabIndex = 17;
            this.checkBoxEnableTrace.Text = "Enable Trace";
            this.checkBoxEnableTrace.UseVisualStyleBackColor = true;
            this.checkBoxEnableTrace.CheckedChanged += new System.EventHandler(this.checkBoxEnableTrace_CheckedChanged);
            // 
            // buttonGCCollect
            // 
            this.buttonGCCollect.Location = new System.Drawing.Point(6, 77);
            this.buttonGCCollect.Name = "buttonGCCollect";
            this.buttonGCCollect.Size = new System.Drawing.Size(98, 23);
            this.buttonGCCollect.TabIndex = 16;
            this.buttonGCCollect.Text = "GC Collect";
            this.buttonGCCollect.UseVisualStyleBackColor = true;
            this.buttonGCCollect.Click += new System.EventHandler(this.buttonGCCollect_Click);
            // 
            // groupBoxSubscriberSettings
            // 
            this.groupBoxSubscriberSettings.Controls.Add(this.buttonSetDeadLineInterval);
            this.groupBoxSubscriberSettings.Controls.Add(this.textBoxDeadLineInterval);
            this.groupBoxSubscriberSettings.Controls.Add(this.labelDeadLineInterval);
            this.groupBoxSubscriberSettings.Controls.Add(this.buttonStartSubscriber);
            this.groupBoxSubscriberSettings.Controls.Add(this.buttonStopSubscriber);
            this.groupBoxSubscriberSettings.Location = new System.Drawing.Point(12, 157);
            this.groupBoxSubscriberSettings.Name = "groupBoxSubscriberSettings";
            this.groupBoxSubscriberSettings.Size = new System.Drawing.Size(240, 114);
            this.groupBoxSubscriberSettings.TabIndex = 13;
            this.groupBoxSubscriberSettings.TabStop = false;
            this.groupBoxSubscriberSettings.Text = "Subscriber Settings";
            // 
            // buttonSetDeadLineInterval
            // 
            this.buttonSetDeadLineInterval.Location = new System.Drawing.Point(199, 83);
            this.buttonSetDeadLineInterval.Name = "buttonSetDeadLineInterval";
            this.buttonSetDeadLineInterval.Size = new System.Drawing.Size(36, 23);
            this.buttonSetDeadLineInterval.TabIndex = 14;
            this.buttonSetDeadLineInterval.Text = "Set";
            this.buttonSetDeadLineInterval.UseVisualStyleBackColor = true;
            this.buttonSetDeadLineInterval.Click += new System.EventHandler(this.buttonSetDeadLineInterval_Click);
            // 
            // textBoxDeadLineInterval
            // 
            this.textBoxDeadLineInterval.Location = new System.Drawing.Point(128, 83);
            this.textBoxDeadLineInterval.Name = "textBoxDeadLineInterval";
            this.textBoxDeadLineInterval.Size = new System.Drawing.Size(65, 20);
            this.textBoxDeadLineInterval.TabIndex = 14;
            this.textBoxDeadLineInterval.Text = "0";
            // 
            // labelDeadLineInterval
            // 
            this.labelDeadLineInterval.AutoSize = true;
            this.labelDeadLineInterval.Location = new System.Drawing.Point(11, 87);
            this.labelDeadLineInterval.Name = "labelDeadLineInterval";
            this.labelDeadLineInterval.Size = new System.Drawing.Size(119, 13);
            this.labelDeadLineInterval.TabIndex = 14;
            this.labelDeadLineInterval.Text = "Dead Line Interval [ms]:";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.checkedListBoxTopics);
            this.groupBox1.Location = new System.Drawing.Point(258, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(265, 259);
            this.groupBox1.TabIndex = 14;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Actions apply to checked Topics";
            // 
            // checkedListBoxTopics
            // 
            this.checkedListBoxTopics.FormattingEnabled = true;
            this.checkedListBoxTopics.Location = new System.Drawing.Point(12, 16);
            this.checkedListBoxTopics.Name = "checkedListBoxTopics";
            this.checkedListBoxTopics.Size = new System.Drawing.Size(247, 229);
            this.checkedListBoxTopics.TabIndex = 3;
            // 
            // timer1
            // 
            this.timer1.Enabled = true;
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // textBoxNumBytes
            // 
            this.textBoxNumBytes.Location = new System.Drawing.Point(663, 207);
            this.textBoxNumBytes.Name = "textBoxNumBytes";
            this.textBoxNumBytes.Size = new System.Drawing.Size(59, 20);
            this.textBoxNumBytes.TabIndex = 16;
            this.textBoxNumBytes.Text = "0";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(529, 210);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(131, 13);
            this.label1.TabIndex = 17;
            this.label1.Text = "Extra VessuvioData bytes:";
            // 
            // timerCyclicWrite
            // 
            this.timerCyclicWrite.Interval = 1000;
            this.timerCyclicWrite.Tick += new System.EventHandler(this.timerCyclicWrite_Tick);
            // 
            // checkBox1
            // 
            this.checkBox1.AutoSize = true;
            this.checkBox1.Location = new System.Drawing.Point(663, 242);
            this.checkBox1.Name = "checkBox1";
            this.checkBox1.Size = new System.Drawing.Size(48, 17);
            this.checkBox1.TabIndex = 18;
            this.checkBox1.Text = "Auto";
            this.checkBox1.UseVisualStyleBackColor = true;
            this.checkBox1.CheckedChanged += new System.EventHandler(this.checkBox1_CheckedChanged);
            // 
            // textBoxSendPeriod
            // 
            this.textBoxSendPeriod.Location = new System.Drawing.Point(663, 180);
            this.textBoxSendPeriod.Name = "textBoxSendPeriod";
            this.textBoxSendPeriod.Size = new System.Drawing.Size(59, 20);
            this.textBoxSendPeriod.TabIndex = 19;
            this.textBoxSendPeriod.Text = "1000";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(570, 183);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(90, 13);
            this.label2.TabIndex = 20;
            this.label2.Text = "Send Period [ms]:";
            // 
            // textBoxPizzaDataVersion
            // 
            this.textBoxPizzaDataVersion.Location = new System.Drawing.Point(663, 154);
            this.textBoxPizzaDataVersion.Name = "textBoxPizzaDataVersion";
            this.textBoxPizzaDataVersion.Size = new System.Drawing.Size(59, 20);
            this.textBoxPizzaDataVersion.TabIndex = 21;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(564, 157);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(96, 13);
            this.label4.TabIndex = 22;
            this.label4.Text = "PizzaData Version:";
            // 
            // Form_TestOps
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(734, 509);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.textBoxPizzaDataVersion);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.textBoxSendPeriod);
            this.Controls.Add(this.checkBox1);
            this.Controls.Add(this.textBoxNumBytes);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.groupBoxSubscriberSettings);
            this.Controls.Add(this.groupBoxMisc);
            this.Controls.Add(this.groupBoxSetup);
            this.Controls.Add(this.buttonSendMessage);
            this.Controls.Add(this.textBoxMessages);
            this.Name = "Form_TestOps";
            this.Text = "TEST Ops";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.Form_TestOps_FormClosed);
            this.Load += new System.EventHandler(this.Form_TestOps_Load);
            this.groupBoxSetup.ResumeLayout(false);
            this.groupBoxMisc.ResumeLayout(false);
            this.groupBoxMisc.PerformLayout();
            this.groupBoxSubscriberSettings.ResumeLayout(false);
            this.groupBoxSubscriberSettings.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button buttonCreateParticipant;
        private System.Windows.Forms.Button buttonSaveParticipantConfiguration;
        private System.Windows.Forms.SaveFileDialog saveFileDialog;
        private System.Windows.Forms.Button buttonStopTransport;
        private System.Windows.Forms.Button buttonCreateSubscriber;
        private System.Windows.Forms.Button buttonCreatePublisher;
        private System.Windows.Forms.TextBox textBoxMessages;
        private System.Windows.Forms.Button buttonSendMessage;
        private System.Windows.Forms.Button buttonCreateAll;
        private System.Windows.Forms.Button buttonStartSubscriber;
        private System.Windows.Forms.Button buttonStopSubscriber;
        private System.Windows.Forms.GroupBox groupBoxSetup;
        private System.Windows.Forms.GroupBox groupBoxMisc;
        private System.Windows.Forms.GroupBox groupBoxSubscriberSettings;
        private System.Windows.Forms.Button buttonSetDeadLineInterval;
        private System.Windows.Forms.TextBox textBoxDeadLineInterval;
        private System.Windows.Forms.Label labelDeadLineInterval;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Timer timer1;
        private System.Windows.Forms.CheckedListBox checkedListBoxTopics;
        private System.Windows.Forms.Button buttonDeleteSubscriber;
        private System.Windows.Forms.Button buttonGCCollect;
        private System.Windows.Forms.Button buttonDeletePublisher;
        private System.Windows.Forms.TextBox textBoxNumBytes;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Timer timerCyclicWrite;
        private System.Windows.Forms.CheckBox checkBox1;
        private System.Windows.Forms.TextBox textBoxSendPeriod;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.CheckBox checkBoxEnableTrace;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.Button buttonDeleteParticipant;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox textBoxPizzaDataVersion;
        private System.Windows.Forms.Label label4;
    }
}

