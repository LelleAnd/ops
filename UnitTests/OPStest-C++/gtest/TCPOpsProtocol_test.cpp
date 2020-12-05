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

#include "gtest/gtest.h"

#include "TCPOpsProtocol.h"

// ===============================
// Helper classes

class MyTcpProtClient : public ops::TCPProtocolCallbacks
{
public:
	ops::TCPProtocol* _prot;

	MyTcpProtClient(ops::TCPProtocol* const prot) : _prot(prot) 
	{
		memset(sbuf, 0, sizeof(sbuf));
	}
	virtual ~MyTcpProtClient() = default;

	MyTcpProtClient() = delete;
	MyTcpProtClient(const MyTcpProtClient& r) = delete;
	MyTcpProtClient& operator= (const MyTcpProtClient& l) = delete;
	MyTcpProtClient(MyTcpProtClient&&) = delete;
	MyTcpProtClient& operator =(MyTcpProtClient&&) = delete;

	bool _connected = false;

	virtual bool isConnected(ops::TCPProtocol& prot) override
	{
		EXPECT_EQ(_prot, &prot);
		return _connected;
	}

	char* _dstPtr = nullptr;
	int _toRead = 0;

	virtual void startAsyncRead(ops::TCPProtocol& prot, char* const bytes, const uint32_t size) override
	{
		EXPECT_EQ(_prot, &prot);
		_dstPtr = bytes;
		_toRead = size;
	}

	ops::BytesSizePair _event = { nullptr, 0 };

	virtual void onEvent(ops::TCPProtocol& prot, const ops::BytesSizePair arg) override
	{
		EXPECT_EQ(_prot, &prot);
		_event = arg;
	}

	char sbuf[1024];
	int sbuf_idx = 0;
	int send_count = 0;

	void ClearBuf()
	{
		memset(sbuf, 0, sizeof(sbuf));
		sbuf_idx = 0;
		send_count = 0;
	}

	virtual int sendBuffer(ops::TCPProtocol& prot, const char* const bytes, const uint32_t size) override
	{
		send_count++;

		EXPECT_EQ(_prot, &prot);

		int const space = (int)sizeof(sbuf) - sbuf_idx;
		if (size > (uint32_t)space) { return -1; }

		memcpy(&sbuf[sbuf_idx], bytes, size);
		sbuf_idx += size;

		return size;
	}

	void verifyProbeMessage()
	{
		EXPECT_EQ(sbuf_idx, 23);

		int sndlen = *((int*)(sbuf + 18));
		EXPECT_EQ(sndlen, 1);

		int version = *((int*)(sbuf + 8));
		EXPECT_EQ(version, 2);

		char const save = sbuf[8];
		sbuf[8] = '\0';

		EXPECT_STREQ(sbuf, "opsprobe");

		sbuf[8] = save;
	}

	void verifyHeartBeatMessage()
	{
		EXPECT_EQ(sbuf_idx, 22);

		int sndlen = *((int*)(sbuf + 18));
		EXPECT_EQ(sndlen, 0);

		int version = *((int*)(sbuf + 8));
		EXPECT_EQ(version, 2);

		char const save = sbuf[8];
		sbuf[8] = '\0';

		EXPECT_STREQ(sbuf, "opsprobe");

		sbuf[8] = save;
	}
};

struct MyTime
{
	static int64_t _now;

	static int64_t clock()
	{
		return _now;
	}

	MyTime() = default;
	MyTime(const MyTime& r) = default;
	MyTime& operator= (const MyTime& l) = default;
	MyTime(MyTime&&) = default;
	MyTime& operator =(MyTime&&) = default;
	~MyTime() = default;
};
int64_t MyTime::_now = 0;

// ===============================

