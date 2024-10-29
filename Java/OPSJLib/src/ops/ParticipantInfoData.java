/**
*
* Copyright (C) 2013, 2020-2024 Lennart Andersson.
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
public class ParticipantInfoData extends OPSObject
{
    public byte ParticipantInfoData_version = ParticipantInfoData_idlVersion;

    public static final byte ParticipantInfoData_idlVersion = 0;
    public String name;
    public String id;
    public String domain;
    public String ip;
    public String languageImplementation;
    public String opsVersion;
    public int mc_udp_port;
    public int mc_tcp_port;

    public java.util.Vector<TopicInfoData> subscribeTopics = new java.util.Vector<TopicInfoData>();

    public java.util.Vector<TopicInfoData> publishTopics = new java.util.Vector<TopicInfoData>();

    // C++ version: std::vector<string> knownTypes;
    public java.util.Vector<String> knownTypes = new java.util.Vector<String>();


    public ParticipantInfoData()
    {
        super();
	      appendType("ops.ParticipantInfoData");
    }

    public void serialize(ArchiverInOut archive) throws IOException
    {
        super.serialize(archive);

        if (idlVersionMask != 0) {
            byte tmp = archive.inout("ParticipantInfoData_version", ParticipantInfoData_version);
            if (tmp > ParticipantInfoData_idlVersion) {
                throw new IOException("ParticipantInfoData: received version '" + tmp + "' > known version '" + ParticipantInfoData_idlVersion + "'");
            }
            ParticipantInfoData_version = tmp;
        } else {
            ParticipantInfoData_version = 0;
        }

        name = archive.inout("name", name);
        domain = archive.inout("domain", domain);
        id = archive.inout("id", id);
        ip = archive.inout("ip", ip);
        languageImplementation = archive.inout("languageImplementation", languageImplementation);
        opsVersion = archive.inout("opsVersion", opsVersion);
        mc_udp_port = archive.inout("mc_udp_port", mc_udp_port);
        mc_tcp_port = archive.inout("mc_tcp_port", mc_tcp_port);
        /// The following requires that TopicInfoData can be created by the OPSObjectFactory. Differs from C++ & C# that has a different solution.
        subscribeTopics = (java.util.Vector<TopicInfoData>)archive.inoutSerializableList("subscribeTopics", subscribeTopics);
        publishTopics = (java.util.Vector<TopicInfoData>)archive.inoutSerializableList("publishTopics", publishTopics);
        knownTypes = (java.util.Vector<String>) archive.inoutStringList("knownTypes", knownTypes);
    }

    @Override
    public Object clone()
    {
        ParticipantInfoData cloneResult = new ParticipantInfoData();
        fillClone(cloneResult);
        return cloneResult;
    }

    @Override
    public void fillClone(OPSObject cloneO)
    {
        super.fillClone(cloneO);
        ParticipantInfoData cloneResult = (ParticipantInfoData)cloneO;
        cloneResult.ParticipantInfoData_version = this.ParticipantInfoData_version;
        cloneResult.name = this.name;
        cloneResult.id = this.id;
        cloneResult.domain = this.domain;
        cloneResult.ip = this.ip;
        cloneResult.languageImplementation = this.languageImplementation;
        cloneResult.opsVersion = this.opsVersion;
        cloneResult.mc_udp_port = this.mc_udp_port;
        cloneResult.mc_tcp_port = this.mc_tcp_port;
        cloneResult.subscribeTopics = new java.util.Vector<TopicInfoData>();
        this.subscribeTopics.forEach((item) -> cloneResult.subscribeTopics.add((TopicInfoData)item.clone()));
        cloneResult.publishTopics = new java.util.Vector<TopicInfoData>();
        this.publishTopics.forEach((item) -> cloneResult.publishTopics.add((TopicInfoData)item.clone()));
        cloneResult.knownTypes = (java.util.Vector)this.knownTypes.clone();
    }
}
