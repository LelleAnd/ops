/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2019-2024 Lennart Andersson.
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
import java.nio.ByteBuffer;
import java.util.logging.Level;
import java.util.logging.Logger;
import ops.archiver.OPSArchiverOut;
import ops.protocol.OPSMessage;
import ops.transport.inprocess.InProcessTransport;

/**
 *
 * @author Anton Gravestam
 */
public class Publisher
{

    private Topic topic;
    private int currentPublicationID = 1;
    private String name = "";
    private String key = "";
    private int reliableWriteNrOfResends = 1;
    private int reliableWriteTimeout = 1000;
    private byte[] bytes;
    private ByteBuffer buffer;
    private Participant participant;
    private SendDataHandler sendDataHandler;
    private InProcessTransport inProcessTransport;
    private long lastPublishTime;
    private volatile long sampleTime1;
    private volatile long sampleTime2;
    private boolean started = false;

    public Publisher(Topic topic) throws ConfigurationException
    {
        this.topic = topic;
        bytes = new byte[topic.getSampleMaxSize()];
        buffer = ByteBuffer.allocateDirect(topic.getSampleMaxSize());

        this.participant = Participant.getInstance(topic.getDomainID(), topic.getParticipantID());
        inProcessTransport = participant.getInProcessTransport();
        start();
    }

    /// <summary>
    /// Start the publisher (necessary when using TCP as transport and it has been stopped)
    /// </summary>
    public synchronized void start()
    {
        if (!started)
        {
            try
            {
                // Tell the sendDataHandler that we need it
                sendDataHandler = participant.getSendDataHandler(topic);
                sendDataHandler.addPublisher(this);
                started = true;

                // We need our own copy since we need to update the topic
                Topic top = (Topic)topic.clone();

                // Update with actual port used
                sendDataHandler.updateTransportInfo(top);
                participant.updatePubPartInfo(top);
            }
            catch (CommException ex)
            {
                participant.report(this.getClass().getName(), "start", ex.getMessage());
            }
        }
    }

    /// <summary>
    /// Close down the publisher (necessary when using TCP as transport)
    /// </summary>
    public synchronized void stop()
    {
        if (started)
        {
            // Tell the sendDataHandler that we don't need it anymore
            sendDataHandler.removePublisher(this);
            participant.releaseSendDataHandler(topic);
            sendDataHandler = null;
            started = false;
        }
    }

    protected void write(OPSObject o)
    {
        if (sendDataHandler == null)
        {
            return;
        }
        sampleTime2 = sampleTime1;
        sampleTime1 = System.currentTimeMillis();

        OPSMessage message = new OPSMessage();
        o.setKey(key);
        message.setData(o);
        message.setPublicationID(currentPublicationID);
        message.setTopicName(topic.getName());
        message.setPublisherName(name);

        //inProcessTransport.copyAndPutMessage(message);

        WriteByteBuffer buf = new WriteByteBuffer(buffer, StaticManager.MAX_SIZE);
        try
        {
            buffer.position(0);
            OPSArchiverOut archiverOut = new OPSArchiverOut(buf, topic.getOptNonVirt());

            archiverOut.inout("message", message);

            //If o has spare bytes, write them to the end of the buf
            if (o.spareBytes.length > 0)
            {
                buf.write(o.spareBytes, 0, o.spareBytes.length);
            }
            //Finish will in nrOf segments in all segments.
            buf.finish();
            int sizeToSend = buffer.position();

            //transport.send(archiverOut.getBytes());
            buffer.position(0);
            buffer.get(bytes);
            sendDataHandler.sendData(bytes, sizeToSend, topic);
            //sendDataHandler.sendData(buffer.array(), buf.position(), topic);

        }
        catch (IOException ex)
        {
            Logger.getLogger(Publisher.class.getName()).log(Level.SEVERE, null, ex);
            ex.printStackTrace();
        }
        incCurrentPublicationID();
    }

    public void writeAsOPSObject(OPSObject o)
    {
        write(o);
    }

    private void incCurrentPublicationID()
    {
        if (currentPublicationID < 10000)
        {
            currentPublicationID++;
        }
        else
        {
            currentPublicationID = 1;
        }
    }

    public String getName()
    {
        return name;
    }

    public void setName(String name)
    {
        this.name = name;
    }

    public Topic getTopic()
    {
        return topic;
    }

    public String getKey()
    {
        return key;
    }

    public void setKey(String key)
    {
        this.key = key;
    }

    public int getCurrentPublicationID()
    {
        return currentPublicationID;
    }

    public int getReliableWriteTimeout()
    {
        return reliableWriteTimeout;
    }

    public double getOutboundRate()
    {
        double sampleRate = 1.0/((sampleTime1 - sampleTime2) / 1000.0);
        double fakeRate  = 1.0/((System.currentTimeMillis() - sampleTime1) / 1000.0);

        if(sampleRate < fakeRate)
        {
            return sampleRate;
        }
        else
        {
            return fakeRate;
        }
    }

    public void setReliableWriteTimeout(int reliableWriteTimeout)
    {
        this.reliableWriteTimeout = reliableWriteTimeout;
    }

    public int getReliableWriteNrOfResends()
    {
        return reliableWriteNrOfResends;
    }

    public void setReliableWriteNrOfResends(int reliableWriteNrOfResends)
    {
        this.reliableWriteNrOfResends = reliableWriteNrOfResends;
    }

}
