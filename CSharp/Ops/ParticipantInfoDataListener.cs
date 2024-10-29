///////////////////////////////////////////////////////////
//  ParticipantInfoDataListener.cs
//  Implementation of the Class ParticipantInfoDataListener
//  Created on:      07-jan-2013 
//  Author:
///////////////////////////////////////////////////////////

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace Ops
{
	public class ParticipantInfoDataListener
	{
        private Participant participant;
        private Subscriber partInfoSub = null;

        private Dictionary<string, McUdpSendDataHandler> sdhs = new Dictionary<string, McUdpSendDataHandler>();
        private Dictionary<string, TcpReceiveDataHandler> rdhs = new Dictionary<string, TcpReceiveDataHandler>();

        public ParticipantInfoDataListener(Participant part)
		{
            participant = part;
        }

        ~ParticipantInfoDataListener()
        {
            participant = null;
        }

        private bool SetupSubscriber()
        {
            if (participant.getDomain().GetMetaDataMcPort() == 0)
            {
                return false;
            }
            if (partInfoSub == null)
            {
                partInfoSub = new Subscriber(participant.CreateParticipantInfoTopic());
                partInfoSub.newDataDefault += new NewDataDefaultEventHandler(SubscriberNewData);
                partInfoSub.Start();
            }
            return true;
        }

        private void RemoveSubscriber()
        {
            if ((rdhs.Count == 0) && (sdhs.Count == 0))
            {
                if (partInfoSub != null)
                {
                    partInfoSub.Stop();
                }
                partInfoSub = null;
            }
        }

        public void ConnectUdp(Topic top, McUdpSendDataHandler sdh)
        {
            string key = top.GetName();

            lock (sdhs)
            {
                ///TODO check if already there with another sdh
                sdhs[key] = sdh;
            }

            SetupSubscriber();
        }

        public void DisconnectUdp(Topic top, McUdpSendDataHandler sdh)
        {
            lock (sdhs)
            {
                ///TODO check that stored sdh is correct
                sdhs.Remove(top.GetName());
            }

            RemoveSubscriber();
        }

        // Register topic with participant info data handler/listener to get callbacks to rdh
        public void ConnectTcp(string topicName, TcpReceiveDataHandler rdh)
        {
            lock (rdhs)
            {
                ///TODO check if already there with another rdh
                rdhs[topicName] = rdh;
            }

            SetupSubscriber();
        }

        public void DisconnectTcp(string topicName, TcpReceiveDataHandler rdh)
        {
            lock (rdhs)
            {
                ///TODO check that stored rdh is correct
                rdhs.Remove(topicName);
            }

            RemoveSubscriber();
        }

        public void SubscriberNewData(Subscriber sender, OPSObject data)
        {
            if (!(data is ParticipantInfoData)) return;

            ParticipantInfoData partInfo = (ParticipantInfoData)data;

            // Is it on our domain?
			if (partInfo.domain.Equals(participant.domainID)) {

                lock (sdhs)
                {
                    foreach (TopicInfoData tid in partInfo.subscribeTopics)
                    {
                        // We are only interrested in topics with UDP as transport
                        if ((tid.transport.Equals(Topic.TRANSPORT_UDP)) && (participant.HasPublisherOn(tid.name)))
                        {
                            if (sdhs.ContainsKey(tid.name))
                            {
                                sdhs[tid.name].AddSink(tid.name, partInfo.ip, partInfo.mc_udp_port, false);
                            }
                        }
                    }
                }

                lock (rdhs)
                {
                    foreach (TopicInfoData tid in partInfo.publishTopics)
                    {
                        // We are only interrested in topics with TCP as transport
                        if ((tid.transport.Equals(Topic.TRANSPORT_TCP)) && (participant.HasSubscriberOn(tid.name)))
                        {
                            if (rdhs.ContainsKey(tid.name))
                            {
                                rdhs[tid.name].AddReceiveChannel(tid.name, tid.address, tid.port);
                            }
                        }
                    }
                }
            }
        }

    };

}

