/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2020-2024 Lennart Andersson.
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

package ops.protocol;

import configlib.ArchiverInOut;
import java.io.IOException;
import ops.*;

/**
 *
 * @author Anton Gravestam
 */
public class OPSMessage extends OPSObject
{
    public byte OPSMessage_version = OPSMessage_idlVersion;

    public static final byte OPSMessage_idlVersion = 0;
    private byte messageType;
    private byte  endianness;
    private byte publisherPriority;
    private int port;
    private long qosMask;
    private long publicationID;
    private int sourcePort = 0;
    private String sourceIP = "";
    private String publisherName = "";
    private String topicName = "";
    private String topLevelKey = "";
    private String address = "";
    private OPSObject data;

    /** Creates a new instance of OPSMessage */
    public OPSMessage()
    {
        super();
        appendType("ops.protocol.OPSMessage");
    }

    public String getAddress()
    {
        return address;
    }

    public void setAddress(String address)
    {
        this.address = address;
    }

    public OPSObject getData()
    {
        return data;
    }

    public void setData(OPSObject data)
    {
        this.data = data;
    }

    public void setSource(String ip, int port)
    {
        this.sourceIP = ip;
        this.sourcePort = port;
    }

    public String getSourceIP()
    {
        return sourceIP;
    }

    public int getSourcePort()
    {
        return sourcePort;
    }

    public byte getEndianness()
    {
        return endianness;
    }

    public void setEndianness(byte endianness)
    {
        this.endianness = endianness;
    }

    public byte getMessageType()
    {
        return messageType;
    }

    public void setMessageType(byte messageType)
    {
        this.messageType = messageType;
    }

    public int getPort()
    {
        return port;
    }

    public void setPort(int port)
    {
        this.port = port;
    }

    public long getPublicationID()
    {
        return publicationID;
    }

    public void setPublicationID(long publicationID)
    {
        this.publicationID = publicationID;
    }

    public String getPublisherName()
    {
        return publisherName;
    }

    public void setPublisherName(String publisherName)
    {
        this.publisherName = publisherName;
    }

    public byte getPublisherPriority()
    {
        return publisherPriority;
    }

    public void setPublisherPriority(byte publisherPriority)
    {
        this.publisherPriority = publisherPriority;
    }

    public long getQosMask()
    {
        return qosMask;
    }

    public void setQosMask(long qosMask)
    {
        this.qosMask = qosMask;
    }

    public String getTopLevelKey()
    {
        return topLevelKey;
    }

    public void setTopLevelKey(String topLevelKey)
    {
        this.topLevelKey = topLevelKey;
    }

    public String getTopicName()
    {
        return topicName;
    }

    public void setTopicName(String topicName)
    {
        this.topicName = topicName;
    }

    @Override
    public void serialize(ArchiverInOut archive) throws IOException
    {
        super.serialize(archive);

        if (idlVersionMask != 0) {
            byte tmp = archive.inout("OPSMessage_version", OPSMessage_version);
            if (tmp > OPSMessage_idlVersion) {
                throw new IOException("OPSMessage: received version '" + tmp + "' > known version '" + OPSMessage_idlVersion + "'");
            }
            OPSMessage_version = tmp;
        } else {
            OPSMessage_version = 0;
        }

        messageType = archive.inout("messageType", messageType);
        publisherPriority = archive.inout("publisherPriority", publisherPriority);
        publicationID = archive.inout("publicationID", publicationID);
        publisherName = archive.inout("publisherName", publisherName);
        topicName = archive.inout("topicName", topicName);
        topLevelKey = archive.inout("topLevelKey", topLevelKey);
        address = archive.inout("address", address);
        data = (OPSObject) archive.inout("data", data);
    }

    @Override
    public Object clone()
    {
        OPSMessage cloneResult = new OPSMessage();
        fillClone(cloneResult);
        return cloneResult;
    }

    @Override
    public void fillClone(OPSObject cloneO)
    {
        super.fillClone(cloneO);
        OPSMessage cloneResult = (OPSMessage) cloneO;
        cloneResult.OPSMessage_version = OPSMessage_version;
        cloneResult.setKey(this.getKey());
        cloneResult.messageType = messageType;
        cloneResult.publisherPriority = publisherPriority;
        cloneResult.publicationID = publicationID;
        cloneResult.publisherName = publisherName;
        cloneResult.topicName = topicName;
        cloneResult.topLevelKey = topLevelKey;
        cloneResult.address = address;
        cloneResult.data = (OPSObject)data.clone();
    }

}
