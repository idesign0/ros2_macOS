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

#ifndef GZ_TRANSPORT_DISCOVERY_HH_
#define GZ_TRANSPORT_DISCOVERY_HH_
#include <errno.h>
#include <string.h>

#ifdef _WIN32
  // For socket(), connect(), send(), and recv().
  #include <Winsock2.h>
  #include <Ws2def.h>
  #include <Ws2ipdef.h>
  #include <Ws2tcpip.h>
  // Type used for raw data on this platform.
  using raw_type = char;
#else
  // For data types
  #include <sys/types.h>
  // For socket(), connect(), send(), and recv()
  #include <sys/socket.h>
  // For gethostbyname()
  #include <netdb.h>
  // For inet_addr()
  #include <arpa/inet.h>
  // For close()
  #include <unistd.h>
  // For sockaddr_in
  #include <netinet/in.h>
  // Type used for raw data on this platform
  using raw_type = void;
#endif

#ifdef _WIN32
  #pragma warning(push, 0)
#endif
#ifdef _WIN32
  #pragma warning(pop)
  // Suppress "decorated name length exceed" warning in STL.
  #pragma warning(disable: 4503)
  // Suppress "depreted API warnings" in WINSOCK.
  #pragma warning(disable: 4996)
#endif

#include <gz/msgs/discovery.pb.h>

#include <algorithm>
#include <condition_variable>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <gz/msgs/Utility.hh>

#include "gz/transport/config.hh"
#include "gz/transport/Export.hh"
#include "gz/transport/Helpers.hh"
#include "gz/transport/NetUtils.hh"
#include "gz/transport/Publisher.hh"
#include "gz/transport/TopicStorage.hh"
#include "gz/transport/TransportTypes.hh"

namespace gz
{
  namespace transport
  {
    // Inline bracket to help doxygen filtering.
    inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
    /// \brief Options for sending discovery messages.
    enum class DestinationType
    {
      /// \brief Send data via unicast only.
      UNICAST,
      /// \brief Send data via multicast only.
      MULTICAST,
      /// \brief Send data via unicast and multicast.
      ALL
    };

    //
    /// \internal
    /// \brief Discovery helper function to poll sockets.
    /// \param[in] _sockets Sockets on which to listen.
    /// \param[in] _timeout Length of time to poll (milliseconds).
    /// \return True if the sockets received a reply.
    bool GZ_TRANSPORT_VISIBLE pollSockets(
      const std::vector<int> &_sockets,
      const int _timeout);

    /// \class Discovery Discovery.hh gz/transport/Discovery.hh
    /// \brief A discovery class that implements a distributed topic discovery
    /// protocol. It uses UDP multicast for sending/receiving messages and
    /// stores updated topic information. The discovery clients can request
    /// the discovery of a topic or the advertisement of a local topic. The
    /// discovery uses heartbeats to track the state of other peers in the
    /// network. The discovery clients can register callbacks to detect when
    /// new topics are discovered or topics are no longer available.
    template<typename Pub>
    class Discovery
    {
      /// \brief Constructor.
      /// \param[in] _pUuid This discovery instance will run inside a
      /// transport process. This parameter is the transport process' UUID.
      /// \param[in] _ip IP address used for discovery traffic.
      /// \param[in] _port UDP port used for discovery traffic.
      /// \param[in] _verbose true for enabling verbose mode.
      public: Discovery(const std::string &_pUuid,
                        const std::string &_ip,
                        const int _port,
                        const bool _verbose = false)
        : multicastGroup(_ip),
          port(_port),
          hostAddr(determineHost()),
          pUuid(_pUuid),
          silenceInterval(kDefSilenceInterval),
          activityInterval(kDefActivityInterval),
          heartbeatInterval(kDefHeartbeatInterval),
          connectionCb(nullptr),
          disconnectionCb(nullptr),
          verbose(_verbose),
          initialized(false),
          numHeartbeatsUninitialized(0),
          exit(false),
          enabled(false)
      {
        std::string gzIp;
        if (env("GZ_IP", gzIp) && !gzIp.empty())
        {
          this->hostInterfaces = {gzIp};
        }
        else
        {
          // Get the list of network interfaces in this host.
          this->hostInterfaces = determineInterfaces();
        }

#ifdef _WIN32
        WORD wVersionRequested;
        WSADATA wsaData;

        // Request WinSock v2.2.
        wVersionRequested = MAKEWORD(2, 2);
        // Load WinSock DLL.
        if (WSAStartup(wVersionRequested, &wsaData) != 0)
        {
          std::cerr << "Unable to load WinSock DLL" << std::endl;
          return;
        }
#endif
        for (const auto &netIface : this->hostInterfaces)
        {
          auto succeed = this->RegisterNetIface(netIface);

          // If the IP address that we're selecting as the main IP address of
          // the host is invalid, we change it to 127.0.0.1 .
          // This is probably because GZ_IP is set to a wrong value.
          if (netIface == this->hostAddr && !succeed)
          {
            this->RegisterNetIface("127.0.0.1");
            std::cerr << "Did you set the environment variable GZ_IP with a "
                      << "correct IP address? " << std::endl
                      << "  [" << netIface << "] seems an invalid local IP "
                      << "address." << std::endl
                      << "  Using 127.0.0.1 as hostname." << std::endl;
            this->hostAddr = "127.0.0.1";
          }
        }

        // Socket option: SO_REUSEADDR. This options is used only for receiving
        // data. We can reuse the same socket for receiving multicast data from
        // multiple interfaces. We will use the socket at position 0 for
        // receiving data.
        int reuseAddr = 1;
        if (setsockopt(this->sockets.at(0), SOL_SOCKET, SO_REUSEADDR,
            reinterpret_cast<const char *>(&reuseAddr), sizeof(reuseAddr)) != 0)
        {
          std::cerr << "Error setting socket option (SO_REUSEADDR)."
                    << std::endl;
          return;
        }

#ifdef SO_REUSEPORT
        // Socket option: SO_REUSEPORT. This options is used only for receiving
        // data. We can reuse the same socket for receiving multicast data from
        // multiple interfaces. We will use the socket at position 0 for
        // receiving data.
        int reusePort = 1;
        // cppcheck-suppress ConfigurationNotChecked
        if (setsockopt(this->sockets.at(0), SOL_SOCKET, SO_REUSEPORT,
            reinterpret_cast<const char *>(&reusePort), sizeof(reusePort)) != 0)
        {
          std::cerr << "Error setting socket option (SO_REUSEPORT)."
                    << std::endl;
          return;
        }
#endif
        // Bind the first socket to the discovery port.
        sockaddr_in localAddr;
        memset(&localAddr, 0, sizeof(localAddr));
        localAddr.sin_family = AF_INET;
        localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        localAddr.sin_port = htons(static_cast<u_short>(this->port));

        if (bind(this->sockets.at(0),
          reinterpret_cast<sockaddr *>(&localAddr), sizeof(sockaddr_in)) < 0)
        {
          std::cerr << "Binding to a local port failed." << std::endl;
          return;
        }

        // Set 'mcastAddr' to the multicast discovery group.
        memset(&this->mcastAddr, 0, sizeof(this->mcastAddr));
        this->mcastAddr.sin_family = AF_INET;
        this->mcastAddr.sin_addr.s_addr =
          inet_addr(this->multicastGroup.c_str());
        this->mcastAddr.sin_port = htons(static_cast<u_short>(this->port));

        std::vector<std::string> relays;
        std::string gzRelay;
        if (env("GZ_RELAY", gzRelay) && !gzRelay.empty())
        {
          relays = transport::split(gzRelay, ':');
        }

        // Register all unicast relays.
        for (auto const &relayAddr : relays)
          this->AddRelayAddress(relayAddr);

        if (this->verbose)
          this->PrintCurrentState();
      }

