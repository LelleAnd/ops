///////////////////////////////////////////////////////////
//  MulticastSender.cs
//  Implementation of the Class MulticastSender
//  Created on:      12-nov-2011 09:25:32
//  Author:
///////////////////////////////////////////////////////////

using System;
using System.Net;
using System.Net.Sockets;

namespace Ops 
{
	internal class MulticastSender : ISender 
    {
        private UdpClient multicastSocket;
        private bool opened = false;
        private int outSocketBufferSize;
        private int ttl;
        private IPEndPoint ipEndPoint;

        public MulticastSender(int port, string localInterface, int ttl, int outSocketBufferSize) 
        {
            this.ipEndPoint = new IPEndPoint(IPAddress.Parse(localInterface), port);
            this.ttl = ttl;
            this.outSocketBufferSize = outSocketBufferSize;
            Open();
        }

        public void Open()
        {
            if (!opened)
            {
                multicastSocket = new UdpClient(ipEndPoint);

                if (outSocketBufferSize > 0)
                {
                    multicastSocket.Client.SendBufferSize = outSocketBufferSize;
                }

                multicastSocket.Client.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.MulticastTimeToLive, ttl);

                opened = true;
            }
        }

        public void Close()
        {
            if (opened)
            {
                multicastSocket.Close();
                opened = false;
            }
        }

        /// <summary>
        /// Get IP and port used as endpoint when sending a message
        /// </summary>
        /// <param name="IP"></param>
        /// <param name="port"></param>
        public void GetLocalEndpoint(ref string IP, ref int port)
        {
            IP = ((IPEndPoint)this.multicastSocket.Client.LocalEndPoint).Address.ToString();
            port = ((IPEndPoint)this.multicastSocket.Client.LocalEndPoint).Port;
        }

		public bool SendTo(byte[] bytes, int offset, int size, string ip, int port)
        {
            try
            {
                return this.SendTo(bytes, offset, size, new InetAddress(ip), port);
            }
            catch (Exception)
            {
                return false;
            }
        }

		public bool SendTo(byte[] bytes, int offset, int size, InetAddress ipAddress, int port)
        {
            try
            {
                IPEndPoint ipep = new IPEndPoint(ipAddress.GetIPAddress(), port);

                // "UdpClient" do not include a send method with an offset.
                // Hence we must use the underlaying socket "Client" to be able to
                // send a byte buffer starting with an offset 
                multicastSocket.Client.SendTo(bytes, offset, size, SocketFlags.None, (EndPoint)ipep);

                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }

		public bool SendTo(byte[] bytes, string ip, int port)
        {
            return SendTo(bytes, 0, bytes.Length, ip, port);
        }

	}

}