using System;
using System.Net;
using System.Net.Sockets;

namespace Ops
{
	public class InetAddress
	{
		internal IPAddress addr;

        public InetAddress(string addr)
		{
			this.addr = IPAddress.Parse(addr);
		}

        public InetAddress(IPAddress addr)
		{
			this.addr = addr;
		}

		public bool IsAnyLocalAddress()
		{
			return IPAddress.IsLoopback(addr);
		}

		public bool Equals(InetAddress addr)
		{
			return this.addr.ToString().Equals( addr.ToString());
		}

		public bool Equals(string addr)
		{
			return this.addr.ToString().Equals( addr.ToString());
		}

		public override string ToString()
		{
			return addr.ToString ();
		}

		public override bool Equals(object obj)
		{
			return Equals (obj.ToString());
		}

        public IPAddress GetIPAddress()
        {
            return this.addr;
        }

        public string GetHostAddress()
		{
			return ToString();
		}

		public override int GetHashCode()
		{
			return base.GetHashCode();
		}

        public ulong ToNum()
        {
            byte[] bip = addr.GetAddressBytes();
            ulong Ip = 0;
            Ip |= ((ulong)bip[0]) << 24;
            Ip |= ((ulong)bip[1]) << 16;
            Ip |= ((ulong)bip[2]) << 8;
            Ip |= ((ulong)bip[3]);
            return Ip;
        }

        public static string GetByName(string name)
		{
            if ((name.Length > 0) && (Char.IsDigit(name[0]))) return name;

            try
            {
                IPHostEntry hostInfo = Dns.GetHostEntry(name);
                for (int index = 0; index < hostInfo.AddressList.Length; index++)
                {
                    if (hostInfo.AddressList[index].AddressFamily == AddressFamily.InterNetwork)
                    {
                        return hostInfo.AddressList[index].ToString();
                    }
                }
            } catch {
            }

            return name;
        }

        public static string GetHostAddressEx(string localInterface)
        {
            string subnetIp = localInterface;
            string subnetMask = "";

            int index = subnetIp.IndexOf('/');
            if (index > 0)
            {
                subnetIp = localInterface.Substring(0, index);
                subnetMask = localInterface.Substring(index);
            }

            if (subnetIp.Length > 0)
            {
                subnetIp = GetByName(subnetIp);
            }

            return subnetIp + subnetMask;
        }

        public static System.Net.IPAddress[] GetHostAddresses(string hostString)
        {
            try
            {
                return Dns.GetHostAddresses(hostString);
            } catch {
                return new System.Net.IPAddress[0];
            }
        }

