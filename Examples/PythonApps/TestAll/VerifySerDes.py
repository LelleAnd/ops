
import time
import re
import sys
import os
import platform
import math

from TestAll import TestAll
from TestAll import TestAllTypeFactory

from ops import Participant,Publisher,Subscriber,Print_Archiver,OPS_Archiver,Checksum_Archiver

## =========================================================

def isclose(a, b, rel_tol=1e-09, abs_tol=0.0):
    '''
    Python 2 implementation of Python 3.5 math.isclose()
    https://hg.python.org/cpython/file/tip/Modules/mathmodule.c#l1993
    '''
    # sanity check on the inputs
    if rel_tol < 0 or abs_tol < 0:
        raise ValueError("tolerances must be non-negative")

    # short circuit exact equality -- needed to catch two infinities of
    # the same sign. And perhaps speeds things up a bit sometimes.
    if a == b:
        return True

    # This catches the case of two infinities of opposite sign, or
    # one infinity and one finite number. Two infinities of opposite
    # sign would otherwise have an infinite relative tolerance.
    # Two infinities of the same sign are caught by the equality check
    # above.
    if math.isinf(a) or math.isinf(b):
        return False

    # now do the regular computation
    # this is essentially the "weak" test from the Boost library
    diff = math.fabs(b - a)
    result = (((diff <= math.fabs(rel_tol * b)) or
               (diff <= math.fabs(rel_tol * a))) or
              (diff <= abs_tol))
    return result

## =========================================================

def AssertEQ(data, exp):
	if not data == exp:
		raise ValueError()

def AssertListEQ(data, exp):
	AssertEQ(len(data), len(exp))
	for i in range(len(data)):
		if isinstance(data[i], TestAll.TestData):
			CompareTestData(data[i], exp[i])
		elif isinstance(data[i], TestAll.Fruit):
			CompareFruit(data[i], exp[i])
		else:
			AssertEQ(data[i], exp[i])

def AssertFloat32EQ(data, exp, abs_tol):
	if not isclose(data, exp, abs_tol = abs_tol):
		raise ValueError()

def AssertFloat32ListEQ(data, exp, abs_tol):
	AssertEQ(len(data), len(exp))
	for i in range(len(data)):
		AssertFloat32EQ(data[i], exp[i], abs_tol)


## =========================================================

def CheckEmptyTestData(data):
	if not isinstance(data,TestAll.TestData):
		raise ValueError()
	data.validate()
	AssertEQ(data.text, "")
	AssertEQ(data.value, 0.0)

def CheckEmptyFruit(data):
	if not isinstance(data,TestAll.Fruit):
		raise ValueError()
	data.validate()
	AssertEQ(data.value, TestAll.Fruit.APPLE)

def CheckEmptyChildData(data):
	if not isinstance(data,TestAll.ChildData):
		raise ValueError()
	data.validate()

	# Data for fields in BaseData
	AssertEQ(data.baseText, "")
	AssertEQ(len(data.stringOpenArr), 0)
	AssertEQ(len(data.stringFixArr), 5)
	AssertEQ(data.fixLengthString, "")
	AssertEQ(len(data.fixLengthStringOpenArr), 0)
	AssertEQ(len(data.fixLengthStringFixArr), 10)

	# Data for fields in ChildData
	#  enums
	AssertEQ(data.enu1, TestAll.ChildData.Order.ABC)
	AssertEQ(len(data.enuVec), 0)
	AssertEQ(len(data.enuFixArr), 6)
	AssertEQ(data.cmd, TestAll.Definitions.Command.START)
	AssertEQ(len(data.cmds), 2)

	#  core types
	AssertEQ(data.bo, False)
	AssertEQ(data.b, 0)
	AssertEQ(data.sh, 0)
	AssertEQ(data.i, 0)
	AssertEQ(data.l, 0)
	AssertEQ(data.f, 0.0)
	AssertEQ(data.d, 0.0)
	AssertEQ(data.s, "")

	CheckEmptyTestData(data.test2)
	CheckEmptyTestData(data.testPointer)
	CheckEmptyFruit(data.fruit)

	AssertEQ(len(data.bos), 0)
	#	bool fbos[11];
	AssertEQ(len(data.fbos), 11)

	AssertEQ(len(data.bs), 0)
	#	char fbs[256];
	AssertEQ(len(data.fbs), 256)

	AssertEQ(len(data.shs), 0)
	#	short fshs[4];
	AssertEQ(len(data.fshs), 4)

	AssertEQ(len(data.is_), 0)
	#	int fis_[3];
	AssertEQ(len(data.fis_), 3)

	AssertEQ(len(data.ls), 0)
	#	int64_t fls[6];
	AssertEQ(len(data.fls), 6)

	AssertEQ(len(data.fs), 0)
	#    float ffs[77];
	AssertEQ(len(data.ffs), 77)

	AssertEQ(len(data.ds), 0)
	#    double fds[5];
	AssertEQ(len(data.fds), 5)

	AssertEQ(len(data.ss), 0)
	#    std::string fss[10];
	AssertEQ(len(data.fss), 10)

	AssertEQ(len(data.test2s), 0)
	#    TestData* ftest2s[5];
	AssertEQ(len(data.ftest2s), 5)

	AssertEQ(len(data.secondVirtArray), 0)
	#    TestData* fsecondVirtArray[7];
	AssertEQ(len(data.fsecondVirtArray), 7)

	AssertEQ(len(data.test2s2), 0)
	#    TestData ftest2s2[4];
	AssertEQ(len(data.ftest2s2), 4)

	AssertEQ(len(data.fruitarr), 0)
	#    Fruit ffruitarr[15];
	AssertEQ(len(data.ffruitarr), 15)

