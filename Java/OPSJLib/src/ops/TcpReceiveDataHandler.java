/**
 * Copyright (C) 2024 Lennart Andersson.
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

/**
 *
 */
public class TcpReceiveDataHandler extends ReceiveDataHandler
{
    private Topic top;
    private boolean usingPartInfo = true;
    private HashMap<String, Integer> topicCounts = new HashMap<String, Integer>();

    public TcpReceiveDataHandler(Topic t, Participant part)
    {
        super(t, part);
        // Make a local copy to be used when adding channels dynamically
        top = (Topic)t.clone();

        if (t.getPort() != 0)
        {
            channels.add(new ReceiveDataChannel(t, part, ReceiverFactory.createReceiver(t, ""), this));
            usingPartInfo = false;
        }
    }

    public void addReceiveChannel(String topicName, String ip, int port)
    {
        if (port == 0) { return; }

        String key = ip + "::" + port;

        // Look for it in channels, if not there, create one
        synchronized (channels)
        {
            for (ReceiveDataChannel chnl : channels)
            {
                if (chnl.Key.equals(key))
                {
                    return;
                }
            }

            top.setDomainAddress(ip);
            top.setPort(port);

            ReceiveDataChannel channel = new ReceiveDataChannel(top, participant, ReceiverFactory.createReceiver(top, ""), this);
            channel.Key = key;
            channels.add(channel);

            if (getNrOfSubscribers() > 0)
            {
                channel.start();
            }
        }
    }

    // Tell derived classes which topics that are active
    @Override
    protected void topicUsage(Topic top, boolean used)
    {
        if (usingPartInfo) {
            // We should only register unique topics
            int count = topicCounts.getOrDefault(top.getName(), 0);

            // Register topic with participant info data handler/listener to get callbacks to handler above
            if (used)
            {
                ++count;
                if (count == 1)
                {
                    participant.connectTcp(top.getName(), this);
                }
            }
            else
            {
                --count;
                if (count == 0)
                {
                    participant.disconnectTcp(top.getName(), this);
                }
            }
            topicCounts.put(top.getName(), count);
        }
    }

}
