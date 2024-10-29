/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package opstestapp;

import java.util.Observable;
import java.util.Observer;
import ops.ConfigurationException;

import ops.Publisher;
import ops.Subscriber;
import ops.Participant;
import ops.OPSConfig;
import ops.Domain;
import ops.Topic;
import ops.KeyFilterQoSPolicy;
import ops.OPSObject;


/**
 *
 * @author Lelle
 */
public class COpsHelper implements IOpsHelper
{
    private IOpsHelperListener client = null;
    private Publisher pub = null;
    private Subscriber sub = null;

    public COpsHelper(IOpsHelperListener client)
    {
        this.client = client;
    }

    public boolean HasPublisher()
    {
      return pub != null;
    }

    public boolean HasSubscriber()
    {
      return sub != null;
    }

    private void Log(String str)
    {
        if (client != null) client.OnLog(str);
    }

    private void LogTopic(Topic t)
    {
        Log("Created topic: " + t.getName() +
            " [ " + t.getTransport() +
            "::" + t.getDomainAddress() +
            "::" + t.getPort() +
            " ]");
        Log("  InSocketBufferSize: " + t.getInSocketBufferSize() + "\n" +
            "  OutSocketBufferSize: " + t.getOutSocketBufferSize() + "\n" +
            "  SampleMaxSize: " + t.getSampleMaxSize() + "\n");
    }

    public void CreateSubscriber(Participant part, String topName)
    {
        if (sub == null) {
            try
            {
                Topic top = part.createTopic(topName);
                LogTopic(top);
                sub = new Subscriber(top);
                //Add a listener to the subscriber
                sub.addObserver(new Observer() {
                    public void update(Observable o, Object arg)
                    {
                        if (client != null) client.OnData(sub.getTopic().getName(), sub.getMessage(), sub.getMessage().getData());
                    }
                });
                sub.deadlineEvent.addObserver(new Observer() {
                    public void update(Observable o, Object arg)
                    {
                        Log("Deadline missed for topic " + sub.getTopic().getName() + " !!\n");
                    }
                });
                sub.start();
            }
            catch (ConfigurationException e)
            {
                Log("Exception: " + e.getMessage());
            }
        } else {
            Log("Subscriber for topic " + sub.getTopic().getName() + " already exist!!\n");
        }
    }

    public void DeleteSubscriber()
    {
        if (sub != null) {
            sub.stop();
            sub.deleteObservers();
            Log("Deleted Subscriber for topic " + sub.getTopic().getName() + "\n");
            sub = null;
        } else {
            Log("Subscriber must be created first!!\n");
        }
    }

    public void StartSubscriber(long deadLineEventInterval)
    {
        if (sub != null) {
            sub.setDeadlineQoS(deadLineEventInterval);
            sub.start();
            Log("Started Subscriber for topic " + sub.getTopic().getName() + "\n");
        } else {
            Log("Subscriber must be created first!!\n");
        }
    }

    public void StopSubscriber(boolean doLog)
    {
        if (sub != null) {
            sub.stop();
            Log("Stopped Subscriber for topic " + sub.getTopic().getName() + "\n");
        } else {
            Log("Subscriber must be created first!!\n");
        }
    }

    public void SetDeadLineInterval(long deadLineEventInterval)
    {
        if (sub != null) {
            sub.setDeadlineQoS(deadLineEventInterval);
            Log("Set deadline timeout to " + deadLineEventInterval + " for topic " + sub.getTopic().getName() + "\n");
        } else {
            Log("Subscriber must be created first!!\n");
        }
    }

    public void CreatePublisher(Participant part, String topName)
    {
        if (pub == null) {
            try
            {
                Topic top = part.createTopic(topName);
                LogTopic(top);
                pub = new Publisher(top);
                pub.setName("OpsTestApp-Java");
            }
            catch (ConfigurationException e)
            {
                Log("Exception: " + e.getMessage());
            }
        } else {
            Log("Publisher for topic " + pub.getTopic().getName() + " already exist!!!\n");
        }
    }

    public void DeletePublisher()
    {
        if (pub != null) {
            pub.stop();
            Log("Deleted Publisher for topic " + pub.getTopic().getName() + "\n");
            pub = null;
        } else {
            Log("Publisher must be created first!!\n");
        }
    }

    public void Write(OPSObject Data)
    {
        if (pub != null) {
            pub.writeAsOPSObject(Data);
        } else {
            Log("Publisher must be created first!!\n");
        }
    }
}
