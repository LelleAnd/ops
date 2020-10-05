/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2019-2020 Lennart Andersson.
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
import configlib.XMLArchiverIn;
import java.io.IOException;
import java.util.Vector;

/**
 *
 * @author angr
 */
public class Domain extends OPSObject
{
    public byte Domain_version = Domain_idlVersion;

    public static final byte Domain_idlVersion = 0;
    private String domainAddress = "";
    private String domainID = "";
    private String localInterface = "0.0.0.0";
    protected Vector<Topic> topics = new Vector<Topic>();
    private int timeToLive = 1;
    private int inSocketBufferSize = -1;    // Use OS default
    private int outSocketBufferSize = -1;   // Use OS default
    private int metaDataMcPort = 9494;
    private boolean optNonVirt = false;

    private Vector<Channel> channels = new Vector<Channel>();
    private Vector<Transport> transports = new Vector<Transport>();

    public Domain()
    {
        appendType("Domain");
    }

    private void checkTopicValues(Topic top)
    {
      if (top.getDomainAddress().equals(""))
      {
        top.setDomainAddress(domainAddress);
      }
      if (top.getLocalInterface().equals(""))
      {
        top.setLocalInterface(localInterface);
      }
      if (top.getTimeToLive() < 0)
      {
        top.setTimeToLive(timeToLive);
      }
      if (top.getInSocketBufferSize() < 0)
      {
        top.setInSocketBufferSize(inSocketBufferSize);
      }
      if (top.getOutSocketBufferSize() < 0)
      {
        top.setOutSocketBufferSize(outSocketBufferSize);
      }
      top.setOptNonVirt(optNonVirt);
    }

    public Topic getTopic(String name)
    {
        for (Topic topic : topics)
        {
            if(topic.getName().equals(name))
            {
                checkTopicValues(topic);
                return topic;
            }
        }
        return null;
    }

    @Override
    public void serialize(ArchiverInOut archive) throws IOException
    {
        // NOTE. Keep this in sync with the C++ version, so it in theory is possible to send these as objects.
        // We need to serialize fields in the same order as C++.
        //OPSObject::serialize(archiver);
        super.serialize(archive);

        if (idlVersionMask != 0) {
            byte tmp = archive.inout("Domain_version", Domain_version);
            if (tmp > Domain_idlVersion) {
                throw new IOException("Domain: received version '" + tmp + "' > known version '" + Domain_idlVersion + "'");
            }
            Domain_version = tmp;
        } else {
            Domain_version = 0;
        }

        //archiver->inout(std::string("domainID"), domainID);
        //archiver->inout<Topic>(std::string("topics"), topics);
        //archiver->inout(std::string("domainAddress"), domainAddress);
        //archiver->inout(std::string("localInterface"), localInterface);
        domainID = archive.inout("domainID", domainID);
        topics = (Vector<Topic>) archive.inoutSerializableList("topics", topics);
        domainAddress = archive.inout("domainAddress", domainAddress);
        localInterface = archive.inout("localInterface", localInterface);

        //archiver->inout(std::string("timeToLive"), timeToLive);
        //archiver->inout(std::string("inSocketBufferSize"), inSocketBufferSize);
        //archiver->inout(std::string("outSocketBufferSize"), outSocketBufferSize);
        //archiver->inout(std::string("metaDataMcPort"), metaDataMcPort);
        timeToLive = archive.inout("timeToLive", timeToLive);
        inSocketBufferSize = archive.inout("inSocketBufferSize", inSocketBufferSize);
        outSocketBufferSize = archive.inout("outSocketBufferSize", outSocketBufferSize);
        metaDataMcPort = archive.inout("metaDataMcPort", metaDataMcPort);

        // To not break binary compatibility we only do this when we know we are
      	// reading from an XML-file
      	if (archive instanceof XMLArchiverIn) {
      		  channels = (Vector<Channel>) archive.inoutSerializableList("channels", channels);
      		  transports = (Vector<Transport>) archive.inoutSerializableList("transports", transports);
      		  optNonVirt = archive.inout("optNonVirt", optNonVirt);
      		  checkTransports();
      	}
    }

    private Channel findChannel(String id)
    {
      if (!id.equals("")) {
        for (Channel channel : channels) {
          if (id.equals(channel.channelID)) return channel;
        }
      }
      return null;
    }

    private Topic findTopic(String id)
    {
      if (!id.equals("")) {
        for (Topic topic : topics) {
          if (id.equals(topic.getName())) return topic;
        }
      }
      return null;
    }

    void checkTransports() throws IOException
    {
      // Now update topics with values from the transports and channels
      // Loop over all transports and for each topic, see if it needs parameters from the channel
      for (Transport transport : transports) {
        // Get channel
        Channel channel = findChannel(transport.channelID);
        if (channel == null) {
          throw new IOException(
            "Non existing channelID: '" + transport.channelID +
            "' used in transport spcification.");
        } else {
          for (String topicName : transport.topics) {
            Topic top = findTopic(topicName);
            if (top == null) {
              throw new IOException(
                "Non existing topicID: '" + topicName +
                "' used in transport spcification.");
            } else {
              channel.populateTopic(top);
            }
          }
        }
      }
    }

    public Vector<Topic> getTopics() {
        for (Topic topic : topics) {
            checkTopicValues(topic);
        }
        return topics;
    }

     public boolean existTopic(String name) {
        for (Topic topic : topics) {
            if (topic.getName().equals(name)) {
                return true;
            }
        }
        return false;
    }

    public String getDomainAddress() {
        return domainAddress;
    }

    public String getDomainID() {
        return domainID;
    }

    public String getLocalInterface() {
        return localInterface;
    }

    public int GetMetaDataMcPort()
    {
        return metaDataMcPort;
    }

    public int GetInSocketBufferSize()
    {
        return inSocketBufferSize;
    }

    public int GetOutSocketBufferSize()
    {
        return outSocketBufferSize;
    }

    public int getTimeToLive()
    {
        return timeToLive;
    }

    public void setDomainAddress(String domainAddress) {
        this.domainAddress = domainAddress;
    }

    public void setDomainID(String domainID) {
        this.domainID = domainID;
    }

    public void setLocalInterface(String localInterface) {
        this.localInterface = localInterface;
    }

    public boolean getOptNonVirt()
    {
        return optNonVirt;
    }

}
