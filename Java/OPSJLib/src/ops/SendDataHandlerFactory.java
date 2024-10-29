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
import java.util.HashMap;

class SendDataHandlerFactory
{
    private HashMap<String, SendDataHandler> sendDataHandlers = new HashMap<String, SendDataHandler>();

    private String makeKey(Topic top, String localIf)
    {
        // We need to store SendDataHandlers with more than just the name as key.
        // Since topics can use the same port, we need to return the same SendDataHandler.
        // Make a key with the transport info that uniquely defines the sender.
        String key = top.getTransport() + "::";
        if (top.getTransport().equals(Topic.TRANSPORT_UDP)) {
            key += localIf + "::";
        }
        if ((top.getTransport().equals(Topic.TRANSPORT_TCP)) && (top.getPort() == 0))
        {
            // We add the channel name so different channels get different TCP Servers
            key += top.getChannelID();
            key += "::";
        }
        key += top.getDomainAddress();
        if (!top.getTransport().equals(Topic.TRANSPORT_UDP)) {
            key += "::" + top.getPort();
        }
        return key;   ///t.getTransport() + "::" + t.getDomainAddress() + "::" + t.getPort();
    }

    private void postSetup(Topic t, Participant participant, McUdpSendDataHandler sdh) throws IOException
    {
        if (NetworkSupport.IsValidNodeAddress(t.getDomainAddress())) {
            sdh.addSink(t.getName(), t.getDomainAddress(), t.getPort(), true);
        } else {
            // Setup a listener on the participant info data published by participants on our domain.
            // We use the information for topics with UDP as transport, to know the destination for UDP sends
            // ie. we extract ip and port from the information and add it to our McUdpSendDataHandler
            participant.connectUdp(t, sdh);
        }
    }

    SendDataHandler getSendDataHandler(Topic t, Participant participant) throws CommException
    {
        String localIf = NetworkSupport.DoSubnetTranslation(t.getLocalInterface());
        String key = makeKey(t, localIf);

        try
        {
            if (sendDataHandlers.containsKey(key))
            {
                SendDataHandler sdh = sendDataHandlers.get(key);
                if (t.getTransport().equals(Topic.TRANSPORT_UDP)) {
                    postSetup(t, participant, (McUdpSendDataHandler)sdh);
                }
                return sdh;
            }

            SendDataHandler sender = null;
            if (t.getTransport().equals(Topic.TRANSPORT_MC))
            {
                sender = new McSendDataHandler(t, localIf, t.getTimeToLive());
            }
            if (t.getTransport().equals(Topic.TRANSPORT_TCP))
            {
                sender = new TcpSendDataHandler(t, localIf);
            }
            if (t.getTransport().equals(Topic.TRANSPORT_UDP))
            {
                // We have only one sender for all topics, so use the domain value for buffer size
                sender = new McUdpSendDataHandler(
                        participant.getDomain().GetOutSocketBufferSize(),
                        localIf);
                postSetup(t, participant, (McUdpSendDataHandler)sender);
            }
            if (sender != null)
            {
                sendDataHandlers.put(key, sender);
                return sender;
            }

            throw new CommException("No such Transport " + t.getTransport());
        }
        catch (IOException ex)
        {
            throw new CommException("Error creating SendDataHandler. IOException --> " + ex.getMessage());
        }
    }

    void releaseSendDataHandler(Topic t, Participant participant)
    {
        String localIf = NetworkSupport.DoSubnetTranslation(t.getLocalInterface());
        String key = makeKey(t, localIf);

        if (sendDataHandlers.containsKey(key))
        {
            SendDataHandler sdh = sendDataHandlers.get(key);
            if (t.getTransport().equals(Topic.TRANSPORT_UDP))
            {
                if (!NetworkSupport.IsValidNodeAddress(t.getDomainAddress()))
                {
                    participant.disconnectUdp(t, (McUdpSendDataHandler)sdh);
                }
            }
            sdh.cleanUp();
            sendDataHandlers.remove(key);
        }
    }
}
