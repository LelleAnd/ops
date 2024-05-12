///////////////////////////////////////////////////////////
//  UdpSender.cs
//  Implementation of the Class UdpSender
//  Created on:      06-jan-2013
//  Author:
///////////////////////////////////////////////////////////

using System;
using System.Net;
using System.Net.Sockets;

namespace Ops 
{
	internal class UdpSender : ISender 
    {
        private UdpClient udpSocket;
        private bool opened = false;
        private int outSocketBufferSize;
        private IPEndPoint ipEndPoint;

        public UdpSender(int port, string localInterface, int outSocketBufferSize) 
        {
            this.ipEndPoint = new IPEndPoint(IPAddress.Parse(localInterface), port);
            this.outSocketBufferSize = outSocketBufferSize;
            Open();
        }

        public void Open()
        {
            if (!this.opened)
            {
                this.udpSocket = new UdpClient(this.ipEndPoint);

                if (this.outSocketBufferSize > 0)
                {
                    this.udpSocket.Client.SendBufferSize = this.outSocketBufferSize;
                }
                this.opened = true;
            }
        }

        public void Close()
        {
            if (this.opened)
            {
                this.udpSocket.Close();
                this.opened = false;
            }
        }

        /// <summary>
        /// Get IP and port used as endpoint when sending a message
        /// </summary>
        /// <param name="IP"></param>
        /// <param name="port"></param>
        public void GetLocalEndpoint(ref string IP, ref int port)
        {
            IP = ((IPEndPoint)this.udpSocket.Client.LocalEndPoint).Address.ToString();
            port = ((IPEndPoint)this.udpSocket.Client.LocalEndPoint).Port;
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
                udpSocket.Client.SendTo(bytes, offset, size, SocketFlags.None, (EndPoint)ipep);

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