## =========================================================

def CompareTestData(data, exp):
	if not isinstance(data,TestAll.TestData):
		raise ValueError()
	if not isinstance(exp,TestAll.TestData):
		raise ValueError()
	data.validate()
	exp.validate()
	if not data.text == exp.text:
		raise ValueError()
	if not data.value == exp.value:
		raise ValueError()

def CompareFruit(data, exp):
	if not isinstance(data,TestAll.Fruit):
		raise ValueError()
	if not isinstance(exp,TestAll.Fruit):
		raise ValueError()
	data.validate()
	exp.validate()
	AssertEQ(data.value, exp.value)

def CompareChildData(data, exp):
	if not isinstance(data,TestAll.ChildData):
		raise ValueError()
	if not isinstance(exp,TestAll.ChildData):
		raise ValueError()
	data.validate()
	exp.validate()

	# Data for fields in BaseData
	AssertEQ(data.baseText, exp.baseText)
	AssertListEQ(data.stringOpenArr, exp.stringOpenArr)
	AssertListEQ(data.stringFixArr, exp.stringFixArr)
	AssertEQ(data.fixLengthString, exp.fixLengthString)
	AssertListEQ(data.fixLengthStringOpenArr, exp.fixLengthStringOpenArr)
	AssertListEQ(data.fixLengthStringFixArr, exp.fixLengthStringFixArr)

	# Data for fields in ChildData
	#  enums
	AssertEQ(data.enu1, exp.enu1)
	AssertListEQ(data.enuVec, exp.enuVec)
	AssertListEQ(data.enuFixArr, exp.enuFixArr)
	AssertEQ(data.cmd, exp.cmd)
	AssertListEQ(data.cmds, exp.cmds)

	#  core types
	AssertEQ(data.bo, exp.bo)
	AssertEQ(data.b, exp.b)
	AssertEQ(data.sh, exp.sh)
	AssertEQ(data.i, exp.i)
	AssertEQ(data.l, exp.l)
	AssertFloat32EQ(data.f, exp.f, 0.00001)
	AssertEQ(data.d, exp.d)
	AssertEQ(data.s, exp.s)

	CompareTestData(data.test2, exp.test2)
	CompareTestData(data.testPointer, exp.testPointer)
	CompareFruit(data.fruit, exp.fruit)

	AssertListEQ(data.bos, exp.bos)
	#	bool fbos[11];
	AssertListEQ(data.fbos, exp.fbos)

	AssertListEQ(data.bs, exp.bs)
	#	char fbs[256];
	AssertListEQ(data.fbs, exp.fbs)

	AssertListEQ(data.shs, exp.shs)
	#	short fshs[4];
	AssertListEQ(data.fshs, exp.fshs)

	AssertListEQ(data.is_, exp.is_)
	#	int fis_[3];
	AssertListEQ(data.fis_, exp.fis_)

	AssertListEQ(data.ls, exp.ls)
	#	int64_t fls[6];
	AssertListEQ(data.fls, exp.fls)

	AssertFloat32ListEQ(data.fs, exp.fs, 0.00001)
	#    float ffs[77];
	AssertFloat32ListEQ(data.ffs, exp.ffs, 0.00001)

	AssertListEQ(data.ds, exp.ds)
	#    double fds[5];
	AssertListEQ(data.fds, exp.fds)

	AssertListEQ(data.ss, exp.ss)
	#    std::string fss[10];
	AssertListEQ(data.fss, exp.fss)

	AssertListEQ(data.test2s, exp.test2s)
	#    TestData* ftest2s[5];
	AssertListEQ(data.ftest2s, exp.ftest2s)

	AssertListEQ(data.secondVirtArray, exp.secondVirtArray)
	#    TestData* fsecondVirtArray[7];
	AssertListEQ(data.fsecondVirtArray, exp.fsecondVirtArray)

	AssertListEQ(data.test2s2, exp.test2s2)
	#    TestData ftest2s2[4];
	AssertListEQ(data.ftest2s2, exp.ftest2s2)

	AssertListEQ(data.fruitarr, exp.fruitarr)
	#    Fruit ffruitarr[15];
	AssertListEQ(data.ffruitarr, exp.ffruitarr)