      /// \brief Destructor.
      public: virtual ~Discovery()
      {
        // Tell the service thread to terminate.
        this->exitMutex.lock();
        this->exit = true;
        this->exitMutex.unlock();

        // Wait for the service threads to finish before exit.
        if (this->threadReception.joinable())
          this->threadReception.join();

        // Broadcast a BYE message to trigger the remote cancellation of
        // all our advertised topics.
        this->SendMsg(DestinationType::ALL, msgs::Discovery::BYE,
          Publisher("", "", this->pUuid, "", AdvertiseOptions()));

        // Close sockets.
        for (const auto &sock : this->sockets)
        {
#ifdef _WIN32
          closesocket(sock);
          WSACleanup();
#else
          close(sock);
#endif
        }
      }

      /// \brief Start the discovery service. You probably want to register the
      /// callbacks for receiving discovery notifications before starting the
      /// service.
      public: void Start()
      {
        {
          std::lock_guard<std::mutex> lock(this->mutex);

          // The service is already running.
          if (this->enabled)
            return;

          this->enabled = true;
        }

        auto now = std::chrono::steady_clock::now();
        this->timeNextHeartbeat = now;
        this->timeNextActivity = now;

        // Start the thread that receives discovery information.
        this->threadReception = std::thread(&Discovery::RecvMessages, this);
      }

      /// \brief Advertise a new message.
      /// \param[in] _publisher Publisher's information to advertise.
      /// \return True if the method succeed or false otherwise
      /// (e.g. if the discovery has not been started).
      public: bool Advertise(const Pub &_publisher)
      {
        DiscoveryCallback<Pub> cb;

        {
          std::lock_guard<std::mutex> lock(this->mutex);

          if (!this->enabled)
            return false;

          // Add the addressing information (local publisher).
          if (!this->info.AddPublisher(_publisher))
            return false;

          cb = this->connectionCb;
        }

        if (cb)
          cb(_publisher);

        // Only advertise a message outside this process if the scope
        // is not 'Process'
        if (_publisher.Options().Scope() != Scope_t::PROCESS)
          this->SendMsg(DestinationType::ALL, msgs::Discovery::ADVERTISE,
              _publisher);

        return true;
      }

      /// \brief Request discovery information about a topic.
      /// When using this method, the user might want to use
      /// SetConnectionsCb() and SetDisconnectionCb(), that registers callbacks
      /// that will be executed when the topic address is discovered or when the
      /// node providing the topic is disconnected.
      /// \sa SetConnectionsCb.
      /// \sa SetDisconnectionsCb.
      /// \param[in] _topic Topic name requested.
      /// \return True if the method succeeded or false otherwise
      /// (e.g. if the discovery has not been started).
      public: bool Discover(const std::string &_topic) const
      {
        DiscoveryCallback<Pub> cb;
        bool found;
        Addresses_M<Pub> addresses;

        {
          std::lock_guard<std::mutex> lock(this->mutex);

          if (!this->enabled)
            return false;

          cb = this->connectionCb;
        }

        Pub pub;
        pub.SetTopic(_topic);
        pub.SetPUuid(this->pUuid);

        // Send a discovery request.
        this->SendMsg(DestinationType::ALL, msgs::Discovery::SUBSCRIBE, pub);

        {
          std::lock_guard<std::mutex> lock(this->mutex);
          found = this->info.Publishers(_topic, addresses);
        }

        if (found)
        {
          // I already have information about this topic.
          for (const auto &proc : addresses)
          {
            for (const auto &node : proc.second)
            {
              if (cb)
              {
                // Execute the user's callback for a service request. Notice
                // that we only execute one callback for preventing receive
                //  multiple service responses for a single request.
                cb(node);
              }
            }
          }
        }

        return true;
      }

      /// \brief Send the response to a SUBSCRIBERS_REQ message.
      /// \param[in] _pub Information to send.
      public: void SendSubscribersRep(const MessagePublisher &_pub) const
      {
        this->SendMsg(
          DestinationType::ALL, msgs::Discovery::SUBSCRIBERS_REP, _pub);
      }

      /// \brief Register a node from this process as a remote subscriber.
      /// \param[in] _pub Contains information about the subscriber.
      public: void Register(const MessagePublisher &_pub) const
      {
        this->SendMsg(
          DestinationType::ALL, msgs::Discovery::NEW_CONNECTION, _pub);
      }

      /// \brief Unregister a node from this process as a remote subscriber.
      /// \param[in] _pub Contains information about the subscriber.
      public: void Unregister(const MessagePublisher &_pub) const
      {
        this->SendMsg(
          DestinationType::ALL, msgs::Discovery::END_CONNECTION, _pub);
      }

