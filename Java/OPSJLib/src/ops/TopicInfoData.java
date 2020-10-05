/**
*
* Copyright (C) 2013, 2020 Lennart Andersson.
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

import ops.OPSObject;
import configlib.ArchiverInOut;
import configlib.SerializableFactory;
import configlib.Serializable;
import java.io.IOException;

// NOTE. Must be kept in sync with C++
public class TopicInfoData extends OPSObject
{
    public byte TopicInfoData_version = TopicInfoData_idlVersion;

    public static final byte TopicInfoData_idlVersion = 0;
    public String name;
    public String type;
    public String transport;
    public String address;
    public int port;

    // C++ version: std::vector<string> keys;
    public java.util.Vector<String> keys = new java.util.Vector<String>();

    // C++ version: //std::vector<OPSObject*> filters;


    public TopicInfoData()
    {
        super();
        appendType("TopicInfoData");
    }

    public TopicInfoData(Topic topic)
    {
        super();
        appendType("TopicInfoData");
        name = topic.getName();
        type = topic.getTypeID();
        transport = topic.getTransport();
        address = topic.getDomainAddress();
        port = topic.getPort();
        //keys;
    }

    public void serialize(ArchiverInOut archive) throws IOException
    {
        super.serialize(archive);

        if (idlVersionMask != 0) {
            byte tmp = archive.inout("TopicInfoData_version", TopicInfoData_version);
            if (tmp > TopicInfoData_idlVersion) {
                throw new IOException("TopicInfoData: received version '" + tmp + "' > known version '" + TopicInfoData_idlVersion + "'");
            }
            TopicInfoData_version = tmp;
        } else {
            TopicInfoData_version = 0;
        }

        name = archive.inout("name", name);
        type = archive.inout("type", type);
        transport = archive.inout("transport", transport);
        address = archive.inout("address", address);
        port = archive.inout("port", port);
        keys = (java.util.Vector<String>) archive.inoutStringList("keys", keys);
    }

}