## =========================================================

def FillChildData(data):
	if not isinstance(data,TestAll.ChildData):
		raise ValueError()

	# Data for fields in BaseData
	data.baseText = "dynamic string"
	data.stringOpenArr = [ "dyn str 1", "dyn str 2" ]
	data.stringFixArr = [ "dsf 0", "dsf 1", "dsf 2", "dsf 3", "dsf 4" ]
	data.fixLengthString = "fixed length string"
	data.fixLengthStringOpenArr = [ "fix len str 1", "fix len str 2" ]
	data.fixLengthStringFixArr = [ "fsf 0", "fsf 1", "fsf 2", "fsf 3", "fsf 4", "fsf 5", "fsf 6", "fsf 7", "fsf 8", "fsf 9" ]

	# Data for fields in ChildData
	#  enums
	data.enu1 = TestAll.ChildData.Order.GHI;

	data.enuVec = [ TestAll.ChildData.Order.GHI, TestAll.ChildData.Order.JKL, TestAll.ChildData.Order.JKL,
					TestAll.ChildData.Order.ABC, TestAll.ChildData.Order.DEF ]

	data.enuFixArr = [ TestAll.ChildData.Order.DEF, TestAll.ChildData.Order.ABC, TestAll.ChildData.Order.ABC, TestAll.ChildData.Order.ABC,
					   TestAll.ChildData.Order.JKL, TestAll.ChildData.Order.DEF ]

	data.cmd = TestAll.Definitions.Command.CONTINUE;
	data.cmds = [ TestAll.Definitions.Command.PAUSE, TestAll.Definitions.Command.STOP ]

	#  core types
	data.bo = True
	data.b = 7
	data.sh = -99
	data.i = 19
	data.l = 3456789
	data.f = 123.4567
	data.d = 987.12345678901
	data.s = "Test of [de]serializing"

	data.test2.text = "TestData"
	data.test2.value = 555.5

	data.testPointer.text = "TestPtr"
	data.testPointer.value = 777.7

	data.fruit.value = TestAll.Fruit.PEAR

	data.bos = [ False, True, False ]

	#	bool fbos[11];
	data.fbos = [ False, False, False, False, False, True, False, False, False, False, True ]

	data.bs = [ 10, 20, 30 ]

	#	char fbs[256];
	part1 = [ i for i in range(128) ]
	part2 = [ i for i in range(-128,0) ]
	data.fbs = part1 + part2

	data.shs = [ 1111, 2222 ]

	#	short fshs[4];
	data.fshs = [ 21, 121, 221, 321 ]

	data.is_ = [ 100000, 101010, 110101, 111111 ]

	#	int fis_[3];
	data.fis_ = [ -1, -2, -3 ]

	data.ls = [ 9, 8, 7, 6 ]

	#	int64_t fls[6];
	data.fls = [ 9999, 9998, 9997, 9996, 9995, 9994 ]

	data.fs = [ 3.1, 31.14, 4.56, 987.0 ]

	#   float ffs[77];
	data.ffs = [ 0.0 for i in range(77) ]
	data.ffs[21] = 3.1415

	data.ds = [ 1.987654321, 2.3456789 ]

	#   double fds[5];
	data.fds = [ 1.1, 2.1, 3.1, 4.1, 5.1 ]

	data.ss = [ "Index 0", "Index 1", "Index 2" ]

	#   std::string fss[10];
	data.fss = [ "" for i in range(10) ]
	data.fss[4] = "4 string"
	data.fss[7] = "7 string"
	data.fss[9] = "9 string"

	#   std::vector<TestData*> test2s;
	data.test2s = [ TestAll.TestData() for i in range(4) ]

	#   TestData* ftest2s[5];
	data.ftest2s = [ TestAll.TestData(), TestAll.TestData(), TestAll.TestData(), TestAll.TestData(), TestAll.TestData() ]
	data.ftest2s[2].text = "Index 2"
	data.ftest2s[2].value = 7.7

	#   std::vector<TestData*> secondVirtArray;
	data.secondVirtArray = [ TestAll.TestData(), TestAll.TestData() ]

	#   TestData* fsecondVirtArray[7];
	data.fsecondVirtArray = [ TestAll.TestData(), TestAll.TestData(), TestAll.TestData(), TestAll.TestData(), TestAll.TestData(), TestAll.TestData(), TestAll.TestData() ]
	data.fsecondVirtArray[5].text = "Index 5"
	data.fsecondVirtArray[5].value = -9.99

	#   std::vector<TestData> test2s2;
	data.test2s2 = [ TestAll.TestData() for i in range(11) ]

	#   TestData ftest2s2[4];
	data.ftest2s2 = [ TestAll.TestData() for i in range(4) ]
	data.ftest2s2[3].text = ""
	data.ftest2s2[1].value = 710.6

	data.fruitarr = [ TestAll.Fruit() for i in range(2) ]
	data.fruitarr[0].value = TestAll.Fruit.PEAR
	data.fruitarr[1].value = TestAll.Fruit.BANANA

	#    Fruit ffruitarr[15];
	data.ffruitarr = [ TestAll.Fruit() for i in range(15) ]
	data.ffruitarr[0].value = TestAll.Fruit.PEAR
	data.ffruitarr[14].value = TestAll.Fruit.PEAR