TEST(Test_TCPProtocol, TestTCPProtocolVer1) {

	// No user data used in this test case
	ops::TCPOpsProtocol prot(&MyTime::clock, 1000, 3000);
	MyTcpProtClient client(&prot);

	char buffer[1024];

	// --------------------
	// Too small buffer
	EXPECT_FALSE(prot.startReceive(buffer, 22));

	// --------------------
	// No connected client
	EXPECT_FALSE(prot.startReceive(buffer, sizeof(buffer)));

	prot.connect(&client);

	// --------------------
	// Buffer large enough
	EXPECT_TRUE(prot.startReceive(buffer, sizeof(buffer)));
	EXPECT_EQ(client._dstPtr, &buffer[0]);
	EXPECT_EQ(client._toRead, 22);

	// Data when no client
	prot.connect(nullptr);
	EXPECT_FALSE(prot.handleReceivedData(0, 18));
	prot.connect(&client);

	// --------------------
	EXPECT_TRUE(prot.startReceive(buffer, sizeof(buffer)));
	EXPECT_EQ(client._dstPtr, &buffer[0]);
	EXPECT_EQ(client._toRead, 22);

	// Data when not connected
	EXPECT_FALSE(client.isConnected(prot));
	EXPECT_FALSE(prot.handleReceivedData(0, 18));
	EXPECT_EQ(client._event.bytes, nullptr);
	EXPECT_EQ(client._event.size, -2);
	client._event.size = 0;

	// --------------------
	EXPECT_TRUE(prot.startReceive(buffer, sizeof(buffer)));
	EXPECT_EQ(client._dstPtr, &buffer[0]);
	EXPECT_EQ(client._toRead, 22);

	// Data when connected, but with an error
	client._connected = true;
	EXPECT_TRUE(client.isConnected(prot));
	EXPECT_FALSE(prot.handleReceivedData(1, 18));
	EXPECT_EQ(client._event.bytes, nullptr);
	EXPECT_EQ(client._event.size, 0);

	// --------------------
	EXPECT_TRUE(prot.startReceive(buffer, sizeof(buffer)));
	EXPECT_EQ(client._dstPtr, &buffer[0]);
	EXPECT_EQ(client._toRead, 22);

	// Data when connected and no error
	EXPECT_TRUE(prot.handleReceivedData(0, 18));
	EXPECT_EQ(client._dstPtr, &buffer[18]);
	EXPECT_EQ(client._toRead, 4);

    // Data with too large length
	buffer[18] = 45;
	buffer[19] = 55;
	buffer[20] = 0;
	buffer[21] = 0;
    OPS_TCP_INFO("This test will trace an error from handleData() and a message dump\n");
    EXPECT_FALSE(prot.handleReceivedData(0, 4));

	// --------------------
	EXPECT_TRUE(prot.startReceive(buffer, sizeof(buffer)));
	EXPECT_EQ(client._dstPtr, &buffer[0]);
	EXPECT_EQ(client._toRead, 22);

	// Data when connected and no error
	EXPECT_TRUE(prot.handleReceivedData(0, 18));
	EXPECT_EQ(client._dstPtr, &buffer[18]);
	EXPECT_EQ(client._toRead, 4);

	// Data with OK length
	buffer[18] = 0x45;
	buffer[19] = 0x2;
	buffer[20] = 0;
	buffer[21] = 0;
	EXPECT_TRUE(prot.handleReceivedData(0, 4));
	EXPECT_EQ(client._dstPtr, &buffer[0]);
	EXPECT_EQ(client._toRead, 0x245);

	// First 100 bytes
	EXPECT_TRUE(prot.handleReceivedData(0, 100));
	EXPECT_EQ(client._dstPtr, &buffer[100]);
	EXPECT_EQ(client._toRead, 0x245 - 100);
	EXPECT_EQ(client._event.bytes, nullptr);
	EXPECT_EQ(client._event.size, 0);

	// The rest of the requested bytes
	EXPECT_TRUE(prot.handleReceivedData(0, 0x245 - 100));
	EXPECT_EQ(client._event.bytes, &buffer[0]);
	EXPECT_EQ(client._event.size, 0x245);

	EXPECT_EQ(prot.detectedVersion(), 1);

	// --------------------
	// Send with OK buffer and amount
	memset(client.sbuf, 0, sizeof(client.sbuf));
	memset(buffer, 0, sizeof(buffer));
#ifdef _MSC_VER
  #pragma warning( disable : 4996 )
#endif
	strcpy(buffer, "Hej hopp i lingonskogen");

	EXPECT_EQ(prot.sendData(buffer, (uint32_t)strlen(buffer)), (int)strlen(buffer));
	EXPECT_EQ(client.sbuf_idx, 22 + (int)strlen(buffer));

	int sndlen = *((int*)(client.sbuf + 18));
	EXPECT_EQ(sndlen, (int)strlen(buffer));

	client.sbuf[18] = '\0';
	EXPECT_STREQ(client.sbuf, "opsp_tcp_size_info");
	EXPECT_STREQ(&client.sbuf[22], "Hej hopp i lingonskogen");

	// --------------------
	// Send with failed write of header
	client.sbuf_idx = sizeof(client.sbuf) - 10;
    OPS_TCP_INFO("This test will trace an error from sendData() with failed header write\n");
    EXPECT_EQ(prot.sendData(buffer, 10), -1);

	// --------------------
	// Send with failed write of data
	client.sbuf_idx = sizeof(client.sbuf) - 50;
    OPS_TCP_INFO("This test will trace an error from sendData() with failed data write\n");
    EXPECT_EQ(prot.sendData(buffer, 60), -1);
}

