/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2020 Lennart Andersson.
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
import configlib.XMLArchiverOut;
import configlib.exception.FormatException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Vector;
import ops.archiver.OPSObjectFactory;
import ops.OPSConfigRepository;

/**
 *
 * @author angr
 */
public class OPSConfig extends OPSObject
{
    public byte OPSConfig_version = OPSConfig_idlVersion;

    public static final byte OPSConfig_idlVersion = 0;

    //static OPSConfig theConfig = null;
    public static OPSConfig getConfig() throws IOException, FormatException
    {
        return OPSConfigRepository.getConfig(); // getConfig(new File("ops_config.xml"));
    }

    public static OPSConfig getConfig(File configFile) throws IOException, FormatException
    {
        FileInputStream fis = new FileInputStream(configFile);
        XMLArchiverIn archiverIn = new XMLArchiverIn(fis, "root");
        archiverIn.add(OPSObjectFactory.getInstance());
        OPSConfig newConfig = null;
        newConfig = (OPSConfig) archiverIn.inout("ops_config", newConfig);

        return newConfig;
    }

    public static void saveConfig(OPSConfig conf, File configFile) throws FileNotFoundException, IOException
    {
        FileOutputStream fos = new FileOutputStream(configFile);
        XMLArchiverOut archiverOut = new XMLArchiverOut(fos, false);
        fos.write("<root xmlns=ops>\n".getBytes());
            archiverOut.inout("ops_config", conf);
        fos.write("\n</root>".getBytes());
    }

    //------------------------------
    protected Vector<Domain> domains = new Vector<Domain>();

    public OPSConfig()
    {
        appendType("OPSConfig");
    }

    public Domain getDomain(String domainID)
    {
        for (Domain domain : domains)
        {
            if (domain.getDomainID().equals(domainID))
            {
                return domain;
            }
        }
        return null;
    }

    @Override
    public void serialize(ArchiverInOut archive) throws IOException
    {
        super.serialize(archive);
        if (idlVersionMask != 0) {
            byte tmp = archive.inout("OPSConfig_version", OPSConfig_version);
            if (tmp > OPSConfig_idlVersion) {
                throw new IOException("OPSConfig: received version '" + tmp + "' > known version '" + OPSConfig_idlVersion + "'");
            }
            OPSConfig_version = tmp;
        } else {
            OPSConfig_version = 0;
        }
        domains = (Vector<Domain>) archive.inoutSerializableList("domains", domains);
    }

    public Vector<Domain> getDomains()
    {
        return domains;
    }

}