      /// \brief Get the discovery information.
      /// \return Reference to the discovery information object.
      public: const TopicStorage<Pub> &Info() const
      {
        std::lock_guard<std::mutex> lock(this->mutex);
        return this->info;
      }

      /// \brief Get all the publishers' information known for a given topic.
      /// \param[in] _topic Topic name.
      /// \param[out] _publishers Publishers requested.
      /// \return True if the topic is found and there is at least one publisher
      public: bool Publishers(const std::string &_topic,
                              Addresses_M<Pub> &_publishers) const
      {
        std::lock_guard<std::mutex> lock(this->mutex);
        return this->info.Publishers(_topic, _publishers);
      }

      /// \brief Get all the subscribers' information known for a given topic.
      /// \param[in] _topic Topic name.
      /// \param[out] _subscribers All remote subscribers for this topic.
      /// \return True if the topic is found and there is at least one publisher
      public: bool RemoteSubscribers(const std::string &_topic,
                                     Addresses_M<Pub> &_subscribers) const
      {
        std::lock_guard<std::mutex> lock(this->mutex);
        return this->remoteSubscribers.Publishers(_topic, _subscribers);
      }

      /// \brief Unadvertise a new message. Broadcast a discovery
      /// message that will cancel all the discovery information for the topic
      /// advertised by a specific node.
      /// \param[in] _topic Topic name to be unadvertised.
      /// \param[in] _nUuid Node UUID.
      /// \return True if the method succeeded or false otherwise
      /// (e.g. if the discovery has not been started).
      public: bool Unadvertise(const std::string &_topic,
                               const std::string &_nUuid)
      {
        Pub inf;
        {
          std::lock_guard<std::mutex> lock(this->mutex);

          if (!this->enabled)
            return false;

          // Don't do anything if the topic is not advertised by any of my nodes
          if (!this->info.Publisher(_topic, this->pUuid, _nUuid, inf))
            return true;

          // Remove the topic information.
          this->info.DelPublisherByNode(_topic, this->pUuid, _nUuid);
        }

        // Only unadvertise a message outside this process if the scope
        // is not 'Process'.
        if (inf.Options().Scope() != Scope_t::PROCESS)
        {
          this->SendMsg(DestinationType::ALL,
              msgs::Discovery::UNADVERTISE, inf);
        }

        return true;
      }

      /// \brief Get the IP address of this host.
      /// \return A string with this host's IP address.
      public: std::string HostAddr() const
      {
        std::lock_guard<std::mutex> lock(this->mutex);
        return this->hostAddr;
      }

      /// \brief The discovery checks the validity of the topic information
      /// every 'activity interval' milliseconds.
      /// \sa SetActivityInterval.
      /// \return The value in milliseconds.
      public: unsigned int ActivityInterval() const
      {
        std::lock_guard<std::mutex> lock(this->mutex);
        return this->activityInterval;
      }

      /// \brief Each node broadcasts periodic heartbeats to keep its topic
      /// information alive in other nodes. A heartbeat message is sent after
      /// 'heartbeat interval' milliseconds.
      /// \sa SetHeartbeatInterval.
      /// \return The value in milliseconds.
      public: unsigned int HeartbeatInterval() const
      {
        std::lock_guard<std::mutex> lock(this->mutex);
        return this->heartbeatInterval;
      }

      /// \brief Get the maximum time allowed without receiving any discovery
      /// information from a node before canceling its entries.
      /// \sa SetSilenceInterval.
      /// \return The value in milliseconds.
      public: unsigned int SilenceInterval() const
      {
        std::lock_guard<std::mutex> lock(this->mutex);
        return this->silenceInterval;
      }

      /// \brief Set the activity interval.
      /// \sa ActivityInterval.
      /// \param[in] _ms New value in milliseconds.
      public: void SetActivityInterval(const unsigned int _ms)
      {
        std::lock_guard<std::mutex> lock(this->mutex);
        this->activityInterval = _ms;
      }

      /// \brief Set the heartbeat interval.
      /// \sa HeartbeatInterval.
      /// \param[in] _ms New value in milliseconds.
      public: void SetHeartbeatInterval(const unsigned int _ms)
      {
        std::lock_guard<std::mutex> lock(this->mutex);
        this->heartbeatInterval = _ms;
      }

      /// \brief Set the maximum silence interval.
      /// \sa SilenceInterval.
      /// \param[in] _ms New value in milliseconds.
      public: void SetSilenceInterval(const unsigned int _ms)
      {
        std::lock_guard<std::mutex> lock(this->mutex);
        this->silenceInterval = _ms;
      }

      /// \brief Register a callback to receive discovery connection events.
      /// Each time a new topic is connected, the callback will be executed.
      /// This version uses a free function as callback.
      /// \param[in] _cb Function callback.
      public: void ConnectionsCb(const DiscoveryCallback<Pub> &_cb)
      {
        std::lock_guard<std::mutex> lock(this->mutex);
        this->connectionCb = _cb;
      }

      /// \brief Register a callback to receive discovery disconnection events.
      /// Each time a topic is no longer active, the callback will be executed.
      /// This version uses a free function as callback.
      /// \param[in] _cb Function callback.
      public: void DisconnectionsCb(const DiscoveryCallback<Pub> &_cb)
      {
        std::lock_guard<std::mutex> lock(this->mutex);
        this->disconnectionCb = _cb;
      }

      /// \brief Register a callback to receive an event when a new remote
      /// node subscribes to a topic within this process.
      /// \param[in] _cb Function callback.
      public: void RegistrationsCb(const DiscoveryCallback<Pub> &_cb)
      {
        std::lock_guard<std::mutex> lock(this->mutex);
        this->registrationCb = _cb;
      }

      /// \brief Register a callback to receive an event when a remote
      /// node unsubscribes to a topic within this process.
      /// \param[in] _cb Function callback.
      public: void UnregistrationsCb(const DiscoveryCallback<Pub> &_cb)
      {
        std::lock_guard<std::mutex> lock(this->mutex);
        this->unregistrationCb = _cb;
      }

      /// \brief Register a callback to receive an event when a node requests
      /// the list of remote subscribers.
      /// \param[in] _cb Function callback.
      public: void SubscribersCb(const std::function<void()> &_cb)
      {
        std::lock_guard<std::mutex> lock(this->mutex);
        this->subscribersCb = _cb;
      }

