from ops.opsTypes import OPS_Object,IdlVersionError

class TopicInfoData(OPS_Object):
	TOPICINFODATA_IDLVERSION = 0

	def __init__(self,topic=None,addr=None):
		super(TopicInfoData, self).__init__()
		self.appendType("TopicInfoData")
		self.TopicInfoData_version = TopicInfoData.TOPICINFODATA_IDLVERSION
		if topic is not None:
			self.name = topic.name
			self.type = topic.typeID
			self.transport = topic.transport
			if addr is not None:
				self.address = addr[0]
				self.port = addr[1]
			else:
				self.address = topic.domainAddress
				self.port = topic.port
		else:
			self.name = ""
			self.type = ""
			self.transport = ""
			self.address = ""
			self.port = 0
		self.keys=[]

	def serialize(self,archiver):
		super(TopicInfoData,self).serialize(archiver)
		if self.idlVersionMask != 0:
			self.TopicInfoData_version = dataBuffer.Int8("TopicInfoData_version",self.TopicInfoData_version)
			if self.TopicInfoData_version > TopicInfoData.TOPICINFODATA_IDLVERSION:
				raise IdlVersionError("TopicInfoData", self.TopicInfoData_version, TopicInfoData.TOPICINFODATA_IDLVERSION)
			else:
				pass
		else:
			self.TopicInfoData_version = 0
		self.name = archiver.String("name", self.name)
		self.type = archiver.String("type", self.type)
		self.transport = archiver.String("transport", self.transport)
		self.address = archiver.String("address", self.address)
		self.port = archiver.Int32("port", self.port)
		archiver.StringVector("keys", self.keys)

class ParticipantInfoData(OPS_Object):
	PARTICIPANTINFODATA_IDLVERSION = 0

	def __init__(self):
		super(ParticipantInfoData, self).__init__()
		self.ParticipantInfoData_version = ParticipantInfoData.PARTICIPANTINFODATA_IDLVERSION
		self.appendType("ops.ParticipantInfoData")

		self.name = ""
		self.id = ""
		self.domain = ""
		self.ip = ""
		self.languageImplementation = ""
		self.opsVersion = ""
		self.mc_udp_port = 0
		self.mc_tcp_port = 0

		self.subscribeTopics = []
		self.publishTopics = []
		self.knownTypes = []

	def serialize(self,archiver):
		super(ParticipantInfoData,self).serialize(archiver)
		if self.idlVersionMask != 0:
			self.ParticipantInfoData_version = dataBuffer.Int8("ParticipantInfoData_version",self.ParticipantInfoData_version)
			if self.ParticipantInfoData_version > ParticipantInfoData.PARTICIPANTINFODATA_IDLVERSION:
				raise IdlVersionError("ParticipantInfoData", self.ParticipantInfoData_version, ParticipantInfoData.PARTICIPANTINFODATA_IDLVERSION)
			else:
				pass
		else:
			self.ParticipantInfoData_version = 0

		self.name                   = archiver.String("name", self.name)
		self.domain                 = archiver.String("domain", self.domain)
		self.id                     = archiver.String("id", self.id)
		self.ip                     = archiver.String("ip", self.ip)
		self.languageImplementation = archiver.String("languageImplementation", self.languageImplementation)
		self.opsVersion             = archiver.String("opsVersion", self.opsVersion)
		self.mc_udp_port            = archiver.Int32("mc_udp_port", self.mc_udp_port)
		self.mc_tcp_port            = archiver.Int32("mc_tcp_port", self.mc_tcp_port)

		archiver.OpsVector("subscribeTopics", self.subscribeTopics,TopicInfoData)
		archiver.OpsVector("publishTopics", self.publishTopics,TopicInfoData)
		archiver.StringVector("knownTypes", self.knownTypes)
