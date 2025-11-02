/**
*
* Copyright (C) 2018-2025 Lennart Andersson.
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

/// TODO:
///   - Tests use a fixed port = 9999, which could lead to failed tests if already in use
///   - Different error cases
///   - ...

#include "gtest/gtest.h"

#include <thread>

#include "UDPSender.h"
#include "UDPReceiver.h"
#include "TCPClient.h"
#include "TCPServer.h"
#include "TCPOpsProtocol.h"
#include "TimeHelper.h"

#include "RAII_ioServ.h"
#include "listener_helpers.h"

using namespace ops;

// ===============================
// Helper

static void WaitWTimeout(Receiver& , RAII_ioServ& ioServ, int64_t timeoutMs)
{
	ops::ops_clock::time_point limit = ops::ops_clock::now() + std::chrono::milliseconds(timeoutMs);
	while (ops::ops_clock::now() < limit) {
		ioServ()->poll();
	}
}

static void PollWTimeout(RAII_ioServ& ioServ, int64_t timeoutMs)
{
	ops::ops_clock::time_point limit = ops::ops_clock::now() + std::chrono::milliseconds(timeoutMs);
	while (ops::ops_clock::now() < limit) {
		ioServ()->poll();
	}
}

static void WaitConnectedWTimeout(TCPClient& rcv, TCPServer& snd, RAII_ioServ& ioServ, int64_t timeoutMs)
{
	ops::ops_clock::time_point limit = ops::ops_clock::now() + std::chrono::milliseconds(timeoutMs);
	while ( ((!rcv.isConnected()) || (snd.numConnected() == 0)) && (ops::ops_clock::now() < limit)) {
		ioServ()->poll();
	}
}

void Log(std::string msg, ConnectStatus status)
{
	std::cout << msg << ": IP: " << status.addr << "::" << status.port;
	if (status.connected) {
		std::cout << " Connected.";
	} else {
		std::cout << " Disconnected.";
	}
	std::cout << " Total: " << status.totalNo << std::endl;
}

class MyTCPServerCallbacks : public TCPServerCallbacks
{
public:
	ops::ConnectStatus cst;
	char buffer[1024];
	BytesSizePair bsp;

	MyTCPServerCallbacks() : cst(false, 0), bsp(nullptr, 0) {}

	// Called from server when a new connection is accepted
	// Could be used to call conn->asynchRead(buffer, size)
	void onConnect(TCPConnection& conn, ConnectStatus status) override
	{
		Log("Server", status);
		cst = status;
		
		conn.setProtocol(new ops::TCPOpsProtocol(ops::ops_clock::now, 1000, 3000));
		
		// Start a read
		conn.asynchWait(buffer, 1024);
	}

	// Called from server when data has been filled into given buffer
	// A new call to conn->asynchRead(buffer, size) need to be done to continue to read
	void onEvent(TCPConnection& , BytesSizePair arg) override
	{
		bsp = arg;
	}

	// Called from server when a connection has been deleted
	// NOTE: 'conn' is invalid and is only provided as an ID.
	// Ev. buffer used in asynchRead() is no longer in use
	void onDisconnect(TCPConnection& , ConnectStatus status) override
	{
		Log("Server", status);
		cst = status;
	}
};

class MyTCPClientCallbacks : public TCPClientCallbacks
{
public:
	ops::ConnectStatus cst;
	BytesSizePair bsp;
	bool protSet = false;

	MyTCPClientCallbacks() : cst(false, 0), bsp(nullptr, 0) {}

	// Called from client when a connection is made
	void onConnect(TCPConnection& conn, ConnectStatus status) override
	{
		if (!protSet) conn.setProtocol(new ops::TCPOpsProtocol(ops::ops_clock::now, 1000, 3000));
		protSet = true;
		Log("Client", status);
		cst = status;
	}

	// Called from client when a connection is closed
	// Ev. buffer used in asynchRead() is no longer in use
	void onDisconnect(TCPConnection& , ConnectStatus status) override
	{
		Log("Client", status);
		cst = status;
	}
};

// ===============================

TEST(Test_Sockets, TestTCPdefault) {

	// Create an IoService
	RAII_ioServ ioServ;
	ASSERT_NE(ioServ(), nullptr);

	// Create a listener for received data
	MyBSPListener listener(false);
	MyTCPServerCallbacks ServerCB;
	MyTCPClientCallbacks ClientCB;

	static const int serverPort = 9999;

	// Create TCP Server with default buffer size. Add listener for connect status
	TCPServer snd(&ServerCB, ioServ(), "127.0.0.1", serverPort);

	EXPECT_STREQ(snd.getLocalAddress().c_str(), "127.0.0.1");
	EXPECT_EQ(snd.getLocalPort(), serverPort);
	EXPECT_EQ(snd.numConnected(), 0);

	// Create TCP Client with default buffersize
	TCPClient rcv(&ClientCB, "127.0.0.1", serverPort, ioServ());
	rcv.Notifier<BytesSizePair>::addListener(&listener);

	EXPECT_FALSE(rcv.isConnected());
	EXPECT_EQ(snd.numConnected(), 0);

	// Start server and client so they connect
	{
		// Need someone to drive the io service while we connect
		std::thread t1(WaitConnectedWTimeout, std::ref(rcv), std::ref(snd), std::ref(ioServ), 40000);

		// Start server
		snd.open();

		// Start client
		rcv.start();

		t1.join();
	}

	// Check connected status
	EXPECT_TRUE(rcv.isConnected());
	ASSERT_EQ(snd.numConnected(), 1);

	// Check ConnectStatus from server
	EXPECT_TRUE(ServerCB.cst.connected);
	EXPECT_STREQ(ServerCB.cst.addr.c_str(), rcv.getLocalAddress().c_str());
	EXPECT_EQ(ServerCB.cst.port, rcv.getLocalPort());
	EXPECT_EQ(ServerCB.cst.totalNo, 1);

	// Check ConnectStatus from client
	EXPECT_TRUE(ClientCB.cst.connected);
	EXPECT_STREQ(ClientCB.cst.addr.c_str(), snd.getLocalAddress().c_str());
	EXPECT_EQ(ClientCB.cst.port, snd.getLocalPort());
	EXPECT_EQ(ClientCB.cst.totalNo, 1);

	EXPECT_STREQ(rcv.getLocalAddress().c_str(), "127.0.0.1");

	// Save connected client for later use
	Address_T connectedIP = ServerCB.cst.addr;
	int connectPort = ServerCB.cst.port;

	// Start an async wait for data
	char rcvbuf[1024];
	rcv.asynchWait(&rcvbuf[0], 1000);

	// Send data
	char sndbuf[1024];
	EXPECT_TRUE(snd.sendTo(&sndbuf[0], 100, "", 0));

	// Wait for data
	WaitWTimeout(rcv, ioServ, 500);
	EXPECT_TRUE(rcv.isConnected());

	EXPECT_EQ(listener.counter, 1);
	EXPECT_EQ(listener.bsp.size, 100);
	EXPECT_EQ(listener.bsp.bytes, &rcvbuf[0]);
	EXPECT_STREQ(listener.srcIp.c_str(), snd.getLocalAddress().c_str());
	EXPECT_EQ(listener.srcPort, snd.getLocalPort());

	// Test send other way, Async recv is started in serverCB.onConnect()
	EXPECT_TRUE(rcv.sendTo(&sndbuf[0], 100));
	
	PollWTimeout(ioServ, 100);

	EXPECT_EQ(ServerCB.bsp.bytes, ServerCB.buffer);
	EXPECT_EQ(ServerCB.bsp.size, 100);

	// Disconnect receiver. Need to send data to discover disconnect
	rcv.stop();
	EXPECT_TRUE(snd.sendTo(&sndbuf[0], 10, "", 0));
	PollWTimeout(ioServ, 100);

	EXPECT_TRUE(snd.sendTo(&sndbuf[10], 90, "", 0));
	PollWTimeout(ioServ, 200);

	EXPECT_FALSE(ServerCB.cst.connected);
	EXPECT_STREQ(ServerCB.cst.addr.c_str(), connectedIP.c_str());
	EXPECT_EQ(ServerCB.cst.port, connectPort);
	EXPECT_EQ(ServerCB.cst.totalNo, 0);

	// Need someone to drive the io service when we call stop() otherwise it will be a deadlock
	{
		std::thread t1(PollWTimeout, std::ref(ioServ), 500);
		rcv.stop();
		snd.close();
		t1.join();
	}
}

TEST(Test_Sockets, TestTCPdelete) {

	// Create an IoService
	RAII_ioServ ioServ;
	ASSERT_NE(ioServ(), nullptr);

	// Create a listener for received data
	MyBSPListener listener(false);
	MyTCPServerCallbacks ServerCB;
	MyTCPClientCallbacks ClientCB;

	static const int serverPort = 9999;
	char rcvbuf[1024];

	std::thread t2;
	{
		// Create TCP Server with default buffer size. Add listener for connect status
		TCPServer snd(&ServerCB, ioServ(), "127.0.0.1", serverPort);

		// Create TCP Client with default buffersize
		TCPClient rcv(&ClientCB, "127.0.0.1", serverPort, ioServ());
		rcv.Notifier<BytesSizePair>::addListener(&listener);

		EXPECT_FALSE(rcv.isConnected());
		EXPECT_EQ(snd.numConnected(), 0);

		// Start server and client so they connect
		{
			// Need someone to drive the io service while we connect
			std::thread t1(WaitConnectedWTimeout, std::ref(rcv), std::ref(snd), std::ref(ioServ), 40000);

			// Start server
			snd.open();

			// Start client
			rcv.start();

			t1.join();
		}

		// Check connected status
		EXPECT_TRUE(rcv.isConnected());
		ASSERT_EQ(snd.numConnected(), 1);

		// Start an async wait for data
		rcv.asynchWait(&rcvbuf[0], 1000);

		// Need someone to drive the io service to finish all asynch callbacks e.g. created @ scope exit
		t2 = std::thread(PollWTimeout, std::ref(ioServ), 500);
	}
	t2.join();

	{
		// Need someone to drive the io service to handle all asynch callbacks
		std::thread t1(PollWTimeout, std::ref(ioServ), 500);

		{
			// Create TCP Server with default buffer size. Add listener for connect status
			TCPServer snd(&ServerCB, ioServ(), "127.0.0.1", serverPort);
			snd.open();
			snd.close();
			snd.open();
			snd.open();
			snd.open();
			snd.close();
			snd.open();
		}

		t1.join();
	}
}
