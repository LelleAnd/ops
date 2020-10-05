///////////////////////////////////////////////////////////
//  Domain.cs
//  Implementation of the Class Domain
//  Created on:      12-nov-2011 09:25:30
//  Author:
///////////////////////////////////////////////////////////

using System.Collections.Generic;

namespace Ops 
{
	public class Domain : OPSObject 
    {
        private byte _Domain_version = Domain_idlVersion;
        public byte Domain_version
        {
            get { return _Domain_version; }
            set {
                if (value > Domain_idlVersion)
                {
                    throw new IdlVersionException(
                        "Domain: received version '" + value + "' > known version '" + Domain_idlVersion + "'");
                }
                _Domain_version = value;
            }
        }

        public const byte Domain_idlVersion = 0;
        private string domainAddress = "";
		private string domainID = "";
		private string localInterface = "0.0.0.0";
		protected List<Topic> topics = new List<Topic>();
        private int timeToLive = 1;
        private int inSocketBufferSize = -1;    // Use OS default
        private int outSocketBufferSize = -1;   // Use OS default
		private int metaDataMcPort = 9494;
        private bool optNonVirt = false;

        private List<Channel> channels = new List<Channel>();
        private List<Transport> transports = new List<Transport>();

        public Domain()
        {
            AppendType("Domain");
        }

        void checkTopicValues(Topic top)
        {
            if (top.GetDomainAddress().Equals(""))
            {
                top.SetDomainAddress(domainAddress);
            }
            if (top.GetLocalInterface().Equals(""))
            {
                top.SetLocalInterface(localInterface);
            }
            if (top.GetTimeToLive() < 0)
            {
                top.SetTimeToLive(timeToLive);
            }
            if (top.GetInSocketBufferSize() < 0)
            {
                top.SetInSocketBufferSize(inSocketBufferSize);
            }
            if (top.GetOutSocketBufferSize() < 0)
            {
                top.SetOutSocketBufferSize(outSocketBufferSize);
            }
            top.SetOptNonVirt(optNonVirt);
        }

        public Topic GetTopic(string name)
        {
            foreach (Topic topic in topics)
            {
                if (topic.GetName().Equals(name))
                {
                    checkTopicValues(topic);
                    return topic;
                }
            }
            return null;
        }

        public override void Serialize(IArchiverInOut archive)
        {
            // NOTE. Keep this in sync with the C++ version, so it in theory is possible to send these as objects.
            // We need to serialize fields in the same order as C++.
            //OPSObject::serialize(archiver);
            base.Serialize(archive);

            if (IdlVersionMask != 0)
            {
                Domain_version = archive.Inout("Domain_version", Domain_version);
            }
            else
            {
                Domain_version = 0;
            }

            //archiver->inout(std::string("domainID"), domainID);
            //archiver->inout<Topic>(std::string("topics"), topics);
            //archiver->inout(std::string("domainAddress"), domainAddress);
            //archiver->inout(std::string("localInterface"), localInterface);
            domainID = archive.Inout("domainID", domainID);
            topics = (List<Topic>)archive.InoutSerializableList("topics", topics);
            domainAddress = archive.Inout("domainAddress", domainAddress);
            localInterface = archive.Inout("localInterface", localInterface);
            
            //archiver->inout(std::string("timeToLive"), timeToLive);
            //archiver->inout(std::string("inSocketBufferSize"), inSocketBufferSize);
            //archiver->inout(std::string("outSocketBufferSize"), outSocketBufferSize);
            //archiver->inout(std::string("metaDataMcPort"), metaDataMcPort);
            timeToLive = archive.Inout("timeToLive", timeToLive);
    		inSocketBufferSize = archive.Inout("inSocketBufferSize", inSocketBufferSize);
	    	outSocketBufferSize = archive.Inout("outSocketBufferSize", outSocketBufferSize);
            metaDataMcPort = archive.Inout("metaDataMcPort", metaDataMcPort);

        	// To not break binary compatibility we only do this when we know we are
        	// reading from an XML-file
            if (archive is XMLArchiverIn) { 
        		//archiver->inout<Channel>(std::string("channels"), channels);
		        //archiver->inout<Transport>(std::string("transports"), transports);
        		channels = (List<Channel>)archive.InoutSerializableList("channels", channels);
                transports = (List<Transport>)archive.InoutSerializableList("transports", transports);
                optNonVirt = archive.Inout("optNonVirt", optNonVirt);
        		checkTransports();
        	}
        }

        private Channel findChannel(string id)
        {
        	if (!id.Equals("")) {
		        for (int i = 0; i < channels.Count; i++) {
        			if (id.Equals(channels[i].channelID)) return channels[i];
		        }
        	}
    	    return null;
        }

        private Topic findTopic(string id)
        {
        	if (!id.Equals("")) {
        		for (int i = 0; i < topics.Count; i++) {
		        	if (id.Equals(topics[i].GetName())) return topics[i];
        		}
        	}
        	return null;
        }

        void checkTransports()
        {
        	// Now update topics with values from the transports and channels
        	// Loop over all transports and for each topic, see if it needs parameters from the channel
        	for (int i = 0; i < transports.Count; i++) {
        		// Get channel
        		Channel channel = findChannel(transports[i].channelID);
        		if (channel == null) {
        			throw new ConfigException(
		        		"Non existing channelID: '" + transports[i].channelID +
				        "' used in transport spcification.");
        		} else {
		        	for (int j = 0; j < transports[i].topics.Count; j++) {
				        Topic top = findTopic(transports[i].topics[j]);
    	    			if (top == null) {
	    	    			throw new ConfigException(
				    	    	"Non existing topicID: '" + transports[i].topics[j] +
					    	    "' used in transport spcification.");
        				} else {
		        			channel.populateTopic(top);
				        }
        			}
		        }
        	}
        }

        public List<Topic> GetTopics() 
        {
            foreach (Topic topic in topics) 
            {
                checkTopicValues(topic);
            }
            return topics;
        }

         public bool ExistTopic(string name) 
         {
            foreach (Topic topic in topics) 
            {
                if (topic.GetName().Equals(name)) 
                {
                    return true;
                }
            }
            return false;
        }

        public string GetDomainAddress() 
        {
            return this.domainAddress;
        }

        public string GetDomainID() 
        {
            return this.domainID;
        }

        public string GetLocalInterface() 
        {
            return this.localInterface;
        }

        public int GetMetaDataMcPort()
        {
            return this.metaDataMcPort;
        }

        public int GetInSocketBufferSize()
        {
            return this.inSocketBufferSize;
        }

        public int GetOutSocketBufferSize()
        {
            return this.outSocketBufferSize;
        }

        public int getTimeToLive()
        {
            return this.timeToLive;
        }

        public void SetDomainAddress(string domainAddress) 
        {
            this.domainAddress = domainAddress;
        }

        public void SetDomainID(string domainID) 
        {
            this.domainID = domainID;
        }

        public void SetLocalInterface(string localInterface) 
        {
            this.localInterface = localInterface;
        }

        public bool getOptNonVirt()
        {
            return optNonVirt;
        }

	}

}