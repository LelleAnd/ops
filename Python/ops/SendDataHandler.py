# SendDataHandler

import struct
import socket
import time
from threading import Lock

try:
	import thread
except ImportError:
	import _thread as thread	# module renamed in Python3

import ops.Support
from ops.Constants import *

class AbstractSendDataHandler(object):
	def __init__(self):
		super(AbstractSendDataHandler,self).__init__()
		self.publishers = set()

	def addPublisher(self,client):
		self.publishers.add(client)
		if len(self.publishers)==1:
			self.open()

	def removePublisher(self,client):
		self.publishers.remove(client)
		if len(self.publishers)==0:
			self.close()

	def open(self):
		pass

	def close(self):
		pass

	def sendData(self,block,topic):
		raise NotImplementedError

	def localAddress(self):
		return (0,0)


class SinkData(object):
	ALIVE_TIMEOUT = 3.0

	def __init__(self,sendAddr,alwaysAlive):
		super(SinkData,self).__init__()
		self.sendAddr = sendAddr		# (ip, port)
		self.alwaysAlive = alwaysAlive
		self.lastTime = time.time()

	def feedWatchdog(self,alwaysAlive):
		self.alwaysAlive = self.alwaysAlive or alwaysAlive
		self.lastTime = time.time()

	def isAlive(self):
		return self.alwaysAlive or ((time.time() - self.lastTime) < self.ALIVE_TIMEOUT)

	def toString(self):
		return "Addr:" + str(self.sendAddr) + ", Alive:" + str(self.isAlive()) + ", Time:" + str(self.lastTime)

class UdpSendDataHandler(AbstractSendDataHandler):
	def __init__(self,outSocketBufferSize):
		super(UdpSendDataHandler,self).__init__()
		self.sinkLock = Lock()
		self.sinkMap = {}
		#self.sendAddress = (topic.domainAddress, topic.port)
		self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
		if outSocketBufferSize > 0:
			self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, outSocketBufferSize)

	def localAddress(self):
		return self.socket.getsockname()

	def sendData(self,block,topic):
		if self.socket is None:
			return
		#self.socket.sendto(block,self.sendAddress)
		with self.sinkLock:
			# find topic map
			if topic.name in self.sinkMap:
				staticR, sinks = self.sinkMap[topic.name]
				for k, sink in list(sinks.items()):
					if sink.isAlive():
						self.socket.sendto(block,sink.sendAddr)
					else:
						del sinks[k]

	def addSink(self,topicName,dest,staticRoute=False):
		# topicName string, dest set with (ip,port)
		key = dest[0] + "::" + str(dest[1])
		with self.sinkLock:
			# find topic map
			if topicName not in self.sinkMap:
				# have no sinks for this topic, add sink
				di = {}
				di[key]=SinkData(dest, staticRoute)
				self.sinkMap[topicName] = (staticRoute, di)
			else:
				staticR, sinks = self.sinkMap[topicName]
				# if created as static route, only add sinks that are static
				if (not staticR) or (staticR and staticRoute):
					if key not in sinks:
						sinks[key] = SinkData(dest, staticRoute)
					else:
						sinks[key].feedWatchdog(staticRoute)

	def printSinks(self):
		with self.sinkLock:
			for tname, v in self.sinkMap.items():
				st = v[0]
				sinks = v[1]
				for k, sink in sinks.items():
					print('Topic:{}, Static:{}, Key:{}, {}'.format(tname, st, k, sink.toString()))


class TcpSendDataHandler(AbstractSendDataHandler):
	def __init__(self,localInterface,topic):
		super(TcpSendDataHandler,self).__init__()
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
		self.sock.bind(('', topic.port))
		self.sock.listen(1)
		self.conns = set()
		self.shouldRun = False

	def localAddress(self):
		return self.sock.getsockname()

	def sendData(self,block,topic):
		data = b'opsp_tcp_size_info' + struct.pack("<I", len(block)) + block
		# Make a copy of the list so we can remove failing items in the original
		cs = self.conns.copy()
		for conn in cs:
			try:
				conn.send(data)
			except socket.error:
				self.conns.remove(conn)

	def run(self):
		self.sock.settimeout(1)		# needed to be able to terminate thread
		while self.shouldRun:
			try:
				conn, addr = self.sock.accept()
				conn.settimeout(None)
				self.conns.add(conn)
			except socket.timeout:
				pass
			except socket.error:
				pass

		for conn in self.conns:
			conn.close()
		self.conns.clear()

	def open(self):
		if not self.shouldRun:
			self.shouldRun = True
			thread.start_new_thread( self.run, () )

	def close(self):
		if self.shouldRun:
			self.shouldRun = False
			self.sock.close()

class McSendDataHandler(AbstractSendDataHandler):
	def __init__(self,localInterface,topic,ttl):
		super(McSendDataHandler,self).__init__()
		self.sendAddress = (topic.domainAddress, topic.port)
		self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
		self.socket.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)
		self.socket.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_IF, socket.inet_aton(localInterface))
		if topic.outSocketBufferSize > 0:
			self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, topic.outSocketBufferSize)

	def localAddress(self):
		return self.socket.getsockname()

	def sendData(self,block,topic):
		if self.socket is None:
			return
		self.socket.sendto(block,self.sendAddress)


def __makeKey(topic, localIf):
	if topic.transport == TRANSPORT_UDP:
		return topic.transport + "::" + localIf + "::" + topic.domainAddress
	if topic.transport == TRANSPORT_TCP and topic.port == 0:
		return topic.transport + "::" + topic.channelID + "::" + topic.domainAddress + "::" + str(topic.port)
	return topic.transport + "::" + topic.domainAddress + "::" + str(topic.port)

__SendDataHandlerList = {}

def getSendDataHandler(participant,topic):
	localInterface = ops.Support.doSubnetTranslation(topic.localInterface)
	key = __makeKey(topic, localInterface)

	sdh = None
	if key in __SendDataHandlerList:
		sdh = __SendDataHandlerList[key]

	else:
		if topic.transport == TRANSPORT_MC:
			sdh = McSendDataHandler(localInterface,topic,topic.timeToLive)

		elif topic.transport == TRANSPORT_UDP:
			# We have only one sender for all topics on the same interface, so use the domain value for buffer size
			sdh = UdpSendDataHandler(participant.domain.outSocketBufferSize)
			if ops.Support.isValidNodeAddress(topic.domainAddress):
				sdh.addSink(topic.name, (topic.domainAddress, topic.port), True)
			else:
				# Setup a listener on the participant info data published by participants on our domain.
				# We use the information for topics with UDP as transport, to know the destination for UDP sends
				# ie. we extract ip and port from the information and add it to our UdpSendDataHandler
				# Note: need to call connectUdp/disconnectUdp equal number of times
				participant.connectUdp(topic, sdh)

			#sdh.printSinks()

		elif topic.transport == TRANSPORT_TCP:
			sdh = TcpSendDataHandler(localInterface,topic)

		__SendDataHandlerList[key] = sdh
	return sdh

def releaseSendDataHandler(participant,topic):
	localInterface = ops.Support.doSubnetTranslation(topic.localInterface)
	key = __makeKey(topic, localInterface)

	sdh = None
	if key in __SendDataHandlerList:
		sdh = __SendDataHandlerList[key]

		if topic.transport == TRANSPORT_UDP:
			if not ops.Support.isValidNodeAddress(topic.domainAddress):
				participant.disconnectUdp(topic, sdh)

		if len(sdh.publishers) == 0:
			sdh.close()
			__SendDataHandlerList.pop(key)
