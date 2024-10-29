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
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.concurrent.Semaphore;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author Anton Gravestam
 */
public class TcpServerSender implements Sender, Runnable
{
    private final String serverIp;
    private final int serverPort;
    private int actualPort = 0;
    private final int sendBufferSize;
    private ServerSocket server = null;
    private TcpSenderList tcpSenderList;
    private volatile boolean listening = false;
    private Semaphore sem = new Semaphore(0);
    private boolean keepRunning = false;

    public TcpServerSender(String serverIp, int serverPort, int sendBufferSize) throws IOException
    {
        if (NetworkSupport.IsValidNodeAddress(serverIp)) {
            this.serverIp = serverIp;
        } else {
            this.serverIp = "0.0.0.0";
        }
        this.serverPort = serverPort;
        this.sendBufferSize = sendBufferSize;

        tcpSenderList = new TcpSenderList();
        keepRunning = true;
        new Thread(this).start();
        open();
    }

    public final void open()
    {
        if (!listening)
        {
          try {
            server = new ServerSocket(this.serverPort, 0, InetAddress.getByName(this.serverIp));
            actualPort = server.getLocalPort();

            listening = true;       // Set flag before we signal the thread
            sem.release();          // Signal semaphore so the thread is released
          } catch (IOException ex) {
              Logger.getLogger(TcpServerSender.class.getName()).log(Level.SEVERE, null, ex);
          }
        }
    }

    public void close()
    {
        if (listening)
        {
            sem.drainPermits();     // Clear semaphore so thread will hang on it
            listening = false;      // clear flag before we call stop()

            try {
                // Stop listening. The accept() call in Run() will exit
                // and the thread will wait on the event again
                if (server != null) server.close();
            } catch (IOException ex) {
                Logger.getLogger(TcpServerSender.class.getName()).log(Level.SEVERE, null, ex);
            }

            // Clear all connections (sockets) to subscribers
            tcpSenderList.emptyList();
        }
    }

    public int getLocalPort()
    {
        return this.actualPort;
    }

    public boolean sendTo(byte[] bytes, String ip, int port)
    {
        return sendTo(bytes, 0, bytes.length, ip, port);
    }

    public boolean sendTo(byte[] bytes, int offset, int size, String ip, int port)
    {
        try
        {
            return sendTo(bytes, offset, size, InetAddress.getByName(ip), port);
        } catch (UnknownHostException ex)
        {
            Logger.getLogger(TcpServerSender.class.getName()).log(Level.SEVERE, null, ex);
            return false;
        }
    }

    public boolean sendTo(byte[] bytes, int offset, int size, InetAddress ipAddress, int port)
    {
        return tcpSenderList.sendTo(bytes, offset, size, ipAddress, port);
    }

    public void run()
    {
        while(keepRunning)
        {
            try {
                /// Wait for semaphore to be set
                sem.acquire();

                while (keepRunning && listening) {
                    try {
                        Socket socket = server.accept();
                        if (sendBufferSize > 0) socket.setSendBufferSize(sendBufferSize);
                        tcpSenderList.add(socket);
                    } catch (IOException ex) {
                        if (listening) Logger.getLogger(TcpServerSender.class.getName()).log(Level.SEVERE, null, ex);
                    }
                }
            } catch (InterruptedException ex) {
                Logger.getLogger(TcpServerSender.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }

    public void stopThread()
    {
        keepRunning = false;
        sem.release();          // Signal semaphore so the thread is released
    }

}
