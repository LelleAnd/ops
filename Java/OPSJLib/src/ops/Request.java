/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package ops;

import configlib.ArchiverInOut;
import java.io.IOException;

/**
 *
 * @author Anton Gravestam
 */
public class Request extends OPSObject
{
    public byte Request_version = Request_idlVersion;

    public static final byte Request_idlVersion = 0;
    public String requestId = "";

    @Override
    public void serialize(ArchiverInOut archive) throws IOException
    {
        super.serialize(archive);
        if (idlVersionMask != 0) {
            byte tmp = archive.inout("Request_version", Request_version);
            if (tmp > Request_idlVersion) {
                throw new IOException("Request: received version '" + tmp + "' > known version '" + Request_idlVersion + "'");
            }
            Request_version = tmp;
        } else {
            Request_version = 0;
        }
        requestId = archive.inout("requestId" , requestId);
    }



}
