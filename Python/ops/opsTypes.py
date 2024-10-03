#import Factory
#from XML_Archiver import XML_Archiver_In
from ops.Constants import *
import ops.Support
import copy

class ConfigError(Exception):
	def __init__(self, value):
		self.value = value
	def __str__(self):
		return repr(self.value)

class IdlVersionError(Exception):
	def __init__(self, msg, gotVer, maxVer):
		self.value = "Error: " + msg + ": received version '" + str(gotVer) + "' > known version '" + str(maxVer) + "'"
	def __str__(self):
		return repr(self.value)

class OPS_Object(object):
	"""docstring for OPS_Object"""
	OPSOBJECT_IDLVERSION = 0

	def __init__(self,key=""):
		super(OPS_Object, self).__init__()
		self.idlVersionMask = 0
		self.OPSObject_version = OPS_Object.OPSOBJECT_IDLVERSION
		self.key = key
		self.typesString = ""
		self.spareBytes = None

	def __str__(self):
		return "OPS_Object:\n  key=%s\n  typesString=%s" % (self.key,self.typesString)

	def serialize(self,dataBuffer):
		if self.idlVersionMask != 0:
			self.OPSObject_version = dataBuffer.Int8("OPSObject_version",self.OPSObject_version)
			if self.OPSObject_version > OPS_Object.OPSOBJECT_IDLVERSION:
				raise IdlVersionError("OPS_Object", self.OPSObject_version, OPS_Object.OPSOBJECT_IDLVERSION)
			else:
				pass
		else:
			self.OPSObject_version = 0
		self.key = dataBuffer.String("key",self.key)

	def validate(self):
		if not isinstance(self.key,str):
			raise ValueError()

	def appendType(self,str):
		self.typesString = str + " " + self.typesString

class Message(OPS_Object):
	"""docstring for Message"""
	OPSMESSAGE_IDLVERSION = 0

	def __init__(self,data=None):
		super(Message,self).__init__()
		self.appendType("ops.protocol.OPSMessage")

		self.OPSMessage_version = Message.OPSMESSAGE_IDLVERSION
		self.messageType=0
		self.publisherPriority=0
		self.publicationID=0
		self.publisherName=""
		self.topicName=""
		self.topLevelKey=""
		self.address=""
		self.data=data
		self.sourceAddr=("",0)

	def __str__(self):
		temp = ""
		if self.printable:
			temp = super(OPS_Message,self).__str__() + "\nOPS_Message:\n  messageType=%s\n  publisherPriority=%s\n  publicationID=%s\n  publisherName=%s\n  topicName=%s\n  topLevelKey=%s\n  address=%s\n" % (self.messageType,self.publisherPriority,self.publicationID,self.publisherName,self.topicName,self.topLevelKey,self.address)

		if self.data is not None:
			return temp + self.data.__str__()
		else:
			return temp

	def serialize(self,dataBuffer):
		super(Message,self).serialize(dataBuffer)
		if self.idlVersionMask != 0:
			self.OPSMessage_version = dataBuffer.Int8("OPSMessage_version",self.OPSMessage_version)
			if self.OPSObject_version > Message.OPSOBJECT_IDLVERSION:
				raise IdlVersionError("Message", self.OPSMessage_version, Message.OPSMESSAGE_IDLVERSION)
			else:
				pass
		else:
			self.OPSMessage_version = 0
		self.messageType = dataBuffer.Int8("messageType",self.messageType)
		self.publisherPriority = dataBuffer.Int8("publisherPriority",self.publisherPriority)
		self.publicationID = dataBuffer.Int64("publicationID",self.publicationID)
		self.publisherName = dataBuffer.String("publisherName",self.publisherName)
		self.topicName = dataBuffer.String("topicName",self.topicName)
		self.topLevelKey = dataBuffer.String("topLevelKey",self.topLevelKey)
		self.address = dataBuffer.String("address",self.address)
		self.data = dataBuffer.Ops("data",self.data)

	def validate(self):
		super(Message,self).validate()
		if not isinstance(self.messageType,int):
			raise ValueError()
		if not isinstance(self.publisherPriority,int):
			raise ValueError()
		if not isinstance(self.publicationID,(int,long)):
			raise ValueError()
		if not isinstance(self.publisherName,str):
			raise ValueError()
		if not isinstance(self.topicName,str):
			raise ValueError()
		if not isinstance(self.topLevelKey,str):
			raise ValueError()
		if not isinstance(self.address,str):
			raise ValueError()
		if not isinstance(self.data,OPS_Object):
			raise ValueError()
		self.data.validate()

	def setSource(self,addr):
		self.sourceAddr = addr

	def getSource(self):
		return self.sourceAddr


