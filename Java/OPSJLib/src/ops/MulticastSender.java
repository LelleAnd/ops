/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package ops;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.net.NetworkInterface;
import java.net.UnknownHostException;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author Anton Gravestam
 */
class MulticastSender implements Sender
{
    private MulticastSocket multicastSocket;
    private int port;
    private String localInterface;
    private int ttl;
    private int outSocketBufferSize;
    private boolean opened = false;

    public MulticastSender(int port, String localInterface, int ttl, int outSocketBufferSize) throws IOException
    {
        this.port = port;
        this.localInterface = localInterface;
        this.ttl = ttl;
        this.outSocketBufferSize = outSocketBufferSize;
        open();
    }

    public final void open() throws IOException
    {
        if (!opened) {
            //Can't use this, we want a separate socket, since we have our own outSocketBufferSize
            //multicastSocket = MulticastSocketCreator.getMulticastSocket(port);
            multicastSocket = new MulticastSocket(port);

            if (!localInterface.equals("0.0.0.0"))
            {//For some reason this method throws an error if we try to set outgoing interface to ANY.
                multicastSocket.setNetworkInterface(NetworkInterface.getByInetAddress(Inet4Address.getByName(localInterface)));
            }

            if (outSocketBufferSize > 0)
            {
                multicastSocket.setSendBufferSize(outSocketBufferSize);
            }
            multicastSocket.setTimeToLive(ttl);
            opened = true;
        }
    }

    public void close()
    {
        if (opened) {
            multicastSocket.close();
            opened = false;
        }
    }

    public int getLocalPort()
    {
        return this.port;
    }

    public boolean sendTo(byte[] bytes, int offset, int size, String ip, int port)
    {
        try
        {
            return this.sendTo(bytes, offset, size, InetAddress.getByName(ip), port);
        } catch (UnknownHostException ex)
        {
            return false;
        }

    }
    public boolean sendTo(byte[] bytes, int offset, int size, InetAddress ipAddress, int port)
    {
        try
        {

            DatagramPacket datagramPacket = new DatagramPacket(bytes, offset, size, ipAddress, port);

            multicastSocket.send(datagramPacket);

            return true;
        } catch (IOException ex)
        {
            return false;
        }

    }

    public boolean sendTo(byte[] bytes, String ip, int port)
    {
        return sendTo(bytes, 0, bytes.length, ip, port);

    }
}
