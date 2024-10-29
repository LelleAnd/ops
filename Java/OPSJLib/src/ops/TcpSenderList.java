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
import java.net.InetAddress;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author angr
 */
public class TcpSenderList implements Sender
{
    Vector<Socket> senders = new Vector<Socket>();
    private static final String sizeHeader = "opsp_tcp_size_info";

    public void open()
    {
        // Nothing to do
    }

    public void close()
    {
        // Nothing to do
    }

    public int getLocalPort()
    {
        return 0;
    }

    public synchronized void emptyList()
    {
        for (Socket s : senders)
        {
            try {
                s.close();
            } catch (IOException ex) {
            }
        }
        senders.clear();
    }

    public synchronized boolean remove(Socket o)
    {
        return senders.remove(o);
    }

    public synchronized boolean add(Socket e)
    {
        return senders.add(e);
    }

    public synchronized boolean sendTo(byte[] bytes, String ip, int port)
    {
        return this.sendTo(bytes, 0, bytes.length, ip, port);
    }

    public synchronized boolean sendTo(byte[] bytes, int offset, int size, String ip, int port)
    {
        Vector<Socket> failedSenders = new Vector<Socket>();

        for (Socket s : senders)
        {
            try
            {
                //First, prepare and send a package of fixed length 22 with information about the size of the data package
                s.getOutputStream().write(sizeHeader.getBytes());
                ByteBuffer bb = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(size);
                s.getOutputStream().write(bb.array());

                //Send the actual data
                s.getOutputStream().write(bytes, offset, size);
            } catch (IOException ex)
            {
                Logger.getLogger(TcpSenderList.class.getName()).log(Level.INFO, null, ex);
                failedSenders.add(s);
            }
        }

        for (Socket s : failedSenders) {
            try {
                s.close();
            } catch (IOException ex) {
            }
            remove(s);
        }

        return true;
    }

    public synchronized boolean sendTo(byte[] bytes, int offset, int size, InetAddress ipAddress, int port)
    {
        return this.sendTo(bytes, offset, size, ipAddress.getHostAddress(), port);
    }

//    public synchronized void remove(String ip, int port) throws IOException
//    {
//        for (int i = 0; i < senders.size(); i++)
//        {
//            if (senders.get(i).getInetAddress().getHostAddress().equals(ip) && senders.get(i).getPort() == port)
//            {
//                senders.get(i).close();
//                senders.remove(i);
//            }
//
//        }
//    }
}
