/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2024 Lennart Andersson.
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
import java.net.DatagramSocket;
import java.net.NetworkInterface;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.util.Enumeration;

/**
 *
 * @author Anton Gravestam
 */
public class UDPReceiver implements Receiver
{
    private DatagramSocket udpSocket = null;
    private int port;
    private String localInterface;
    private int receiveBufferSize;
    private Event newBytesEvent = new Event();
    private boolean opened = false;

    private int actualPort;
    private String actualIP;
    private String remoteSenderIp = "";
    private int remoteSenderPort = 0;

    byte[] tempBytes = new byte[StaticManager.MAX_SIZE];

    /** Creates a new instance of MulticastReceiver */
    public UDPReceiver(int port, String localInterface, int receiveBufferSize) throws IOException
    {
        this.port = port;
        this.localInterface = localInterface;
        this.receiveBufferSize = receiveBufferSize;

        Open();
    }

    public UDPReceiver(int port) throws IOException
    {
        this(port, "0.0.0.0", 0);
    }

    private String getLocalInterface()
    {
        try
        {
            Enumeration<NetworkInterface> nets = NetworkInterface.getNetworkInterfaces();
            for (NetworkInterface netint : java.util.Collections.list(nets))
            {
                java.util.List<java.net.InterfaceAddress> ifAddresses = netint.getInterfaceAddresses();
                for (java.net.InterfaceAddress ifAddress : ifAddresses) {
                    if (ifAddress.getAddress() instanceof Inet4Address) {
                        // split "hostname/127.0.0.1/8 [0.255.255.255]"
                        String s[] = ifAddress.toString().split("/");
                        return s[1];
                    }
                }
            }
        }
        catch (Exception e)
        {
        }
        return "127.0.0.1";
    }

    public final void Open() throws IOException
    {
        if (!opened) {
            String ipAddress = "0.0.0.0";

            // We need to bind to a specific interface so we can get an IP address to publish to udp senders
            if (localInterface.equals("0.0.0.0"))
            {
                ipAddress = getLocalInterface();
            }
            else
            {
                ipAddress = localInterface;
            }

            // If this.port == 0, the system will assign us a port. We fetch the used port below.
            SocketAddress ep = new InetSocketAddress(ipAddress, port);
            udpSocket = new DatagramSocket(ep);

            // Fetch actualy used ip and port that we listen to
            // split "hostname/127.0.0.1/8 [0.255.255.255]"
            String s = udpSocket.getLocalSocketAddress().toString().split("/")[1];
            actualIP = s.split(":")[0];
            actualPort = udpSocket.getLocalPort();

            if (receiveBufferSize > 0)
            {
                udpSocket.setReceiveBufferSize(receiveBufferSize);
            }

            opened = true;
        }
    }

    public void Close()
    {
        if (opened && (udpSocket != null)) {
            udpSocket.close();
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

    public String getIP()
    {
        return actualIP;
    }

    public int getPort()
    {
        return actualPort;
    }

    public boolean receive(byte[] headerBytes, byte[] bytes, int offset)
    {
        DatagramPacket p = new DatagramPacket(tempBytes, 0, StaticManager.MAX_SIZE);
        try
        {
            udpSocket.receive(p);

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
