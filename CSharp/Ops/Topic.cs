///////////////////////////////////////////////////////////
//  Topic.cs
//  Implementation of the Class Topic
//  Created on:      12-nov-2011 09:25:37
//  Author:
///////////////////////////////////////////////////////////

namespace Ops 
{
	public class Topic : OPSObject 
    {
        public const byte Topic_idlVersion = 0;
        private byte _Topic_version = Topic_idlVersion;
        public byte Topic_version
        {
            get { return _Topic_version; }
            set {
                if (value > Topic_idlVersion)
                {
                    throw new IdlVersionException(
                        "Topic: received version '" + value + "' > known version '" + Topic_idlVersion + "'");
                }
                _Topic_version = value;
            }
        }

        private string _ChannelID = "";
        public string ChannelID
        {
            get { return _ChannelID; }
            set { _ChannelID = value; }
        }

        private string domainAddress = "";
		private string domainID;
		private long inSocketBufferSize = -1;
        private string localInterface = "";
		private string name = "";
		private long outSocketBufferSize = -1;
		private string participantID;
		private int port = -1;
		private int replyPort = 0;
        private int sampleMaxSize = Globals.MAX_SEGMENT_SIZE;
        private int timeToLive = -1;
        private bool optNonVirt = false;
        private string transport;
        public static readonly string TRANSPORT_INPROC = "inprocess";
        public static readonly string TRANSPORT_MC = "multicast";
		public static readonly string TRANSPORT_TCP = "tcp";
		public static readonly string TRANSPORT_UDP = "udp";
        public static readonly string TRANSPORT_SHMEM = "shmem";
        private string typeID = "";


        // Creates a new instance of Topic
        public Topic(string name, int port, string typeID, string domainAddress)
        {
            this.name = name;
            this.port = port;
            this.typeID = typeID;
            this.domainAddress = domainAddress;
            this.sampleMaxSize = Globals.MAX_SEGMENT_SIZE;
            AppendType("Topic");
        }

        public Topic()
        {
            AppendType("Topic");
        }

        public string GetDomainAddress()
        {
            return this.domainAddress;
        }

        public void SetDomainAddress(string domainAddress)
        {
            this.domainAddress = domainAddress;
        }

        public string GetLocalInterface()
        {
            return this.localInterface;
        }

        public void SetLocalInterface(string localInterface)
        {
            this.localInterface = localInterface;
        }

        public override string ToString()
        {
            return this.name;
        }

        public int GetReplyPort()
        {
            return this.replyPort;
        }

        public int GetPort()
        {
            return this.port;
        }

        public string GetName()
        {
            return this.name;
        }

        public string GetTypeID()
        {
            return this.typeID;
        }

        protected void SetReplyPort(int replyPort)
        {
            this.replyPort = replyPort;
        }

        public void SetTypeID(string typeID)
        {
            this.typeID = typeID;
        }

        public void SetPort(int port)
        {
            this.port = port;
        }

        public void SetName(string name)
        {
            this.name = name;
        }

        public string GetDomainID()
        {
            return this.domainID;
        }

        public string GetParticipantID()
        {
            return this.participantID;
        }

        public int GetSampleMaxSize()
        {
            return this.sampleMaxSize;
        }

        public void SetSampleMaxSize(int size)
        {
            if (size < Globals.MAX_SEGMENT_SIZE)
            {
                this.sampleMaxSize = Globals.MAX_SEGMENT_SIZE;
            } else
            {
                this.sampleMaxSize = size;
            }
        }

        public override void Serialize(IArchiverInOut archive)
        {
            base.Serialize(archive);
            if (IdlVersionMask != 0)
            {
                Topic_version = archive.Inout("Topic_version", Topic_version);
            }
            else
            {
                Topic_version = 0;
            }
            this.name = archive.Inout("name", name);
            this.typeID = archive.Inout("dataType", typeID);
            this.port = archive.Inout("port", port);
            this.domainAddress = archive.Inout("address", domainAddress);

            this.outSocketBufferSize = archive.Inout("outSocketBufferSize", outSocketBufferSize);
            this.inSocketBufferSize = archive.Inout("inSocketBufferSize", inSocketBufferSize);

            int tSampleMaxSize = GetSampleMaxSize();
            tSampleMaxSize = archive.Inout("sampleMaxSize", tSampleMaxSize);
            SetSampleMaxSize(tSampleMaxSize);

            this.transport = archive.Inout("transport", transport);
            if (this.transport.Equals(""))
            {
                this.transport = TRANSPORT_MC;
            }
            else if ((this.transport == TRANSPORT_TCP) || (this.transport == TRANSPORT_UDP))
            {
                if (this.domainAddress.Length > 0)
                {
                    this.domainAddress = InetAddress.GetByName(this.domainAddress);
                }
            }
        }

        public void SetDomainID(string domainID)
        {
            this.domainID = domainID;
        }

        public void SetParticipantID(string participantID)
        {
            this.participantID = participantID;
        }

        public int GetInSocketBufferSize()
        {
            return (int)this.inSocketBufferSize;
        }

        public void SetInSocketBufferSize(int inSocketBufferSize)
        {
            this.inSocketBufferSize = (long)inSocketBufferSize;
        }

        public int GetOutSocketBufferSize()
        {
            return (int)this.outSocketBufferSize;
        }

        public void SetOutSocketBufferSize(int outSocketBufferSize)
        {
            this.outSocketBufferSize = (long)outSocketBufferSize;
        }

        public string GetTransport()
        {
            return this.transport;
        }

        public void SetTransport(string transport)
        {
            this.transport = transport;
        }

        public int GetTimeToLive()
        {
            return this.timeToLive;
        }

        public void SetTimeToLive(int timeToLive)
        {
            this.timeToLive = timeToLive;
        }

        public bool GetOptNonVirt()
        {
            return optNonVirt;
        }

        public void SetOptNonVirt(bool optNonVirt)
        {
            this.optNonVirt = optNonVirt;
        }

        public override object Clone()
        {
            Topic cloneResult = new Topic();
            FillClone(cloneResult);
            return cloneResult;
        }

        public override void FillClone(OPSObject cloneO)
        {
            base.FillClone(cloneO);
            Topic cloneResult = (Topic)cloneO;
            cloneResult.Topic_version = Topic_version;
            cloneResult.ChannelID = ChannelID;
            cloneResult.domainAddress = domainAddress;
            cloneResult.domainID = domainID;
            cloneResult.inSocketBufferSize = inSocketBufferSize;
            cloneResult.localInterface = localInterface;
            cloneResult.name = name;
            cloneResult.outSocketBufferSize = outSocketBufferSize;
            cloneResult.participantID = participantID;
            cloneResult.port = port;
            cloneResult.replyPort = replyPort;
            cloneResult.sampleMaxSize = sampleMaxSize;
            cloneResult.timeToLive = timeToLive;
            cloneResult.optNonVirt = optNonVirt;
            cloneResult.transport = transport;
            cloneResult.typeID = typeID;
        }
    }

}