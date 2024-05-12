import struct
import socket

def getHostAddresses(name):
	ads = []
	try:
		for ad in socket.getaddrinfo(name, None):
			if ad[0] == socket.AF_INET:
				ads.append(ad[4][0])
	except socket.gaierror:
		pass
	return ads

def getHostAddress(name):
	for addr in getHostAddresses(name):
		return addr
	return name

def getHostAddressEx(localInterface):
	if "/" not in localInterface:
		return getHostAddress(localInterface)
	networkAddress,netMask = localInterface.split("/")
	return getHostAddress(networkAddress) + "/" + netMask


def getInterfaceList():
	return socket.gethostbyname_ex(socket.gethostname())[2] + socket.gethostbyname_ex('localhost')[2]

def doSubnetTranslation(localInterface):
	if "/" not in localInterface:
		return localInterface
	networkAddress,netMask = localInterface.split("/")
	if len(netMask)<3:
		netMask = (0xffffffff << (32 - int(netMask))) & 0xffffffff
	else:
		netMask = struct.unpack(">I",socket.inet_aton(netMask))[0]
	networkAddress = struct.unpack(">I",socket.inet_aton(networkAddress))[0]

	for iface in getInterfaceList():
		temp = struct.unpack(">I",socket.inet_aton(iface))[0]
		if ((temp ^ networkAddress) & netMask) == 0:
			return iface


def isValidNodeAddress(addr):
	if addr == "":
		return False
	ip = struct.unpack(">I",socket.inet_aton(addr))[0]
	if ip == 0:
		return False
	if ip >= 0xE0000000:
	   return False
	return True
