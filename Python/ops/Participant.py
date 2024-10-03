# Participant

from socket import gethostname
from os import getpid
from threading import Lock

try:
	import thread
except ImportError:
	import _thread as thread	# module renamed in Python3

from time import sleep

from ops.opsTypes import Config,Topic
from ops.Factory import OpsDefaultFactory
import ops.SendDataHandler
import ops.ReceiveDataHandler
import ops.ParticipantInfoData
import ops.Subscriber
import ops.Publisher

class Participant(object):
	instances = {}
	@staticmethod
	def __createName(domainID, participantID):
		return domainID+"_"+participantID

	@staticmethod
	def getInstance(domainID, participantID="DEFAULT_PARTICIPANT", configFile=None):
		name = Participant.__createName(domainID, participantID)
		if not name in Participant.instances:
			part = Participant(domainID, participantID, configFile)
			if part.domain is None:
				return None
			Participant.instances[name] = part
		return Participant.instances[name]

	def __init__(self,domainID, participantID, configFile):
		super(Participant, self).__init__()
		self.domainID = domainID
		self.participantID = participantID

		if configFile is None:
			self.config = Config.getConfig()
		else:
			self.config = Config.getConfig(configFile)
		self.domain = self.config.getDomain(domainID)
		self.objectFactory = OpsDefaultFactory()

		self.shouldRun = False
		self.tcprdhs = {}
		self.udpsdhs = {}

		self.partInfoData = ops.ParticipantInfoData.ParticipantInfoData()

		self.partInfoData.name = gethostname() + "(" + str(getpid()) + ")"
		self.partInfoData.languageImplementation = "Python"
		self.partInfoData.id = participantID
		self.partInfoData.domain = domainID
		self.partInfoLock = Lock()

	def __str__(self):
		return "Participant\nConfig:\n" + self.config.__str__() + "\nDomain:\n" + self.domain.__str__()

	def createTopic(self,name):
		topic = self.domain.getTopic(name)
		topic.participantID = self.participantID;
		topic.domainID = self.domainID;
		topic.participant = self;
		return topic;

	def addTypeSupport(self,typeSupport):
		self.objectFactory.addFactory(typeSupport)

	def setUdpTransportInfo(self,ip,port):
		with self.partInfoLock:
			self.partInfoData.ip = ip
			self.partInfoData.mc_udp_port = port

	def connectUdp(self,topic,sdh):
		# Should only be one
		if not topic.name in self.udpsdhs:
			self.udpsdhs[topic.name] = (sdh, 1)
		else:
			print("Participant: Topic already in udpsdhs")

	def disconnectUdp(self,topic,sdh):
		if topic.name in self.udpsdhs:
			handler = self.udpsdhs[topic.name][0]
			if handler == sdh:
				self.udpsdhs.pop(topic.name)
			else:
				print("Participant: Wrong sdh in udpsdhs")
		else:
			print("Participant: Topic NOT in udpsdhs")

	def getSendDataHandler(self,topic):
		self.start()
		sdh = ops.SendDataHandler.getSendDataHandler(self,topic)
		# For tcp we may need to give the specific address bound to the TCP server
		addr = None
		if topic.transport == ops.Constants.TRANSPORT_TCP and topic.port == 0:
			addr = sdh.localAddress()
			##addr[0] is address, will be 0.0.0.0
			##addr[1] is port
			adr = topic.domainAddress
			if not ops.Support.isValidNodeAddress(adr):
				adr = ops.Support.doSubnetTranslation(topic.localInterface)
			addr = (adr, addr[1])
		with self.partInfoLock:
			self.partInfoData.publishTopics.append(ops.ParticipantInfoData.TopicInfoData(topic,addr))
		return sdh

	def releaseSendDataHandler(self,topic):
		with self.partInfoLock:
			for i, ti in enumerate(self.partInfoData.publishTopics):
				if ti.name == topic.name:
					del self.partInfoData.publishTopics[i]
					break
		ops.SendDataHandler.releaseSendDataHandler(self,topic)

	def getReceiveDataHandler(self,topic):
		self.start()
		rdh = ops.ReceiveDataHandler.getReceiveDataHandler(self,topic)
		with self.partInfoLock:
			self.partInfoData.subscribeTopics.append(ops.ParticipantInfoData.TopicInfoData(topic))
		if topic.transport == ops.Constants.TRANSPORT_TCP:
			# Need to count topic instances
			if topic.name in self.tcprdhs:
				self.tcprdhs[topic.name] = (rdh, self.tcprdhs[topic.name][1]+1)
			else:
				self.tcprdhs[topic.name] = (rdh, 1)
		return rdh

	def releaseReceiveDataHandler(self,topic):
		with self.partInfoLock:
			for i, ti in enumerate(self.partInfoData.subscribeTopics):
				if ti.name == topic.name:
					del self.partInfoData.subscribeTopics[i]
					break
		if topic.transport == ops.Constants.TRANSPORT_TCP:
			# Need to count topic instances
			if topic.name in self.tcprdhs:
				self.tcprdhs[topic.name] = (self.tcprdhs[topic.name][0], self.tcprdhs[topic.name][1]-1)
				if self.tcprdhs[topic.name][1] == 0:
					self.tcprdhs.pop(topic.name)
		ops.ReceiveDataHandler.releaseReceiveDataHandler(self,topic)

	def createParticipantInfoTopic(self):
		topic = Topic()
		topic.domainAddress = self.domain.domainAddress
		topic.localInterface = self.domain.localInterface;
		topic.timeToLive = self.domain.timeToLive;
		topic.participantID = self.participantID;
		topic.domainID = self.domainID;
		topic.participant = self;
		topic.port = self.domain.metaDataMcPort
		topic.name = "ops.bit.ParticipantInfoTopic"
		topic.typeID = "ops.ParticipantInfoData"
		return topic

	def createDebugTopic(self):
		topic = Topic()
		topic.domainAddress = self.domain.domainAddress
		topic.localInterface = self.domain.localInterface;
		topic.timeToLive = self.domain.timeToLive;
		topic.participantID = self.participantID;
		topic.domainID = self.domainID;
		topic.participant = self;
		topic.port = self.domain.debugMcPort
		topic.name = "ops.DebugTopic"
		topic.typeID = "opsidls.DebugRequestResponseData"
		return topic

	def run(self):
		partInfoSub = None
		partInfoPub = None
		while self.shouldRun:
			sleep(1)
			# Create subscriber
			if partInfoSub is None and self.domain.metaDataMcPort > 0:
				partInfoSub = ops.Subscriber.Subscriber(self.createParticipantInfoTopic())
				partInfoSub.addCallback(self.onParticipantInfoData)
				partInfoSub.start()
			# create publisher
			if partInfoPub is None and self.domain.metaDataMcPort > 0:
				partInfoPub = ops.Publisher.Publisher(self.createParticipantInfoTopic())

			# periodic publish of our ParticipantInfoData
			if partInfoPub is not None:
				with self.partInfoLock:
					partInfoPub.write(self.partInfoData)

		if partInfoSub is not None:
			partInfoSub.stop()
			partInfoSub = None
		if partInfoPub is not None:
			partInfoPub.stop()
			partInfoPub = None

	def start(self):
		if not self.shouldRun and self.domain.metaDataMcPort > 0:
			self.shouldRun = True
			thread.start_new_thread( self.run, () )

	def stop(self):
		if self.shouldRun:
			self.shouldRun = False

	def hasPublisherOn(self,topicname):
		with self.partInfoLock:
			for pubtop in self.partInfoData.publishTopics:
				if pubtop.name == topicname:
					return True
		return False

	def hasSubscriberOn(self,topicname):
		with self.partInfoLock:
			for subtop in self.partInfoData.subscribeTopics:
				if subtop.name == topicname:
					return True
		return False

	def onParticipantInfoData(self,sub,message):
		data=message.data

		#addr,port = map(str,message.getSource())
		#tempStr = "(From " + addr + ":" + port
		#tempStr +=") PartInfoData: Name: " + data.name
		#tempStr +=",  languageImplementation: " + data.languageImplementation
		#print(tempStr)

		if data.mc_udp_port != 0:
			for subtop in data.subscribeTopics:
				if subtop.transport == ops.Constants.TRANSPORT_UDP:
					if self.hasPublisherOn( subtop.name ):
						if subtop.name in self.udpsdhs:
							sdh = self.udpsdhs[subtop.name][0]
							sdh.addSink(subtop.name, (data.ip, data.mc_udp_port), False)
							#sdh.printSinks()

		for pubtop in data.publishTopics:
			if pubtop.transport == ops.Constants.TRANSPORT_TCP:
				if self.hasSubscriberOn( pubtop.name ):
					if pubtop.name in self.tcprdhs:
						self.tcprdhs[pubtop.name][0].addChannel( pubtop.address, pubtop.port )
