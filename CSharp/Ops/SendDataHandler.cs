///////////////////////////////////////////////////////////
//  SendDataHandler.cs
//  Implementation of the Interface SendDataHandler
//  Created on:      12-nov-2011 09:25:35
//  Author:
///////////////////////////////////////////////////////////

using System.Collections.Generic;
using System.Runtime.CompilerServices;  // Needed for the "MethodImpl" synchronization attribute

namespace Ops 
{
	public interface ISendDataHandler 
    {
        void AddPublisher(Publisher pub);
        bool RemovePublisher(Publisher pub);

        void UpdateTransportInfo(Topic top);

        /// <summary>
        /// Get IP and port used as endpoint when sending a message
        /// </summary>
        /// <param name="IP"></param>
        /// <param name="port"></param>
        void GetLocalEndpoint(ref string IP, ref int port);

		/// 
		/// <param name="bytes"></param>
		/// <param name="size"></param>
		/// <param name="t"></param>
		bool SendData(byte[] bytes, int size, Topic t);
	}


    public abstract class SendDataHandler : ISendDataHandler
    {
        protected ISender sender;
        private List<Publisher> publishers = new List<Publisher>();

        [MethodImpl(MethodImplOptions.Synchronized)]
        public void AddPublisher(Publisher pub)
        {
            publishers.Add(pub);

            if (publishers.Count == 1)
            {
                // The first publisher
                sender.Open();
            }
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public bool RemovePublisher(Publisher pub)
        {
            bool result = publishers.Remove(pub);

            if (publishers.Count == 0)
            {
                // No more publishers
                sender.Close();
            }

            return result;
        }

        /// <summary>
        /// Get IP and port used as endpoint when sending a message
        /// </summary>
        /// <param name="IP"></param>
        /// <param name="port"></param>
        public void GetLocalEndpoint(ref string IP, ref int port)
        {
            sender.GetLocalEndpoint(ref IP, ref port);
        }

        public virtual void UpdateTransportInfo(Topic top)
        {
        }

        public abstract bool SendData(byte[] bytes, int size, Topic t);

        protected bool SendData(byte[] bytes, int size, string ip, int port)
        {
            int nrSegmentsNeeded = (int)(size / Globals.MAX_SEGMENT_SIZE);
            if (size % Globals.MAX_SEGMENT_SIZE != 0)
            {
                nrSegmentsNeeded++;
            }
            for (int i = 0; i < nrSegmentsNeeded; i++)
            {
                //If this is the last element, only send the bytes that remains, otherwise send a full package.
                int sizeToSend = (i == nrSegmentsNeeded - 1) ? size - (nrSegmentsNeeded - 1) * Globals.MAX_SEGMENT_SIZE : Globals.MAX_SEGMENT_SIZE;
                if (!sender.SendTo(bytes, i * Globals.MAX_SEGMENT_SIZE, sizeToSend, ip, port))
                {
                    return false;
                }
            }
            return true;
        }

    }
}