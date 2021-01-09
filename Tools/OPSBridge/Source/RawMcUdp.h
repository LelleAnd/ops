/**
*
* Copyright (C) 2018-2019 Lennart Andersson.
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

#include <string>
#include <map>
#include <set>
#include <memory>

#include "IOService.h"
#include "SingleThreadPool.h"
#include "Receiver.h"
#include "Sender.h"

#include "BridgeMessages.h"

namespace opsbridge {

	class RawMcUdp;

	class RawMcUdpListener
	{
	public:
		virtual void onUdpMcMessage(RawMcUdp* sender, TUdpMcMessage& mess, const char* data) = 0;
	};

	class RawMcUdp : ops::Runnable, ops::Listener<ops::BytesSizePair>
	{
	public:
		RawMcUdp(RawMcUdpListener* const client);
		~RawMcUdp();
		bool AddReceiver(ops::Address_T ip, uint16_t const port, ops::Address_T const ifc);
		void AddTranslation(ops::Address_T ip, int port, ops::Address_T newIp, int newPort, ops::Address_T ifc = "", int ttl = 1);

		void Write(const TUdpMcMessage& mess, const char* const data);

	private:
		bool _keepRunning = true;
		bool _started = false;
		RawMcUdpListener* _client;
		std::unique_ptr<ops::IOService> _ioService;
		ops::ThreadPool* _threadPool = nullptr;
		struct entry_t {
			std::unique_ptr<ops::Receiver> receiver;
			char buffer[65536];
		};
		std::map<ops::InternalKey_T, entry_t> _receivers;
		struct translation_t {
			ops::Address_T dstIp;
			int dstPort = 0;
			ops::Address_T Ifc = "127.0.0.1";
			int ttl = 1;
		};
		std::map<ops::InternalKey_T, translation_t> _translations;
		struct SEntry_t {
			std::unique_ptr<ops::Sender> sender;
			translation_t t;
		};
		std::map<ops::InternalKey_T, SEntry_t> _senders;
		std::set<int> _senderPorts;

		virtual void run();
#if defined(_MSC_VER) && (_MSC_VER == 1900)
#pragma warning( disable : 4373)
#endif
		void onNewEvent(ops::Notifier<ops::BytesSizePair>* const sender, ops::BytesSizePair byteSizePair);
	};
}
