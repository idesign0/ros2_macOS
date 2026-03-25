/*
 * Copyright (C) 2014 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#include <errno.h>

#ifdef _WIN32
  #include <Winsock2.h>
  #include <iphlpapi.h>
  #include <windows.h>
  #include <Lmcons.h>
#else
  #include <arpa/inet.h>
  #include <net/if.h>
  #include <netdb.h>
  #include <pwd.h>
  #include <unistd.h>
#endif

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

#include "gz/transport/config.hh"
#include "gz/transport/Helpers.hh"
#include "gz/transport/NetUtils.hh"
#include "gz/transport/Uuid.hh"

#ifdef HAVE_IFADDRS
# include <ifaddrs.h>
# include <sys/ioctl.h>
#endif

using namespace gz;

namespace gz::transport
{
inline namespace GZ_TRANSPORT_VERSION_NAMESPACE
{
  /// \brief Get the preferred local IP address.
  /// Note that we don't consider private IP addresses.
  /// \param[out] _ip The preferred local IP address.
  /// \return true if a public local IP was found or false otherwise.
  bool preferredPublicIP(std::string &_ip)
  {
    char host[1024];
    memset(host, 0, sizeof(host));
    if (gethostname(host, sizeof(host) - 1) != 0)
      return false;

    // We don't want "localhost" to be our hostname.
    if (!strlen(host) || !strcmp("localhost", host))
      return false;

    std::string hostIP;
    const std::string kPrefix = "127.0.";
    if ((hostnameToIp(host, hostIP) != 0) || isPrivateIP(hostIP.c_str()) ||
        hostIP.compare(0, kPrefix.size(), kPrefix) == 0)
    {
      return false;
    }

    // Get the complete list of compatible interfaces.
    auto interfaces = determineInterfaces();

    // Make sure that this interface is compatible with Discovery.
    if (std::find(interfaces.begin(), interfaces.end(), hostIP) ==
          interfaces.end())
    {
      return false;
    }

    _ip = hostIP;
    return true;
  }

  //////////////////////////////////////////////////
  bool isPrivateIP(const char *_ip)
  {
    bool b = !strncmp("192.168", _ip, 7) || !strncmp("10.", _ip, 3) ||
             !strncmp("169.254", _ip, 7);
    return b;
  }

  //////////////////////////////////////////////////
  int hostnameToIp(char *_hostname, std::string &_ip)
  {
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ((he = gethostbyname(_hostname)) == nullptr)
      return 1;

    addr_list = (struct in_addr **) he->h_addr_list;

    for (i = 0; addr_list[i] != nullptr; ++i)
    {
      // Return the first one;
      _ip = std::string(inet_ntoa(*addr_list[i]));
      return 0;
    }

    return 1;
  }

  //////////////////////////////////////////////////
  std::string determineHost()
  {
    // First, did the user set GZ_IP?
    std::string gzIp;
    if (env("GZ_IP", gzIp) && !gzIp.empty())
    {
      return gzIp;
    }

    // Second, try the preferred local and public IP address.
    std::string hostIP;
    if (preferredPublicIP(hostIP))
      return hostIP;

    // Third, fall back on interface search, which will yield an IP address
    auto interfaces = determineInterfaces();
    for (const auto &ip : interfaces)
    {
      // Return the first public IP address.
      if (!isPrivateIP(ip.c_str()))
        return ip;
    }

    return interfaces.front();
  }

  //////////////////////////////////////////////////
  std::vector<std::string> determineInterfaces()
  {
#ifdef HAVE_IFADDRS
    std::vector<std::string> result;
    struct ifaddrs *ifa = nullptr, *ifp = NULL;
    int rc;
    if ((rc = getifaddrs(&ifp)) < 0)
    {
      std::cerr << "error in getifaddrs: " << strerror(rc) << std::endl;
      exit(-1);
    }

#if defined(SIOCGIFINDEX)
    // Open a socket to use IOCTL later.
    int sockfd;
    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
    {
      std::cerr << "Unable to open a socket for using IOCTL" << std::endl;
    }

    // Stores a set of SIOCGIFINDEX.
    std::unordered_set<int> realIdx = {};
#endif

    for (ifa = ifp; ifa; ifa = ifa->ifa_next)
    {
      char ip_[200];
      socklen_t salen;
      std::string interface;
      if (!ifa->ifa_addr)
        continue;  // evidently this interface has no ip address
      if (ifa->ifa_addr->sa_family == AF_INET)
        salen = sizeof(struct sockaddr_in);
      else if (ifa->ifa_addr->sa_family == AF_INET6)
        salen = sizeof(struct sockaddr_in6);
      // Unknown family.
      else
        continue;

#if defined(SIOCGIFINDEX)
      // We don't want to return multiple subinterfaces, as you won't be able
      // to join the multicast group in all of them (using IP_ADD_MEMBERSHIP).
      // All the subinterfaces share the same SIOCGIFINDEX.
      struct ifreq ifIdx;
      memset(&ifIdx, 0, sizeof(struct ifreq));
      strncpy(ifIdx.ifr_name, ifa->ifa_name, IFNAMSIZ-1);
      // cppcheck-suppress ConfigurationNotChecked
      if (ioctl(sockfd, SIOCGIFINDEX, &ifIdx) < 0)
      {
        std::cerr << "Error requesting SIOCGIFINDEX for ["
                  << ifa->ifa_name << "]" << std::endl;
        continue;
      }

      // If we already have the same SIOCGIFINDEX, ignore it (subinterface).
      if (realIdx.find(ifIdx.ifr_ifindex) != realIdx.end())
        continue;

      realIdx.insert(ifIdx.ifr_ifindex);
#endif

      if (getnameinfo(ifa->ifa_addr, salen, ip_, sizeof(ip_), nullptr, 0,
                      NI_NUMERICHOST) < 0)
      {
        std::cerr << "getnameinfo couldn't get the ip of interface "
                  << ifa->ifa_name << std::endl;
        continue;
      }
      // prefer non-loopback IPs
      if (!strcmp("127.0.0.1", ip_) || strchr(ip_, ':'))
        continue;  // ignore loopback unless we have no other choice
      // Does not support multicast.
      if (!(ifa->ifa_flags & IFF_MULTICAST))
        continue;
      // Is not running.
      if (!(ifa->ifa_flags & IFF_UP))
        continue;
      // Any other interface.
      interface = std::string(ip_);

      // Add the new interface if it's new and unique.
      if (!interface.empty() &&
          std::find(std::begin(result), std::end(result),
            interface) == std::end(result))
      {
        result.push_back(interface);
      }
    }
    freeifaddrs(ifp);
    if (result.empty())
    {
      std::cerr <<
        "Couldn't find a preferred IP via the getifaddrs() call; "
        "I'm assuming that your IP "
        "address is 127.0.0.1.  This should work for local processes, "
        "but will almost certainly not work if you have remote processes."
        "Report to the disc-zmq development team to seek a fix." << std::endl;
      return {"127.0.0.1"};
    }
    return result;
#elif defined(_WIN32)
    std::vector<std::string> result;
    // Establish our default return value, in case everything below fails.
    std::string ret_addr("127.0.0.1");
    // Look up our address.
    ULONG outBufLen = 0;
    PIP_ADAPTER_ADDRESSES addrs = NULL;
    // Not sure whether these are the right flags, but they work for
    // me on Windows 7
    ULONG flags = (GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
                   GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME);
    // The first time, it'll fail; we're asking how much space is needed to
    // store the result.
    GetAdaptersAddresses(AF_INET, flags, NULL, addrs, &outBufLen);
    // Allocate the required space.
    addrs = new IP_ADAPTER_ADDRESSES[outBufLen];
    ULONG ret;
    // Now the call should succeed.
    if ((ret = GetAdaptersAddresses(AF_INET, flags, NULL, addrs, &outBufLen)) ==
      NO_ERROR)
    {
      // Iterate over all returned adapters, arbitrarily sticking with the
      // last non-loopback one that we find.
      for (PIP_ADAPTER_ADDRESSES curr = addrs; curr; curr = curr->Next)
      {
        // This adapter does not support multicast.
        if (curr->Flags & IP_ADAPTER_NO_MULTICAST)
          continue;

        // The interface is not running.
        if (curr->OperStatus != IfOperStatusUp)
          continue;

        // Iterate over all unicast addresses for this adapter
        for (PIP_ADAPTER_UNICAST_ADDRESS unicast = curr->FirstUnicastAddress;
             unicast; unicast = unicast->Next)
        {
          // Cast to get an IPv4 numeric address (the AF_INET flag used above
          // ensures that we're only going to get IPv4 address here).
          sockaddr_in* sockaddress =
            reinterpret_cast<sockaddr_in*>(unicast->Address.lpSockaddr);
          // Make it a dotted quad
          char ipv4_str[3*4+3+1];
          gz_sprintf(ipv4_str, "%d.%d.%d.%d",
            sockaddress->sin_addr.S_un.S_un_b.s_b1,
            sockaddress->sin_addr.S_un.S_un_b.s_b2,
            sockaddress->sin_addr.S_un.S_un_b.s_b3,
            sockaddress->sin_addr.S_un.S_un_b.s_b4);
          // Ignore loopback address (that's our default anyway)
          if (!strcmp(ipv4_str, "127.0.0.1"))
            continue;
          ret_addr = ipv4_str;

          // Add the new interface if it's new and unique.
          if (!ret_addr.empty() &&
          std::find(std::begin(result),
                    std::end(result), ret_addr) == std::end(result))
          {
            result.push_back(std::string(ret_addr));
          }
        }
      }
    }
    else
      std::cerr << "GetAdaptersAddresses() failed: " << ret << std::endl;
    delete [] addrs;
    if (result.empty() || (result.size() == 1 && result.at(0) == "127.0.0.1"))
    {
      std::cerr <<
        "Couldn't find a preferred IP via the GetAdaptersAddresses() call; "
        "I'm assuming that your IP "
        "address is 127.0.0.1.  This should work for local processes, "
        "but will almost certainly not work if you have remote processes."
        "Report to the disc-zmq development team to seek a fix." << std::endl;
    }
    return result;
#else
    // @todo(anyone) Fix IP determination in the case where getifaddrs() isn't
    // available.
    std::cerr <<
      "You don't have the getifaddrs() call; I'm assuming that your IP "
      "address is 127.0.0.1.  This should work for local processes, "
      "but will almost certainly not work if you have remote processes."
      "Report to the disc-zmq development team to seek a fix." << std::endl;
    return {"127.0.0.1"};
#endif
  }

  //////////////////////////////////////////////////
  std::string hostname()
  {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    char hostname[200 + 1];
    gethostname(hostname, sizeof hostname);

#ifdef _WIN32
    WSACleanup();
#endif

    return hostname;
  }

  //////////////////////////////////////////////////
  std::string username()
  {
    char buffer[200 + 1];
    size_t bufferLen = sizeof(buffer);
#ifdef _WIN32
    DWORD usernameLen = static_cast<DWORD>(bufferLen);
    GetUserName(buffer, &usernameLen);
    return buffer;
#else
    // First, try to get the username through the standard environment variable
    // for it.
    const auto userVariable = std::getenv("USER");
    if (userVariable)
    {
      return userVariable;
    }

    // No USER variable, request it from the system.
    struct passwd pd;
    struct passwd *pdResult;
    Uuid uuid;
    std::string result = "error-" + uuid.ToString();

    // Get the username via getpwuid_r, and retry on failure.
    // Refer to the manpage for error cases:
    // https://linux.die.net/man/3/getpwuid_r
    //
    // There can be a general I/O errors, which might get resolved with
    // repeated attempts.
    for (int count = 0; count < 10; ++count)
    {
      errno = 0;
      if (getpwuid_r(getuid(), &pd, buffer, bufferLen, &pdResult) == 0)
      {
        if (pdResult != nullptr)
        {
          // Success. Store the username and break.
          result = pd.pw_name;
          break;
        }
      }
    }

    return result;
#endif
  }
}  // namespace GZ_TRANSPORT_VERSION_NAMESPACE
}  // namespace gz::transport
