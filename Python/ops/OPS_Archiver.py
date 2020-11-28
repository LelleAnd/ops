import struct
import sys

from ops.Archiver import Archiver_In,Archiver_Out

#Helper to handle Python 2 and 3 differences
if sys.version_info < (3,):
	def b(x):
		return x
	def zzz(x):
		return x
else:
	import codecs
	def b(x):
		return codecs.latin_1_encode(x)[0]
	def zzz(x):
		return codecs.latin_1_decode(x)[0]

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
		res = zzz(struct.unpack_from(fmt,self.buffer,self.index)[0])
		self.index += struct.calcsize(fmt)
		return res
	def Ops(self,name,value,prototype=None):
		fmt = '<%ss' % struct.unpack_from('<i',self.buffer,self.index)[0]
		self.index += struct.calcsize('<i')
		typename = zzz(struct.unpack_from(fmt,self.buffer,self.index)[0])
		self.index += struct.calcsize(fmt)

		verMask = 0
		if len(typename) > 0:
			if typename[0] == '0':
				typename = typename[2:]
				verMask = 1		## Just a value != 0

		res = None
		if prototype is not None:
			res = prototype()
		else:
			res = self.factory.create(typename)
		if res is not None:
			res.idlVersionMask = verMask
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
			value.append(zzz(struct.unpack_from(fmt,self.buffer,self.index)[0]))
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
	def __init__(self,size,optNonVirt):
		super(OPS_Archiver_Out, self).__init__()
		self.index = 0
		self.buffer = bytearray(size)
		self.optNonVirt=optNonVirt
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
		struct.pack_into(fmt,self.buffer,self.index,len(value),b(value))
		self.index += struct.calcsize(fmt)
		return value
	def Ops(self,name,value,prototype=None):
		if value.idlVersionMask != 0:
			typesString="0 "
		else:
			typesString=""
		if self.optNonVirt==True and prototype!=None:
			##typesString=""
			pass
		else:
			typesString += value.typesString
		fmt = '<i%ss' % len(typesString)
		struct.pack_into(fmt,self.buffer,self.index,len(typesString),b(typesString))
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
			struct.pack_into(fmt,self.buffer,self.index,len(itr),b(itr))
			self.index += struct.calcsize(fmt)
	def OpsVector(self,name,value,prototype=None):
		struct.pack_into('<i',self.buffer,self.index,len(value))
		self.index += struct.calcsize('<i')
		for itr in value:
			if itr.idlVersionMask != 0:
				typesString="0 "
			else:
				typesString=""
			if self.optNonVirt==True and prototype!=None:
				##typesString=""
				pass
			else:
				typesString += itr.typesString
			fmt = '<i%ss' % len(typesString)
			struct.pack_into(fmt,self.buffer,self.index,len(typesString),b(typesString))
			self.index += struct.calcsize(fmt)
			itr.serialize(self)
	def Spare(self,data):
		if data is not None:
			self.buffer[self.index : ] = data
			self.index += len(data)
