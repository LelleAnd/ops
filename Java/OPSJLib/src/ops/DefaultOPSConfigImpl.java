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
import java.io.IOException;

/**
 *
 * @author angr
 */
public class DefaultOPSConfigImpl extends OPSConfig
{
    public byte DefaultOPSConfigImpl_version = DefaultOPSConfigImpl_idlVersion;

    public static final byte DefaultOPSConfigImpl_idlVersion = 0;

    public DefaultOPSConfigImpl()
    {
        appendType("DefaultOPSConfigImpl");
    }

    @Override
    public void serialize(ArchiverInOut archive) throws IOException
    {
        super.serialize(archive);
        if (idlVersionMask != 0) {
            byte tmp = archive.inout("DefaultOPSConfigImpl_version", DefaultOPSConfigImpl_version);
            if (tmp > DefaultOPSConfigImpl_idlVersion) {
                throw new IOException("DefaultOPSConfigImpl: received version '" + tmp + "' > known version '" + DefaultOPSConfigImpl_idlVersion + "'");
            }
            DefaultOPSConfigImpl_version = tmp;
        } else {
            DefaultOPSConfigImpl_version = 0;
        }
    }

}
