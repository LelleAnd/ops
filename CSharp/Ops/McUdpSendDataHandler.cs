///////////////////////////////////////////////////////////
//  McUdpSendDataHandler.cs
//  Implementation of the Class McUdpSendDataHandler
//  Created on:      06-jan-2013
//  Author:
///////////////////////////////////////////////////////////

using System.Collections.Generic;
using System.Runtime.CompilerServices;  // Needed for the "MethodImpl" synchronization attribute

namespace Ops 
{
	public class McUdpSendDataHandler : SendDataHandler 
    {
        private class Entry_T
        {
            public bool staticRoute = false;
            //                port
            public Dictionary<string, IpPortPair> portMap = new Dictionary<string, IpPortPair>();
            public Entry_T(bool staticRoute) { this.staticRoute = staticRoute; }
        }

        //                 Topic
        private Dictionary<string, Entry_T > topicSinkMap = new Dictionary<string,Entry_T>();
        

        public McUdpSendDataHandler(int outSocketBufferSize, string localInterface) 
        {
            sender = new UdpSender(0, localInterface, outSocketBufferSize);  
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public override bool SendData(byte[] bytes, int size, Topic t)
        {
            bool result = true;

            if (this.topicSinkMap.ContainsKey(t.GetName()))
            {
                Entry_T dict = this.topicSinkMap[t.GetName()];
                List<string> sinksToDelete = new List<string>();

                // Loop over all sinks and send data, remove items that isn't "alive".
                foreach (KeyValuePair<string, IpPortPair> kvp in dict.portMap)
                {
                    // Check if this sink is alive
                    if (kvp.Value.IsAlive())
                    {
                        result &= SendData(bytes, size, kvp.Value.Ip, kvp.Value.Port);
                    }
                    else //Remove it.
                    {
                        sinksToDelete.Add(kvp.Key);
                    }
                }
                foreach (string key in sinksToDelete)
                {
                    dict.portMap.Remove(key);
                }
            }
            return result;
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public void AddSink(string topic, string ip, int port, bool staticRoute)
        {
            IpPortPair ipp = new IpPortPair(ip, port, staticRoute);

            if (this.topicSinkMap.ContainsKey(topic))
            {
                Entry_T dict = this.topicSinkMap[topic];

                //If created as static route, we only add sinks that are static
                if ((!dict.staticRoute) || (dict.staticRoute && staticRoute))
                {
                    //Check if sink already exist
                    if (dict.portMap.ContainsKey(ipp.Key))
                    {
                        dict.portMap[ipp.Key].FeedWatchdog(staticRoute);
                    }
                    else
                    {
                        dict.portMap.Add(ipp.Key, ipp);
                    }
                }
            }
            else
            {
                Entry_T dict = new Entry_T(staticRoute);
                dict.portMap.Add(ipp.Key, ipp);
                this.topicSinkMap.Add(topic, dict);
            }
        }

        private class IpPortPair
        {
            public string Ip {get; private set; }
            public int Port {get; private set; }
            public string Key { get; private set; }
            private bool alwaysAlive = false;

            private System.DateTime lastTimeAlive;
            private System.TimeSpan ALIVE_TIMEOUT = System.TimeSpan.FromMilliseconds(3000.0);

            public IpPortPair(string ip, int port, bool alwaysAlive)
            {
                this.Ip = ip;
                this.Port = port;
                this.Key = Ip + ":" + Port.ToString();
                this.alwaysAlive = alwaysAlive;
                this.lastTimeAlive = System.DateTime.Now;
            }

            public bool IsAlive()
            {
                return this.alwaysAlive || ((System.DateTime.Now - this.lastTimeAlive) < ALIVE_TIMEOUT);
            }

            public void FeedWatchdog(bool alwaysAlive)
            {
                this.alwaysAlive |= alwaysAlive;
                this.lastTimeAlive = System.DateTime.Now;
            }
        }
	}

}