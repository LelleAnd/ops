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

import java.lang.management.ManagementFactory;
import java.util.logging.Level;
import java.util.logging.Logger;
import configlib.SerializableFactory;
import configlib.exception.FormatException;
import java.io.File;
import java.io.IOException;
import ops.archiver.OPSObjectFactory;
import ops.transport.inprocess.InProcessTransport;
import ops.ConfigurationException;

/**
 *
 * @author Anton Gravestam
 */
public class Participant
{
    //This factory is used through static facade methods getInstance(...)
    private static ParticipantFactory participantFactory = new ParticipantFactory();

    protected String domainID;
    private String participantID;
    protected Domain domain;
    private ErrorService errorService = new ErrorService();
    private ReceiveDataHandlerFactory receiveDataHandlerFactory = new ReceiveDataHandlerFactory();
    private SendDataHandlerFactory sendDataHandlerFactory = new SendDataHandlerFactory();
    private InProcessTransport inProcessTransport = new InProcessTransport();

    private final ParticipantInfoData partInfoData = new ParticipantInfoData();
    private ParticipantInfoDataListener partInfoListener = new ParticipantInfoDataListener(this);

    private Publisher partInfoPub = null;
    private boolean keepRunning = false;

    private OPSConfig config = null;

    /**
     * Method for retreiving the default Participant instance for the @param domainID
     * @param domainID
     * @return a Participant or null if this method fails.
     */
    public static synchronized Participant getInstance(String domainID) throws ConfigurationException
    {
        return participantFactory.getParticipant(domainID, "DEFAULT_PARTICIPANT");
    }

    /**
     * Method for retreiving the @param participantID Participant instance for @param domainID
     * @param domainID
     * @param participantID
     * @return a Participant or null if this method fails.
     */
    public static synchronized Participant getInstance(String domainID, String participantID) throws ConfigurationException
    {
        return participantFactory.getParticipant(domainID, participantID, null);
    }

    /**
     * Method for retreiving the @param participantID Participant instance for
     * @param domainID using @param file if this participant has not yet been created.
     * @param domainID
     * @param participantID
     * @param file the file to use as configuration. Ignored if a participant for
     * participantID already exist.
     * @return a Participant or null if this method fails.
     */
    public static synchronized Participant getInstance(String domainID, String participantID, File file) throws ConfigurationException
    {
        return participantFactory.getParticipant(domainID, participantID, file);
    }


    public static synchronized Participant getInstance(Domain domain, String participantID)
    {
        return participantFactory.getParticipant(domain, participantID);
    }

    protected Participant(String domainID, String participantID, File configFile) throws ConfigurationException
    {
        this.domainID = domainID;
        this.participantID = participantID;
        try
        {
            if (configFile == null)
            {
                config = OPSConfig.getConfig();
                if (config != null) domain = config.getDomain(domainID);
            }
            else
            {
                config = OPSConfig.getConfig(configFile);
                if (config != null) domain = config.getDomain(domainID);
            }
            if (domain != null)
            {
                inProcessTransport.start();
                setupCyclicThread();
            }
            else
            {
                throw new ConfigurationException("Participant(): Failed to find requested domain in configuration file");
            }
        }
        catch (IOException ex)
        {
            throw new ConfigurationException("Participant(): Configuration file missing");
        }
        catch (FormatException ex)
        {
            throw new ConfigurationException("Participant(): Format error in Configuration file");
        }
    }

    protected Participant(Domain domain, String participantID)
    {
        this.domainID = domain.getDomainID();
        this.participantID = participantID;

        this.domain = domain;

        inProcessTransport.start();
        setupCyclicThread();
    }

    /**
     * Return the OPSConfig object used (if any).
     * Note that null can be returned if Participant is created from a Domain object
     */
    public OPSConfig getConfig()
    {
        return config;
    }

    // Initialize static data in partInfoData
    private void initPartInfoData()
    {
        String processId = "";
        String localhostname = "";
        try
        {
            processId = ManagementFactory.getRuntimeMXBean().getName().split("@")[0];
            localhostname = java.net.InetAddress.getLocalHost().getHostName();
        } catch (Exception e)
        {
        }
        String Name = localhostname + " (" + processId + ")";

        synchronized (partInfoData)
        {
            partInfoData.name = Name;
            partInfoData.languageImplementation = "Java";
            partInfoData.id = participantID;
            partInfoData.domain = domainID;
            partInfoData.ip = "";
            partInfoData.opsVersion = "";
        }
    }

    public void setUdpTransportInfo(String ip, int port)
    {
        synchronized (partInfoData)
        {
            partInfoData.ip = ip;
            partInfoData.mc_udp_port = port;
        }
    }

    public void connectUdp(Topic t, McUdpSendDataHandler udpSendDataHandler)
    {
        partInfoListener.connectUdp(t, udpSendDataHandler);
    }

    public void disconnectUdp(Topic t, McUdpSendDataHandler udpSendDataHandler)
    {
        partInfoListener.disconnectUdp(t, udpSendDataHandler);
    }

    public void connectTcp(String topicName, TcpReceiveDataHandler rdh)
    {
        partInfoListener.connectTcp(topicName, rdh);
    }

    public void disconnectTcp(String topicName, TcpReceiveDataHandler rdh)
    {
        partInfoListener.disconnectTcp(topicName, rdh);
    }

    //
    public boolean hasPublisherOn(String topicName)
    {
        synchronized (partInfoData)
        {
            for (int i = 0; i < partInfoData.publishTopics.size(); i++)
            {
                if (partInfoData.publishTopics.elementAt(i).name.equals(topicName))
                {
                    return true;
                }
            }
        }
        return false;
    }

