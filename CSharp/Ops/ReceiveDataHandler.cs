///////////////////////////////////////////////////////////
//  ReceiveDataHandler.cs
//  Implementation of the Class ReceiveDataHandler
//  Created on:      12-nov-2011 09:25:34
//  Author:
///////////////////////////////////////////////////////////

using System;
using System.IO;
using System.Collections.Generic;       // Needed for the "List"
using System.Runtime.CompilerServices;  // Needed for the "MethodImpl" synchronization attribute
using System.Threading;
using System.Windows.Forms;
using System.Runtime.Remoting.Channels;

namespace Ops
{
    public  class ReceiveDataHandler : IDisposable
    {
        protected Participant participant;
        private List<Subscriber> subscribers = new List<Subscriber>();
        private readonly Topic topic;
        protected List<ReceiveDataChannel> channels = new List<ReceiveDataChannel>();

        public ReceiveDataHandler(Topic t, Participant part)
        {
            topic = t;
            participant = part;
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
            participant = null;
        }

        public int GetSampleMaxSize()
        {
            return topic.GetSampleMaxSize();
        }

        public string GetTransport()
        {
            return topic.GetTransport();
        }

        // Tell derived classes which topics that are active
        protected virtual void TopicUsage(Topic top, bool used)
        {
        }

        public int GetNrOfSubscribers()
        {
            return subscribers.Count;
        }

        public void AddSubscriber(Subscriber sub, Topic top)
        {
            lock (subscribers)
            {
                subscribers.Add(sub);
            }

            if (subscribers.Count == 1)
            {
                lock (channels)
                {
                    foreach (ReceiveDataChannel channel in channels)
                    {
                        channel.Start();
                    }
                }
            }

            TopicUsage(top, true);
        }

        public bool RemoveSubscriber(Subscriber sub, Topic top)
        {
            TopicUsage(top, false);

            bool result = false;
            lock (subscribers)
            {
                result = subscribers.Remove(sub);
            }

            if (subscribers.Count == 0)
            {
                lock (channels)
                {
                    foreach (ReceiveDataChannel channel in channels)
                    {
                        channel.Stop();
                    }
                }
            }

            return result;
        }

        public void OnNewMessage(OPSMessage message)
        {
            try
            {
                // Debug support
                if (Globals.TRACE_RECEIVE)
                {
                    Logger.ExceptionLogger.LogMessage("TRACE: ReceiveDataHandler.OnNewBytes() [" + topic.GetName() + "], got message");
                }

                lock (subscribers)
                {
                    //TODO: error checking
                    foreach (Subscriber subscriber in subscribers)
                    {
                        try
                        {
                            subscriber.NotifyNewOPSMessage(message);
                        }
                        catch (Exception ex)
                        {
                            Logger.ExceptionLogger.LogMessage(this.GetType().Name + ", Exception thrown in event notification thread " + ex.ToString());
                        }
                    }
                }
            }
            catch (System.IO.IOException)
            {
            }
        }

    }

    public class McReceiveDataHandler : ReceiveDataHandler
    {
        public McReceiveDataHandler(Topic t, Participant part) : base(t, part)
        {
            // Get the local interface, doing a translation from subnet if necessary
            string localIF = InetAddress.DoSubnetTranslation(t.GetLocalInterface());

            channels.Add(new ReceiveDataChannel(t, part, ReceiverFactory.CreateReceiver(t, localIF), this));
        }
    }

    public class UdpReceiveDataHandler : ReceiveDataHandler
    {
        private bool commonReceiver;

        public UdpReceiveDataHandler(Topic t, Participant part, bool commonReceiver) : base(t, part)
        {
            // Get the local interface, doing a translation from subnet if necessary
            string localIF = InetAddress.DoSubnetTranslation(t.GetLocalInterface());
            IReceiver rec = ReceiverFactory.CreateReceiver(t, localIF);

            channels.Add(new ReceiveDataChannel(t, part, rec, this));

            this.commonReceiver = commonReceiver;
            if (commonReceiver)
            {
                part.SetUdpTransportInfo(((UdpReceiver)rec).IP, ((UdpReceiver)rec).Port);
            }
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing && commonReceiver)
            {
                participant?.SetUdpTransportInfo("", 0);
                commonReceiver = false;
            }

            // Call base class implementation
            base.Dispose(disposing);
        }
    }

    public class TcpReceiveDataHandler : ReceiveDataHandler
    {
        private Topic top;
        private bool usingPartInfo = true;
        private Dictionary<string, UInt32> topicCounts = new Dictionary<string, UInt32>();

        public TcpReceiveDataHandler(Topic t, Participant part) : base(t, part)
        {
            // Make a local copy to be used when adding channels dynamically
            top = (Topic)t.Clone();

            if (t.GetPort() != 0)
            {
                channels.Add(new ReceiveDataChannel(t, part, ReceiverFactory.CreateReceiver(t, ""), this));
                usingPartInfo = false;
            }
        }

        public void AddReceiveChannel(string topicName, string ip, int port)
        {
            if (port == 0) { return; }

            string key = ip + "::" + port;

            // Look for it in channels, if not there, create one
            lock (channels)
            {
                foreach (ReceiveDataChannel chnl in channels)
                {
                    if (chnl.Key == key)
                    {
                        return;
                    }
                }

                top.SetDomainAddress(ip);
                top.SetPort(port);

                ReceiveDataChannel channel = new ReceiveDataChannel(top, participant, ReceiverFactory.CreateReceiver(top, ""), this);
                channel.Key = key;
                channels.Add(channel);

                if (GetNrOfSubscribers() > 0)
                {
                    channel.Start();
                }
            }
        }
        
        protected override void TopicUsage(Topic top, bool used)
        {
            if (usingPartInfo)
            {
                // We should only register unique topics
                UInt32 count = 0;
                if (topicCounts.ContainsKey(top.GetName()))
                {
                    count = topicCounts[top.GetName()];
                }
                // Register topic with participant info data handler/listener to get callbacks to handler above
                if (used)
                {
                    ++count;
                    if (count == 1)
                    {
                        participant.partInfoListener.ConnectTcp(top.GetName(), this);
                    }
                }
                else
                {
                    --count;
                    if (count == 0)
                    {
                        participant.partInfoListener.DisconnectTcp(top.GetName(), this);
                    }
                }
                topicCounts[top.GetName()] = count;
            }
        }

    }

}

