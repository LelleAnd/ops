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
public class Reply extends OPSObject
{
    public byte Reply_version = Reply_idlVersion;

    public static final byte Reply_idlVersion = 0;
    public String requestId = "";
    public boolean requestAccepted = false;
    public String message = "";

    @Override
    public void serialize(ArchiverInOut archive) throws IOException
    {
        super.serialize(archive);
        if (idlVersionMask != 0) {
            byte tmp = archive.inout("Reply_version", Reply_version);
            if (tmp > Reply_idlVersion) {
                throw new IOException("Reply: received version '" + tmp + "' > known version '" + Reply_idlVersion + "'");
            }
            Reply_version = tmp;
        } else {
            Reply_version = 0;
        }
        requestId = archive.inout("requestId", requestId);
        requestAccepted = archive.inout("requestAccepted", requestAccepted);
        message = archive.inout("message", message);
    }

}
