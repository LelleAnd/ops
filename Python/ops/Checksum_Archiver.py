import struct
import sys

from ops.Archiver import Archiver_Out

#Helper to handle Python 2 and 3 differences
if sys.version_info < (3,):
	def b(x):
		return x
else:
	import codecs
	def b(x):
		return codecs.latin_1_encode(x)[0]

#Example checksum calculator
class Checksum_Calc_8bit_xor:
	def __init__(self):
		self.sum = 0
		self.totalbytes = 0
		self.totalfields = 0

	def Calc(self,name,buffer,num):
		##print("Calc: num bytes = " + str(num))
		self.totalbytes += num
		self.totalfields += 1
		if num > 0:
			for idx in range(num):
				self.sum = self.sum ^ buffer[idx]

class Checksum_Archiver(Archiver_Out):

	def __init__(self,size,calc):
		super(Checksum_Archiver, self).__init__()
		self.calc = calc
		self.buffer = bytearray(size)

	def CoreValue(self,name,fmt,value):
		struct.pack_into(fmt,self.buffer,0,value)
		num = struct.calcsize(fmt)
		self.calc.Calc(name, self.buffer,num)

	def Bool(self,name,value):
		self.CoreValue(name,'<?',value)
		return value
	def Int8(self,name,value):
		self.CoreValue(name,'<b',value)
		return value
	def Int16(self,name,value):
		self.CoreValue(name,'<h',value)
		return value
	def Int32(self,name,value):
		self.CoreValue(name,'<i',value)
		return value
	def Int64(self,name,value):
		self.CoreValue(name,'<q',value)
		return value
	def Float32(self,name,value):
		self.CoreValue(name,'<f',value)
		return value
	def Float64(self,name,value):
		self.CoreValue(name,'<d',value)
		return value
	def String(self,name,value):
		fmt = '%ss' % len(value)
		struct.pack_into(fmt,self.buffer,0,b(value))
		num = struct.calcsize(fmt)
		self.calc.Calc(name,self.buffer,num)
		return value

	def Ops(self,name,value,prototype=None):
		if value is not None:
			value.serialize(self)
		return value

	def BoolVector(self,name,value):
		fmt = '%s?' % len(value)
		struct.pack_into(fmt,self.buffer,0,*value)
		num = struct.calcsize(fmt)
		self.calc.Calc(name,self.buffer,num)
	def Int8Vector(self,name,value):
		fmt = '%sb' % len(value)
		struct.pack_into(fmt,self.buffer,0,*value)
		num = struct.calcsize(fmt)
		self.calc.Calc(name,self.buffer,num)
	def Int16Vector(self,name,value):
		fmt = '%sh' % len(value)
		struct.pack_into(fmt,self.buffer,0,*value)
		num = struct.calcsize(fmt)
		self.calc.Calc(name,self.buffer,num)
	def Int32Vector(self,name,value):
		fmt = '%si' % len(value)
		struct.pack_into(fmt,self.buffer,0,*value)
		num = struct.calcsize(fmt)
		self.calc.Calc(name,self.buffer,num)
	def Int64Vector(self,name,value):
		fmt = '%sq' % len(value)
		struct.pack_into(fmt,self.buffer,0,*value)
		num = struct.calcsize(fmt)
		self.calc.Calc(name,self.buffer,num)
	def Float32Vector(self,name,value):
		fmt = '%sf' % len(value)
		struct.pack_into(fmt,self.buffer,0,*value)
		num = struct.calcsize(fmt)
		self.calc.Calc(name,self.buffer,num)
	def Float64Vector(self,name,value):
		fmt = '%sd' % len(value)
		struct.pack_into(fmt,self.buffer,0,*value)
		num = struct.calcsize(fmt)
		self.calc.Calc(name,self.buffer,num)
	def StringVector(self,name,value):
		for itr in value:
			fmt = '%ss' % len(itr)
			struct.pack_into(fmt,self.buffer,0,b(itr))
			num = struct.calcsize(fmt)
			self.calc.Calc(name,self.buffer,num)

	def OpsVector(self,name,value,prototype=None):
		for v in value:
			self.Ops("element", v,prototype)

	def Spare(self,data):
		raise NotImplementedError
