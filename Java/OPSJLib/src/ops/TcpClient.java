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
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 *
 * @author Anton
 */
public class TcpClient implements Receiver
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


    public TcpClient(String serverIP, int serverPort, int receiveBufferSize) throws IOException
    {
        this.serverPort = serverPort;
        this.serverIp = serverIP;
        this.receiveBufferSize = receiveBufferSize;
        setupSocket(serverIP, serverPort, receiveBufferSize);

        sizePackBuffer = ByteBuffer.wrap(sizePackBytes).order(ByteOrder.LITTLE_ENDIAN);
    }

    public void Open() throws IOException
    {
        throw new IOException("Not Implemented Yet");
    }

    public void Close()
    {
    }

    public String getRemoteIp()
    {
        return "";
    }

    public int getRemotePort()
    {
        return 0;
    }

    public boolean receive(byte[] headerBytes, byte[] bytes, int offset)
    {
        try
        {
            if(socket == null)
            {
                setupSocket(serverIp, serverPort, receiveBufferSize);
            }

            //First read the size of the incomming tcp packet
            int accumulatedSize = 0;
            while(accumulatedSize < sizePackSize)
            {
                accumulatedSize += inputStream.read(sizePackBytes, accumulatedSize, sizePackSize - accumulatedSize);
            }

            //max_length = *((int*)(data + 18));

            //TODO: header and error check.
            int dataSize =  sizePackBuffer.getInt(18) - headerBytes.length;

            accumulatedSize = 0;
            while(accumulatedSize < headerBytes.length)
            {
                accumulatedSize += inputStream.read(headerBytes, accumulatedSize, headerBytes.length - accumulatedSize);
            }

            accumulatedSize = 0;
            while(accumulatedSize < dataSize)
            {
                accumulatedSize += inputStream.read(bytes, accumulatedSize + offset, dataSize - accumulatedSize);
            }

            newBytesEvent.fireEvent(Integer.valueOf(dataSize + headerBytes.length));
            return true;

        } catch (IOException ex)
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
           return false;
        }


    }

    public Event getNewBytesEvent()
    {
        return newBytesEvent;
    }

    private void setupSocket(String serverIP, int serverPort, int receiveBufferSize) throws SocketException, IOException
    {
        socket = new Socket(serverIP, serverPort);
        socket.setReceiveBufferSize(receiveBufferSize);
        //socket.setSoTimeout(100);
        inputStream = socket.getInputStream();
    }
}