class Topic(OPS_Object):
	"""docstring for Topic"""
	TOPIC_IDLVERSION = 0

	def __init__(self):
		super(Topic,self).__init__()
		self.Topic_version = Topic.TOPIC_IDLVERSION
		self.appendType("Topic");
		self.name=""
		self.typeID=""
		self.port=0
		self.domainAddress=""
		self.outSocketBufferSize=-1
		self.inSocketBufferSize=-1
		self.sampleMaxSize=PACKET_MAX_SIZE
		self.transport=TRANSPORT_MC
		self.participant=None
		self.timeToLive=-1
		self.localInterface=""
		self.optNonVirt=False
		self.channelID=""

	def __str__(self):
		temp ="\nTopic:"
		temp += "\n  name: %s" % self.name
		temp += "\n  typeID: %s" % self.typeID
		temp += "\n  port: %s" % self.port
		temp += "\n  domainAddress: %s" % self.domainAddress
		temp += "\n  outSocketBufferSize: %s" % self.outSocketBufferSize
		temp += "\n  inSocketBufferSize: %s" % self.inSocketBufferSize
		temp += "\n  sampleMaxSize: %s" % self.sampleMaxSize
		temp += "\n  transport: %s" % self.transport
		temp += "\n  timeToLive: %s" % self.timeToLive
		temp += "\n  localInterface: %s" % self.localInterface
		return  super(Topic,self).__str__() + temp

	def setSampleMaxSize(self,size):
		self.sampleMaxSize =  max(PACKET_MAX_SIZE,size)

	def serialize(self,archiver):
		super(Topic,self).serialize(archiver)
		if self.idlVersionMask != 0:
			self.Topic_version = dataBuffer.Int8("Topic_version",self.Topic_version)
			if self.Topic_version > Topic.TOPIC_IDLVERSION:
				raise IdlVersionError("Topic", self.Topic_version, Topic.TOPIC_IDLVERSION)
			else:
				pass
		else:
			self.Topic_version = 0
		self.name = archiver.String("name",self.name)
		self.typeID = archiver.String("dataType",self.typeID)
		self.port = archiver.Int32("port",self.port)
		self.domainAddress = archiver.String("address",self.domainAddress)
		self.outSocketBufferSize = archiver.Int64("outSocketBufferSize",self.outSocketBufferSize)
		self.inSocketBufferSize = archiver.Int64("inSocketBufferSize",self.inSocketBufferSize)

		temp = self.sampleMaxSize
		temp = archiver.Int32("sampleMaxSize",temp)
		self.setSampleMaxSize(temp)

		self.transport = archiver.String("transport",self.transport)
		if self.transport == "":
			self.transport = TRANSPORT_MC
		if self.transport == TRANSPORT_TCP or self.transport == TRANSPORT_UDP:
			self.domainAddress = ops.Support.getHostAddress(self.domainAddress)

	def validate(self):
		super(Topic,self).validate()
		if not isinstance(self.name,str):
			raise ValueError()
		if not isinstance(self.typeID,str):
			raise ValueError()
		if not isinstance(self.port,int):
			raise ValueError()
		if not isinstance(self.domainAddress,str):
			raise ValueError()
		if not isinstance(self.outSocketBufferSize,int):
			raise ValueError()
		if not isinstance(self.inSocketBufferSize,int):
			raise ValueError()
		if not isinstance(self.sampleMaxSize,int):
			raise ValueError()
		if not isinstance(self.transport,str):
			raise ValueError()

	def getName(self):
		return self.name