      /// \brief Print the current discovery state.
      public: void PrintCurrentState() const
      {
        std::lock_guard<std::mutex> lock(this->mutex);

        std::cout << "---------------" << std::endl;
        std::cout << std::boolalpha << "Enabled: "
                  << this->enabled << std::endl;
        std::cout << "Discovery state" << std::endl;
        std::cout << "\tUUID: " << this->pUuid << std::endl;
        std::cout << "Settings" << std::endl;
        std::cout << "\tActivity: " << this->activityInterval
                  << " ms." << std::endl;
        std::cout << "\tHeartbeat: " << this->heartbeatInterval
                  << "ms." << std::endl;
        std::cout << "\tSilence: " << this->silenceInterval
                  << " ms." << std::endl;
        std::cout << "Known information:" << std::endl;
        this->info.Print();

        // Used to calculate the elapsed time.
        Timestamp now = std::chrono::steady_clock::now();

        std::cout << "Activity" << std::endl;
        if (this->activity.empty())
          std::cout << "\t<empty>" << std::endl;
        else
        {
          for (auto &proc : this->activity)
          {
            // Elapsed time since the last update from this publisher.
            std::chrono::duration<double> elapsed = now - proc.second;

            std::cout << "\t" << proc.first << std::endl;
            std::cout << "\t\t" << "Since: " << std::chrono::duration_cast<
              std::chrono::milliseconds>(elapsed).count() << " ms. ago. "
              << std::endl;
          }
        }
        std::cout << "---------------" << std::endl;
      }

      /// \brief Get the list of topics currently advertised and subscribed
      /// in the network.
      /// \param[out] _topics List of advertised topics.
      public: void TopicList(std::vector<std::string> &_topics)
      {
        {
          std::lock_guard<std::mutex> lock(this->mutex);
          this->remoteSubscribers.Clear();
        }

        // Request the list of subscribers.
        Publisher pub("", "", this->pUuid, "", AdvertiseOptions());
        this->SendMsg(
          DestinationType::ALL, msgs::Discovery::SUBSCRIBERS_REQ, pub);

        this->WaitForInit();
        std::lock_guard<std::mutex> lock(this->mutex);
        this->info.TopicList(_topics);

        std::vector<std::string> remoteSubs;
        this->remoteSubscribers.TopicList(remoteSubs);

        // Add the remote subscribers
        for (auto const &t : remoteSubs)
        {
          if (std::find(_topics.begin(), _topics.end(), t) == _topics.end())
          {
            _topics.push_back(t);
          }
        }
      }

      /// \brief Check if ready/initialized. If not, then wait on the
      /// initializedCv condition variable.
      public: void WaitForInit() const
      {
        std::unique_lock<std::mutex> lk(this->mutex);

        if (!this->initialized)
        {
          this->initializedCv.wait(lk, [this]{return this->initialized;});
        }
      }

      /// \brief Check the validity of the topic information. Each topic update
      /// has its own timestamp. This method iterates over the list of topics
      /// and invalids the old topics.
      private: void UpdateActivity()
      {
        // The UUIDs of the processes that have expired.
        std::vector<std::string> uuids;

        // A copy of the disconnection callback.
        DiscoveryCallback<Pub> disconnectCb;

        Timestamp now = std::chrono::steady_clock::now();

        {
          std::lock_guard<std::mutex> lock(this->mutex);

          if (now < this->timeNextActivity)
            return;

          disconnectCb = this->disconnectionCb;

          for (auto it = this->activity.cbegin(); it != this->activity.cend();)
          {
            // Elapsed time since the last update from this publisher.
            auto elapsed = now - it->second;

            // This publisher has expired.
            if (std::chrono::duration_cast<std::chrono::milliseconds>
                 (elapsed).count() > this->silenceInterval)
            {
              // Remove all the info entries for this process UUID.
              this->info.DelPublishersByProc(it->first);

              uuids.push_back(it->first);

              // Remove the activity entry.
              this->activity.erase(it++);
            }
            else
              ++it;
          }

          this->timeNextActivity = std::chrono::steady_clock::now() +
            std::chrono::milliseconds(this->activityInterval);
        }

        if (!disconnectCb)
          return;

        // Notify without topic information. This is useful to inform the
        // client that a remote node is gone, even if we were not
        // interested in its topics.
        for (auto const &uuid : uuids)
        {
          Pub publisher;
          publisher.SetPUuid(uuid);
          disconnectCb(publisher);
        }
      }

      /// \brief Register a new relay address.
      /// \param[in] _ip New IP address.
      public: void AddRelayAddress(const std::string &_ip)
      {
        std::lock_guard<std::mutex> lock(this->mutex);
        // Sanity check: Make sure that this IP address is not already saved.
        for (auto const &addr : this->relayAddrs)
        {
          if (addr.sin_addr.s_addr == inet_addr(_ip.c_str()))
            return;
        }

        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(_ip.c_str());
        addr.sin_port = htons(static_cast<u_short>(this->port));

        this->relayAddrs.push_back(addr);
      }

      // \brief Gets this instance's relay addresses.
      // \return The list of relay addresses.
      public: std::vector<std::string> RelayAddresses() const
      {
        std::vector<std::string> result;

        std::lock_guard<std::mutex> lock(this->mutex);

        for (auto const &addr : this->relayAddrs) {
          result.push_back(inet_ntoa(addr.sin_addr));
        }

        return result;
      }

      /// \brief Broadcast periodic heartbeats.
      private: void UpdateHeartbeat()
      {
        Timestamp now = std::chrono::steady_clock::now();

        {
          std::lock_guard<std::mutex> lock(this->mutex);

          if (now < this->timeNextHeartbeat)
            return;
        }

        Publisher pub("", "", this->pUuid, "", AdvertiseOptions());
        this->SendMsg(DestinationType::ALL, msgs::Discovery::HEARTBEAT, pub);

        std::map<std::string, std::vector<Pub>> nodes;
        {
          std::lock_guard<std::mutex> lock(this->mutex);

          // Re-advertise topics that are advertised inside this process.
          this->info.PublishersByProc(this->pUuid, nodes);
        }

        for (const auto &topic : nodes)
        {
          for (const auto &node : topic.second)
          {
            this->SendMsg(DestinationType::ALL,
                msgs::Discovery::ADVERTISE, node);
          }
        }

        {
          std::lock_guard<std::mutex> lock(this->mutex);
          if (!this->initialized)
          {
            if (this->numHeartbeatsUninitialized == 2u)
            {
              // We consider discovery initialized after two heartbeat cycles.
              this->initialized = true;

              // Notify anyone waiting for the initialization phase to finish.
              this->initializedCv.notify_all();
            }
            ++this->numHeartbeatsUninitialized;
          }

          this->timeNextHeartbeat = std::chrono::steady_clock::now() +
            std::chrono::milliseconds(this->heartbeatInterval);
        }
      }

