    ///////////////////////////////////////////////////////////
//  Subscriber.cs
//  Implementation of the Class Subscriber
//  Created on:      12-nov-2011 09:25:36
//  Author:
///////////////////////////////////////////////////////////

using System.Runtime.CompilerServices;  // Needed for the "MethodImpl" synchronization attribute
using System.Collections.Generic;       // Needed for the "List"
using System.Threading;                 // Needed for the "Monitor"

namespace Ops 
{
    public delegate void NewDataDefaultEventHandler(Subscriber sender, OPSObject data);
    
    public class Subscriber
    {
        public event NewDataDefaultEventHandler newDataDefault;
        private OPSObject data;
		public Event deadlineEvent = new Event();
		private readonly DeadlineNotifier deadlineNotifier;
		private long deadlineTimeout = 0;
		private List<IFilterQoSPolicy> filterQoSPolicies = new List<IFilterQoSPolicy>();
		private string identity = "";
		private readonly InProcessTransport inProcessTransport;
		private long lastDeadlineTime;
		private OPSMessage message;
		private MessageFilterSet messageFilters = new MessageFilterSet();
		private readonly object newDataEvent = new object();
		protected Participant participant = null;
		private ReceiveDataHandler receiveDataHandler = null;
		private long sampleTime1;  
        private long sampleTime2;  
		private long timeBaseMinSeparationTime;
		private long timeLastDataForTimeBase;
		private Topic topic;
        private System.Object lockThis = new System.Object();
        private bool active = false;

        public Subscriber(Topic t)
        {
            if (t == null)
            {
                throw new OPSInvalidTopicException("Not possible to create Subscriber with null Topic!");
            }
            this.topic = t;
            this.participant = Participant.GetInstance(topic.GetDomainID(), topic.GetParticipantID());
            deadlineNotifier = DeadlineNotifier.GetInstance();
            inProcessTransport = participant.GetInProcessTransport();
        }

        ~Subscriber()
        {
            // Must tell the receiveDataHandler that we don't need it anymore
            Stop();
            this.participant = null;
        }

        public void CheckTypeString(string typeString)
        {
            if (topic.GetTypeID() != typeString)
            {
                throw new OPSInvalidTopicException("Topic type string mismatch!");
            }
        }

        public bool Active
        {
            get
            {
                return this.active;
            }
        }

