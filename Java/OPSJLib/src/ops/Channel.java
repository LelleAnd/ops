/**
 *
 * Copyright (C) 2016 Lennart Andersson.
 *
 * This file is part of OPS (Open Publish Subscribe).
 *
 * OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
 */
package ops;

import configlib.ArchiverInOut;
import java.io.IOException;

public class Channel extends OPSObject
{
    public String channelID = "";
    public String linktype = "";
    public String localInterface = "";     // If multicast, this specifies interface to use
    public String domainAddress = "";
    public int timeToLive = -1;            // if multicast, this specifies the ttl parameter
    public int port = 0;
    public long outSocketBufferSize = -1;
    public long inSocketBufferSize = -1;

    public static final String LINKTYPE_MC = "multicast";
    public static final String LINKTYPE_TCP = "tcp";
    public static final String LINKTYPE_UDP = "udp";

    public Channel()
    {
        appendType("Channel");
    }

		@Override
    public void serialize(ArchiverInOut archive) throws IOException
    {
        // NOTE. Keep this in sync with the C++ version, so it in theory is possible to send these as objects.
        // We need to serialize fields in the same order as C++.
        //OPSObject::serialize(archiver);
        super.serialize(archive);

        //archiver->inout(std::string("name"), channelID);
        //archiver->inout(std::string("linktype"), linktype);
        //archiver->inout(std::string("localInterface"), localInterface);
        //archiver->inout(std::string("address"), domainAddress);
        channelID = archive.inout("name", channelID);
        linktype = archive.inout("linktype", linktype);
        localInterface = archive.inout("localInterface", localInterface);
        domainAddress = archive.inout("address", domainAddress);

        //archiver->inout(std::string("timeToLive"), timeToLive);
        //archiver->inout(std::string("port"), port);
        //archiver->inout(std::string("outSocketBufferSize"), outSocketBufferSize);
        //archiver->inout(std::string("inSocketBufferSize"), inSocketBufferSize);
        timeToLive = archive.inout("timeToLive", timeToLive);
        port = archive.inout("port", port);
        outSocketBufferSize = archive.inout("outSocketBufferSize", outSocketBufferSize);
        inSocketBufferSize = archive.inout("inSocketBufferSize", inSocketBufferSize);

        if (linktype.equals(""))
        {
            linktype = LINKTYPE_MC;
        }
        else if ((!linktype.equals(LINKTYPE_MC)) && (!linktype.equals(LINKTYPE_TCP)) && (!linktype.equals(LINKTYPE_UDP)))
        {
            throw new IOException(
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
        top.setTransport(linktype);
        top.setLocalInterface(localInterface);
        top.setDomainAddress(domainAddress);
        top.setPort(port);
        top.setOutSocketBufferSize((int)outSocketBufferSize);
        top.setInSocketBufferSize((int)inSocketBufferSize);
        top.setTimeToLive(timeToLive);
    }

}