      /// \brief Calculate the next timeout. There are three main activities to
      /// perform by the discovery component:
      /// 1. Receive discovery messages.
      /// 2. Send heartbeats.
      /// 3. Maintain the discovery information up to date.
      ///
      /// Tasks (2) and (3) need to be checked at fixed intervals. This function
      /// calculates the next timeout to satisfy (2) and (3).
      /// \return A timeout (milliseconds).
      private: int NextTimeout() const
      {
        auto now = std::chrono::steady_clock::now();
        auto timeUntilNextHeartbeat = this->timeNextHeartbeat - now;
        auto timeUntilNextActivity = this->timeNextActivity - now;

        int t = static_cast<int>(
          std::chrono::duration_cast<std::chrono::milliseconds>
            (std::min(timeUntilNextHeartbeat, timeUntilNextActivity)).count());
        int t2 = std::min(t, this->kTimeout);
        return std::max(t2, 0);
      }

      /// \brief Receive discovery messages.
      private: void RecvMessages()
      {
        bool timeToExit = false;
        while (!timeToExit)
        {
          // Calculate the timeout.
          int timeout = this->NextTimeout();

          if (pollSockets(this->sockets, timeout))
          {
            this->RecvDiscoveryUpdate();

            if (this->verbose)
              this->PrintCurrentState();
          }

          this->UpdateHeartbeat();
          this->UpdateActivity();

          // Is it time to exit?
          {
            std::lock_guard<std::mutex> lock(this->exitMutex);
            if (this->exit)
              timeToExit = true;
          }
        }
      }

      /// \brief Method in charge of receiving the discovery updates.
      private: void RecvDiscoveryUpdate()
      {
        char rcvStr[Discovery::kMaxRcvStr];
        sockaddr_in clntAddr;
        socklen_t addrLen = sizeof(clntAddr);

        int64_t received = recvfrom(this->sockets.at(0),
              reinterpret_cast<raw_type *>(rcvStr),
              this->kMaxRcvStr, 0,
              reinterpret_cast<sockaddr *>(&clntAddr),
              reinterpret_cast<socklen_t *>(&addrLen));
        if (received > 0)
        {
          uint16_t len = 0;
          memcpy(&len, &rcvStr[0], sizeof(len));

          // Gazebo Transport delimits each discovery message with a
          // frame_delimiter that contains byte size information.
          // A discovery message has the form:
          //
          // <frame_delimiter><frame_body>
          //
          // Gazebo Transport version < 8 sends a frame delimiter that
          // contains the value of sizeof(frame_delimiter)
          // + sizeof(frame_body). In other words, the frame_delimiter
          // contains a value that represents the total size of the
          // frame_body and frame_delimiter in bytes.
          //
          // Gazebo Transport version >= 8 sends a frame_delimiter
          // that contains the value of sizeof(frame_body). In other
          // words, the frame_delimiter contains a value that represents
          // the total size of only the frame_body.
          //
          // It is possible that two incompatible versions of Gazebo
          // Transport exist on the same network. If we receive an
          // unexpected size, then we ignore the message.

          // If-condition for version 8+
          if (len + sizeof(len) == static_cast<uint16_t>(received))
          {
            std::string srcAddr = inet_ntoa(clntAddr.sin_addr);
            uint16_t srcPort = ntohs(clntAddr.sin_port);

            if (this->verbose)
            {
              std::cout << "\nReceived discovery update from "
                << srcAddr << ": " << srcPort << std::endl;
            }

            this->DispatchDiscoveryMsg(srcAddr, rcvStr + sizeof(len), len);
          }
        }
        else if (received < 0)
        {
          std::cerr << "Discovery::RecvDiscoveryUpdate() recvfrom error"
            << std::endl;
        }
      }

