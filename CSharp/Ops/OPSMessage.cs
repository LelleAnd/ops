///////////////////////////////////////////////////////////
//  OPSMessage.cs
//  Implementation of the Class OPSMessage
//  Created on:      12-nov-2011 09:25:33
//  Author:
///////////////////////////////////////////////////////////

namespace Ops
{
	public class OPSMessage : OPSObject 
    {
        private byte _OPSMessage_version = OPSMessage_idlVersion;
        public byte OPSMessage_version
        {
            get { return _OPSMessage_version; }
            set {
                if (value > OPSMessage_idlVersion)
                {
                    throw new IdlVersionException(
                        "OPSMessage: received version '" + value + "' > known version '" + OPSMessage_idlVersion + "'");
                }
                _OPSMessage_version = value;
            }
        }

        public const byte OPSMessage_idlVersion = 0;
        private string address = "";            // Serialized
		private OPSObject data;                 // serialized
		private byte endianness;
		private byte messageType;               // Serialized
		private int sourcePort;
        private string sourceIP;
		private long publicationID;             // Serialized
		private string publisherName = "";      // Serialized
		private byte publisherPriority;         // Serialized
		private long qosMask;
		private string topicName = "";          // Serialized
		private string topLevelKey = "";        // Serialized

        public OPSMessage()
        {
            AppendType("ops.protocol.OPSMessage");
        }

        public string GetAddress()
        {
            return this.address;
        }

        public void SetAddress(string address)
        {
            this.address = address;
        }

        public OPSObject GetData()
        {
            return this.data;
        }

        public void SetData(OPSObject data)
        {
            this.data = data;
        }

        public byte GetEndianness()
        {
            return this.endianness;
        }

        public void SetEndianness(byte endianness)
        {
            this.endianness = endianness;
        }

        public byte GetMessageType()
        {
            return this.messageType;
        }

        public void SetMessageType(byte messageType)
        {
            this.messageType = messageType;
        }

        public int GetSourcePort()
        {
            return this.sourcePort;
        }

        public string GetSourceIP()
        {
            return this.sourceIP;
        }

        internal void SetSource(string IP, int port)
        {
            this.sourceIP = IP;
            this.sourcePort = port;
        }

        public long GetPublicationID()
        {
            return this.publicationID;
        }

        public void SetPublicationID(long publicationID)
        {
            this.publicationID = publicationID;
        }

        public string GetPublisherName()
        {
            return this.publisherName;
        }

        public void SetPublisherName(string publisherName)
        {
            this.publisherName = publisherName;
        }

        public byte GetPublisherPriority()
        {
            return this.publisherPriority;
        }

        public void SetPublisherPriority(byte publisherPriority)
        {
            this.publisherPriority = publisherPriority;
        }

        public long GetQosMask()
        {
            return this.qosMask;
        }

        public void SetQosMask(long qosMask)
        {
            this.qosMask = qosMask;
        }

        public string GetTopLevelKey()
        {
            return this.topLevelKey;
        }

        public void SetTopLevelKey(string topLevelKey)
        {
            this.topLevelKey = topLevelKey;
        }

        public string GetTopicName()
        {
            return this.topicName;
        }

        public void SetTopicName(string topicName)
        {
            this.topicName = topicName;
        }

        public override void Serialize(IArchiverInOut archive)
        {
            base.Serialize(archive);

            if (IdlVersionMask != 0)
            {
                OPSMessage_version = archive.Inout("OPSMessage_version", OPSMessage_version);
            }
            else
            {
                OPSMessage_version = 0;
            }

            this.messageType = archive.Inout("messageType", messageType);
            this.publisherPriority = archive.Inout("publisherPriority", publisherPriority);
            this.publicationID = archive.Inout("publicationID", publicationID);
            this.publisherName = archive.Inout("publisherName", publisherName);
            this.topicName = archive.Inout("topicName", topicName);
            this.topLevelKey = archive.Inout("topLevelKey", topLevelKey);
            this.address = archive.Inout("address", address);
            this.data = (OPSObject) archive.Inout("data", data);
        }

        public override object Clone()
        {
            OPSMessage cloneResult = new OPSMessage();
            FillClone(cloneResult);
            return cloneResult;
        }

        public override void FillClone(OPSObject cloneO)
        {
            base.FillClone(cloneO);
            OPSMessage cloneResult = (OPSMessage) cloneO;
            cloneResult.OPSMessage_version = this.OPSMessage_version;
            cloneResult.SetKey(this.GetKey());
            cloneResult.messageType = this.messageType;
            cloneResult.publisherPriority = this.publisherPriority;
            cloneResult.publicationID = this.publicationID;
            cloneResult.publisherName = this.publisherName;
            cloneResult.topicName = this.topicName;
            cloneResult.topLevelKey = this.topLevelKey;
            cloneResult.address = this.address;
            cloneResult.data = (OPSObject)this.data.Clone();
        }

	}

}