        // The reason why this method is NOT synchronized is to prevent deadlocks.
        // More information regarding this matter can be found in comment for method "Stop" in this class.
        /////[MethodImpl(MethodImplOptions.Synchronized)]
        public void SetDeadlineQoS(long timeoutMs)
        {
            // A timeout <= 0 disables dead line notification.
            // If timeout > 0 we enable dead line notification and ensure that we exist in the deadlineNotifier list.
            if (timeoutMs > 0)
            {
                if (!deadlineNotifier.Contains(this))
                {
                    deadlineNotifier.Add(this);
                }
            }
            else
            {
                deadlineNotifier.Remove(this);
            }
            lastDeadlineTime = System.DateTime.Now.Ticks;
            deadlineTimeout = timeoutMs * 10000; // There are 10000 ticks in a millisecond.
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public void SetTimeBasedFilterQoS(long minSeparationTime)
        {
            timeBaseMinSeparationTime = minSeparationTime;
        }

        public long GetTimeBaseFilterQos()
        {
            return timeBaseMinSeparationTime;
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public void Start()
        {
            if (!this.active)
            {
                timeLastDataForTimeBase = System.DateTime.Now.Ticks;

                // A call to "SetDeadlineQoS" enables dead line notification if current timeout is > 0.
                SetDeadlineQoS(deadlineTimeout / 10000);

                receiveDataHandler = participant.GetReceiveDataHandler(topic);
                receiveDataHandler.AddSubscriber(this, topic);
                inProcessTransport.AddSubscriber(this);
                this.active = true;
            }
        }

        // The ReceiveDataHandler calls our "NotifyNewOPSMessage" from a synchronized context.
        // The "RemoveSubscriber" call below on receiveDataHandler is also synchronized in ReceiveDataHandler.
        // Both "Stop" and "NotifyNewOPSMessage" can therefore NOT be Synchronized (could lead to deadlock)
        // The same scenario is possible for the deadlineNotifier.
        // That's why we have commented the synchronize attribute below.
        /////[MethodImpl(MethodImplOptions.Synchronized)]
        public bool Stop()
        {
            bool retVal = true;

            if (this.active)
            {
                this.active = false;
                deadlineNotifier.Remove(this);
                inProcessTransport.RemoveSubscriber(this);
                retVal = receiveDataHandler.RemoveSubscriber(this, topic);
                receiveDataHandler = null;
                participant.ReleaseReceiveDataHandler(topic);
            }

            return retVal;
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public bool IsDeadlineMissed()
        {
            if (deadlineTimeout <= 0) return false;

            if (System.DateTime.Now.Ticks - lastDeadlineTime > deadlineTimeout)
            {
                return true;
            }
            return false;
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public void CheckDeadline()
        {
            if (deadlineTimeout <= 0) return;

            long currentTime = System.DateTime.Now.Ticks;

            if (currentTime - lastDeadlineTime > deadlineTimeout)
            {
                deadlineEvent.FireEvent();
                lastDeadlineTime = currentTime;
            }
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public string GetIdentity()
        {
            return identity;
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        void NotifyNewOPSObject(OPSObject o)
        {
            if (ApplyFilterQoSPolicies(o))
            {
                long currentTime = System.DateTime.Now.Ticks;
                if (currentTime - timeLastDataForTimeBase > timeBaseMinSeparationTime || timeBaseMinSeparationTime == 0)
                {
                    lastDeadlineTime = currentTime;
                    timeLastDataForTimeBase = currentTime;

                    sampleTime2 = sampleTime1;
                    sampleTime1 = currentTime;

                    data = o;

                    if (data != null)
                        NewDataArrived(data);
                    
                    lock (newDataEvent)
                    {
                        Monitor.PulseAll(newDataEvent);
                    }
                }
            }
        }

        // This is the default implementation for when new data has arrived.
        // The method can be overriden to implement a type safe callback event.
        protected virtual void NewDataArrived(OPSObject o)
        {
            // Use a delegate to send data to interested receivers.
            newDataDefault(this, o);
        }

        public double GetInboundRate()
        {
            double sampleRate = 1.0/((sampleTime1 - sampleTime2) / 10000000.0);
            double fakeRate = 1.0 / ((System.DateTime.Now.Ticks - sampleTime1) / 10000000.0);

            if(sampleRate < fakeRate)
            {
                return sampleRate;
            }
            else
            {
                return fakeRate;
            }
        }

        public OPSObject WaitForNextOpsObjectData(long millis)
        {
            try
            {
                lock(newDataEvent)
                {
                    if (Monitor.Wait(newDataEvent, (int)millis))
                    {
                        return data;
                    }
                    else
                    {
                        return null;
                    }
                }
            }
            catch (ThreadAbortException)
            {
                return null;
            }
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public void AddFilterQoSPolicy(IFilterQoSPolicy qosPolicy)
        {
            lock(this.lockThis)
            {
                GetFilterQoSPolicies().Add(qosPolicy);
            }
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public void RemoveFilterQoSPolicy(IFilterQoSPolicy qosPolicy)
        {
            lock(this.lockThis)
            {
                GetFilterQoSPolicies().Remove(qosPolicy);
            }
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public void NotifyNewOPSMessage(OPSMessage message)
        {
            //Check that this message is delivered on the same topic as this Subscriber use
            //This is needed when we allow several topics to use the same port
            if (message.GetTopicName() != topic.GetName())
            {
                return;
            }
            ///For now we don't do this type check to minimize performance loss
            ////Check that the type of the delivered data can be interpreted as the type we expect in this Subscriber
            //if (message.GetData().GetTypesString().IndexOf(topic.GetTypeID()) < 0)
            //{
            //    return;
            //}
            if (messageFilters.ApplyFilter(message))
            {
                this.message = message;
                this.NotifyNewOPSObject(message.GetData());
            }
        }

        private bool ApplyFilterQoSPolicies(OPSObject o)
        {
            foreach (IFilterQoSPolicy filter in filterQoSPolicies)
            {
                if (!filter.ApplyFilter(o))
                {
                    //Indicates that this data sample NOT should be delivered to the application.
                    return false;
                }

            }
            return true;
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public List<IFilterQoSPolicy> GetFilterQoSPolicies()
        {
            return filterQoSPolicies;
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public OPSMessage GetMessage()
        {
            return message;
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        protected OPSObject GetData()
        {
            return data;
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public void RemoveFilter(IMessageFilter filter)
        {
            messageFilters.RemoveFilter(filter);
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public void AddFilter(IMessageFilter filter)
        {
            messageFilters.AddFilter(filter);
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public MessageFilterSet GetMessageFilters()
        {
            return messageFilters;
        }

        public Topic GetTopic()
        {
            return topic;
        }

	}

}