      /// \brief Parse a discovery message received via the UDP socket
      /// \param[in] _fromIp IP address of the message sender.
      /// \param[in] _msg Received message.
      /// \param[in] _len Entire length of the package in octets.
      private: void DispatchDiscoveryMsg(const std::string &_fromIp,
                                         char *_msg, uint16_t _len)
      {
        gz::msgs::Discovery msg;

        // Parse the message, and return if parsing failed. Parsing could
        // fail when another discovery node is publishing messages using an
        // older (or newer) format.
        if (!msg.ParseFromArray(_msg, _len))
          return;

        // Discard the message if the wire protocol is different than mine.
        if (this->Version() != msg.version())
          return;

        std::string recvPUuid = msg.process_uuid();

        // Discard our own discovery messages.
        if (recvPUuid == this->pUuid)
          return;

        // Forwarding summary:
        //   - From a unicast peer  -> to multicast group (with NO_RELAY flag).
        //   - From multicast group -> to unicast peers (with RELAY flag).

        // If the RELAY flag is set, this discovery message is coming via a
        // unicast transmission. In this case, we don't process it, we just
        // forward it to the multicast group, and it will be dispatched once
        // received there. Note that we also unset the RELAY flag and set the
        // NO_RELAY flag, to avoid forwarding the message anymore.
        if (msg.has_flags() && msg.flags().relay())
        {
          // Unset the RELAY flag in the header and set the NO_RELAY.
          msg.mutable_flags()->set_relay(false);
          msg.mutable_flags()->set_no_relay(true);
          this->SendMulticast(msg);

          // A unicast peer contacted me. I need to save its address for
          // sending future messages in the future.
          this->AddRelayAddress(_fromIp);
          return;
        }
        // If we are receiving this discovery message via the multicast channel
        // and the NO_RELAY flag is not set, we forward this message via unicast
        // to all our relays. Note that this is the most common case, where we
        // receive a regular multicast message and we forward it to any remote
        // relays.
        else if (!msg.has_flags() || !msg.flags().no_relay())
        {
          msg.mutable_flags()->set_relay(true);
          this->SendUnicast(msg);
        }

        bool isSenderLocal = (std::find(this->hostInterfaces.begin(),
          this->hostInterfaces.end(), _fromIp) != this->hostInterfaces.end()) ||
          (_fromIp.find("127.") == 0);

        // Update timestamp and cache the callbacks.
        DiscoveryCallback<Pub> connectCb;
        DiscoveryCallback<Pub> disconnectCb;
        DiscoveryCallback<Pub> registerCb;
        DiscoveryCallback<Pub> unregisterCb;
        std::function<void()> subscribersReqCb;
        {
          std::lock_guard<std::mutex> lock(this->mutex);
          this->activity[recvPUuid] = std::chrono::steady_clock::now();
          connectCb = this->connectionCb;
          disconnectCb = this->disconnectionCb;
          registerCb = this->registrationCb;
          unregisterCb = this->unregistrationCb;
          subscribersReqCb = this->subscribersCb;
        }

        switch (msg.type())
        {
          case msgs::Discovery::ADVERTISE:
          {
            // Read the rest of the fields.
            Pub publisher;
            publisher.SetFromDiscovery(msg);

            // Check scope of the topic.
            if ((publisher.Options().Scope() == Scope_t::PROCESS) ||
                (publisher.Options().Scope() == Scope_t::HOST &&
                 !isSenderLocal))
            {
              return;
            }

            // Register an advertised address for the topic.
            bool added;
            {
              std::lock_guard<std::mutex> lock(this->mutex);
              added = this->info.AddPublisher(publisher);
            }

            if (added && connectCb)
            {
              // Execute the client's callback.
              connectCb(publisher);
            }

            break;
          }
          case msgs::Discovery::SUBSCRIBE:
          {
            std::string recvTopic;
            // Read the topic information.
            if (msg.has_sub())
            {
              recvTopic = msg.sub().topic();
            }
            else
            {
              std::cerr << "Subscription discovery message is missing "
                << "Subscriber information.\n";
              break;
            }

            // Check if at least one of my nodes advertises the topic requested.
            Addresses_M<Pub> addresses;
            {
              std::lock_guard<std::mutex> lock(this->mutex);
              if (!this->info.HasAnyPublishers(recvTopic, this->pUuid))
              {
                break;
              }

              if (!this->info.Publishers(recvTopic, addresses))
                break;
            }

            for (const auto &nodeInfo : addresses[this->pUuid])
            {
              // Check scope of the topic.
              if ((nodeInfo.Options().Scope() == Scope_t::PROCESS) ||
                  (nodeInfo.Options().Scope() == Scope_t::HOST &&
                   !isSenderLocal))
              {
                continue;
              }

              // Answer an ADVERTISE message.
              this->SendMsg(DestinationType::ALL,
                  msgs::Discovery::ADVERTISE, nodeInfo);
            }

            break;
          }
          case msgs::Discovery::SUBSCRIBERS_REQ:
          {
            if (subscribersReqCb)
              subscribersReqCb();

            break;
          }
          case msgs::Discovery::SUBSCRIBERS_REP:
          {
            // Save the remote subscriber.
            Pub publisher;
            publisher.SetFromDiscovery(msg);

            {
              std::lock_guard<std::mutex> lock(this->mutex);
              this->remoteSubscribers.AddPublisher(publisher);
            }
            break;
          }
          case msgs::Discovery::NEW_CONNECTION:
          {
            // Read the rest of the fields.
            Pub publisher;
            publisher.SetFromDiscovery(msg);

            if (registerCb)
              registerCb(publisher);

            break;
          }
          case msgs::Discovery::END_CONNECTION:
          {
            // Read the rest of the fields.
            Pub publisher;
            publisher.SetFromDiscovery(msg);

            if (unregisterCb)
              unregisterCb(publisher);

            break;
          }
          case msgs::Discovery::HEARTBEAT:
          {
            // The timestamp has already been updated.
            break;
          }
          case msgs::Discovery::BYE:
          {
            // Remove the activity entry for this publisher.
            {
              std::lock_guard<std::mutex> lock(this->mutex);
              this->activity.erase(recvPUuid);
            }

            if (disconnectCb)
            {
              Pub pub;
              pub.SetPUuid(recvPUuid);
              // Notify the new disconnection.
              disconnectCb(pub);
            }

            // Remove the address entry for this topic.
            {
              std::lock_guard<std::mutex> lock(this->mutex);
              this->info.DelPublishersByProc(recvPUuid);
            }

            break;
          }
          case msgs::Discovery::UNADVERTISE:
          {
            // Read the address.
            Pub publisher;
            publisher.SetFromDiscovery(msg);

            // Check scope of the topic.
            if ((publisher.Options().Scope() == Scope_t::PROCESS) ||
                (publisher.Options().Scope() == Scope_t::HOST &&
                 !isSenderLocal))
            {
              return;
            }

            if (disconnectCb)
            {
              // Notify the new disconnection.
              disconnectCb(publisher);
            }

            // Remove the address entry for this topic.
            {
              std::lock_guard<std::mutex> lock(this->mutex);
              this->info.DelPublisherByNode(publisher.Topic(),
                publisher.PUuid(), publisher.NUuid());
            }

            break;
          }
          default:
          {
            std::cerr << "Unknown message type [" << msg.type() << "].\n";
            break;
          }
        }
      }