static bool MyTCPUserDataCalled = false;

class MyTCPUserData : public ops::TCPUserBase
{
public:
	virtual ~MyTCPUserData() { MyTCPUserDataCalled = true; }
	MyTCPUserData() = default;
	MyTCPUserData(const MyTCPUserData& r) = default;
	MyTCPUserData& operator= (const MyTCPUserData& l) = default;
	MyTCPUserData(MyTCPUserData&&) = delete;
	MyTCPUserData& operator =(MyTCPUserData&&) = delete;
};

TEST(Test_TCPProtocol, TestTCPProtocolVer2) {

	{
		ops::TCPOpsProtocol prot(&MyTime::clock, 1000, 3000);
		MyTcpProtClient client(&prot);

		prot.userData = new MyTCPUserData();

		prot.connect(&client);
		client._connected = true;

		char buffer[1024];

		// --------------------
		// Send Probe message
		client.ClearBuf();

		EXPECT_EQ(prot.sendProbe(), 23);
		client.verifyProbeMessage();

		// Save Probe message for use below
		memcpy(buffer, client.sbuf, 23);

		// --------------------
		// Receive Probe Message
		EXPECT_TRUE(prot.startReceive(buffer, sizeof(buffer)));
		EXPECT_EQ(client._dstPtr, &buffer[0]);
		EXPECT_EQ(client._toRead, 22);

		// Data when connected and no error
		EXPECT_TRUE(prot.handleReceivedData(0, 22));
		EXPECT_EQ(client._dstPtr, &buffer[22]);
		EXPECT_EQ(client._toRead, 1);

		EXPECT_TRUE(prot.handleReceivedData(0, 1));
		EXPECT_EQ(client._dstPtr, &buffer[0]);
		EXPECT_EQ(client._toRead, 22);

		EXPECT_EQ(prot.detectedVersion(), 2);

		// --------------------
		// Send Heartbeat message
		client.ClearBuf();

		EXPECT_EQ(prot.sendHeartbeat(), 22);
		client.verifyHeartBeatMessage();

		// Save Probe message for use below
		memcpy(buffer, client.sbuf, 23);

		// --------------------
		// Receive Heartbeat Message
		EXPECT_TRUE(prot.startReceive(buffer, sizeof(buffer)));
		EXPECT_EQ(client._dstPtr, &buffer[0]);
		EXPECT_EQ(client._toRead, 22);

		// Data when connected and no error
		EXPECT_TRUE(prot.handleReceivedData(0, 22));
		EXPECT_EQ(client._dstPtr, &buffer[0]);
		EXPECT_EQ(client._toRead, 22);

		EXPECT_EQ(prot.detectedVersion(), 2);

		// --------------------
		// Reset protocol
		prot.resetProtocol();
		EXPECT_EQ(prot.detectedVersion(), 1);

		// Data when connected and no error
		EXPECT_TRUE(prot.handleReceivedData(0, 22));
		EXPECT_EQ(client._dstPtr, &buffer[0]);
		EXPECT_EQ(client._toRead, 22);

		EXPECT_EQ(prot.detectedVersion(), 2);
	}

	// Verify delete of userdata
	EXPECT_TRUE(MyTCPUserDataCalled);
}