    //
    public boolean hasSubscriberOn(String topicName)
    {
        synchronized (partInfoData)
        {
            for (int i = 0; i < partInfoData.subscribeTopics.size(); i++)
            {
                if (partInfoData.subscribeTopics.elementAt(i).name.equals(topicName))
                {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Remove listener for Error events.
     * @param listener
     */
    public void removeListener(Listener<Error> listener)
    {
        errorService.removeListener(listener);
    }

    /**
     * Add listener for Error events.
     * @param listener
     */
    public synchronized void addListener(Listener<Error> listener)
    {
        errorService.addListener(listener);
    }

    /**
     * Report a core Error.
     * @param className the class in which the error occured.
     * @param methodName the method in which the error occured.
     * @param errorMessage a message descriing the error.
     */
    void report(String className, String methodName, String errorMessage)
    {
        errorService.report(className, methodName, errorMessage);
    }

    /**
     * Report a core Error
     * @param error the Error to report.
     */
    void report(Error error)
    {
        errorService.report(error);
    }


    /**
     * Adds type support in form of a SerializablFactory to this participant.
     * Normally this is the IDL project generated TypeFactory (e.g. FooProject.FooProjectTypeFactory())
     * @param typeSupport
     */
    public void addTypeSupport(SerializableFactory typeSupport)
    {
        OPSObjectFactory.getInstance().add(typeSupport);
    }

    /**
     * Creates a Topic that can be used to create Publishers and/or Subscribers.
     * The fields of the Topic returned are fetched from the participants underlying config file.
     * @param name
     * @return a new Topic based on the config of this participant.
     */
    public Topic createTopic(String name)
    {
        Topic topic = domain.getTopic(name);

        if(topic == null)
            return null;

        topic.setParticipantID(participantID);
        topic.setDomainID(domainID);

        return topic;
    }

    /**
     * Creates a Topic that can be used to Subscribe to the participant info data
     * @return a new Topic for the participant info data.
     */
    public Topic createParticipantInfoTopic()
    {
      Topic infoTopic = new Topic("ops.bit.ParticipantInfoTopic", domain.GetMetaDataMcPort(), "ops.ParticipantInfoData", domain.getDomainAddress());
      infoTopic.setLocalInterface(domain.getLocalInterface());
      infoTopic.setTimeToLive(domain.getTimeToLive());
    	infoTopic.setDomainID(domainID);
      infoTopic.setParticipantID(participantID);
	    infoTopic.setTransport(Topic.TRANSPORT_MC);
    	return infoTopic;
    }

    ///By modified singelton
    synchronized ReceiveDataHandler getReceiveDataHandler(Topic top)
    {
        ReceiveDataHandler rdh = receiveDataHandlerFactory.getReceiveDataHandler(top, this);
        if (rdh != null)
        {
            synchronized (partInfoData)
            {
                partInfoData.subscribeTopics.add(new TopicInfoData(top));
            }
        }
        return rdh;
    }

    synchronized void releaseReceiveDataHandler(Topic topic)
    {
        receiveDataHandlerFactory.ReleaseReceiveDataHandler(topic, this);

        synchronized (partInfoData)
        {
            for (int i = 0; i < partInfoData.subscribeTopics.size(); i++)
            {
                if (partInfoData.subscribeTopics.elementAt(i).name.equals(topic.getName())) {
                    partInfoData.subscribeTopics.remove(i);
                    break;
                }
            }
        }
    }

    ///By modified singelton
    synchronized SendDataHandler getSendDataHandler(Topic t) throws CommException
    {
        SendDataHandler sdh = sendDataHandlerFactory.getSendDataHandler(t, this);
        // We can't update Participant Info here, delayed until updatePubPartInfo()
        return sdh;
    }

    void updatePubPartInfo(Topic t)
    {
        synchronized (partInfoData)
        {
            partInfoData.publishTopics.add(new TopicInfoData(t));
        }
    }

    synchronized void releaseSendDataHandler(Topic topic)
    {
        sendDataHandlerFactory.releaseSendDataHandler(topic, this);

        synchronized (partInfoData)
        {
            for (int i = 0; i < partInfoData.publishTopics.size(); i++)
            {
                if (partInfoData.publishTopics.elementAt(i).name.equals(topic.getName())) {
                    partInfoData.publishTopics.remove(i);
                    break;
                }
            }
        }
    }

    public Domain getDomain()
    {
        return domain;
    }

    public InProcessTransport getInProcessTransport()
    {
        return inProcessTransport;
    }

    private void setupCyclicThread()
    {
        keepRunning = true;
        Thread thread = new Thread(new Runnable()
        {
            public void run()
            {
                initPartInfoData();

                while (keepRunning)
                {
                    try
                    {
                        Thread.sleep(1000);

                        // Create publisher for participant info data
                        if ( (partInfoPub == null) && (domain.GetMetaDataMcPort() != 0) )
                        {
                            partInfoPub = new Publisher(createParticipantInfoTopic());
                        }

                        // Publish data
                        if (partInfoPub != null)
                        {
                            synchronized (partInfoData)
                            {
                                partInfoPub.writeAsOPSObject(partInfoData);
                            }
                        }
                    }
                    catch (InterruptedException ex)
                    {
                        Logger.getLogger(Participant.class.getName()).log(Level.SEVERE, null, ex);
                    }
                    catch (ConfigurationException ex)
                    {
                        Logger.getLogger(Participant.class.getName()).log(Level.SEVERE, null, ex);
                    }
                }
                System.out.println("participant thread exit");
            }
        });

        thread.setName("ParticipantThread_" + domainID + "_" + participantID);
        thread.start();
    }

    public void stopThread()
    {
        keepRunning = false;
        inProcessTransport.stopTransport();
    }

}
