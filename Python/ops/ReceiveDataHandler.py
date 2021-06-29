# ReceiveDataHandler

# Each ReceiveDataChannel (Mc / Tcp / Udp) has a thread that reads from a socket

# The TcpReceiveDataHandler can have multiple TcpReceiveDataChannels

import struct
import socket
from threading import Lock

try:
	import thread
except ImportError:
	import _thread as thread	# module renamed in Python3

import ops.Support
from ops.Constants import *
import ops.DataAssembly

class AbstractReceiveDataChannel(object):
	def __init__(self,topic,owner):
		super(AbstractReceiveDataChannel,self).__init__()
		self.topic = topic
		self.owner = owner
		self.assembler = None

	## called from the thread doing the actual reading from the socket
	def segmentReceived(self,data,addr):
		segment = ops.DataAssembly.Segment(data)
		if segment.isValid():
			if (self.assembler==None):
				self.assembler = ops.DataAssembly.Assembler()

			if (self.assembler.addSegment(segment)==False):
				self.assembler = ops.DataAssembly.Assembler()
			else:
				if (self.assembler.isFull()):
					obj = self.assembler.createOPS(self.topic.participant.objectFactory)
					obj.setSource(addr)
					self.owner.distributeMessage(obj)
					self.assembler = None
		else:
			print("Not a valid segment")

	def run(self):
		raise NotImplementedError
	def start(self):
		self.shouldRun = True
		thread.start_new_thread( self.run, () )
	def end(self):
		self.shouldRun = False

class AbstractReceiveDataHandler(object):
	def __init__(self,topic):
		super(AbstractReceiveDataHandler,self).__init__()
		self.topic = topic
		self.started = False
		self.channels = set()
		# subscribers need to be protected. Used from user-land and from internal threads
		self.subscribers = set()
		self.subscribersLock = Lock()

	def addSubscriber(self,subs):
		with self.subscribersLock:
			self.subscribers.add(subs)

	def removeSubscriber(self,subs):
		with self.subscribersLock:
			self.subscribers.remove(subs)

	# called from the thread(s) doing the actual reading from socket(s),
	# for tcp it can be several
	def distributeMessage(self,obj):
		if obj is not None:
			with self.subscribersLock:
				for subs in self.subscribers:
					subs.newMessage(obj)

	def start(self):
		self.started = True
		for chan in self.channels:
			chan.start()

	def end(self):
		self.started = False
		for chan in self.channels:
			chan.end()

class UdpReceiveDataChannel(AbstractReceiveDataChannel):
	def __init__(self,topic,owner):
		super(UdpReceiveDataChannel,self).__init__(topic,owner)

	def run(self):
		sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
		sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
		sock.bind(('',self.topic.port))

		if self.topic.inSocketBufferSize > 0:
			sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, self.topic.inSocketBufferSize)

		while self.shouldRun:
			try:
				data, addr = sock.recvfrom(PACKET_MAX_SIZE);
				self.segmentReceived(data,addr)
			except socket.timeout:
				pass

		sock.close()

class UdpReceiveDataHandler(AbstractReceiveDataHandler):
	def __init__(self,localInterface,topic):
		super(UdpReceiveDataHandler,self).__init__(topic)
		self.channels.add(UdpReceiveDataChannel(topic,self))

def recvall(sock, n):
	# Helper function to recv n bytes or return None if EOF is hit
	data = b''
	try:
		while len(data) < n:
			fragment = sock.recv(n - len(data))
			if not fragment:
				break
			data += fragment
	except socket.error:
		pass
	return data

class TcpReceiveDataChannel(AbstractReceiveDataChannel):
	def __init__(self,topic,owner):
		super(TcpReceiveDataChannel,self).__init__(topic,owner)
		self.addr = (topic.domainAddress, topic.port)
		self.inbuffersize = topic.inSocketBufferSize
		self.connected = False

	def run(self):
		while self.shouldRun:
			self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			self.sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
			if self.inbuffersize > 0:
				self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, self.inbuffersize)

			while not self.connected and self.shouldRun:
				try:
					self.sock.connect(self.addr)
				except socket.error:
					pass
				else:
					self.connected = True

			while self.connected and self.shouldRun:
				data = ''
				size = 0
				try:
					data = recvall(self.sock, 18)
				except socket.timeout:
					pass

				if data == b'opsp_tcp_size_info':
					try:
						data = recvall(self.sock, 4)
					except socket.timeout:
						pass

					if len(data) == 4:
						size = struct.unpack("<I", data)[0]
						if size > 0 and size <= PACKET_MAX_SIZE:
							try:
								data = recvall(self.sock, size)
							except socket.timeout:
								pass
						else:
							self.connected = False

						if len(data) == size and size > 0:
							self.segmentReceived(data, self.addr)
							data = ''
						else:
							self.connected = False
					else:
						self.connected = False
				else:
					self.connected = False

			try:
				self.sock.shutdown(socket.SHUT_RDWR)
			except:
				pass
			self.sock.close()

