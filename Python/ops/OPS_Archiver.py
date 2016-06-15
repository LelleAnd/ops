from Archiver import Archiver_In,Archiver_Out
import struct

class OPS_Archiver_In(Archiver_In):
	def __init__(self,factory,data):
		super(OPS_Archiver_In, self).__init__(factory)
		self.index = 0
		self.buffer = data
	def Bool(self,name,value):
		fmt = '<?'
		res =  struct.unpack_from(fmt,self.buffer,self.index)[0]
		self.index += struct.calcsize(fmt)
		return res
	def Int8(self,name,value):
		fmt = '<b'
		res =  struct.unpack_from(fmt,self.buffer,self.index)[0]
		self.index += struct.calcsize(fmt)
		return res
	def Int16(self,name,value):
		fmt = '<h'
		res =  struct.unpack_from(fmt,self.buffer,self.index)[0]
		self.index += struct.calcsize(fmt)
		return res
	def Int32(self,name,value):
		fmt = '<i'
		res =  struct.unpack_from(fmt,self.buffer,self.index)[0]
		self.index += struct.calcsize(fmt)
		return res
	def Int64(self,name,value):
		fmt = '<q'
		res =  struct.unpack_from(fmt,self.buffer,self.index)[0]
		self.index += struct.calcsize(fmt)
		return res
	def Float32(self,name,value):
		fmt = '<f'
		res =  struct.unpack_from(fmt,self.buffer,self.index)[0]
		self.index += struct.calcsize(fmt)
		return res
	def Float64(self,name,value):
		fmt = '<d'
		res =  struct.unpack_from(fmt,self.buffer,self.index)[0]
		self.index += struct.calcsize(fmt)
		return res
	def String(self,name,value):
		fmt = '<%ss' % struct.unpack_from('<i',self.buffer,self.index)[0]
		self.index += struct.calcsize('<i')
		res =  struct.unpack_from(fmt,self.buffer,self.index)[0]
		self.index += struct.calcsize(fmt)
		return res
	def Ops(self,name,value,prototype=None):
		fmt = '<%ss' % struct.unpack_from('<i',self.buffer,self.index)[0]
		self.index += struct.calcsize('<i')
		typename =  struct.unpack_from(fmt,self.buffer,self.index)[0]
		self.index += struct.calcsize(fmt)

		res = None
		if prototype is not None:
			res = prototype()
		else:
			res = self.factory.create(typename)
		if res is not None:
			res.serialize(self)
		return res


	def BoolVector(self,name,value):
		del value[:]
		fmt = '<%s?' % struct.unpack_from('<i',self.buffer,self.index)
		self.index += struct.calcsize('<i')
		value.extend(list(struct.unpack_from(fmt,self.buffer,self.index)))
		self.index += struct.calcsize(fmt)
	def Int8Vector(self,name,value):
		del value[:]
		fmt = '<%sb' % struct.unpack_from('<i',self.buffer,self.index)
		self.index += struct.calcsize('<i')
		value.extend(list(struct.unpack_from(fmt,self.buffer,self.index)))
		self.index += struct.calcsize(fmt)
	def Int16Vector(self,name,value):
		del value[:]
		fmt = '<%sh' % struct.unpack_from('<i',self.buffer,self.index)
		self.index += struct.calcsize('<i')
		value.extend(list(struct.unpack_from(fmt,self.buffer,self.index)))
		self.index += struct.calcsize(fmt)
	def Int32Vector(self,name,value):
		del value[:]
		fmt = '<%si' % struct.unpack_from('<i',self.buffer,self.index)
		self.index += struct.calcsize('<i')
		value.extend(list(struct.unpack_from(fmt,self.buffer,self.index)))
		self.index += struct.calcsize(fmt)
	def Int64Vector(self,name,value):
		del value[:]
		fmt = '<%sq' % struct.unpack_from('<i',self.buffer,self.index)
		self.index += struct.calcsize('<i')
		value.extend(list(struct.unpack_from(fmt,self.buffer,self.index)))
		self.index += struct.calcsize(fmt)
	def Float32Vector(self,name,value):
		del value[:]
		fmt = '<%sf' % struct.unpack_from('<i',self.buffer,self.index)
		self.index += struct.calcsize('<i')
		value.extend(list(struct.unpack_from(fmt,self.buffer,self.index)))
		self.index += struct.calcsize(fmt)
	def Float64Vector(self,name,value):
		del value[:]
		fmt = '<%sd' % struct.unpack_from('<i',self.buffer,self.index)
		self.index += struct.calcsize('<i')
		value.extend(list(struct.unpack_from(fmt,self.buffer,self.index)))
		self.index += struct.calcsize(fmt)
	def StringVector(self,name,value):
		del value[:]
		stringCount = struct.unpack_from('<i',self.buffer,self.index)[0]
		self.index += struct.calcsize('<i')
		for i in range(stringCount):
			fmt = '<%ss' % struct.unpack_from('<i',self.buffer,self.index)[0]
			self.index += struct.calcsize('<i')
			value.append(struct.unpack_from(fmt,self.buffer,self.index)[0])
			self.index += struct.calcsize(fmt)
	def OpsVector(self,name,value,prototype=None):
		del value[:]
		stringCount = struct.unpack_from('<i',self.buffer,self.index)[0]
		self.index += struct.calcsize('<i')
		for i in range(stringCount):
			res = self.Ops("element", value,prototype)
			if res is not None:
				value.append(res)

	def Spare(self):
 		return self.buffer[self.index : ]


