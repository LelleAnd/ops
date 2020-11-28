///////////////////////////////////////////////////////////
//  SendDataHandlerFactory.cs
//  Implementation of the Class SendDataHandlerFactory
//  Created on:      12-nov-2011 09:25:35
//  Author:
///////////////////////////////////////////////////////////

using System;
using System.IO;
using System.Collections.Generic;

namespace Ops 
{
	internal class SendDataHandlerFactory 
    {
        private Dictionary<string, ISendDataHandler> SendDataHandlers = new Dictionary<string, ISendDataHandler>();

        private ParticipantInfoDataListener partInfoListener = null;
        private Subscriber partInfoSub = null;

        ~SendDataHandlerFactory()
        {
            if (partInfoSub != null) partInfoSub.Stop();
            partInfoSub = null;
            partInfoListener = null;
        }

        private string MakeKey(Topic top, string localIf)
        {
            // In the case that we use the same port for several topics, we need to find the sender for the transport::address::port used
            string key = top.GetTransport() + "::";
            if (top.GetTransport().Equals(Topic.TRANSPORT_UDP))
            {
                key += localIf + "::";
            }
            key += top.GetDomainAddress();
            if (!top.GetTransport().Equals(Topic.TRANSPORT_UDP))
            {
                key += "::" + top.GetPort();
            }
            return key;   // t.GetTransport() + "::" + t.GetDomainAddress() + "::" + t.GetPort();
        }

        private void PostSetup(Topic t, Participant participant, McUdpSendDataHandler sdh)
        {
            // If topic specifies a valid node address, add that as a static destination address for topic
            if (Ops.InetAddress.IsValidNodeAddress(t.GetDomainAddress()))
            {
                sdh.AddSink(t.GetName(), t.GetDomainAddress(), t.GetPort(), true);
            }
            else
            {
                if (partInfoListener == null)
                {
                    // Setup a listener on the participant info data published by participants on our domain.
                    // We use the information for topics with UDP as transport, to know the destination for UDP sends
                    // ie. we extract ip and port from the information and add it to our McUdpSendDataHandler
                    partInfoListener = new ParticipantInfoDataListener(participant, sdh);

                    partInfoSub = new Subscriber(participant.CreateParticipantInfoTopic());
                    partInfoSub.newDataDefault += new NewDataDefaultEventHandler(partInfoListener.SubscriberNewData);
                    partInfoSub.Start();
                }
            }
        }

        ///LA TODO Protection ?? What if several subscribers at the same time
        /// Not needed since all calls go through the participant which is synched
        public ISendDataHandler GetSendDataHandler(Topic t, Participant participant)
        {
            // Get the local interface, doing a translation from subnet if necessary
            string localIF = InetAddress.DoSubnetTranslation(t.GetLocalInterface());
            string key = MakeKey(t, localIF);

            if (SendDataHandlers.ContainsKey(key))
            {
                ISendDataHandler sender = SendDataHandlers[key];
                if (t.GetTransport().Equals(Topic.TRANSPORT_UDP)) {
                    PostSetup(t, participant, (McUdpSendDataHandler)sender);
                }
                return sender;
            }

            try
            {
                ISendDataHandler sender = null;
                if (t.GetTransport().Equals(Topic.TRANSPORT_MC))
                {
                    sender = new McSendDataHandler(t, localIF, t.GetTimeToLive());
                }
                else if (t.GetTransport().Equals(Topic.TRANSPORT_TCP))
                {
                    sender = new TcpSendDataHandler(t, localIF);
                }
                else if (t.GetTransport().Equals(Topic.TRANSPORT_UDP))
                {
                    // We have only one sender for all topics on an interface, so use the domain value for buffer size
                    sender = new McUdpSendDataHandler(participant.getDomain().GetOutSocketBufferSize(), localIF);
                    PostSetup(t, participant, (McUdpSendDataHandler)sender);
                }
                if (sender != null)
                {
                    SendDataHandlers.Add(key, sender);
                    return sender;
                }

                throw new CommException("No such Transport " + t.GetTransport());
            }
            catch (System.IO.IOException ex)
            {
                throw new CommException("Error creating SendDataHandler. IOException -->" + ex.Message);
            }
        }

	}

}