/**
*
* Copyright (C) 2018 Lennart Andersson.
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

#pragma once

#include "Listener.h"
#include "BytesSizePair.h"
#include "ConnectStatus.h"
#include "UDPReceiver.h"
#include "TCPClient.h"

class MyBSPListener : public ops::Listener<ops::BytesSizePair>
{
public:
	bool isUdp;
	int counter;
	ops::Address_T srcIp;
	uint16_t srcPort;
	ops::BytesSizePair bsp;

	MyBSPListener(bool udp) : isUdp(udp), counter(0), srcPort(0), bsp(nullptr, 0) {}

	virtual void onNewEvent(ops::Notifier<ops::BytesSizePair>* sender, ops::BytesSizePair arg)
	{
		//std::cout << "callback from receiver" << std::endl;
		counter++;
		bsp = arg;
		if (isUdp) {
			ops::UDPReceiver* rcv = dynamic_cast<ops::UDPReceiver*>(sender);
			EXPECT_NE(rcv, nullptr);
			if (rcv) {
				rcv->getSource(srcIp, srcPort);
			}
		} else {
			ops::TCPClient* rcv = dynamic_cast<ops::TCPClient*>(sender);
			EXPECT_NE(rcv, nullptr);
			if (rcv) {
				rcv->getSource(srcIp, srcPort);
			}
		}
	}
};

class MyConnectListener : public ops::Listener<ops::ConnectStatus>
{
public:
	ops::ConnectStatus cst;
	std::string msg;

	MyConnectListener(const std::string& mess) : cst(false, 0), msg(mess) {}

	virtual void onNewEvent(ops::Notifier<ops::ConnectStatus>* , ops::ConnectStatus arg)
	{
		std::cout << msg << "IP: " << arg.addr << "::" << arg.port;
		if (arg.connected) {
			std::cout << " Connected.";
		} else {
			std::cout << " Disconnected.";
		}
		std::cout << " Total: " << arg.totalNo << std::endl;
		cst = arg;
	}
};