class Channel(OPS_Object):
	"""docstring for Channel"""
	CHANNEL_IDLVERSION = 0

	def __init__(self):
		super(Channel,self).__init__()
		self.Channel_version = Channel.CHANNEL_IDLVERSION
		self.appendType("Channel")
		self.channelID=""
		self.linktype=""
		self.localInterface=""
		self.domainAddress=""
		self.timeToLive=-1
		self.port=0
		self.outSocketBufferSize=-1
		self.inSocketBufferSize=-1

	def __str__(self):
		temp ="\nChannel:"
		temp += "\n  channelID: %s" % self.channelID
		temp += "\n  linktype: %s" % self.linktype
		temp += "\n  localInterface: %s" % self.localInterface
		temp += "\n  domainAddress: %s" % self.domainAddress
		temp += "\n  timeToLive: %s" % self.timeToLive
		temp += "\n  port: %s" % self.port
		temp += "\n  outSocketBufferSize: %s" % self.outSocketBufferSize
		temp += "\n  inSocketBufferSize: %s" % self.inSocketBufferSize
		return super(Channel,self).__str__() + temp

	def serialize(self,archiver):
		super(Channel,self).serialize(archiver)
		if self.idlVersionMask != 0:
			self.Channel_version = dataBuffer.Int8("Channel_version",self.Channel_version)
			if self.Channel_version > Channel.CHANNEL_IDLVERSION:
				raise IdlVersionError("Channel", self.Channel_version, Channel.CHANNEL_IDLVERSION)
			else:
				pass
		else:
			self.Channel_version = 0
		self.channelID = archiver.String("name",self.channelID)
		self.linktype = archiver.String("linktype",self.linktype)
		self.localInterface = ops.Support.getHostAddressEx(archiver.String("localInterface",self.localInterface))
		self.domainAddress = archiver.String("address",self.domainAddress)
		self.timeToLive = archiver.Int32("timeToLive",self.timeToLive)
		self.port = archiver.Int32("port",self.port)
		self.outSocketBufferSize = archiver.Int64("outSocketBufferSize",self.outSocketBufferSize)
		self.inSocketBufferSize = archiver.Int64("inSocketBufferSize",self.inSocketBufferSize)

		if self.linktype == "":
			self.linktype = TRANSPORT_MC
		if self.linktype == TRANSPORT_TCP or self.linktype == TRANSPORT_UDP:
			self.domainAddress = ops.Support.getHostAddress(self.domainAddress)

	def validate(self):
		super(Channel,self).validate()
		if not isinstance(self.channelID,str):
			raise ValueError()
		if not isinstance(self.linktype,str):
			raise ValueError()
		if not isinstance(self.localInterface,str):
			raise ValueError()
		if not isinstance(self.domainAddress,str):
			raise ValueError()
		if not isinstance(self.timeToLive,int):
			raise ValueError()
		if not isinstance(self.port,int):
			raise ValueError()
		if not isinstance(self.outSocketBufferSize,int):
			raise ValueError()
		if not isinstance(self.inSocketBufferSize,int):
			raise ValueError()

	def populateTopic(self,top):
		top.transport = self.linktype
		top.localInterface = self.localInterface
		top.domainAddress = self.domainAddress
		top.port = self.port
		top.outSocketBufferSize = self.outSocketBufferSize
		top.inSocketBufferSize = self.inSocketBufferSize
		top.timeToLive = self.timeToLive
		top.channelID = self.channelID

class Transport(OPS_Object):
	"""docstring for Transport"""
	TRANSPORT_IDLVERSION = 0

	def __init__(self):
		super(Transport,self).__init__()
		self.Transport_version = Transport.TRANSPORT_IDLVERSION
		self.appendType("Transport")
		self.channelID=""
		self.topics=[]

	def __str__(self):
		temp ="\nTransport:"
		temp += "\n  channelID: %s" % self.channelID
		temp += "\n  topics: "
		for tname in self.topics:
			temp += tname + ", "
		return super(Transport,self).__str__() + temp

	def serialize(self,archiver):
		super(Transport,self).serialize(archiver)
		if self.idlVersionMask != 0:
			self.Transport_version = dataBuffer.Int8("Transport_version",self.Transport_version)
			if self.Transport_version > Transport.TRANPSORT_IDLVERSION:
				raise IdlVersionError("Transport", self.Transport_version, Transport.TRANSPORT_IDLVERSION)
			else:
				pass
		else:
			self.Transport_version = 0
		self.channelID = archiver.String("channelID",self.channelID)
		archiver.StringVector("topics", self.topics)

	def validate(self):
		super(Transport,self).validate()
		if not isinstance(self.channelID,str):
			raise ValueError()
		for x in self.topics:
			if not isinstance(x,str):
				raise ValueError()


