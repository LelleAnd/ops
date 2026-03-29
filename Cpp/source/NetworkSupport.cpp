/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2019-2026 Lennart Andersson.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>

#include "OPSTypeDefs.h"
#include "NetworkSupport.h"

#ifndef REPLACE_TRANSPORT_LAYER

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include "BoostIOServiceImpl.h"

namespace ops
{

	uint32_t IPString2Addr(const Address_T addr) 
	{
		return (uint32_t)boost::asio::ip::make_address_v4(addr.c_str()).to_uint();
	}

	Address_T IPAddr2String(const uint32_t addr)
	{
		return boost::asio::ip::address_v4(addr).to_string();
	}

	// Return true if a valid MC address (224.0.0.0 to 239.255.255.255)
	bool isValidMCAddress(const Address_T addr)
	{
		//std::cout << "isValidNodeAddress(): " << addr << std::endl;
        if (addr == "") { return false; }
		const unsigned long Ip = boost::asio::ip::make_address_v4(addr.c_str()).to_uint();
		//std::cout << "isValidNodeAddress(): " << std::hex << Ip << std::dec << std::endl;
        if ((Ip >= 0xE0000000) && (Ip < 0xF0000000)) { return true; }
		return false;
	}

	// Return true if a valid node address
	bool isValidNodeAddress(const Address_T addr)
	{
		//std::cout << "isValidNodeAddress(): " << addr << std::endl;
        if (addr == "") { return false; }
		const unsigned long Ip = boost::asio::ip::make_address_v4(addr.c_str()).to_uint();
		//std::cout << "isValidNodeAddress(): " << std::hex << Ip << std::dec << std::endl;
        if (Ip == 0) { return false; }
        if (Ip >= 0xE0000000) { return false; }  // Skip multicast and above
		return true;
	}