class OPS_Archiver_Out(Archiver_Out):
	def __init__(self,size):
		super(OPS_Archiver_Out, self).__init__()
		self.index = 0
		self.buffer = bytearray(size)
	def Bool(self,name,value):
		fmt = '<?'
		struct.pack_into(fmt,self.buffer,self.index,value)
		self.index += struct.calcsize(fmt)
		return value
	def Int8(self,name,value):
		fmt = '<b'
		struct.pack_into(fmt,self.buffer,self.index,value)
		self.index += struct.calcsize(fmt)
		return value
	def Int16(self,name,value):
		fmt = '<h'
		struct.pack_into(fmt,self.buffer,self.index,value)
		self.index += struct.calcsize(fmt)
		return value
	def Int32(self,name,value):
		fmt = '<i'
		struct.pack_into(fmt,self.buffer,self.index,value)
		self.index += struct.calcsize(fmt)
		return value
	def Int64(self,name,value):
		fmt = '<q'
		struct.pack_into(fmt,self.buffer,self.index,value)
		self.index += struct.calcsize(fmt)
		return value
	def Float32(self,name,value):
		fmt = '<f'
		struct.pack_into(fmt,self.buffer,self.index,value)
		self.index += struct.calcsize(fmt)
		return value
	def Float64(self,name,value):
		fmt = '<d'
		struct.pack_into(fmt,self.buffer,self.index,value)
		self.index += struct.calcsize(fmt)
		return value
	def String(self,name,value):
		fmt = '<i%ss' % len(value)
		struct.pack_into(fmt,self.buffer,self.index,len(value),value)
		self.index += struct.calcsize(fmt)
		return value
	def Ops(self,name,value,prototype=None):
		typesString = value.typesString
		fmt = '<i%ss' % len(typesString)
		struct.pack_into(fmt,self.buffer,self.index,len(typesString),typesString)
		self.index += struct.calcsize(fmt)
		value.serialize(self)
		return value

	def BoolVector(self,name,value):
		fmt = '<i%s?' % len(value)
		struct.pack_into(fmt,self.buffer,self.index,len(value),*value)
		self.index += struct.calcsize(fmt)
	def Int8Vector(self,name,value):
		fmt = '<i%sb' % len(value)
		struct.pack_into(fmt,self.buffer,self.index,len(value),*value)
		self.index += struct.calcsize(fmt)
	def Int16Vector(self,name,value):
		fmt = '<i%sh' % len(value)
		struct.pack_into(fmt,self.buffer,self.index,len(value),*value)
		self.index += struct.calcsize(fmt)
	def Int32Vector(self,name,value):
		fmt = '<i%si' % len(value)
		struct.pack_into(fmt,self.buffer,self.index,len(value),*value)
		self.index += struct.calcsize(fmt)
	def Int64Vector(self,name,value):
		fmt = '<i%sq' % len(value)
		struct.pack_into(fmt,self.buffer,self.index,len(value),*value)
		self.index += struct.calcsize(fmt)
	def Float32Vector(self,name,value):
		fmt = '<i%sf' % len(value)
		struct.pack_into(fmt,self.buffer,self.index,len(value),*value)
		self.index += struct.calcsize(fmt)
	def Float64Vector(self,name,value):
		fmt = '<i%sd' % len(value)
		struct.pack_into(fmt,self.buffer,self.index,len(value),*value)
		self.index += struct.calcsize(fmt)
	def StringVector(self,name,value):
		struct.pack_into('<i',self.buffer,self.index,len(value))
		self.index += struct.calcsize('<i')
		for itr in value:
			fmt = '<i%ss' % len(itr)
			struct.pack_into(fmt,self.buffer,self.index,len(itr),itr)
			self.index += struct.calcsize(fmt)
	def OpsVector(self,name,value,prototype=None):
		struct.pack_into('<i',self.buffer,self.index,len(value))
		self.index += struct.calcsize('<i')
		for itr in value:
			typesString = itr.typesString
			fmt = '<i%ss' % len(typesString)
			struct.pack_into(fmt,self.buffer,self.index,len(typesString),typesString)
			self.index += struct.calcsize(fmt)
			itr.serialize(self)
	def Spare(self,data):
		if data is not None:
			self.buffer[self.index : ] = data
		else:
			self.buffer[self.index : ] = ""
 		

