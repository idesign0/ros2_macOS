\page relay Relay

Next Tutorial: \ref logging
Previous Tutorial: \ref security

## Overview

In this tutorial, we are going to create two nodes that are not able to
communicate with the default configuration of Gazebo Transport. This
limitation arises when the nodes are separated by a router, typically when they
are part of different local networks. Routers do not propagate UDP multicast
traffic and this is the reason for this limitation. We'll create a scenario to
simulate this configuration, and then we'll enable the relay capabilities of
Gazebo Transport to make the communication possible.

```{.sh}
mkdir -p ~/gz_transport_tutorial/docker
cd ~/gz_transport_tutorial/docker
```

## Setup

We'll use Docker to configure the environment for this example. Feel free to
install Docker following any of the existing guides available
([here](https://docs.docker.com/get-docker/)'s one).

We're going to build a Docker image and run it inside your host computer.
Download the [build.bash](https://github.com/gazebosim/gz-transport/raw/gz-transport14/docker/build.bash), [run.bash](https://github.com/gazebosim/gz-transport/raw/gz-transport14/docker/run.bash) and
[Dockerfile](https://github.com/gazebosim/gz-transport/raw/gz-transport14/docker/gz-transport/Dockerfile) files.

```{.sh}
wget https://github.com/gazebosim/gz-transport/raw/gz-transport14/docker/gz-transport/Dockerfile -O Dockerfile
```

Now, it's time to build the Docker image:
```
docker build -t gz-transport-relay - < Dockerfile
```

Run your Docker container:
```
docker run -it --rm gz-transport-relay /bin/bash
```

Back on your host, make sure that you have Gazebo Tools and net-tools
installed:
```
sudo apt install gz-tools2 net-tools
```

Now, let's configure Gazebo Transport to block all multicast traffic going
into your Docker instance. Run the command `ifconfig` to list your network
interfaces:
```
caguero@bb9:~/gz_transport_tutorial/docker$ ifconfig
docker0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 172.17.0.1  netmask 255.255.0.0  broadcast 172.17.255.255
        inet6 fe80::42:73ff:fe1c:351e  prefixlen 64  scopeid 0x20<link>
        ether 02:42:73:1c:35:1e  txqueuelen 0  (Ethernet)
        RX packets 943154  bytes 72074740 (72.0 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 1646253  bytes 2714146135 (2.7 GB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eno1: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 172.23.1.7  netmask 255.255.252.0  broadcast 172.23.3.255
        inet6 fe80::c115:c109:18df:d327  prefixlen 64  scopeid 0x20<link>
        ether d8:cb:8a:34:c0:50  txqueuelen 1000  (Ethernet)
        RX packets 21694702  bytes 12539677170 (12.5 GB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 7206435  bytes 1107442513 (1.1 GB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
        device interrupt 20  memory 0xfb400000-fb420000

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1000  (Local Loopback)
        RX packets 71920529  bytes 9057867879 (9.0 GB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 71920529  bytes 9057867879 (9.0 GB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```

We want to isolate Gazebo Transport to the network interface not connected to
your Docker instance. Thus, try to identify the IP address of the network
interface not associated with Docker or the loopback interface. In our case,
the IP address is `172.23.1.7`.

## Launch the publisher

Go back to the terminal inside the Docker container and run the publisher
example:
```
GZ_PARTITION=relay ./publisher
```

## Launch the subscriber

Open a terminal in your host and launch your subscriber, forcing Gazebo
Transport to only bind to the IP address that we found in the previous step:

```
GZ_IP=172.23.1.7 GZ_PARTITION=relay gz topic -e -t /foo
```

You shouldn't receive anything as the discovery messages are not reaching both
nodes.

Now, setup your host to relay messages to the node running inside your
Docker container. For that purpose, you'll need to know the IP address used
in your Docker container. Run the `ifconfig` command inside your Docker
instance:
```
developer@b98e0f32f32f:~/gz-transport/example/build$ ifconfig
eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 172.17.0.3  netmask 255.255.0.0  broadcast 172.17.255.255
        ether 02:42:ac:11:00:03  txqueuelen 0  (Ethernet)
        RX packets 3255  bytes 809362 (809.3 KB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 761  bytes 85342 (85.3 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        loop  txqueuelen 1000  (Local Loopback)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```

Go back to your terminal in the host and configure the environment variable
`GZ_RELAY` with the IP address used inside the container.

```
GZ_RELAY=172.17.0.3 GZ_IP=172.23.1.7 GZ_PARTITION=relay gz topic -e -t /foo
```

Now, you should receive the messages, as your node in the host is directly
relaying the discovery messages inside your Docker instance via unicast.

## Known limitations

Keep in mind that the end points of all the nodes should be reachable both
ways. The relay feature will overcome the UDP multicast limitation but remember
that after the discovery phase the nodes will exchange data directly using a
different set of end points. These end points should be reachable from any node,
otherwise the communication will not work.

Example: Imagine that you're running a publisher in your home machine.
Typically, you'll be using a private IP address behind your home router doing
NAT. If you try to run a subscriber node inside a computer over the internet
using a public IP, things will not work even using `GZ_RELAY`. The discovery
protocol will reach the subscriber and back (thanks to the NAT), but things will
stop at that point. The real data exchange will not be possible, as the
subscriber will not be able to communicate with the publisher's endpoint using
a private IP address. A solution to this problem is to create a VPN to create
the abstraction that both machines are within the same local network.
