///////////////////////////////////////////////////////////
//  Participant.cs
//  Implementation of the Class Participant
//  Created on:      12-nov-2011 09:25:33
//  Author:
///////////////////////////////////////////////////////////

using System.IO;
using System.Net;
using System.Runtime.CompilerServices;  // Needed for the "MethodImpl" synchronization attribute
using System.Threading;

namespace Ops 
{
	public class Participant
    {
        // This factory is used through static facade methods getInstance(...)
        private static ParticipantFactory participantFactory = new ParticipantFactory();

        private OPSConfig config;
        protected Domain domain;
        public string domainID { get; private set; }
		private InProcessTransport inProcessTransport = new InProcessTransport();
        public string participantID { get; private set; }
		private ReceiveDataHandlerFactory receiveDataHandlerFactory = new ReceiveDataHandlerFactory();
		private SendDataHandlerFactory sendDataHandlerFactory = new SendDataHandlerFactory();

        ///The data type factory used in this Participant. 
        private OPSObjectFactory objectFactory = new OPSObjectFactory();

        public ParticipantInfoDataListener partInfoListener = null;
        private ParticipantInfoData partInfoData = new ParticipantInfoData();
        private Publisher partInfoPub = null;

        private Thread thread = null;
        private bool doRun = true;

        private static int safeInstanceCount = 0;

        static public int SafeInstanceCount
        {
            get { return safeInstanceCount; }
        }

        /**
         * Method for retreiving the default Participant instance for the @param domainID
         * @param domainID
         * @return a Participant or null if this method fails.
         */
        [MethodImpl(MethodImplOptions.Synchronized)]
        public static Participant GetInstance(string domainID)
        {
            return participantFactory.GetParticipant(domainID, "DEFAULT_PARTICIPANT");
        }

        /**
         * Method for retreiving the @param participantID Participant instance for @param domainID
         * @param domainID
         * @param participantID
         * @return a Participant or null if this method fails.
         */
        [MethodImpl(MethodImplOptions.Synchronized)]
        public static Participant GetInstance(string domainID, string participantID)
        {
            return participantFactory.GetParticipant(domainID, participantID, null);
        }

