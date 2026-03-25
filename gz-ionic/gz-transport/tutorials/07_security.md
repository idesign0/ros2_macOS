\page security Security

Next Tutorial: \ref relay
Previous Tutorial: \ref python

## Overview

Gazebo Transport's default mode of communication is insecure, which means
no authentication or encryption is used. Insecure communication is the
default because it is simple to implement and use, supports introspection,
and reduces third-party development effort.

There are many cases when security is required by an application. For
example, authentication may be needed to verify participants in a network.
Gazebo Transport relies on security features available in ZeroMQ. The
available security features include authentication, and encryption.  

The remainder of the this tutorial describes how to setup and use security
with Gazebo Transport.

## Authentication

Authentication relies on a username and password to limit access to
communication topics. Currently, authentication in Gazebo Transport
operates at the process level. This means all topics within a process will
either use, or not use, authentication.

Two environment variables are used to enable authentications:

1. `GZ_TRANSPORT_USERNAME` : The username
2. `GZ_TRANSPORT_PASSWORD` : The password

When both `GZ_TRANSPORT_USERNAME` and `GZ_TRANSPORT_PASSWORD` are set,
the authentication is enabled for a process. Every publisher in a secure
process will require subscribers to provide the correct username and
password. Also, every subscriber will only connect to secure publishers.

### Example

> **NOTE**
> It is essential to have a valid value of `GZ_PARTITION` environment variable
> and to have it set to the same value in all open terminals. As `GZ_PARTITION`
> is based on hostname and username, especially Windows and Mac users might
> have problems due to spaces in their username, which are not a valid character
> in `GZ_PARTITION`. gz-transport prints error `Invalid partition name` in such
> case. To resolve that, set `GZ_PARTITION` explicitly to a valid value:
> ```bash
> # Linux and Mac
> export GZ_PARTITION=test
> # Windows
> set GZ_PARTITION=test
> ```

First, let's test insecure communication. This example requires
[gz-tools](https://github.com/gazebosim/gz-tools).

1. Open a terminal, and echo topic `/foo`.
```
gz topic -t /foo -e
```
2. Open a second terminal and publish a message on topic `/foo`.
```
gz topic -t /foo -m gz.msgs.StringMsg -p 'data:"Insecure message"'
```
3. The first terminal should see the following output.
```
data: "Insecure message"
```

Now let's try a secure publisher and an insecure subscriber.

1. Leave the first terminal running `gz topic -t /foo -e`.
2. Setup authentication in the second terminal:
```
# Linux and MacOS
export GZ_TRANSPORT_USERNAME=user
export GZ_TRANSPORT_PASSWORD=pass

# Windows
set GZ_TRANSPORT_USERNAME=user
set GZ_TRANSPORT_PASSWORD=pass
```
3. Now publish a message in the second terminal:
```
gz topic -t /foo -m gz.msgs.StringMsg -p 'data:"Secure message"'
```
4. The first terminal should not change, which indicates that subscriber was
   not able to authenticate with the secure publisher.

Finally, let's create secure subscriber.

1. Open a third terminal, and setup authentication in that terminal.
```
# Linux and MacOS
export GZ_TRANSPORT_USERNAME=user
export GZ_TRANSPORT_PASSWORD=pass

# Windows
set GZ_TRANSPORT_USERNAME=user
set GZ_TRANSPORT_PASSWORD=pass
```
2. Echo the `/foo` topic in the secure third terminal.
```
gz topic -t /foo -e
```
3. Go back to the secure publisher in the second terminal, and re-run the
   publish command.
```
gz topic -t /foo -m gz.msgs.StringMsg -p 'data:"Secure message"'
```
4. The third terminal, running the secure subscriber, should output the
   following.
```
data: "Secure message"
```
5. The insecure subscriber in the first terminal should not change.

## Encryption

Coming soon.
