/**
*
* Copyright (C) 2019-2024 Lennart Andersson.
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

package ops;

import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.Enumeration;
import java.util.ArrayList;
import java.util.Arrays;

public class NetworkSupport
{
  public static String[] GetHostAddresses(String name)
  {
      ArrayList<String> al = new ArrayList<>();
      try
      {
          InetAddress[] ads = InetAddress.getAllByName(name);
          for (InetAddress iAddr : ads) {
              if (iAddr instanceof Inet4Address) {
                  al.add(iAddr.getHostAddress());
              }
          }
      }
      catch (Exception e)
      {
      }

      return al.toArray(new String[al.size()]);
  }

  public static String GetHostAddress(String name)
  {
      String[] ads = GetHostAddresses(name);
      if (ads.length > 0) return GetHostAddresses(name)[0];
      return name;
  }

  public static String GetHostAddressEx(String localInterface)
  {
    String subnetIp = localInterface;
    String subnetMask = "";

    int index = subnetIp.indexOf('/');
    if (index > 0)
    {
        subnetIp = localInterface.substring(0, index);
        subnetMask = localInterface.substring(index + 1);
    }

    if (subnetIp.length() > 0)
    {
        subnetIp = GetHostAddress(subnetIp);
    }

    if (index > 0) { return subnetIp + "/" + subnetMask; }

    return subnetIp;
  }

  // If argument contains a "/" we assume it is on the form:  subnet-address/subnet-mask
  // e.g "192.168.10.0/255.255.255.0" or "192.168.10.0/24"
  // In that case we loop over all interfaces and take the first one that matches
  // i.e. the one whos interface address is on the subnet
  public static String DoSubnetTranslation(String ip)
  {
      int index = ip.indexOf('/');
      if (index < 0) return ip;

      String subnetIp = ip.substring(0, index);
      String subnetMask = ip.substring(index + 1);

      try
      {
          byte[] bip = InetAddress.getByName(subnetIp).getAddress();
          byte[] bmask;

          if (subnetMask.length() <= 2) {
              // Expand to the number of bits given
              long bitmask = Integer.parseInt(subnetMask);
              bitmask = (((1 << bitmask)-1) << (32 - bitmask)) & 0xFFFFFFFF;
              bmask = new byte[] {
                (byte)(bitmask >>> 24),
                (byte)(bitmask >>> 16),
                (byte)(bitmask >>> 8),
                (byte)bitmask};
          } else {
              InetAddress ipAddress = InetAddress.getByName(subnetMask);
              bmask = ipAddress.getAddress();
          }

          for (int j = 0; j < bip.length; j++) bip[j] = (byte)((int)bip[j] & (int)bmask[j]);

          Enumeration<NetworkInterface> nets = NetworkInterface.getNetworkInterfaces();
          for (NetworkInterface netint : java.util.Collections.list(nets))
          {
              java.util.List<java.net.InterfaceAddress> ifAddresses = netint.getInterfaceAddresses();
              for (java.net.InterfaceAddress ifAddress : ifAddresses) {
                  if (ifAddress.getAddress() instanceof Inet4Address) {
                      byte[] addr = ifAddress.getAddress().getAddress();
                      for (int j = 0; j < addr.length; j++) addr[j] = (byte)((int)addr[j] & (int)bmask[j]);

                      boolean eq = true;
                      for (int j = 0; j < addr.length; j++) eq = eq & (addr[j] == bip[j]);

                      if (eq) {
                          // split "hostname/127.0.0.1/8 [0.255.255.255]"
                          String s[] = ifAddress.toString().split("/");
                          return s[1];
                      }
                  }
              }
          }
      }
      catch (Exception e)
      {
      }
      return subnetIp;
  }

  public static boolean IsValidNodeAddress(String ip)
  {
      try
      {
          if (ip.equals("")) return false;
          byte[] bip = InetAddress.getByName(ip).getAddress();

          long Ip = 0;
          Ip |= ((long)bip[0] & 0xFF) << 24;
          Ip |= ((long)bip[1] & 0xFF) << 16;
          Ip |= ((long)bip[2] & 0xFF) << 8;
          Ip |= ((long)bip[3] & 0xFF);

          //System.out.println("  " + ip + " --> " + Ip);

          if (Ip == 0) return false;
          if (Ip >= 0xE0000000L) return false;  // Skip multicast and above
          return true;
      } catch (Exception e) {
          return false;
      }
  }

  public static boolean IsMyNodeAddress(String ip)
  {
      try
      {
          if (ip.equals("")) return false;
          InetAddress nodeaddr = InetAddress.getByName(ip);
          if (nodeaddr.isLoopbackAddress()) return true;

          Enumeration<NetworkInterface> nets = NetworkInterface.getNetworkInterfaces();
          for (NetworkInterface netint : java.util.Collections.list(nets))
          {
              java.util.List<java.net.InterfaceAddress> ifAddresses = netint.getInterfaceAddresses();
              for (java.net.InterfaceAddress ifAddress : ifAddresses) {
                  if (ifAddress.getAddress() instanceof Inet4Address) {
                    //String s[] = ifAddress.toString().split("/");
                    //System.out.println("  >> " + s[1]);
                    if (nodeaddr.equals(ifAddress.getAddress())) return true;
                  }
              }
          }
          return false;
      }
      catch (Exception e) {
          return false;
      }
  }

}
