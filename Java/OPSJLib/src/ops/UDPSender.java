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
import java.net.DatagramSocket;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.SocketAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;

/**
 *
 * @author Anton Gravestam
 */
public class UDPSender implements Sender
{
    private DatagramSocket udpSocket;
    private int port;
    private int outSocketBufferSize;
    private boolean opened = false;
    private SocketAddress ipEndPoint;


    public UDPSender(int port, String localInterface, int outSocketBufferSize) throws IOException
    {
        this.ipEndPoint = new InetSocketAddress(localInterface, port);
        this.outSocketBufferSize = outSocketBufferSize;
        open();
    }

    public final void open() throws IOException
    {
        if (!opened) {
            udpSocket = new DatagramSocket(this.ipEndPoint);

            if (outSocketBufferSize > 0)
            {
                udpSocket.setSendBufferSize(outSocketBufferSize);
            }
            opened = true;
        }
    }

    public void close()
    {
        if (opened) {
            udpSocket.close();
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
            udpSocket.send(datagramPacket);
            return true;
        } catch (IOException ex)
        {
            return false;
        }
    }

    public boolean sendTo(byte[] bytes, String ip, int port)
    {
        return this.sendTo(bytes, 0, bytes.length, ip, port);

    }
}
