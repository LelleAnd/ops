///////////////////////////////////////////////////////////
//  Channel.cs
//  Implementation of the Class Channel
//  Created on:      18-oct-2016
//  Author:
///////////////////////////////////////////////////////////

namespace Ops
{
    public class Channel : OPSObject 
    {
        public const byte Channel_idlVersion = 0;
        private byte _Channel_version = Channel_idlVersion;
        public byte Channel_version
        {
            get { return _Channel_version; }
            set {
                if (value > Channel_idlVersion)
                {
                    throw new IdlVersionException(
                        "Channel: received version '" + value + "' > known version '" + Channel_idlVersion + "'");
                }
                _Channel_version = value;
            }
        }

        public string channelID = "";
        public string linktype = "";
        public string localInterface = "";     // If multicast, this specifies interface to use
        public string domainAddress = "";
        public int timeToLive = -1;            // if multicast, this specifies the ttl parameter
        public int port = 0;
        public long outSocketBufferSize = -1;
        public long inSocketBufferSize = -1;

        public static readonly string LINKTYPE_MC = "multicast";
        public static readonly string LINKTYPE_TCP = "tcp";
        public static readonly string LINKTYPE_UDP = "udp";

        public Channel()
        {
            AppendType("Channel");
        }

        public override void Serialize(IArchiverInOut archive)
        {
            // NOTE. Keep this in sync with the C++ version, so it in theory is possible to send these as objects.
            // We need to serialize fields in the same order as C++.
            //OPSObject::serialize(archiver);
            base.Serialize(archive);

            if (IdlVersionMask != 0)
            {
                Channel_version = archive.Inout("Channel_version", Channel_version);
            }
            else
            {
                Channel_version = 0;
            }

            //archiver->inout(std::string("name"), channelID);
            //archiver->inout(std::string("linktype"), linktype);
            //archiver->inout(std::string("localInterface"), localInterface);
            //archiver->inout(std::string("address"), domainAddress);
            channelID = archive.Inout("name", channelID);
            linktype = archive.Inout("linktype", linktype);
            localInterface = InetAddress.GetHostAddressEx(archive.Inout("localInterface", localInterface));
            domainAddress = archive.Inout("address", domainAddress);

            //archiver->inout(std::string("timeToLive"), timeToLive);
            //archiver->inout(std::string("port"), port);
            //archiver->inout(std::string("outSocketBufferSize"), outSocketBufferSize);
            //archiver->inout(std::string("inSocketBufferSize"), inSocketBufferSize);
            timeToLive = archive.Inout("timeToLive", timeToLive);
            port = archive.Inout("port", port);
            outSocketBufferSize = archive.Inout("outSocketBufferSize", outSocketBufferSize);
            inSocketBufferSize = archive.Inout("inSocketBufferSize", inSocketBufferSize);

            if (linktype.Equals("") || (linktype == LINKTYPE_MC))
            {
                linktype = LINKTYPE_MC;
            }
            else if ((linktype == LINKTYPE_TCP) || (linktype == LINKTYPE_UDP))
            {
                if (domainAddress.Length > 0)
                {
                    domainAddress = InetAddress.GetByName(domainAddress);
                }
            }
            else
            {
                throw new ConfigException(
                    "Illegal linktype: '" + linktype +
                    "'. Linktype for Channel must be either 'multicast', 'tcp', 'udp' or left blank( = multicast)");
            }
        }

        public void populateTopic(Topic top)
        {
            // If Topic doesn't specify a transport it will default to 'multicast', therefore
            // we can't just check for an empty 'top.getTransport()' to know when to replace value.
            // Therfore, if a topic is listed in a 'Transport/Channel', we assume it shall
            // use the channel values, so replace all values.
            top.SetTransport(linktype);
            top.SetLocalInterface(localInterface);
            top.SetDomainAddress(domainAddress);
            top.SetPort(port);
            top.SetOutSocketBufferSize((int)outSocketBufferSize);
            top.SetInSocketBufferSize((int)inSocketBufferSize);
            top.SetTimeToLive(timeToLive);
            top.ChannelID = channelID;
        }

        public override object Clone()
        {
            Channel cloneResult = new Channel();
            FillClone(cloneResult);
            return cloneResult;
        }

        public override void FillClone(OPSObject cloneO)
        {
            base.FillClone(cloneO);
            Channel cloneResult = (Channel)cloneO;
            cloneResult.Channel_version = Channel_version;
            cloneResult.channelID = channelID;
            cloneResult.linktype = linktype;
            cloneResult.localInterface = localInterface;
            cloneResult.domainAddress = domainAddress;
            cloneResult.timeToLive = timeToLive;
            cloneResult.port = port;
            cloneResult.outSocketBufferSize = outSocketBufferSize;
            cloneResult.inSocketBufferSize = inSocketBufferSize;
        }
    }
}