TEST(Test_TCPProtocol, TestTCPProtocolPeriodic) {

	char buffer[1024];

	{
		ops::TCPOpsProtocol prot(&MyTime::clock, 1000, 3000);
		MyTcpProtClient client(&prot);

		prot.connect(&client);
		client._connected = true;

		// --------------------
		// Periodic check with version 1
		EXPECT_EQ(prot.detectedVersion(), 1);
		EXPECT_TRUE(prot.periodicCheck());
		EXPECT_EQ(prot.sendData(nullptr, 0), 0);

		// --------------------
		// Prepare buffer with probe message (needed later to get protocol to version 2)
		client.ClearBuf();
		EXPECT_EQ(prot.sendProbe(), 23);
		client.verifyProbeMessage();
		memcpy(buffer, client.sbuf, 23);
	}

	{
		ops::TCPOpsProtocol prot(&MyTime::clock, 1000, 3000);
		MyTcpProtClient client(&prot);
		MyTime::_now = 10000;

		prot.connect(&client);
		client._connected = true;

		EXPECT_EQ(prot.detectedVersion(), 1);

		// Change protocol to version 2 by providing the earlier prepared probe message
		EXPECT_TRUE(prot.startReceive(buffer, sizeof(buffer)));

		EXPECT_TRUE(prot.handleReceivedData(0, 22));
		EXPECT_EQ(client._dstPtr, &buffer[22]);
		EXPECT_EQ(client._toRead, 1);

		EXPECT_TRUE(prot.handleReceivedData(0, 1));
		EXPECT_EQ(client._dstPtr, &buffer[0]);
		EXPECT_EQ(client._toRead, 22);

		EXPECT_EQ(prot.detectedVersion(), 2);

		// --------------------
		client.ClearBuf();

		// Periodic check with version 2. Should lead to a sent heartbeat
		EXPECT_TRUE(prot.periodicCheck());
		EXPECT_EQ(client.send_count, 1);
		client.verifyHeartBeatMessage();

		// No new send before time is right
		MyTime::_now = 10999;
		EXPECT_TRUE(prot.periodicCheck());
		EXPECT_EQ(client.send_count, 1);

		MyTime::_now = 11000;
		EXPECT_TRUE(prot.periodicCheck());
		EXPECT_EQ(client.send_count, 2);

		MyTime::_now = 12000;
		EXPECT_TRUE(prot.periodicCheck());
		EXPECT_EQ(client.send_count, 3);

		MyTime::_now = 12999;
		EXPECT_TRUE(prot.periodicCheck());
		EXPECT_EQ(client.send_count, 3);

		MyTime::_now = 13000;
		EXPECT_FALSE(prot.periodicCheck());
		EXPECT_EQ(client.send_count, 4);
	}

	{
		ops::TCPOpsProtocol prot(&MyTime::clock, 1000, 3000);
		MyTcpProtClient client(&prot);
		MyTime::_now = 10000;

		prot.connect(&client);
		client._connected = true;

		EXPECT_EQ(prot.detectedVersion(), 1);

		// Change protocol to version 2 by providing the earlier prepared probe message
		EXPECT_TRUE(prot.startReceive(buffer, sizeof(buffer)));

		EXPECT_TRUE(prot.handleReceivedData(0, 22));
		EXPECT_EQ(client._dstPtr, &buffer[22]);
		EXPECT_EQ(client._toRead, 1);

		EXPECT_TRUE(prot.handleReceivedData(0, 1));
		EXPECT_EQ(client._dstPtr, &buffer[0]);
		EXPECT_EQ(client._toRead, 22);

		EXPECT_EQ(prot.detectedVersion(), 2);

		// --------------------
		client.ClearBuf();

		// Periodic check with version 2. Should lead to a sent heartbeat
		EXPECT_EQ(prot.sendData(nullptr, 0), 0);
		EXPECT_EQ(client.send_count, 1);
		client.verifyHeartBeatMessage();

		// Failure to send heartbeat, should lead to failed send
		MyTime::_now = 11000;
		client.sbuf_idx = sizeof(client.sbuf);
		EXPECT_EQ(prot.sendData(nullptr, 0), -1);
	}
}