	bool isMyNodeAddress(const Address_T addr, IOService* const ioServ)
	{
		//std::cout << "isMyNodeAddress(): " << addr << std::endl;
        if (addr == "") { return false; }
		const unsigned long Ip = boost::asio::ip::make_address_v4(addr.c_str()).to_uint();
		//std::cout << "isMyNodeAddress(): " << std::hex << Ip << std::dec << std::endl;
        if (Ip == 0x7F000001) { return true; }  // localhost

		boost::asio::io_context* const ioService = BoostIOServiceImpl::get(ioServ);
		if (ioService == nullptr) { return false; }

		using boost::asio::ip::udp;

		// Note: The resolver requires that the hostname can be used to resolve to an ip
		// e.g due to the hostname beeing listed with an ipv4 address in /etc/hosts.
		// On linux this can be tested by using the command "hostname -i"
		udp::resolver resolver(*ioService);
		udp::resolver::results_type endpoints = resolver.resolve(boost::asio::ip::host_name(), "");
		for (auto const& endp : endpoints) {
			const boost::asio::ip::address ipaddr = endp.endpoint().address();
			if (ipaddr.is_v4()) {
				const unsigned long myIp = ipaddr.to_v4().to_uint();
				//std::cout << "isMyNodeAddress() avail: " << std::hex << myIp << std::dec << std::endl;
                if (myIp == Ip) { return true; }
			}
		}
		return false;
	}

// If argument contains a "/" we assume it is on the form:  subnet-address/subnet-mask
// e.g "192.168.10.0/255.255.255.0" or "192.168.10.0/24"
// In that case we loop over all interfaces and take the first one that matches
// i.e. the one whos interface address is on the subnet
Address_T doSubnetTranslation(const Address_T addr, IOService* const ioServ)
{
	using boost::asio::ip::udp;

	Address_T::size_type index;

	index = addr.find("/");
    if (index == Address_T::npos) { return addr; }

	Address_T subnet = addr.substr(0, index);
	const Address_T mask = addr.substr(index+1);

	const unsigned long subnetIp = boost::asio::ip::make_address_v4(subnet.c_str()).to_uint();
	unsigned long subnetMask;
	if (mask.length() <= 2) {
		// Expand to the number of bits given
		subnetMask = atoi(mask.c_str());
        if ((subnetMask == 0) || (subnetMask > 31)) { return subnet; }
		subnetMask = (((1u << subnetMask)-1u) << (32u - subnetMask)) & 0xFFFFFFFF;
	} else {
		subnetMask = boost::asio::ip::make_address_v4(mask.c_str()).to_uint();
	}

	boost::asio::io_context* const ioService = BoostIOServiceImpl::get(ioServ);
	if (ioService == nullptr) { return subnet; }

	// Note: The resolver requires that the hostname can be used to resolve to an ip
	// e.g due to the hostname beeing listed with an ipv4 address in /etc/hosts.
	// On linux this can be tested by using the command "hostname -i"
	udp::resolver resolver(*ioService);
	udp::resolver::results_type endpoints = resolver.resolve(boost::asio::ip::host_name(), "");
	for (auto const& endp : endpoints) {
		const boost::asio::ip::address ipaddr = endp.endpoint().address();
		if (ipaddr.is_v4()) {
			const unsigned long Ip = ipaddr.to_v4().to_uint();
			if ((Ip & subnetMask) == (subnetIp & subnetMask)) {
				return ipaddr.to_string().c_str();
			}
		}
	}

	return subnet;
}

void ShowKnownInterfaces(IOService* const ioServ, InternalString_T name)
{
	using boost::asio::ip::udp;

	boost::asio::io_context* const ioService = BoostIOServiceImpl::get(ioServ);
	if (ioService == nullptr) { return; }

	if (name == "") {
		name = GetHostName();
		std::cout << "\nHostname: " << name << "\n";
		std::cout << "Interfaces:\n";
	}

	// Note: The resolver requires that the hostname can be used to resolve to an ip
	// e.g due to the hostname beeing listed with an ipv4 address in /etc/hosts.
	// On linux this can be tested by using the command "hostname -i"
	udp::resolver resolver(*ioService);
	boost::system::error_code ec;
	udp::resolver::results_type endpoints = resolver.resolve(name.c_str(), "", ec);

	if (ec) {
		std::cout << "Host '" << name << "' not found (" << ec.message() << ")\n";

	} else {
		for (auto const& endp : endpoints) {
			const boost::asio::ip::address ipaddr = endp.endpoint().address();
			if (ipaddr.is_v4()) {
				std::cout << "  Ip: " << ipaddr.to_string() << "\n";
			}
		}
	}
}

InternalString_T GetHostName()
{
	return boost::asio::ip::host_name();
}

// Return first address found for name
Address_T GetAddrFromName(const InternalString_T name, IOService* const ioServ)
{
	boost::asio::io_context* const ioService = BoostIOServiceImpl::get(ioServ);
	if (ioService == nullptr) { return ""; }

	boost::asio::ip::tcp::resolver resolver(*ioService);
	boost::system::error_code ec;
	boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(name.c_str(), "", ec);

	if (ec) { return name; }

	for (auto const& endp : endpoints) {
        const boost::asio::ip::address ipaddr = endp.endpoint().address();
        if (ipaddr.is_v4()) { return ipaddr.to_string(); }
    }
	return name;
}

Address_T GetAddrFromNameEx(const InternalString_T localInterface, IOService* const ioServ)
{
	Address_T subnet = localInterface;
	Address_T mask = "";

	Address_T::size_type index = localInterface.find("/");
	if (index != Address_T::npos) {
		subnet = localInterface.substr(0, index);
		mask = localInterface.substr(index + 1);
	}

	if (subnet.size() > 0) {
		subnet = GetAddrFromName(subnet, ioServ);
	}

	if (index != Address_T::npos) {
		subnet += "/";
		subnet += mask;
	}

	return subnet;
}

}
#endif // REPLACE_TRANSPORT_LAYER