def onChildData(sub,mess):
	addr,port = map(str,mess.getSource())
	data=mess.data
	tempStr = "[From: " + mess.publisherName
	tempStr +="] (From " + addr + ":" + port + ") Comparing..."
	print(tempStr)
	#prt.printObject("data", data)
	CompareChildData(data, cd1)
	print("Comparing finished")

print("Test initial state...")

enumTest = TestAll.Definitions.Command.PAUSE
constTest = TestAll.Definitions.CONST_B

cd1 = TestAll.ChildData()
cd2 = TestAll.ChildData()
cd3 = TestAll.ChildData()
CheckEmptyChildData(cd1)
CheckEmptyChildData(cd2)
CheckEmptyChildData(cd3)

CompareChildData(cd1, cd2)

print("Finished")
print("Test filled object...")

FillChildData(cd1)
FillChildData(cd2)

CompareChildData(cd1, cd2)

print("Finished")
print("Test Print Archiver...")

prt = Print_Archiver.Print_Archiver_Out()
prt.printObject("cd1", cd1)

print("Print Archiver Test Finished")

print("Test Checksum Archiver...")

calc = Checksum_Archiver.Checksum_Calc_8bit_xor()
chk = Checksum_Archiver.Checksum_Archiver(4096,calc)
cd1.serialize(chk)
AssertEQ(calc.sum, 140)

print ("Checksum Archiver # fields = " + str(calc.totalfields))
print ("Checksum Archiver # bytes = " + str(calc.totalbytes))
print ("Checksum Archiver 8-bit XOR = " + str(calc.sum))

print("Serialize filled object...")

archiver = OPS_Archiver.OPS_Archiver_Out(65536,False)
print("  GetSize()= " + str(archiver.index))
archiver.Ops("data",cd1,TestAll.ChildData)
print("  optNonVirt = false, GetSize()= " + str(archiver.index))
AssertEQ(archiver.index, 3150)

archiver = OPS_Archiver.OPS_Archiver_Out(65536,True)
print("  GetSize()= " + str(archiver.index))
archiver.Ops("data",cd1,TestAll.ChildData)

#filedesciptor = open('python-dump-opt.bin','wb')
#filedesciptor.write(archiver.buffer)
#filedesciptor.close()

print("  optNonVirt = true,  GetSize()= " + str(archiver.index))
AssertEQ(archiver.index, 2591)

print("Serialize finished")
print("Test publish/subscribe...")

cfg_file = None
import os.path
from os import path
if not path.exists("ops_config.xml"):
	cwd = os.getcwd()
	idx = cwd.index("Examples")
	if idx > 0:
		cfg_file = cwd[0:idx] + "Examples/OPSIdls/TestAll/ops_config.xml"

if cfg_file == None:
	print("Using config file in CWD")
else:
	print("Using config file: " + cfg_file)

participant = Participant.Participant.getInstance("TestAllDomain", "TestAllDomain", cfg_file)
if participant == None:
	print("Failed to create Participant. Missing ops_config.xml ??")
	sys.exit(-1)
participant.addTypeSupport(TestAllTypeFactory.TestAllTypeFactory())

topic = participant.createTopic("ChildTopic");
print("Created topic " + topic.name + " [" + topic.transport + "." + topic.domainAddress +"." + str(topic.port) + "]")

sub = Subscriber.Subscriber(topic)
sub.addCallback(onChildData)
sub.start()

pub = Publisher.Publisher(topic)
pub.name = "Python"
pub.start()

pub.write(cd1)

print("Waiting for data (60 seconds) ...")

for x in range(10):
	time.sleep(6)
	pub.write(cd1)

sub.stop()
pub.stop()

print("Sleeping for 5 seconds ...")

time.sleep(5)
