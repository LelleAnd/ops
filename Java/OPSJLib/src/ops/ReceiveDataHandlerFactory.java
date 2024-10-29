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

import java.util.HashMap;
import java.util.logging.Level;
import java.util.logging.Logger;

class ReceiveDataHandlerFactory
{
    private HashMap<String, ReceiveDataHandler> receiveDataHandlers = new HashMap<String, ReceiveDataHandler>();

    // Since topics can use the same port for transports multicast & tcp, or
    // use transport udp which in most cases use a single ReceiveDataHandler,
    // we need to return the same ReceiveDataHandler in these cases.
    // Make a key with the transport info that uniquely defines the receiver.
    private String makeKey(Topic top)
    {
        if (top.getTransport().equals(Topic.TRANSPORT_UDP) && (!NetworkSupport.IsMyNodeAddress(top.getDomainAddress())))
        {
            return top.getTransport();
        }
        else if (top.getTransport().equals(Topic.TRANSPORT_TCP) && (top.getPort() == 0))
        {
            return top.getTransport() + "::" + top.getChannelID() + "::" + top.getDomainAddress() + "::" + top.getPort();
        }
        else
        {
            return top.getTransport() + "::" + top.getDomainAddress() + "::" + top.getPort();
        }
    }

    /// Protection is not needed since all calls go through the participant which is synched
    ReceiveDataHandler getReceiveDataHandler(Topic top, Participant participant)
    {
        // In the case that we use the same port for several topics, we need to find the receiver for the transport::address::port used
        String key = makeKey(top);

        if (receiveDataHandlers.containsKey(key))
        {
            ReceiveDataHandler rdh = receiveDataHandlers.get(key);

            // Check if any of the topics have a sample size larger than MAX_SIZE
            // This will lead to a problem when using the same port, if samples becomes > MAX_SIZE
            if ((rdh.getSampleMaxSize() > StaticManager.MAX_SIZE) || (top.getSampleMaxSize() > StaticManager.MAX_SIZE))
            {
                if (top.getTransport().equals(Topic.TRANSPORT_UDP))
                {
                    Logger.getLogger(ReceiveDataHandlerFactory.class.getName()).log(
                        Level.WARNING,
                        "Warning: UDP transport is used with Topics with ''sampleMaxSize'' > {0}",
                        StaticManager.MAX_SIZE);
                }
                else
                {
                    Logger.getLogger(ReceiveDataHandlerFactory.class.getName()).log(
                        Level.WARNING,
                        "Same port ({0}) is used with Topics with ''sampleMaxSize'' > {1}",
                        new Object[]{top.getPort(), StaticManager.MAX_SIZE});
                }
            }
            return rdh;
        }

        if (top.getTransport().equals(Topic.TRANSPORT_MC))
        {
            receiveDataHandlers.put(key, new McReceiveDataHandler(top, participant));
            return receiveDataHandlers.get(key);
        }
        else if (top.getTransport().equals(Topic.TRANSPORT_TCP))
        {
            receiveDataHandlers.put(key, new TcpReceiveDataHandler(top, participant));
            return receiveDataHandlers.get(key);
        }
        else if (top.getTransport().equals(Topic.TRANSPORT_UDP))
        {
            boolean commonReceiver = key.equals(Topic.TRANSPORT_UDP);
            receiveDataHandlers.put(key, new UdpReceiveDataHandler(top, participant, commonReceiver));
            return receiveDataHandlers.get(key);
        }
        return null;
    }

    /// Protection is not needed since all calls go through the participant which is synched
    public void ReleaseReceiveDataHandler(Topic top, Participant participant)
    {
        // In the case that we use the same port for several topics, we need to find the receiver for the transport::address::port used
        String key = makeKey(top);

        if (receiveDataHandlers.containsKey(key))
        {
            ReceiveDataHandler rdh = receiveDataHandlers.get(key);

            if (rdh.getNrOfSubscribers() == 0)
            {
                receiveDataHandlers.remove(key);
                rdh.cleanup();
            }
        }
    }

}
