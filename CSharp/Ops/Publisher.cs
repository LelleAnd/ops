///////////////////////////////////////////////////////////
//  Publisher.cs
//  Implementation of the Class Publisher
//  Created on:      12-nov-2011 09:25:34
//  Author:
///////////////////////////////////////////////////////////

using System;
using System.IO;

namespace Ops 
{
	public class Publisher : IDisposable
    {
		private byte [] bytes;
		private long currentPublicationID = 0;
		//private InProcessTransport inProcessTransport;
		private string key = "";
		private string name = "";
		private Participant participant;
		private int reliableWriteNrOfResends = 1;
		private int reliableWriteTimeout = 1000;
        private long sampleTime1;
        private long sampleTime2;
        private ISendDataHandler sendDataHandler;
		private Topic topic;
        private bool started = false;

        public Publisher(Topic topic)
        {
            this.topic = topic;
            this.bytes = new byte[topic.GetSampleMaxSize()];

            this.participant = Participant.GetInstance(topic.GetDomainID(), topic.GetParticipantID());
            //this.inProcessTransport = participant.GetInProcessTransport();
            Init();
        }

        // Implement IDisposable.
        // Do not make this method virtual.
        // A derived class should not be able to override this method.
        public void Dispose()
        {
            Dispose(disposing: true);
        }

        // Dispose(bool disposing) executes in two distinct scenarios.
        // If disposing equals true, the method has been called directly
        // or indirectly by a user's code. Managed and unmanaged resources
        // can be disposed.
        // If disposing equals false, the method has been called by the
        // runtime from inside the finalizer and you should not reference
        // other objects. Only unmanaged resources can be disposed.
        protected virtual void Dispose(bool disposing)
        {
            if (disposing && (participant != null))
            {
                // Must tell the sendDataHandler that we don't need it anymore
                Stop();
                sendDataHandler = null;
                participant = null;
                topic = null;
            }
        }

        private void Init()
        {
            Start();
        }

        /// <summary>
        /// Start the publisher (necessary if it has been stopped)
        /// </summary>
        public void Start()
        {
            if (!started)
            {
                try
                {
                    // Tell the sendDataHandler that we need it
                    sendDataHandler = participant.GetSendDataHandler(topic);
                    sendDataHandler.AddPublisher(this);
                    started = true;

                    // We need our own copy since we need to update the topic
                    Topic top = (Topic)topic.Clone();

                    // Update with actual port used
                    sendDataHandler.UpdateTransportInfo(top);
                    participant.UpdatePubPartInfo(top);
                }
                catch (Exception ex)
                {
                    Logger.ExceptionLogger.LogMessage(this.GetType().Name + " Init: " + ex.ToString());
                }
            }
        }

        /// <summary>
        /// Close down the publisher (necessary when using TCP as transport)
        /// </summary>
        public void Stop()
        {
            if (started)
            {
                // Tell the sendDataHandler that we don't need it anymore
                sendDataHandler.RemovePublisher(this);
                participant.ReleaseSendDataHandler(topic);
                started = false;
            }
        }

        /// <summary>
        /// Get IP and port used as endpoint when sending a message
        /// </summary>
        /// <param name="IP"></param>
        /// <param name="port"></param>
        public void GetLocalEndpoint(ref string IP, ref int port)
        {
            sendDataHandler.GetLocalEndpoint(ref IP, ref port);
        }

        public void CheckTypeString(string typeString)
        {
            if (topic.GetTypeID() != typeString)
            {
                throw new OPSInvalidTopicException("Topic type string mismatch!");
            }
        }

        protected void Write(OPSObject opsObject)
        {
            if (opsObject == null)
            {
                throw new CommException("Trying to send OPSObject that is null!");
            }

            if (sendDataHandler == null)
            {
                Init();
            }

            if (sendDataHandler == null)
            {
                return;
            }

            this.sampleTime2 = this.sampleTime1;
            this.sampleTime1 = System.DateTime.Now.Ticks;

            OPSMessage message = new OPSMessage();
            opsObject.SetKey(this.key);
            message.SetData(opsObject);
            message.SetPublicationID(this.currentPublicationID);
            message.SetTopicName(this.topic.GetName());
            message.SetPublisherName(this.name);

            //inProcessTransport.copyAndPutMessage(message);
            WriteByteBuffer buf = new WriteByteBuffer(this.bytes, Globals.MAX_SEGMENT_SIZE);
            try
            {
                OPSArchiverOut archiverOut = new OPSArchiverOut(buf, topic.GetOptNonVirt());

                archiverOut.Inout("message", message);

                // If object has spare bytes, write them to the end of the buf
                if (opsObject.spareBytes.Length > 0)
                {
                    buf.Write(opsObject.spareBytes, 0, opsObject.spareBytes.Length);
                }
                // Finish will fill in nrOf segments in all segments.
                buf.Finish();
                int sizeToSend = buf.Position();

                sendDataHandler.SendData(this.bytes, sizeToSend, this.topic);
            }
            catch (System.IO.IOException ex)
            {
                Logger.ExceptionLogger.LogException(ex);
            }
            IncCurrentPublicationID();
        }

        public void WriteAsOPSObject(OPSObject opsObject)
        {
            Write(opsObject);
        }

        private void IncCurrentPublicationID()
        {
            if (this.currentPublicationID < long.MaxValue)
            {
                this.currentPublicationID++;
            }
            else
            {
                this.currentPublicationID = 0;
            }
        }

        public string GetName()
        {
            return this.name;
        }

        public void SetName(string name)
        {
            this.name = name;
        }

        public Topic GetTopic()
        {
            return this.topic;
        }

        public string GetKey()
        {
            return this.key;
        }

        public void SetKey(string key)
        {
            this.key = key;
        }

        public long GetCurrentPublicationID()
        {
            return this.currentPublicationID;
        }

        public int GetReliableWriteTimeout()
        {
            return this.reliableWriteTimeout;
        }

        public double GetOutboundRate()
        {
            double sampleRate = 1.0 / ((this.sampleTime1 - this.sampleTime2) / 10000000.0);
            double fakeRate = 1.0 / ((System.DateTime.Now.Ticks - this.sampleTime1) / 10000000.0);

            if (sampleRate < fakeRate)
            {
                return sampleRate;
            }
            else
            {
                return fakeRate;
            }
        }

        public void SetReliableWriteTimeout(int reliableWriteTimeout)
        {
            this.reliableWriteTimeout = reliableWriteTimeout;
        }

        public int GetReliableWriteNrOfResends()
        {
            return this.reliableWriteNrOfResends;
        }

        public void SetReliableWriteNrOfResends(int reliableWriteNrOfResends)
        {
            this.reliableWriteNrOfResends = reliableWriteNrOfResends;
        }

	}

}