/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package ops;

import java.io.IOException;
import java.net.InetAddress;
import java.util.Vector;

/**
 *
 * @author Lelle
 */
public class TcpSendDataHandler extends SendDataHandlerBase
{
    private final InetAddress sinkIP;

    public TcpSendDataHandler(Topic t, String localInterface) throws IOException
    {
        sender = new TcpServerSender(t.getDomainAddress(), t.getPort(), t.getOutSocketBufferSize());
        sinkIP = InetAddress.getByName(t.getDomainAddress());
    }

    @Override
    public void cleanUp()
    {
        ((TcpServerSender)sender).stopThread();
    }

    @Override
    public void updateTransportInfo(Topic t)
    {
        int port = sender.getLocalPort();

        if (!NetworkSupport.IsValidNodeAddress(t.getDomainAddress()))
        {
            t.setDomainAddress(NetworkSupport.DoSubnetTranslation(t.getLocalInterface()));
        }

        t.setPort(port);
    }

    public synchronized boolean sendData(byte[] bytes, int size, Topic t)
    {
        return sendData(bytes, size, sinkIP, t.getPort());
    }

}
