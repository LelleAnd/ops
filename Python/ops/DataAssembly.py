import struct
import OPS_Archiver

class Segment(object):
	"""docstring for OPS_Segment"""

	OPS_CURRENT_VERSION=5
	OPS_IDENTIFIER="opsp"

	def __init__(self,data=None):
		super(Segment, self).__init__()
		if data is None:
			self.type=self.OPS_IDENTIFIER
			self.version=self.OPS_CURRENT_VERSION
			self.NumberOfSegments=0
			self.currentSegment=0
			self.data=None
		else:
			self.deserialize(data)
		

	def __str__(self):
		return "OPS_Segment:\n  type=%s\n  version=%s\n  NumberOfSegments=%s\n  currentSegment=%s" % (self.type,self.version,self.NumberOfSegments,self.currentSegment)

	def isValid(self):
		return (self.type==self.OPS_IDENTIFIER) and (self.version==self.OPS_CURRENT_VERSION)

	def serialize(self):
		return struct.pack('<4shii',self.type,self.version,self.NumberOfSegments,self.currentSegment)

	def deserialize(self,bs):
		self.type,self.version,self.NumberOfSegments,self.currentSegment = struct.unpack_from('<4shii',bs)
		self.data=bs[struct.calcsize('<4shii') : ]

class Assembler(object):

	def __init__(self,length = None):
		super(Assembler, self).__init__()
		self.buffers=[]
		self.length = 0
		if length is not None:
			self.setup(length)

	def __str__(self):
		return "length %s\nbuffers %s" % (self.length,self.buffers)
	
	def setup(self,length):
		self.length = length
		self.buffers=[0]*length
	
	def isEmpty(self):
		return self.length == 0

	def isFull(self):
		for b in self.buffers:
			if b==0:
				return False
		return True

	def addSegment(self,segment):
		if self.length==0:
			self.setup(segment.NumberOfSegments)
		if self.buffers[segment.currentSegment]!=0:
			return False
		self.buffers[segment.currentSegment] = segment.data
		return True

	def createOPS(self,factory):
		data = OPS_Archiver.OPS_Archiver_In(factory,"".join(self.buffers))
		message = data.Ops("message",None)
		message.spareBytes = data.Spare()
		return message