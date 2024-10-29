///////////////////////////////////////////////////////////
//  ReceiveDataHandlerFactory.cs
//  Implementation of the Class ReceiveDataHandlerFactory
//  Created on:      12-nov-2011 09:25:34
//  Author:
///////////////////////////////////////////////////////////

using System;
using System.Collections.Generic;

namespace Ops 
{
	public class ReceiveDataHandlerFactory 
    {
        private Dictionary<string, ReceiveDataHandler> ReceiveDataHandlers = new Dictionary<string, ReceiveDataHandler>();

        // Since topics can use the same port for transports multicast & tcp, or 
        // use transport udp which in most cases use a single ReceiveDataHandler, 
        // we need to return the same ReceiveDataHandler in these cases.
        // Make a key with the transport info that uniquely defines the receiver.
        private string MakeKey(Topic top)
        {
            if (top.GetTransport().Equals(Topic.TRANSPORT_UDP) && (!Ops.InetAddress.IsMyNodeAddress(top.GetDomainAddress())))
            {
                return top.GetTransport();
            }
            else if ((top.GetTransport() == Topic.TRANSPORT_TCP) && (top.GetPort() == 0))
            {
                return top.GetTransport() + "::" + top.ChannelID + "::" + top.GetDomainAddress() + "::" + top.GetPort();
            }
            else
            {
                return top.GetTransport() + "::" + top.GetDomainAddress() + "::" + top.GetPort();
            }
        }

        /// Protection is not needed since all calls go through the participant which is synched
        public ReceiveDataHandler GetReceiveDataHandler(Topic top, Participant participant)
        {
            // In the case that we use the same port for several topics, we need to find the receiver for the transport::address::port used
            string key = MakeKey(top);

            if (ReceiveDataHandlers.ContainsKey(key))
            {
                ReceiveDataHandler rdh = ReceiveDataHandlers[key];

                // Check if any of the topics have a sample size larger than MAX_SEGMENT_SIZE
                // This will lead to a problem when using the same port, if samples becomes > MAX_SEGMENT_SIZE
                if ((rdh.GetSampleMaxSize() > Globals.MAX_SEGMENT_SIZE) || (top.GetSampleMaxSize() > Globals.MAX_SEGMENT_SIZE))
                {
                    if (top.GetTransport().Equals(Topic.TRANSPORT_UDP))
                    {
                        Logger.ExceptionLogger.LogMessage("Warning: UDP transport is used with Topics with 'sampleMaxSize' > " + Globals.MAX_SEGMENT_SIZE);
                    }
                    else
                    {
                        Logger.ExceptionLogger.LogMessage("Warning: Same port (" + top.GetPort() + ") is used with Topics with 'sampleMaxSize' > " + Globals.MAX_SEGMENT_SIZE);
                    }
                }
                return rdh;
            }

            // Didn't exist, create one if transport is known
            if ( top.GetTransport().Equals(Topic.TRANSPORT_MC) )
            {
                ReceiveDataHandlers.Add(key, new McReceiveDataHandler(top, participant));
                return ReceiveDataHandlers[key];
            }
            else if ( top.GetTransport().Equals(Topic.TRANSPORT_TCP) )
            {
                ReceiveDataHandlers.Add(key, new TcpReceiveDataHandler(top, participant));
                return ReceiveDataHandlers[key];
            }
            else if (top.GetTransport().Equals(Topic.TRANSPORT_UDP))
            {
                bool commonReceiver = (key == Topic.TRANSPORT_UDP);
                ReceiveDataHandlers.Add(key, new UdpReceiveDataHandler(top, participant, commonReceiver));
                return ReceiveDataHandlers[key];
            }
            return null;
        }

        /// Protection is not needed since all calls go through the participant which is synched
        public void ReleaseReceiveDataHandler(Topic top, Participant participant)
        {
            // In the case that we use the same port for several topics, we need to find the receiver for the transport::address::port used
            string key = MakeKey(top);

            if (ReceiveDataHandlers.ContainsKey(key))
            {
                ReceiveDataHandler rdh = ReceiveDataHandlers[key];

                if (rdh.GetNrOfSubscribers() == 0)
                {
                    ReceiveDataHandlers.Remove(key);
                    rdh.Dispose();
                }

            }        
        }

    }

}