from ops.XML_Archiver import XML_Archiver_In

class Domain(OPS_Object):
	"""docstring for Domain"""
	DOMAIN_IDLVERSION = 0

	def __init__(self):
		super(Domain,self).__init__()
		self.Domain_version = Domain.DOMAIN_IDLVERSION
		self.appendType("Domain")
		self.domainAddress=""
		self.timeToLive = 1
		self.localInterface = "0.0.0.0"
		self.inSocketBufferSize = 16000000
		self.outSocketBufferSize = 16000000
		self.metaDataMcPort = 9494
		self.debugMcPort = 0
		self.topics=[]
		self.domainID=""
		self.channels=[]
		self.transports=[]
		self.optNonVirt=False

	def __str__(self):
		temp ="\nDomain:"
		temp += "\n  domainAddress: %s" % self.domainAddress
		temp += "\n  timeToLive: %s" % self.timeToLive
		temp += "\n  localInterface: %s" % self.localInterface
		temp += "\n  inSocketBufferSize: %s" % self.inSocketBufferSize
		temp += "\n  outSocketBufferSize: %s" % self.outSocketBufferSize
		temp += "\n  metaDataMcPort: %s" % self.metaDataMcPort
		temp += "\n  debugMcPort: %s" % self.debugMcPort
		temp += "\n  domainID: %s" % self.domainID
		for t in self.topics:
		 	temp+="\n" + t.__str__()
		for chn in self.channels:
		 	temp+="\n" + chn.__str__()
		for trn in self.transports:
		 	temp+="\n" + trn.__str__()
		return super(Domain,self).__str__() + temp

	def serialize(self,archiver):
		super(Domain,self).serialize(archiver)
		if self.idlVersionMask != 0:
			self.Domain_version = dataBuffer.Int8("Domain_version",self.Domain_version)
			if self.Domain_version > Domain.DOMAIN_IDLVERSION:
				raise IdlVersionError("Domain", self.Domain_version, Domain.DOMAIN_IDLVERSION)
			else:
				pass
		else:
			self.Domain_version = 0
		self.domainID = archiver.String("domainID",self.domainID)
		archiver.OpsVector("topics",self.topics,Topic)
		self.domainAddress = archiver.String("domainAddress",self.domainAddress)
		self.localInterface = ops.Support.getHostAddressEx(archiver.String("localInterface",self.localInterface))
		self.timeToLive = archiver.Int32("timeToLive",self.timeToLive)
		self.inSocketBufferSize = archiver.Int32("inSocketBufferSize",self.inSocketBufferSize)
		self.outSocketBufferSize = archiver.Int32("outSocketBufferSize",self.outSocketBufferSize)
		self.metaDataMcPort = archiver.Int32("metaDataMcPort",self.metaDataMcPort)

		# To not break binary compatibility we only do this when we know we are
		# reading from an XML-file
		if isinstance(archiver,XML_Archiver_In):
			archiver.OpsVector("channels",self.channels,Channel)
			archiver.OpsVector("transports",self.transports,Transport)
			self.debugMcPort = archiver.Int32("debugMcPort",self.debugMcPort)
			self.optNonVirt = archiver.Bool("optNonVirt",self.optNonVirt)
			self.checkTransports()

	def validate(self):
		super(Domain,self).validate()
		if not isinstance(self.domainID,str):
			raise ValueError()
		for x in self.topics:
			if not isinstance(x,Topic):
				raise ValueError()
			x.validate()
		if not isinstance(self.domainAddress,str):
			raise ValueError()
		if not isinstance(self.localInterface,str):
			raise ValueError()
		if not isinstance(self.timeToLive,int):
			raise ValueError()
		if not isinstance(self.inSocketBufferSize,int):
			raise ValueError()
		if not isinstance(self.outSocketBufferSize,int):
			raise ValueError()
		if not isinstance(self.metaDataMcPort,int):
			raise ValueError()
		if not isinstance(self.debugMcPort,int):
			raise ValueError()
		for x in self.channels:
			if not isinstance(x,Channel):
				raise ValueError()
			x.validate()
		for x in self.transports:
			if not isinstance(x,Transport):
				raise ValueError()
			x.validate()

	def getTopic(self,name):
		for t in self.topics:
			if t.domainAddress == "":
				t.domainAddress = self.domainAddress
			if t.localInterface == "":
				t.localInterface = self.localInterface
			if t.timeToLive < 0:
				t.timeToLive = self.timeToLive
			if t.inSocketBufferSize < 0:
				t.inSocketBufferSize = self.inSocketBufferSize
			if t.outSocketBufferSize < 0:
				t.outSocketBufferSize = self.outSocketBufferSize
			t.optNonVirt = self.optNonVirt
			if t.name == name:
				return copy.deepcopy(t)
		return None

	def findChannel(self,name):
		for chn in self.channels:
			if chn.channelID == name:
				return chn
		return None

	def findTopic(self,name):
		for t in self.topics:
			if t.name == name:
				return t
		return None

	def checkTransports(self):
		for trn in self.transports:
			chn = self.findChannel(trn.channelID)
			if chn == None:
				raise ConfigError("Non existing channelID used in transport specification")
			else:
				for tname in trn.topics:
					top = self.findTopic(tname)
					if top == None:
						raise ConfigError("Non existing topicID used in transport specification")
					else:
						chn.populateTopic(top)