class TcpReceiveDataHandler(AbstractReceiveDataHandler):
	def __init__(self,localInterface,topic):
		super(TcpReceiveDataHandler,self).__init__(topic)
		if topic.port > 0:
			self.channels.add(TcpReceiveDataChannel(topic, self))

	def addChannel(self,addr,port):
		if port == 0:
			return
		newaddr = ( addr, port)
		# Check if already added
		for chan in self.channels:
			if chan.addr == newaddr:
				return
		# reuse our topic
		self.topic.domainAddress = addr
		self.topic.port = port
		chan = TcpReceiveDataChannel(self.topic, self)
		if self.started == True:
			chan.start()
		self.channels.add(chan)

class McReceiveDataChannel(AbstractReceiveDataChannel):
	def __init__(self,topic,localInterface,owner):
		super(McReceiveDataChannel,self).__init__(topic,owner)
		self.localInterface = localInterface

	def run(self):
		sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
		sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
		sock.bind(('',self.topic.port))

		group = socket.inet_aton(self.topic.domainAddress)
		iface = socket.inet_aton(self.localInterface)

		sock.setsockopt(socket.IPPROTO_IP,socket.IP_ADD_MEMBERSHIP,group + iface)
		if self.topic.inSocketBufferSize > 0:
			sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, self.topic.inSocketBufferSize)

		while self.shouldRun:
			try:
				data, addr = sock.recvfrom(PACKET_MAX_SIZE);
				self.segmentReceived(data,addr)
			except socket.timeout:
				pass

		sock.close()

class McReceiveDataHandler(AbstractReceiveDataHandler):
	def __init__(self,localInterface,topic):
		super(McReceiveDataHandler,self).__init__(topic)
		self.channels.add(McReceiveDataChannel(topic,localInterface,self))

def __makeKey(topic):
	if topic.transport == TRANSPORT_TCP and topic.port == 0:
		return topic.domainID + "::" + topic.channelID + "::" + topic.transport + "::" + topic.domainAddress + "::" + str(topic.port)
	else:
		return topic.transport + "::" + topic.domainAddress + "::" + str(topic.port)

__ReceiveDataHandlerList = {}

def getReceiveDataHandler(participant,topic):
	key = __makeKey(topic)

	rdh = None
	if key in __ReceiveDataHandlerList:
		rdh = __ReceiveDataHandlerList[key]
		if max(rdh.topic.sampleMaxSize,topic.sampleMaxSize) > PACKET_MAX_SIZE:
			message = "Warning: "
			if topic.transport == TRANSPORT_UDP:
				message += "UDP Transport"
			elif topic.transport == TRANSPORT_TCP:
				message += "TCP Transport"
			else:
				message += "Same port (%s)" % topic.port
			message += " is used with Topics with 'sampleMaxSize' > %s" % PACKET_MAX_SIZE
			print(message)
	else:
		localInterface = ops.Support.doSubnetTranslation(topic.localInterface)
		if topic.transport == TRANSPORT_MC:
			rdh = McReceiveDataHandler(localInterface,topic)
		elif topic.transport == TRANSPORT_UDP:
			rdh = UdpReceiveDataHandler(localInterface,topic)
		elif topic.transport == TRANSPORT_TCP:
			rdh = TcpReceiveDataHandler(localInterface,topic)
		rdh.start()
		__ReceiveDataHandlerList[key] = rdh
	return rdh

def releaseReceiveDataHandler(participant,topic):
	key = __makeKey(topic)

	rdh = None
	if key in __ReceiveDataHandlerList:
		rdh = __ReceiveDataHandlerList[key]

		if len(rdh.subscribers) == 0:
			rdh.end()
			__ReceiveDataHandlerList.pop(key)