        /**
         * Method for retreiving the @param participantID Participant instance for
         * @param domainID using @param file if this participant has not yet been created.
         * @param domainID
         * @param participantID
         * @param file the file to use as configuration. Ignored if a participant for participantID already exist.
         * @return a Participant or null if this method fails.
         */
        [MethodImpl(MethodImplOptions.Synchronized)]
        public static Participant GetInstance(string domainID, string participantID, string configFile)
        {
            return participantFactory.GetParticipant(domainID, participantID, configFile);
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public static Participant GetInstance(Domain domain, string participantID)
        {
            return participantFactory.GetParticipant(domain, participantID);
        }

        //---------------------------------------------------------------

        public Participant(string domainID, string participantID, string configFile)
        {
            Interlocked.Increment(ref safeInstanceCount);  ///TEST
            this.domainID = domainID;
            this.participantID = participantID;
            partInfoListener = new ParticipantInfoDataListener(this);
            try
            {
                if (string.IsNullOrEmpty(configFile))
                {
                    this.config = OPSConfig.GetConfig();
                    this.domain = this.config.GetDomain(domainID);
                } 
                else
                {
                    this.config = OPSConfig.GetConfig(configFile);
                    this.domain = this.config.GetDomain(domainID);
                }
                if (this.domain != null)
                {
                    this.inProcessTransport.Start();
                    SetupCyclicThread();
                }
                else
                {
                    Logger.ExceptionLogger.LogMessage(this.GetType().Name + ", Failed to find requested domain in configuration file");
                    throw new System.Exception("Failed to find requested domain in configuration file");
                }
            }
            catch (IOException)
            {
                Logger.ExceptionLogger.LogMessage(this.GetType().Name + ", Configuration file missing");
                throw;
            }
            catch (Ops.FormatException)
            {
                Logger.ExceptionLogger.LogMessage(this.GetType().Name + ", Format error in configuration file");
                throw;
            }
        }

        public Participant(Domain domain, string participantID)
        {
            Interlocked.Increment(ref safeInstanceCount);  ///TEST
            this.domainID = domain.GetDomainID();
            this.participantID = participantID;
            partInfoListener = new ParticipantInfoDataListener(this);

            this.domain = domain;

            this.inProcessTransport.Start();
            SetupCyclicThread();
        }

        ~Participant()
        {
            Interlocked.Decrement(ref safeInstanceCount);   ///TEST
        }

        /// <summary>
        /// Method used to Stop the participant and remove it from the participant factory.
        /// The participant object will be deleted when no one have references to it.
        /// </summary>
        public void StopAndReleaseResources()
        {
            // Stop the cyclic thread
            if (this.doRun)
            {
                this.doRun = false;
                this.thread.Join();
            }
            // Remove us from the singleton factory
            participantFactory.RemoveParticipant(this.domainID, this.participantID);
            sendDataHandlerFactory = null;
        }

        // Initialize static data in partInfoData
        private void InitPartInfoData()
        {
            string processId = System.Diagnostics.Process.GetCurrentProcess().Id.ToString();
            System.Net.NetworkInformation.IPGlobalProperties gp = System.Net.NetworkInformation.IPGlobalProperties.GetIPGlobalProperties();
            string Name = gp.HostName + " (" + processId + ")";

            lock (partInfoData)
            {
                partInfoData.name = Name;
                partInfoData.languageImplementation = "C#";
                partInfoData.id = participantID;
                partInfoData.domain = domainID;
            }
        }

        public void SetUdpTransportInfo(string ip, int port)
        {
            lock (partInfoData)
            {
                partInfoData.ip = ip;
                partInfoData.mc_udp_port = port;
            }
        }

        // 
        public bool HasPublisherOn(string topicName)
        {
            lock (partInfoData)
            {
                for (int i = 0; i < partInfoData.publishTopics.Count; i++)
                {
                    if (partInfoData.publishTopics[i].name.Equals(topicName))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        public bool HasSubscriberOn(string topicName)
        {
            lock (partInfoData)
            {
                for (int i = 0; i < partInfoData.subscribeTopics.Count; i++)
                {
                    if (partInfoData.subscribeTopics[i].name.Equals(topicName))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        /**
         * Adds type support in form of a SerializablFactory to this participant.
         * Normally this is the IDL project generated TypeFactory (e.g. FooProject.FooProjectTypeFactory())
         * @param typeSupport
         */
        public void AddTypeSupport(ISerializableFactory typeSupport)
        {
            this.objectFactory.Add(typeSupport);
        }

        ///Get a pointer to the data type factory used in this Participant. 
        public OPSObjectFactory getObjectFactory()
        {
            return this.objectFactory;
        }

        /**
         * Creates a Topic that can be used to create Publishers and/or Subscribers.
         * The fields of the Topic returned are fetched from the participants underlying config file.
         * @param name
         * @return a new Topic based on the config of this participant.
         */
        public Topic CreateTopic(string name)
        {
            Topic topic = domain.GetTopic(name);
            
            if (topic == null)
                return null;

            topic.SetParticipantID(participantID);
            topic.SetDomainID(domainID);

            return topic;
        }

        /**
         * Creates a Topic that can be used to Subscribe to the participant info data
         * @return a new Topic for the participant info data.
         */
        public Topic CreateParticipantInfoTopic()
    	{
    		Topic infoTopic = new Topic("ops.bit.ParticipantInfoTopic", domain.GetMetaDataMcPort(), "ops.ParticipantInfoData", domain.GetDomainAddress());
            infoTopic.SetLocalInterface(domain.GetLocalInterface());
            infoTopic.SetTimeToLive(domain.getTimeToLive());
            infoTopic.SetDomainID(domainID);
		    infoTopic.SetParticipantID(participantID);
    		infoTopic.SetTransport(Topic.TRANSPORT_MC);
	    	return infoTopic;
    	}

        // By modified singelton
        [MethodImpl(MethodImplOptions.Synchronized)]
        public ReceiveDataHandler GetReceiveDataHandler(Topic topic)
        {
            ReceiveDataHandler rdh = this.receiveDataHandlerFactory.GetReceiveDataHandler(topic, this);
            if (rdh != null)
            {
                lock (partInfoData)
                {
                    partInfoData.subscribeTopics.Add(new TopicInfoData(topic));
                }
            }
            return rdh;
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public void ReleaseReceiveDataHandler(Topic topic)
        {
            this.receiveDataHandlerFactory.ReleaseReceiveDataHandler(topic, this);

            lock (partInfoData)
            {
                for (int i = 0; i < partInfoData.subscribeTopics.Count; i++)
                {
                    if (partInfoData.subscribeTopics[i].name.Equals(topic.GetName())) {
                        partInfoData.subscribeTopics.RemoveAt(i);
                        break;
                    }
                }
            }
        }

        // By modified singelton
        [MethodImpl(MethodImplOptions.Synchronized)]
        public ISendDataHandler GetSendDataHandler(Topic topic) 
        {
            ISendDataHandler sdh = this.sendDataHandlerFactory.GetSendDataHandler(topic, this);
            // We can't update Participant Info here, delayed until UpdatePubPartInfo()
            return sdh;
        }

        public void UpdatePubPartInfo(Topic topic)
        {
            lock (partInfoData)
            {
                partInfoData.publishTopics.Add(new TopicInfoData(topic));
            }
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public void ReleaseSendDataHandler(Topic topic)
        {
            this.sendDataHandlerFactory.ReleaseSendDataHandler(topic, this);

            lock (partInfoData)
            {
                for (int i = 0; i < partInfoData.publishTopics.Count; i++)
                {
                    if (partInfoData.publishTopics[i].name.Equals(topic.GetName())) {
                        partInfoData.publishTopics.RemoveAt(i);
                        break;
                    }
                }
            }
        }

        public Domain getDomain()
        {
            return this.domain;
        }

        public InProcessTransport GetInProcessTransport()
        {
            return this.inProcessTransport;
        }

        public void SaveConfig(string configFile)
        {
            if (this.config != null)
            {
                OPSConfig.SaveConfig(this.config, configFile);
            }
        }
    
        private void Run()
        {
            InitPartInfoData();

            while (this.doRun)
            {
                Thread.Sleep(1000);

                // Create publisher for participant info data
                if ( (partInfoPub == null) && (domain.GetMetaDataMcPort() != 0) )
                {
                    partInfoPub = new Publisher(CreateParticipantInfoTopic());
                }

                // Publish data
                if (partInfoPub != null)
                {
                    lock (partInfoData)
                    {
                        partInfoPub.WriteAsOPSObject(partInfoData);
                    }
                }
            }

            if (partInfoPub != null) { partInfoPub.Stop(); }

            partInfoPub = null;
        }

        private void SetupCyclicThread()
        {
            thread = new Thread(new ThreadStart(Run));
            thread.IsBackground = true;
            thread.Name = "ParticipantThread_" + domainID + "_" + participantID;
            thread.Start();
        }

    }

}
