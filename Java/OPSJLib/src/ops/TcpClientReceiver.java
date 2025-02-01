/**
*
* Copyright (C) 2006-2010 Anton Gravestam.
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
import java.io.InputStream;
import java.net.Socket;
import java.net.SocketException;
import java.net.InetAddress;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author Anton
 */
public class TcpClientReceiver implements Receiver
{

    Event newBytesEvent = new Event();
    static final int sizePackSize = 22;
    private Socket socket;
    private InputStream inputStream;
    private byte[] sizePackBytes = new byte[sizePackSize];
    private final ByteBuffer sizePackBuffer;
    private final int serverPort;
    private final String serverIp;
    private final int receiveBufferSize;
    private boolean opened = false;
    private String remoteSenderIp = "";
    private int remoteSenderPort = 0;


    public TcpClientReceiver(String serverIP, int serverPort, int receiveBufferSize) throws IOException
    {
        this.serverPort = serverPort;
        this.serverIp = serverIP;
        this.receiveBufferSize = receiveBufferSize;
        Open();

        sizePackBuffer = ByteBuffer.wrap(sizePackBytes).order(ByteOrder.LITTLE_ENDIAN);
    }

    public final void Open() throws IOException
    {
        // Open is done by the Receive() method called by the ReceiveDataHandler
        // If we do it here, we will hang for about 1 second if the publisher (i.e server) isn't available
        //if (!this.opened)
        //{
        //    try
        //    {
        //        setupSocket(serverIp, serverPort, receiveBufferSize);
        //        this.opened = true;
        //    }
        //    catch (SocketException)
        //    {
        //    }
        //}
    }

    public void Close()
    {
        if (opened && (socket != null))
        {
            try {
                inputStream.close();
                socket.close();
            } catch (IOException ex) {
                Logger.getLogger(TcpClientReceiver.class.getName()).log(Level.SEVERE, null, ex);
            }
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

    public boolean receive(byte[] headerBytes, byte[] bytes, int offset)
    {
        boolean failureDetected = false;
        try
        {
            if(socket == null)
            {
                setupSocket(serverIp, serverPort, receiveBufferSize);
                opened = true;
            }

            //First read the size of the incomming tcp packet
            int accumulatedSize = 0;
            while(!failureDetected && (accumulatedSize < sizePackSize))
            {
                int numRead = inputStream.read(sizePackBytes, accumulatedSize, sizePackSize - accumulatedSize);
                if (numRead <= 0)
                {
                    failureDetected = true; // Other side disconnected
                }
                accumulatedSize += numRead;
            }

            if (!failureDetected)
            {
                //max_length = *((int*)(data + 18));

                //TODO: header and error check.
                int dataSize =  sizePackBuffer.getInt(18) - headerBytes.length;

                accumulatedSize = 0;
                while(!failureDetected && (accumulatedSize < headerBytes.length))
                {
                    int numRead = inputStream.read(headerBytes, accumulatedSize, headerBytes.length - accumulatedSize);
                    if (numRead <= 0)
                    {
                        failureDetected = true; // Other side disconnected
                    }
                    accumulatedSize += numRead;
                }

                accumulatedSize = 0;
                while(!failureDetected && (accumulatedSize < dataSize))
                {
                    int numRead = inputStream.read(bytes, accumulatedSize + offset, dataSize - accumulatedSize);
                    if (numRead <= 0)
                    {
                        failureDetected = true; // Other side disconnected
                    }
                    accumulatedSize += numRead;
                }

                if (!failureDetected)
                {
                    newBytesEvent.fireEvent(Integer.valueOf(dataSize + headerBytes.length));
                    return true;
                }
            }

        }
        catch (IOException ex)
        {
            failureDetected = true;
        }

        if (failureDetected)
        {
            try
            {
                if(socket != null)
                    socket.close();
            }
            catch (IOException ex1)
            {
                //Already closed?
                System.out.println("IOException ex1");
            }
            finally
            {
                socket = null;
            }
        }
        return false;
    }

    public Event getNewBytesEvent()
    {
        return newBytesEvent;
    }

    private void setupSocket(String serverIP, int serverPort, int receiveBufferSize) throws SocketException, IOException
    {
        socket = new Socket(serverIP, serverPort);
        if(receiveBufferSize > 0)
        {
            socket.setReceiveBufferSize(receiveBufferSize);
        }
        InetAddress ia = socket.getInetAddress();
        if (ia != null) {
            remoteSenderIp = ia.getHostAddress();
            remoteSenderPort = socket.getPort();
        }

        //socket.setSoTimeout(100);
        inputStream = socket.getInputStream();
    }
}
