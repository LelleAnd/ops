# SendDataHandler

import struct
import socket

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


class UdpSendDataHandler(AbstractSendDataHandler):
	def __init__(self,topic):
		super(UdpSendDataHandler,self).__init__()
		self.sendAddress = (topic.domainAddress, topic.port)
		self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
		if topic.outSocketBufferSize > 0:
			self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, topic.outSocketBufferSize)

	def localAddress(self):
		return self.socket.getsockname()

	def sendData(self,block,topic):
		if self.socket is None:
			return
		self.socket.sendto(block,self.sendAddress)


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


def __makeKey(topic):
	if topic.transport == TRANSPORT_TCP and topic.port == 0:
		return topic.domainID + "::" + topic.channelID + "::" + topic.transport + "::" + topic.domainAddress + "::" + str(topic.port)
	else:
		return topic.transport + "::" + topic.domainAddress + "::" + str(topic.port)

__SendDataHandlerList = {}

def getSendDataHandler(participant,topic):
	key = __makeKey(topic)

	sdh = None
	if key in __SendDataHandlerList:
		sdh = __SendDataHandlerList[key]

	else:
		localInterface = ops.Support.doSubnetTranslation(topic.localInterface)
		if topic.transport == TRANSPORT_MC:
			sdh = McSendDataHandler(localInterface,topic,topic.timeToLive)
		elif topic.transport == TRANSPORT_UDP:
			sdh = UdpSendDataHandler(topic)
		elif topic.transport == TRANSPORT_TCP:
			sdh = TcpSendDataHandler(localInterface,topic)

		__SendDataHandlerList[key] = sdh
	return sdh

def releaseSendDataHandler(participant,topic):
	key = __makeKey(topic)

	sdh = None
	if key in __SendDataHandlerList:
		sdh = __SendDataHandlerList[key]

		if len(sdh.publishers) == 0:
			sdh.close()
			__SendDataHandlerList.pop(key)
