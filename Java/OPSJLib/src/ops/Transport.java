/**
 *
 * Copyright (C) 2016-2024 Lennart Andersson.
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

public class Transport extends OPSObject
{
    public byte Transport_version = Transport_idlVersion;

    public static final byte Transport_idlVersion = 0;
    public String channelID = "";
		public java.util.Vector<String> topics = new java.util.Vector<String>();

    public Transport()
    {
        appendType("Transport");
    }

		@Override
    public void serialize(ArchiverInOut archive) throws IOException
    {
        // NOTE. Keep this in sync with the C++ version, so it in theory is possible to send these as objects.
        // We need to serialize fields in the same order as C++.
        //OPSObject::serialize(archiver);
        super.serialize(archive);

        if (idlVersionMask != 0) {
            byte tmp = archive.inout("Transport_version", Transport_version);
            if (tmp > Transport_idlVersion) {
                throw new IOException("Transport: received version '" + tmp + "' > known version '" + Transport_idlVersion + "'");
            }
            Transport_version = tmp;
        } else {
            Transport_version = 0;
        }

        //archiver->inout(std::string("channelID"), channelID);
        channelID = archive.inout("channelID", channelID);

        //archiver->inout(std::string("topics"), topics);
				topics = (java.util.Vector<String>) archive.inoutStringList("topics", topics);
    }

    @Override
    public Object clone()
    {
        Transport cloneResult = new Transport();
        fillClone(cloneResult);
        return cloneResult;
    }

    @Override
    public void fillClone(OPSObject cloneO)
    {
        super.fillClone(cloneO);
        Transport cloneResult = (Transport)cloneO;
        cloneResult.Transport_version = this.Transport_version;
        cloneResult.channelID = this.channelID;
        cloneResult.topics = (java.util.Vector)this.topics.clone();
    }

}
