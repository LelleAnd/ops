/**
 *
 * Copyright (C) 2006-2009 Anton Gravestam.
 * Copyright (C) 2020-2021 Lennart Andersson.
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
#ifndef ops_McUdpSendDataHandler_h
#define	ops_McUdpSendDataHandler_h

#include <map>
#include <iostream>
#include <sstream>
#include <stack>
#include <chrono>

#include "SendDataHandler.h"
#include "Sender.h"
#include "OPSObject.h"
#include "TimeHelper.h"

namespace ops
{

    class McUdpSendDataHandler : public SendDataHandler
    {
    public:
        McUdpSendDataHandler(IOService* ioService, Address_T localInterface, int64_t outSocketBufferSize = 16000000)
        {
            sender = Sender::createUDPSender(ioService, localInterface, 1, outSocketBufferSize);
        }

        bool sendData(char* buf, int bufSize, Topic& topic) override
        {
            bool result = true;
            std::stack<InternalKey_T> sinksToDelete;

            SafeLock lock(mutex);

            Entry_T& topicSinks = topicSinkMap[topic.getName()];

            //Loop over all sinks and send data, remove items that isn't "alive".
            for (const auto& kv : topicSinks.portMap) {
                const IpPortPair& sink = kv.second;
                if (sink.isAlive()) {
                    //std::cout << topic.getName() << ", sending to sink: " << sink.second.getKey() << std::endl;
                    result &= sender->sendTo(buf, bufSize, sink._ip, (uint16_t)sink._port);
                }
                else
                {
                    //std::cout << topic.getName() << ", removing sink: " << it->second.getKey() << std::endl;
                    sinksToDelete.push(sink._key);
                }
            }
            while (!sinksToDelete.empty()) {
                topicSinks.portMap.erase(topicSinks.portMap.find(sinksToDelete.top()));
                sinksToDelete.pop();
            }
            return result;
        }

		void addSink(ObjectName_T& topic, Address_T& ip, int& port, bool staticRoute = false)
		{
            SafeLock lock(mutex);

			const IpPortPair ipPort(ip, port, staticRoute);

			//Check if we already have any sinks for this topic
            if (topicSinkMap.find(topic) == topicSinkMap.end())
            {
                //We have no sinks for this topic.
                Entry_T ent;
                ent.staticRoute = staticRoute;

                //Add the new sink to the port map.
                ent.portMap[ipPort._key] = ipPort;

                //And add the port map to the sink map
                topicSinkMap[topic] = ent;

                //std::cout << topic << ", added first sink: " << ipPort.getKey() << std::endl;
            }
            else
            {
				Entry_T& topicSinks = topicSinkMap[topic];

				//If created as static route, we only add sinks that are static
				if ( (!topicSinks.staticRoute) || (topicSinks.staticRoute && staticRoute)) {
					//Check if sink already exist
					if (topicSinks.portMap.find(ipPort._key) == topicSinks.portMap.end())
					{
						//No, add the new sink to the map.
						topicSinks.portMap[ipPort._key] = ipPort;

						//std::cout << topic << ", added next sink: " << ipPort.getKey() << std::endl;
					}
					else
					{
						//this sink is already registered with this topic, just update
						topicSinks.portMap[ipPort._key].feedWatchdog(staticRoute);
						//std::cout << topic << ", feedWatchDog for sink: " << ipPort.getKey() << std::endl;
					}
				}
            }
        }

        virtual ~McUdpSendDataHandler() = default;

    private:
        class IpPortPair
        {
        public:
            IpPortPair(Address_T ip, int port, bool alwaysAlive):
				_ip(ip), _port(port), _alwaysAlive(alwaysAlive)
            {
                _lastTimeAlive = ops_clock::now();
                _key = _ip.c_str();
                _key += ':';
                _key += NumberToString(_port);
            }

            IpPortPair() noexcept {}

            bool isAlive() const
            {
                return _alwaysAlive || ((ops_clock::now() - _lastTimeAlive) < std::chrono::milliseconds(ALIVE_TIMEOUT));
            }

            void feedWatchdog(bool alwaysAlive)
            {
                _alwaysAlive |= alwaysAlive;	// once set to true, always true
                _lastTimeAlive = ops_clock::now();
            }

			Address_T _ip;
            int _port{ 0 };
            InternalKey_T _key;
            bool _alwaysAlive{ false };
            ops_clock::time_point _lastTimeAlive;
            const static int ALIVE_TIMEOUT = 3000;
        };

		typedef struct entry {
			bool staticRoute = false;
			std::map<InternalKey_T, IpPortPair> portMap;
		} Entry_T;

		std::map<ObjectName_T, Entry_T > topicSinkMap;
    };
}
#endif
