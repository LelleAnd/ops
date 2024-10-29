///////////////////////////////////////////////////////////
//  TcpServerSender.cs
//  Implementation of the Class TcpServerSender
//  Created on:      12-nov-2011 09:25:37
//  Author:
///////////////////////////////////////////////////////////

using System;
using System.Net;
using System.Net.Sockets;

namespace Ops 
{
	public class TcpServerSender : EasyThread, ISender
    {
        private TcpListener tcpListener;
		private readonly string serverIp;
		private readonly int serverPort;
        private int actualPort = 0;
        private readonly int sendBufferSize;
		private TcpSenderList tcpSenderList;
        private bool listening = false;
        private System.Threading.ManualResetEvent signal = new System.Threading.ManualResetEvent(false);

        public TcpServerSender(string serverIp, int serverPort, int sendBufferSize) 
        {
            this.serverIp = serverIp;
            this.serverPort = serverPort;
            this.sendBufferSize = sendBufferSize;

            if (!Ops.InetAddress.IsValidNodeAddress(this.serverIp))
            {
                this.serverIp = "0.0.0.0";
            }
            IPAddress serverAddr = IPAddress.Parse(this.serverIp);
            tcpListener = new TcpListener(serverAddr, serverPort);
            tcpSenderList = new TcpSenderList();
            Start();
            Open();
        }

        public void Open()
        {
            if (!listening)
            {
                tcpListener.Start();

                // Get actual local port used in case we let OS decide
                IPEndPoint ep = (IPEndPoint)tcpListener.LocalEndpoint;
                actualPort = ep.Port;

                listening = true;       // Set flag before we signal the thread
                signal.Set();           // Set event so the thread is released
            }
        }

        public void Close()
        {
            if (listening)
            {
                signal.Reset();         // Clear event so thread will hang on it
                listening = false;      // Clear flag before we call stop()

                // Stop listening. The AcceptTcpClient() call in Run() will exit
                // and the thread will wait on the event again
                tcpListener.Stop();

                // Clear all connections (sockets) to subscribers
                tcpSenderList.EmptyList();
            }
        }

        /// <summary>
        /// Get IP and port used as endpoint when sending a message
        /// </summary>
        /// <param name="IP"></param>
        /// <param name="port"></param>
        public void GetLocalEndpoint(ref string IP, ref int port)
        {
            IP = this.serverIp;
            port = this.actualPort;
        }

        public bool SendTo(byte[] bytes, string ip, int port)
        {
            return SendTo(bytes, 0, bytes.Length, ip, port);
        }

        public bool SendTo(byte[] bytes, int offset, int size, string ip, int port)
        {
            try
            {
                return SendTo(bytes, offset, size, new InetAddress(ip), port);
            } catch (Exception ex)
            {
                Logger.ExceptionLogger.LogException(ex);
                return false;
            }
        }

        public bool SendTo(byte[] bytes, int offset, int size, InetAddress ipAddress, int port)
        {
            return tcpSenderList.SendTo(bytes, offset, size, ipAddress, port);
        }

        protected override void Run()
        {
            while (true)
            {
                try
                {
                    /// Wait for event to be set
                    this.signal.WaitOne();

                    while (listening)
                    {
                        try
                        {
                            // Blocks until a client has connected to the server
                            System.Net.Sockets.TcpClient client = this.tcpListener.AcceptTcpClient();
                            if (this.sendBufferSize > 0) client.SendBufferSize = this.sendBufferSize;
                            tcpSenderList.Add(client);
                        }
                        catch (Exception ex)
                        {
                            if (listening) Logger.ExceptionLogger.LogException(ex);
                        }
                    }
                }
                catch (SocketException ex)
                {
                    signal.Reset();         // Clear event so thread will hang on it
                    if (ex.SocketErrorCode == System.Net.Sockets.SocketError.AddressAlreadyInUse)
                    {
                        Logger.ExceptionLogger.LogMessage("TcpServer [" + this.serverIp + "::" + this.serverPort + "] Error: " + ex.SocketErrorCode.ToString());
                    }
                    else
                    {
                        Logger.ExceptionLogger.LogException(ex);
                    }
                }
            }
        }

	}

}