      /// \brief Broadcast a discovery message.
      /// \param[in] _type Message type.
      /// \param[in] _pub Publishers's information to send.
      /// \param[in] _flags Optional flags. Currently, the flags are not used
      /// but they will in the future for specifying things like compression,
      /// or encryption.
      private: template<typename T>
      void SendMsg(const DestinationType &_destType,
                   const msgs::Discovery::Type _type,
                   const T &_pub) const
      {
        gz::msgs::Discovery discoveryMsg;
        discoveryMsg.set_version(this->Version());
        discoveryMsg.set_type(_type);
        discoveryMsg.set_process_uuid(this->pUuid);
        _pub.FillDiscovery(discoveryMsg);

        switch (_type)
        {
          case msgs::Discovery::ADVERTISE:
          case msgs::Discovery::UNADVERTISE:
          case msgs::Discovery::NEW_CONNECTION:
          case msgs::Discovery::END_CONNECTION:
          {
            _pub.FillDiscovery(discoveryMsg);
            break;
          }
          case msgs::Discovery::SUBSCRIBE:
          {
            discoveryMsg.mutable_sub()->set_topic(_pub.Topic());
            break;
          }
          case msgs::Discovery::HEARTBEAT:
          case msgs::Discovery::BYE:
          case msgs::Discovery::SUBSCRIBERS_REQ:
          case msgs::Discovery::SUBSCRIBERS_REP:
            break;
          default:
            std::cerr << "Discovery::SendMsg() error: Unrecognized message"
                      << " type [" << _type << "]" << std::endl;
            return;
        }

        if (_destType == DestinationType::MULTICAST ||
            _destType == DestinationType::ALL)
        {
          this->SendMulticast(discoveryMsg);
        }

        // Send the discovery message to the unicast relays.
        if (_destType == DestinationType::UNICAST ||
            _destType == DestinationType::ALL)
        {
          // Set the RELAY flag in the header.
          discoveryMsg.mutable_flags()->set_relay(true);
          this->SendUnicast(discoveryMsg);
        }

        if (this->verbose)
        {
          std::cout << "\t* Sending " << msgs::ToString(_type)
                    << " msg [" << _pub.Topic() << "]" << std::endl;
        }
      }

      /// \brief Send a discovery message through all unicast relays.
      /// \param[in] _msg Discovery message.
      private: void SendUnicast(const msgs::Discovery &_msg) const
      {
        uint16_t msgSize;

#if GOOGLE_PROTOBUF_VERSION >= 3004000
        size_t msgSizeFull = _msg.ByteSizeLong();
#else
        int msgSizeFull = _msg.ByteSize();
#endif
        if (msgSizeFull + sizeof(msgSize) > this->kMaxRcvStr)
        {
          std::cerr << "Discovery message too large to send. Discovery won't "
            << "work. This shouldn't happen.\n";
          return;
        }
        msgSize = msgSizeFull;

        uint16_t totalSize = sizeof(msgSize) + msgSize;
        char *buffer = static_cast<char *>(new char[totalSize]);
        memcpy(&buffer[0], &msgSize, sizeof(msgSize));

        if (_msg.SerializeToArray(buffer + sizeof(msgSize), msgSize))
        {
          // Send the discovery message to the unicast relays.
          std::lock_guard<std::mutex> lock(this->mutex);

          for (const auto &sockAddr : this->relayAddrs)
          {
            errno = 0;
            auto sent = sendto(this->sockets.at(0),
              reinterpret_cast<const raw_type *>(
                reinterpret_cast<const unsigned char*>(buffer)),
              totalSize, 0,
              reinterpret_cast<const sockaddr *>(&sockAddr),
              sizeof(sockAddr));

            if (sent != totalSize)
            {
              std::cerr << "Exception sending a unicast message:" << std::endl;
              std::cerr << "  Return value: " << sent << std::endl;
              std::cerr << "  Error code: " << strerror(errno) << std::endl;
              break;
            }
          }
        }
        else
        {
          std::cerr << "Discovery::SendUnicast: Error serializing data."
            << std::endl;
        }

        delete [] buffer;
      }

      /// \brief Send a discovery message through the multicast group.
      /// \param[in] _msg Discovery message.
      private: void SendMulticast(const msgs::Discovery &_msg) const
      {
        uint16_t msgSize;

#if GOOGLE_PROTOBUF_VERSION >= 3004000
        size_t msgSizeFull = _msg.ByteSizeLong();
#else
        int msgSizeFull = _msg.ByteSize();
#endif
        if (msgSizeFull + sizeof(msgSize) > this->kMaxRcvStr)
        {
          std::cerr << "Discovery message too large to send. Discovery won't "
            << "work. This shouldn't happen.\n";
          return;
        }

        msgSize = msgSizeFull;
        uint16_t totalSize = sizeof(msgSize) + msgSize;
        char *buffer = static_cast<char *>(new char[totalSize]);
        memcpy(&buffer[0], &msgSize, sizeof(msgSize));

        if (_msg.SerializeToArray(buffer + sizeof(msgSize), msgSize))
        {
          // Send the discovery message to the multicast group through all the
          // sockets.
          for (const auto &sock : this->Sockets())
          {
            errno = 0;
            if (sendto(sock, reinterpret_cast<const raw_type *>(
              reinterpret_cast<const unsigned char*>(buffer)),
              totalSize, 0,
              reinterpret_cast<const sockaddr *>(this->MulticastAddr()),
              sizeof(*(this->MulticastAddr()))) != totalSize)
            {
              // Ignore EPERM and ENOBUFS errors.
              //
              // See issue #106
              //
              // Rationale drawn from:
              //
              // * https://groups.google.com/forum/#!topic/comp.protocols.tcp-ip/Qou9Sfgr77E
              // * https://stackoverflow.com/questions/16555101/sendto-dgrams-do-not-block-for-enobufs-on-osx
              if (errno != EPERM && errno != ENOBUFS)
              {
                std::cerr << "Exception sending a multicast message:"
                  << strerror(errno) << std::endl;
              }
              break;
            }
          }
        }
        else
        {
          std::cerr << "Discovery::SendMulticast: Error serializing data."
            << std::endl;
        }

        delete [] buffer;
      }

      /// \brief Get the list of sockets used for discovery.
      /// \return The list of sockets.
      private: const std::vector<int> &Sockets() const
      {
        return this->sockets;
      }

      /// \brief Get the data structure used for multicast communication.
      /// \return The data structure containing the multicast information.
      private: const sockaddr_in *MulticastAddr() const
      {
        return &this->mcastAddr;
      }

      /// \brief Get the discovery protocol version.
      /// \return The discovery version.
      private: uint8_t Version() const
      {
        static std::string gzStats;
        static int topicStats;

        if (env("GZ_TRANSPORT_TOPIC_STATISTICS", gzStats) && !gzStats.empty())
        {
          topicStats = (gzStats == "1");
        }

        return this->kWireVersion + (topicStats * 100);
      }