        // If argument contains a "/" we assume it is on the form:  subnet-address/subnet-mask
        // e.g "192.168.10.0/255.255.255.0" or "192.168.10.0/24"
        // In that case we loop over all interfaces and take the first one that matches
        // i.e. the one whos interface address is on the subnet
        public static string DoSubnetTranslation(string ip)
        {
            int index = ip.IndexOf('/');
            if (index < 0) return ip;

            string subnetIp = ip.Substring(0, index);
            string subnetMask = ip.Substring(index + 1);

            byte[] bip = System.Net.IPAddress.Parse(subnetIp).GetAddressBytes();
            byte[] bmask;

            if (subnetMask.Length <= 2)
            {
                // Expand to the number of bits given
                int numBits = int.Parse(subnetMask);
                long binMask = (((1 << numBits) - 1) << (32 - numBits)) & 0xFFFFFFFF;

                bmask = new byte[4];
                bmask[0] = (byte)((binMask >> 24) & 0xFF);
                bmask[1] = (byte)((binMask >> 16) & 0xFF);
                bmask[2] = (byte)((binMask >> 8) & 0xFF);
                bmask[3] = (byte)((binMask) & 0xFF);
            }
            else
            {
                bmask = System.Net.IPAddress.Parse(subnetMask).GetAddressBytes();
            }

            for (int j = 0; j < bip.Length; j++) bip[j] = (byte)((int)bip[j] & (int)bmask[j]);

            System.Net.IPAddress[] ads = GetHostAddresses("");
            foreach (System.Net.IPAddress address in ads)
            {
                if (address.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork)
                {
                    byte[] addr = address.GetAddressBytes();
                    for (int j = 0; j < addr.Length; j++) addr[j] = (byte)((int)addr[j] & (int)bmask[j]);

                    bool eq = true;
                    for (int j = 0; j < addr.Length; j++) eq = eq & (addr[j] == bip[j]);

                    if (eq)
                    {
                        return address.ToString();
                    }
                }
            }

            ////            System.Net.NetworkInformation.IPGlobalProperties computerProperties = System.Net.NetworkInformation.IPGlobalProperties.GetIPGlobalProperties();
            //System.Net.NetworkInformation.NetworkInterface[] nics = System.Net.NetworkInformation.NetworkInterface.GetAllNetworkInterfaces();

            //if (nics != null && nics.Length > 0)
            //{
            //    foreach (System.Net.NetworkInformation.NetworkInterface adapter in nics)
            //    {
            //        System.Net.NetworkInformation.IPInterfaceProperties properties = adapter.GetIPProperties();
            //        System.Net.NetworkInformation.UnicastIPAddressInformationCollection uniCast = properties.UnicastAddresses;
            //        if (uniCast == null) continue;

            //        foreach (System.Net.NetworkInformation.UnicastIPAddressInformation uni in uniCast)
            //        {
            //            if (uni.Address.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork) //IPV4
            //            {
            //                byte[] addr = uni.Address.GetAddressBytes();
            //                for (int j = 0; j < addr.Length; j++) addr[j] = (byte)((int)addr[j] & (int)bmask[j]);

            //                bool eq = true;
            //                for (int j = 0; j < addr.Length; j++) eq = eq & (addr[j] == bip[j]);

            //                if (eq)
            //                {
            //                    return uni.Address.ToString();
            //                }
            //            }
            //        }
            //    }
            //}
            return subnetIp;

            //// This only works on Vista and later
            //System.Net.NetworkInformation.IPGlobalProperties gp = System.Net.NetworkInformation.IPGlobalProperties.GetIPGlobalProperties();
            //System.Net.NetworkInformation.UnicastIPAddressInformationCollection x = gp.GetUnicastAddresses();
            //for (int i = 0; i < x.Count; i++)
            //{
            //    if (x[i].Address.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork) //IPV4
            //    {
            //        byte[] addr = x[i].Address.GetAddressBytes();
            //        byte[] mask = x[i].IPv4Mask.GetAddressBytes();
            //        for (int j = 0; j < addr.Length; j++) addr[j] = (byte)((int)addr[j] & (int)mask[j]);
            //        string Subnet = new System.Net.IPAddress(addr).ToString();

            //        if (Subnet.Equals(subnetIp))
            //        {
            //            return x[i].Address.ToString();
            //        }
            //    }
            //}
            //return subnetIp;
        }

        // Return true if a valid node address
        public static bool IsValidNodeAddress(string addr)
        {
            try
            {
                InetAddress ad = new InetAddress(addr);
                ulong Ip = ad.ToNum();
                if (Ip == 0) return false;
                if (Ip >= 0xE0000000) return false;  // Skip multicast and above
                return true;
            } catch {
                return false;
            }
        }

        public static bool IsMyNodeAddress(String addr)
        {
            try
            {
                InetAddress nodeaddr = new InetAddress(addr);
                if (nodeaddr.IsAnyLocalAddress()) return true;

                System.Net.NetworkInformation.NetworkInterface[] nics = System.Net.NetworkInformation.NetworkInterface.GetAllNetworkInterfaces();

                if (nics != null && nics.Length > 0)
                {
                    foreach (System.Net.NetworkInformation.NetworkInterface adapter in nics)
                    {
                        System.Net.NetworkInformation.IPInterfaceProperties properties = adapter.GetIPProperties();
                        System.Net.NetworkInformation.UnicastIPAddressInformationCollection uniCast = properties.UnicastAddresses;
                        if (uniCast == null) continue;

                        foreach (System.Net.NetworkInformation.UnicastIPAddressInformation uni in uniCast)
                        {
                            if (uni.Address.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork) //IPV4
                            {
                                if (nodeaddr.Equals(uni.Address)) return true;
                            }
                        }
                    }
                }
                return false;
            }
            catch
            {
                return false;
            }
        }

    }
}
