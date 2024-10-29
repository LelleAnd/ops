/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
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

import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;
import ops.protocol.OPSMessage;

/**
 *
 * @author angr
 */
public class ReceiveDataHandler
{

    private Vector<Subscriber> subscribers = new Vector<Subscriber>();
    protected Participant participant;
    private final Topic topic;
    protected Vector<ReceiveDataChannel> channels = new Vector<ReceiveDataChannel>();

    public ReceiveDataHandler(Topic t, Participant part)
    {
        topic = t;
        participant = part;
    }

    public void cleanup()
    {

    }

    public int getSampleMaxSize()
    {
        return topic.getSampleMaxSize();
    }

    public String getTransport()
    {
        return topic.getTransport();
    }

    // Tell derived classes which topics that are active
    protected void topicUsage(Topic top, boolean used)
    {
    }

    public int getNrOfSubscribers()
    {
        return subscribers.size();
    }

    public void addSubscriber(Subscriber sub, Topic top)
    {
        synchronized (subscribers)
        {
            subscribers.add(sub);
        }

        if (subscribers.size() == 1)
        {
            synchronized (channels)
            {
                for (ReceiveDataChannel channel : channels) {
                    channel.start();
                }
            }
        }

        topicUsage(top, true);
    }

    public boolean removeSubscriber(Subscriber sub, Topic top)
    {
        topicUsage(top, false);

        boolean result = false;
        synchronized (subscribers)
        {
            result = subscribers.remove(sub);
        }

        if (subscribers.size() == 0)
        {
            synchronized (channels)
            {
                for (ReceiveDataChannel channel : channels) {
                    channel.stop();
                }
            }
        }

        return result;
    }

    public void onNewMessage(OPSMessage message)
    {
        synchronized (subscribers)
        {
            //TODO: error checking
            for (Subscriber subscriber : subscribers)
            {
                try
                {
                    subscriber.notifyNewOPSMessage(message);
                } catch (Throwable t)
                {
                    Logger.getLogger(ReceiveDataHandler.class.getName()).log(Level.WARNING, "Exception thrown in event notification thread" + t.getMessage());
                }
            }
        }
    }

}
