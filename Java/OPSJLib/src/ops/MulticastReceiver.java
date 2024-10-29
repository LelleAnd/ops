/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
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

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.MulticastSocket;
import java.net.NetworkInterface;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

/**
 *
 * @author Anton Gravestam
 */
public class MulticastReceiver implements Receiver
{
    private MulticastSocket multicastSocket = null;
    int port;
    InetAddress ipAddress;
    String localInterface;
    int receiveBufferSize;
    private Event newBytesEvent = new Event();
    boolean opened = false;
    String remoteSenderIp = "";
    int remoteSenderPort = 0;

    byte[] tempBytes = new byte[StaticManager.MAX_SIZE];
    /** Creates a new instance of MulticastReceiver */
    public MulticastReceiver(String ip, int port, String localInterface, int receiveBufferSize) throws IOException
    {
        ipAddress = InetAddress.getByName(ip);
        this.port = port;
        this.localInterface = localInterface;
        this.receiveBufferSize = receiveBufferSize;
        Open();
    }

    public MulticastReceiver(String ip, int port) throws IOException
    {
        this(ip, port, "0.0.0.0", 0);
    }

    public final void Open() throws IOException
    {
        if (!opened) {
            SocketAddress mcSocketAddress = new InetSocketAddress(ipAddress, port);
            ///LA We can't use the MulticastSocketCreator() since it will return the same socket
            /// even if they are on different domains.
            /// And we don't need it, since the ReceiveDataHandler() handles the case when
            /// several subscribers use the same port
            ///multicastSocket = MulticastSocketCreator.getMulticastSocket(port);
            multicastSocket = new MulticastSocket(port);

            if(!localInterface.equals("0.0.0.0"))
            {//For some reason this method throws an error if we try to set outgoing interface to ANY.
                multicastSocket.setNetworkInterface(NetworkInterface.getByInetAddress(Inet4Address.getByName(localInterface)));
            }
            if(receiveBufferSize > 0)
            {
                multicastSocket.setReceiveBufferSize(receiveBufferSize);
            }
//            try
//            {
//                setReceiveTimeout(100);
//            } catch (CommException ex)
//            {
//                //Logger.getLogger(MulticastReceiver.class.getName()).log(Level.SEVERE, null, ex);
//            }

            //multicastSocket.setReuseAddress(true);
            //multicastSocket.bind(new InetSocketAddress(port));
            multicastSocket.setTimeToLive(1);
            multicastSocket.joinGroup(mcSocketAddress, NetworkInterface.getByInetAddress(InetAddress.getByName(localInterface)));
            opened = true;
        }
    }

    public void Close()
    {
        if (opened && (multicastSocket != null)) {
            multicastSocket.close();
        }
        opened = false;
    }

    public String getRemoteIp()
    {
        return remoteSenderIp;
    }

    public int getRemotePort()
    {
        return remoteSenderPort;
    }

//    public boolean receive(byte[] b, int offset)
//    {
//        DatagramPacket p = new DatagramPacket(b, offset, StaticManager.MAX_SIZE);
//        try
//        {
//            multicastSocket.receive(p);
//            newBytesEvent.fireEvent(p);
//            return true;
//        }
//        catch (SocketTimeoutException ex)
//        {
//            return false;
//        }
//        catch (IOException ex)
//        {
//            return false;
//        }
//    }

//    public boolean receive(byte[] b) //throws ReceiveTimedOutException
//    {
//        return receive(b, 0);
//    }

    public boolean receive(byte[] headerBytes, byte[] bytes, int offset)
    {
        DatagramPacket p = new DatagramPacket(tempBytes, 0, StaticManager.MAX_SIZE);
        try
        {
            multicastSocket.receive(p);
            if(p.getSocketAddress().equals(multicastSocket.getLocalSocketAddress()))
            {
                System.out.println("Ingnoring packaege from my self");
                return false;
            }

            ByteBuffer nioBuf = ByteBuffer.wrap(tempBytes);
            nioBuf.get(headerBytes, 0, headerBytes.length);
            nioBuf.get(bytes, offset, p.getLength() - headerBytes.length);

            remoteSenderIp = p.getAddress().getHostAddress();
            remoteSenderPort = p.getPort();

            newBytesEvent.fireEvent(new Integer(p.getLength()));
            return true;
        }
        catch (SocketTimeoutException ex)
        {
            return false;
        }
        catch (IOException ex)
        {
            return false;
        }
    }

    public Event getNewBytesEvent()
    {
        return newBytesEvent;
    }

}
