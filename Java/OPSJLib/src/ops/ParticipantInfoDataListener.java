/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2020-2024 Lennart Andersson.
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
import java.util.Observer;
import java.util.Observable;
import java.util.HashMap;

public class ParticipantInfoDataListener
{
    private Participant participant;
    private Subscriber partInfoSub = null;

    private HashMap<String, McUdpSendDataHandler> sdhs = new HashMap<String, McUdpSendDataHandler>();
    private HashMap<String, TcpReceiveDataHandler> rdhs = new HashMap<String, TcpReceiveDataHandler>();

    public ParticipantInfoDataListener(Participant part)
    {
        this.participant = part;
    }

    private void setupSubscriber()
    {
        if (partInfoSub == null) {
            try {
                partInfoSub = new Subscriber(participant.createParticipantInfoTopic());
                //Add a listener to the subscriber
                partInfoSub.addObserver(new Observer() {
                    public void update(Observable o, Object arg)
                    {
                        subscriberNewData(partInfoSub, partInfoSub.getMessage().getData());
                    }
                });
                partInfoSub.start();
            }
            catch (ConfigurationException e)
            {
            }
        }
    }

    private void removeSubscriber()
    {
        if (sdhs.size() == 0) {
            if (partInfoSub != null) {
                partInfoSub.stop();
                partInfoSub = null;
            }
        }
    }

    public void connectUdp(Topic top, McUdpSendDataHandler udpSendDataHandler)
    {
        String key = top.getName();

        synchronized (sdhs)
        {
            ///TODO check if already there with another sdh
            sdhs.put(key, udpSendDataHandler);
        }

        setupSubscriber();
    }

    public void disconnectUdp(Topic top, McUdpSendDataHandler udpSendDataHandler)
    {
        String key = top.getName();

        synchronized (sdhs)
        {
            sdhs.remove(key, udpSendDataHandler);
        }

        removeSubscriber();
    }

    public void connectTcp(String topicName, TcpReceiveDataHandler rdh)
    {
        synchronized (rdhs)
        {
            ///TODO check if already there with another rdh
            rdhs.put(topicName, rdh);
        }

        setupSubscriber();
    }

    public void disconnectTcp(String topicName, TcpReceiveDataHandler rdh)
    {
        synchronized (rdhs)
        {
            rdhs.remove(topicName, rdh);
        }

        removeSubscriber();
    }

    private void subscriberNewData(Subscriber sender, OPSObject data)
    {
        if (!(data instanceof ParticipantInfoData)) return;

        ParticipantInfoData partInfo = (ParticipantInfoData)data;

        // Is it on our domain?
	      if (partInfo.domain.equals(participant.domainID)) {
            synchronized (sdhs)
            {
                for (TopicInfoData tid : partInfo.subscribeTopics) {
                    // We are only interrested in topics with UDP as transport
                    if ( (tid.transport.equals(Topic.TRANSPORT_UDP)) && (participant.hasPublisherOn(tid.name)) ) {
                        try {
                            McUdpSendDataHandler sdh = sdhs.get(tid.name);
                            if (sdh != null) sdh.addSink(tid.name, partInfo.ip, partInfo.mc_udp_port, false);
                        }
                        catch (IOException e)
                        {
                        }
                    }
                }
            }
            synchronized (rdhs)
            {
                for (TopicInfoData tid : partInfo.publishTopics)
                {
                    // We are only interrested in topics with TCP as transport
                    if ((tid.transport.equals(Topic.TRANSPORT_TCP)) && (participant.hasSubscriberOn(tid.name)))
                    {
                        TcpReceiveDataHandler rdh = rdhs.get(tid.name);
                        if (rdh != null) rdh.addReceiveChannel(tid.name, tid.address, tid.port);
                    }
                }
            }
        }
    }

}