      /// \brief Register a new network interface in the discovery system.
      /// \param[in] _ip IP address to register.
      /// \return True when the interface was successfully registered or false
      /// otherwise (e.g.: invalid IP address).
      private: bool RegisterNetIface(const std::string &_ip)
      {
        // Make a new socket for sending discovery information.
        int sock = static_cast<int>(socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP));
        if (sock < 0)
        {
          std::cerr << "Socket creation failed." << std::endl;
          return false;
        }

        // Socket option: IP_MULTICAST_IF.
        // This socket option needs to be applied to each socket used to send
        // data. This option selects the source interface for outgoing messages.
        struct in_addr ifAddr;
        ifAddr.s_addr = inet_addr(_ip.c_str());
        if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF,
          reinterpret_cast<const char*>(&ifAddr), sizeof(ifAddr)) != 0)
        {
          std::cerr << "Error setting socket option (IP_MULTICAST_IF)."
                    << std::endl;
          return false;
        }

        this->sockets.push_back(sock);

        // Join the multicast group. We have to do it for each network interface
        // but we can do it on the same socket. We will use the socket at
        // position 0 for receiving multicast information.
        struct ip_mreq group;
        group.imr_multiaddr.s_addr =
          inet_addr(this->multicastGroup.c_str());
        group.imr_interface.s_addr = inet_addr(_ip.c_str());
        if (setsockopt(this->sockets.at(0), IPPROTO_IP, IP_ADD_MEMBERSHIP,
          reinterpret_cast<const char*>(&group), sizeof(group)) != 0)
        {
          std::cerr << "Error setting socket option (IP_ADD_MEMBERSHIP)."
                    << std::endl;
          return false;
        }

        return true;
      }

      /// \brief Default activity interval value (ms.).
      /// \sa ActivityInterval.
      /// \sa SetActivityInterval.
      private: static const unsigned int kDefActivityInterval = 100;

      /// \brief Default heartbeat interval value (ms.).
      /// \sa HeartbeatInterval.
      /// \sa SetHeartbeatInterval.
      private: static const unsigned int kDefHeartbeatInterval = 1000;

      /// \brief Default silence interval value (ms.).
      /// \sa MaxSilenceInterval.
      /// \sa SetMaxSilenceInterval.
      private: static const unsigned int kDefSilenceInterval = 3000;

      /// \brief IP Address used for multicast.
      private: std::string multicastGroup;

      /// \brief Timeout used for receiving messages (ms.).
      private: const int kTimeout = 250;

      /// \brief Longest string to receive.
      private: static const uint16_t kMaxRcvStr =
               std::numeric_limits<uint16_t>::max();

      /// \brief Wire protocol version. Bump up the version number if you modify
      /// the wire protocol (for discovery or message/service exchange).
      private: static const uint8_t kWireVersion = 10;

      /// \brief Port used to broadcast the discovery messages.
      private: int port;

      /// \brief Host IP address.
      private: std::string hostAddr;

      /// \brief List of host network interfaces.
      private: std::vector<std::string> hostInterfaces;

      /// \brief Process UUID.
      private: std::string pUuid;

      /// \brief Silence interval value (ms.).
      /// \sa MaxSilenceInterval.
      /// \sa SetMaxSilenceInterval.
      private: unsigned int silenceInterval;

      /// \brief Activity interval value (ms.).
      /// \sa ActivityInterval.
      /// \sa SetActivityInterval.
      private: unsigned int activityInterval;

      /// \brief Heartbeat interval value (ms.).
      /// \sa HeartbeatInterval.
      /// \sa SetHeartbeatInterval.
      private: unsigned int heartbeatInterval;

      /// \brief Callback executed when new topics are discovered.
      private: DiscoveryCallback<Pub> connectionCb;

      /// \brief Callback executed when new topics are invalid.
      private: DiscoveryCallback<Pub> disconnectionCb;

      /// \brief Callback executed when a new remote subscriber is registered.
      private: DiscoveryCallback<Pub> registrationCb;

      /// \brief Callback executed when a new remote subscriber is unregistered.
      private: DiscoveryCallback<Pub> unregistrationCb;

      /// \brief Callback executed when a SUBSCRIBERS_REQ message is received.
      private: std::function<void()> subscribersCb;

      /// \brief Addressing information.
      private: TopicStorage<Pub> info;

      /// \brief Remote subscribers.
      private: TopicStorage<Pub> remoteSubscribers;

      /// \brief Activity information. Every time there is a message from a
      /// remote node, its activity information is updated. If we do not hear
      /// from a node in a while, its entries in 'info' will be invalided. The
      /// key is the process uuid.
      protected: std::map<std::string, Timestamp> activity;

      /// \brief Print discovery information to stdout.
      private: bool verbose;

      /// \brief UDP socket used for sending/receiving discovery messages.
      private: std::vector<int> sockets;

      /// \brief Internet socket address for sending to the multicast group.
      private: sockaddr_in mcastAddr;

      /// \brief Collection of socket addresses used as remote relays.
      private: std::vector<sockaddr_in> relayAddrs;

      /// \brief Mutex to guarantee exclusive access between the threads.
      private: mutable std::mutex mutex;

      /// \brief Thread in charge of receiving and handling incoming messages.
      private: std::thread threadReception;

      /// \brief Time at which the next heartbeat cycle will be sent.
      private: Timestamp timeNextHeartbeat;

      /// \brief Time at which the next activity check will be done.
      private: Timestamp timeNextActivity;

      /// \brief Mutex to guarantee exclusive access to the exit variable.
      private: std::mutex exitMutex;

      /// \brief Once the discovery starts, it can take up to
      /// HeartbeatInterval milliseconds to discover the existing nodes on the
      /// network. This variable is 'false' during the first HeartbeatInterval
      /// period and is set to 'true' after that.
      private: bool initialized;

      /// \brief Number of heartbeats sent while discovery is uninitialized.
      private: unsigned int numHeartbeatsUninitialized;

      /// \brief Used to block/unblock until the initialization phase finishes.
      private: mutable std::condition_variable initializedCv;

      /// \brief When true, the service thread will finish.
      private: bool exit;

      /// \brief When true, the service is enabled.
      private: bool enabled;
    };

    /// \def MsgDiscovery
    /// \brief A discovery object for topics.
    using MsgDiscovery = Discovery<MessagePublisher>;

    /// \def SrvDiscovery
    /// \brief A discovery object for services.
    using SrvDiscovery = Discovery<ServicePublisher>;
    }
  }
}

#endif