import ops.Factory

class Config(OPS_Object):
	OPSCONFIG_IDLVERSION = 0

	def __init__(self):
		super(Config, self).__init__()
		self.OPSConfig_version = Config.OPSCONFIG_IDLVERSION
		self.domains=[]

	def __str__(self):
		temp = "Config:"
		for d in self.domains:
			temp += "\n" +  d.__str__()
		return super(Config,self).__str__() + "\n" + temp


	@staticmethod
	def getConfig(filename="ops_config.xml"):
		xmlArchiver = XML_Archiver_In(ops.Factory.OpsDefaultFactory(),filename,"root")
		config = xmlArchiver.Ops("ops_config",None)
		return config


	def getDomain(self,domainID):
		for d in self.domains:
			if d.domainID == domainID:
				return d
		return None


	def serialize(self,dataBuffer):
		super(Config,self).serialize(dataBuffer)
		if self.idlVersionMask != 0:
			self.OPSConfig_version = dataBuffer.Int8("OPSConfig_version",self.OPSConfig_version)
			if self.OPSConfig_version > Config.OPSCONFIG_IDLVERSION:
				raise IdlVersionError("Config", self.OPSConfig_version, Config.OPSCONFIG_IDLVERSION)
			else:
				pass
		else:
			self.OPSConfig_version = 0
		dataBuffer.OpsVector("domains",self.domains,Domain)

	def validate(self):
		super(Config,self).validate()
		for x in self.domains:
			if not isinstance(x,Domain):
				raise ValueError()
			x.validate()


class DefaultOPSConfigImpl(Config):
	DEFAULTOPSCONFIGIMPL_IDLVERSION = 0

	def __init__(self):
		super(DefaultOPSConfigImpl, self).__init__()
		self.DefaultOPSConfigImpl_version = DefaultOPSConfigImpl.DEFAULTOPSCONFIGIMPL_IDLVERSION
		self.appendType("DefaultOPSConfigImpl")

	def serialize(self,dataBuffer):
		super(DefaultOPSConfigImpl,self).serialize(dataBuffer)
		if self.idlVersionMask != 0:
			self.DefaultOPSConfigImpl_version = dataBuffer.Int8("DefaultOPSConfigImpl_version",self.DefaultOPSConfigImpl_version)
			if self.DefaultOPSConfigImpl_version > DefaultOPSConfigImpl.DEFAULTOPSCONFIGIMPL_IDLVERSION:
				raise IdlVersionError("DefaultOPSConfigImpl", self.DefaultOPSConfigImpl_version, DefaultOPSConfigImpl.DEFAULTOPSCONFIGIMPL_IDLVERSION)
			else:
				pass
		else:
			self.DefaultOPSConfigImpl_version = 0
