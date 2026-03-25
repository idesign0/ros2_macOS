\page services Services

Next Tutorial: \ref python
Previous Tutorial: \ref messages

## Overview

In this tutorial, we are going to create two nodes that are going to communicate
via services. You can see a service as a function that is going to be executed
in a different node. Services have two main components: a service provider and a
service consumer. A service provider is the node that offers the service to the
rest of the world. The service consumers are the nodes that request the function
offered by the provider. Note that in Gazebo Transport the location of the
service is hidden. The discovery layer of the library is in charge of
discovering and keeping an updated list of services available.

Multiple nodes can advertise the same service. When a request is made, only one
of the available service providers will handle it. There is no guarantee of
which provider will be selected. If one provider becomes unavailable, the
remaining providers can still handle subsequent requests.

In the next tutorial, one node will be the service provider that offers an
*echo* service, whereas the other node will be the service consumer requesting
an *echo* call.

```{.sh}
mkdir ~/gz_transport_tutorial
cd ~/gz_transport_tutorial
```

## Responser

Download the [responser.cc](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/responser.cc) file within the ``gz_transport_tutorial``
folder and open it with your favorite editor:

```{.cpp}
#include <iostream>
#include <string>
#include <gz/msgs.hh>
#include <gz/transport.hh>

bool srvEcho(const gz::msgs::StringMsg &_req,
  gz::msgs::StringMsg &_rep)
{
  // Set the response's content.
  _rep.set_data(_req.data());

  // The response succeed.
  return true;
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Let's print the list of our network interfaces.
  std::cout << "List of network interfaces in this machine:" << std::endl;
  for (const auto &netIface : gz::transport::determineInterfaces())
    std::cout << "\t" << netIface << std::endl;

  // Create a transport node.
  gz::transport::Node node;
  std::string service = "/echo";

  // Advertise a service call.
  if (!node.Advertise(service, srvEcho))
  {
    std::cerr << "Error advertising service [" << service << "]" << std::endl;
    return -1;
  }

  // Zzzzzz.
  gz::transport::waitForShutdown();
}
```

### Walkthrough

```{.cpp}
#include <gz/msgs.hh>
#include <gz/transport.hh>
```

The line ``#include <gz/transport.hh>`` contains the Gazebo Transport
header for using the transport library.

The next line includes the generated Protobuf code that we are going to use
for our messages. We are going to use ``StringMsg`` type Protobuf messages
for our services.


```{.cpp}
bool srvEcho(const gz::msgs::StringMsg &_req,
  gz::msgs::StringMsg &_rep)
{
  // Set the response's content.
  _rep.set_data(_req.data());

  // The response succeed.
  return true;
}
```

As a service provider, our node needs to register a function callback that will
execute every time a new service request is received. The signature of the
callback is always similar to the one shown in this example with the exception
of the Protobuf messages types for the ``_req`` (request) and ``_rep``
(response). The request parameter contains the input parameters of the request.
The response message contains any resulting data from the service call. The
return value denotes if the overall service call was considered
successful or not. In our example, as a simple *echo* service, we just fill the
response with the same data contained in the request.

```{.cpp}
// Create a transport node.
gz::transport::Node node;
std::string service = "/echo";

// Advertise a service call.
if (!node.Advertise(service, srvEcho))
{
  std::cerr << "Error advertising service [" << service << "]" << std::endl;
  return -1;
}

// Zzzzzz.
gz::transport::waitForShutdown();
```

We declare a *Node* that will offer all the transport functionality. In our
case, we are interested in offering a service, so the first step is to announce
our service name. Once a service name is advertised, we can accept service
requests.

If you don't have any other tasks to do besides waiting for service requests,
you can use the call `waitForShutdown()` that will block your current thread
until you hit *CTRL-C*. Note that this function captures the *SIGINT* and
*SIGTERM* signals.

## Synchronous requester

Download the [requester.cc](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/requester.cc) file within the ``gz_transport_tutorial``
folder and open it with your favorite editor:

```{.cpp}
#include <iostream>
#include <gz/msgs.hh>
#include <gz/transport.hh>

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Create a transport node.
  gz::transport::Node node;

  // Prepare the input parameters.
  gz::msgs::StringMsg req;
  req.set_data("HELLO");

  gz::msgs::StringMsg rep;
  bool result;
  unsigned int timeout = 5000;

  // Request the "/echo" service.
  bool executed = node.Request("/echo", req, timeout, rep, result);

  if (executed)
  {
    if (result)
      std::cout << "Response: [" << rep.data() << "]" << std::endl;
    else
      std::cout << "Service call failed" << std::endl;
  }
  else
    std::cerr << "Service call timed out" << std::endl;
}
```

### Walkthrough

```{.cpp}
// Create a transport node.
gz::transport::Node node;

// Prepare the input parameters.
gz::msgs::StringMsg req;
req.set_data("HELLO");

gz::msgs::StringMsg rep;
bool result;
unsigned int timeout = 5000;
```

We declare the *Node* that allows us to request a service. Next, we declare and
fill the message used as an input parameter for our *echo* request. Then, we
declare the Protobuf message that will contain the response and the variable
that will tell us if the service request succeed or failed. In this example, we
will use a synchronous request, meaning that our code will block until the
response is received or a timeout expires. The value of the timeout is expressed
in milliseconds.

```{.cpp}
// Request the "/echo" service.
bool executed = node.Request("/echo", req, timeout, rep, result);

if (executed)
{
  if (result)
    std::cout << "Response: [" << rep.data() << "]" << std::endl;
  else
    std::cout << "Service call failed" << std::endl;
}
else
  std::cerr << "Service call timed out" << std::endl;
```

In this section of the code we use the method ``Request()`` for forwarding the
service call to any service provider of the service ``/echo``.
Gazebo Transport will find a node, communicate the input data, capture the
response and pass it to your output parameter. The return value will tell you
if the request expired or the response was received. The ``result`` value will
tell you if the service provider considered the operation valid.

Imagine for example that we are using a division service, where our input
message contains the numerator and denominator. If there are no nodes offering
this service, our request will timeout (return value ``false``). On the other
hand, if there's at least one node providing the service, the request will
return ``true`` signaling that the request was received. However, if we set our
denominator to ``0`` in the input message, ``result`` will be ``false``
reporting that something went wrong in the request. If the input parameters are
valid, we'll receive a result value of ``true`` and we can use our response
message.


## Asynchronous requester

Download the [requester_async.cc](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/requester_async.cc) file within the
``gz_transport_tutorial`` folder and open it with your favorite editor:

```{.cpp}
#include <iostream>
#include <gz/msgs.hh>
#include <gz/transport.hh>

void responseCb(const gz::msgs::StringMsg &_rep, const bool _result)
{
  if (_result)
    std::cout << "Response: [" << _rep.data() << "]" << std::endl;
  else
    std::cerr << "Service call failed" << std::endl;
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Create a transport node.
  gz::transport::Node node;

  // Prepare the input parameters.
  gz::msgs::StringMsg req;
  req.set_data("HELLO");

  std::cout << "Press <CTRL-C> to exit" << std::endl;

  // Request the "/echo" service.
  node.Request("/echo", req, responseCb);

  // Zzzzzz.
  gz::transport::waitForShutdown();
}
```


### Walkthrough

```{.cpp}
void responseCb(const gz::msgs::StringMsg &_rep, const bool _result)
{
  if (_result)
    std::cout << "Response: [" << _rep.data() << "]" << std::endl;
  else
    std::cerr << "Service call failed" << std::endl;
}
```

We need to register a function callback that will execute when we receive our
service response. The signature of the callback is always similar to the one
shown in this example with the only exception of the Protobuf message type used
in the response. You should create a function callback with the appropriate
Protobuf type depending on the response type of the service requested. In our
case, we know that the service ``/echo`` will answer with a Protobuf
``StringMsg`` type.

```{.cpp}
// Create a transport node.
gz::transport::Node node;

// Prepare the input parameters.
gz::msgs::StringMsg req;
req.set_data("HELLO");

// Request the "/echo" service.
node.Request("/echo", req, responseCb);
```

In this section of the code we declare a node and a Protobuf message that is
filled with the input parameters for our request. Next, we just use the
asynchronous variant of the ``Request()`` method that forwards a service call to
any service provider of the service ``/echo``.
Gazebo Transport will find a node, communicate the data, capture the response
and pass it to your callback, in addition of the service call result. Note that
this variant of ``Request()`` is asynchronous, so your code will not block while
your service request is handled.


## Oneway responser

Not all the service requests require a response. In these cases we can use a
oneway service to process service requests without sending back responses.
Oneway services don't accept any output parameters nor the requests have to wait
for the response.

Download the [responser_oneway.cc](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/responser_oneway.cc) file within the
``gz_transport_tutorial`` folder and open it with your favorite editor:

```{.cpp}
#include <iostream>
#include <string>
#include <gz/transport.hh>
#include <gz/msgs.hh>

//////////////////////////////////////////////////
void srvOneway(const gz::msgs::StringMsg &_req)
{
  std::cout << "Request received: [" << _req.data() << "]" << std::endl;
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Create a transport node.
  gz::transport::Node node;
  std::string service = "/oneway";

  // Advertise a oneway service.
  if (!node.Advertise(service, srvOneway))
  {
    std::cerr << "Error advertising service [" << service << "]" << std::endl;
    return -1;
  }

  // Zzzzzz.
  gz::transport::waitForShutdown();
}
```


### Walkthrough

```{.cpp}
//////////////////////////////////////////////////
void srvOneway(const gz::msgs::StringMsg &_req)
{
  std::cout << "Request received: [" << _req.data() << "]" << std::endl;
}
```

As a oneway service provider, our node needs to advertise a service that doesn't
send a response back. The signature of the callback contains only one parameter
that is the input parameter, ``_req`` (request). We don't need ``_rep``
(response) or ``_result`` as there is no response expected. In our example,
the value of the input parameter is printed on the screen.

```{.cpp}
// Create a transport node.
gz::transport::Node node;
std::string service = "/oneway";

// Advertise a oneway service.
if (!node.Advertise(service, srvOneway))
{
  std::cerr << "Error advertising service [" << service << "]" << std::endl;
  return -1;
}
```

We declare a *Node* that will offer all the transport functionality. In our
case, we are interested in offering a oneway service, so the first step is to
announce our service name. Once a service name is advertised, we can accept
service requests.

## Oneway requester

This case is similar to the oneway service provider. This code can be used for
requesting a service that does not need a response back. We don't need any
output parameters in this case nor we have to wait for the response.

Download the [requester_oneway.cc](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/requester_oneway.cc) file within the
``gz_transport_tutorial`` folder and open it with your favorite editor:

```{.cpp}
#include <iostream>
#include <gz/transport.hh>
#include <gz/msgs.hh>

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Create a transport node.
  gz::transport::Node node;

  // Prepare the input parameters.
  gz::msgs::StringMsg req;
  req.set_data("HELLO");

  // Request the "/oneway" service.
  bool executed = node.Request("/oneway", req);

  if (!executed)
    std::cerr << "Service call failed" << std::endl;

  std::cout << "Press <CTRL-C> to exit" << std::endl;

  // Zzzzzz.
  gz::transport::waitForShutdown();
}
```


### Walkthrough

```{.cpp}
// Create a transport node.
gz::transport::Node node;

// Prepare the input parameters.
gz::msgs::StringMsg req;
req.set_data("HELLO");

// Request the "/oneway" service.
bool executed = node.Request("/oneway", req);

if (!executed)
  std::cerr << "Service call failed" << std::endl;
```

First of all we declare a node and a Protobuf message that is filled with the
input parameters for our ``/oneway`` service. Next, we just use the oneway
variant of the ``Request()`` method that forwards a service call to any service
provider of the service ``/oneway``. Gazebo Transport will find a node and
communicate the data without waiting for the response. The return value of
``Request()`` indicates if the request was successfully queued. Note that this
variant of ``Request()`` is also asynchronous, so your code will not block while
your service request is handled. In this example, we also call
`waitForShutdown()` to minimize the risk of terminating the program before the
request was already published.

## Service without input parameter

Sometimes we want to receive some result but don't have any input parameter to
send.

Download the [responser_no_input.cc](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/responser_no_input.cc)
 file within the ``gz_transport_tutorial`` folder and open it with your
favorite editor:

```{.cpp}
#include <iostream>
#include <string>
#include <gz/msgs.hh>
#include <gz/transport.hh>

bool srvQuote(gz::msgs::StringMsg &_rep)
{
  std::string awesomeQuote = "This is it! This is the answer. It says here..."
    "that a bolt of lightning is going to strike the clock tower at precisely "
    "10:04pm, next Saturday night! If...If we could somehow...harness this "
    "lightning...channel it...into the flux capacitor...it just might work. "
    "Next Saturday night, we're sending you back to the future!";

  // Set the response's content.
  _rep.set_data(awesomeQuote);

  // The response succeed.
  return true;
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Create a transport node.
  gz::transport::Node node;
  std::string service = "/quote";

  // Advertise a service call.
  if (!node.Advertise(service, srvQuote))
  {
    std::cerr << "Error advertising service [" << service << "]" << std::endl;
    return -1;
  }

  // Zzzzzz.
  gz::transport::waitForShutdown();
}
```

### Walkthrough

```{.cpp}
bool srvQuote(gz::msgs::StringMsg &_rep)
```

Service doesn't receive anything. The signature of the callback contains the
parameters ``_rep`` (response). In our example, we return
the quote.

```{.cpp}
// Create a transport node.
gz::transport::Node node;
std::string service = "/quote";

// Advertise a service call.
if (!node.Advertise(service, srvQuote))
{
  std::cerr << "Error advertising service [" << service << "]" << std::endl;
  return -1;
}

// Zzzzzz.
gz::transport::waitForShutdown();
```

We declare a *Node* that will offer all the transport functionality. In our
case, we are interested in offering service without input, so the first step is
to announce the service name. Once a service name is advertised, we can accept
service requests.

## Empty requester sync and async

This case is similar to the service without input parameter. We don't send any
request.

Download the [requester_no_input.cc](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/requester_no_input.cc)
file within the ``gz_transport_tutorial`` folder and open it with your
favorite editor:

```{.cpp}
#include <iostream>
#include <gz/msgs.hh>
#include <gz/transport.hh>

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Create a transport node.
  gz::transport::Node node;

  gz::msgs::StringMsg rep;
  bool result;
  unsigned int timeout = 5000;

  // Request the "/quote" service.
  bool executed = node.Request("/quote", timeout, rep, result);

  if (executed)
  {
    if (result)
      std::cout << "Response: [" << rep.data() << "]" << std::endl;
    else
      std::cout << "Service call failed" << std::endl;
  }
  else
    std::cerr << "Service call timed out" << std::endl;
}
```

### Walkthrough

First of all we declare a node and a message that will contain the response from
``/quote`` service. Next, we use the variant without input parameter of the
``Request()`` method. The return value of ``Request()`` indicates whether the
request timed out or reached the service provider and ``result`` shows if the
service was successfully executed.

We also have the async version for service request without input. You should
download [requester_async_no_input.cc](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/requester_async_no_input.cc)
file within the ``gz_transport_tutorial`` folder.

## Building the code

Download the [CMakeLists.txt](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/CMakeLists.txt) file
within the ``gz_transport_tutorial`` folder. Then, create a `msgs` directory
and download [CMakeLists.txt](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/msgs/CMakeLists.txt) and [stringmsg.proto](https://github.com/gazebosim/gz-transport/raw/gz-transport14/example/msgs/stringmsg.proto) inside the
``msgs`` directory.

Once you have all your files, go ahead and create a ``build/`` folder within
the ``gz_transport_tutorial`` directory.

```{.sh}
mkdir build
cd build
```

Run ``cmake`` and build the code.

```{.sh}
# Linux and MacOS
cmake ..
make responser responser_oneway requester requester_async requester_oneway
make responser_no_input requester_no_input requester_async_no_input

# Windows
cmake --build . --config Release
```

## Running the examples

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

> **NOTE**
> On Windows, you can see firewall or antivirus prompts when running the examples.
> For them to work properly, you should allow all communication to the
> example programs.

Open three new terminals and from your ``build/`` directory run the executables.

From terminal 1:

```{.sh}
# Linux and MacOS
./responser

# Windows
.\Release\responser.exe
```

From terminal 2:

```{.sh}
# Linux and MacOS
./requester

# Windows
.\Release\requester.exe
```

From terminal 3:

```{.sh}
# Linux and MacOS
./requester_async

# Windows
.\Release\requester_async.exe
```

In your requester terminals, you should expect an output similar to this one,
showing that your requesters have received their responses:

```{.sh}
$ ./requester
Response: [HELLO]
```

```{.sh}
$ ./requester_async
Press <CTRL-C> to exit
Response: [HELLO]
```

For running the oneway examples, open two terminals and from your ``build/``
directory run the executables.

From terminal 1:

```{.sh}
# Linux and MacOS
./responser_oneway

# Windows
.\Release\responser_oneway.exe
```

From terminal 2:

```{.sh}
# Linux and MacOS
./requester_oneway

# Windows
.\Release\requester_oneway.exe
```

In your responser terminal, you should expect an output similar to this one,
showing that your service provider has received a request:

```{.sh}
./responser_oneway
Request received: [HELLO]
```

For running the examples without input, open three terminals and from your
``build/`` directory run the executables.

From terminal 1:

```{.sh}
# Linux and MacOS
./responser_no_input

# Windows
.\Release\responser_no_input.exe
```

From terminal 2:

```{.sh}
# Linux and MacOS
./requester_no_input

# Windows
.\Release\requester_no_input.exe
```

From terminal 3:

```{.sh}
# Linux and MacOS
./requester_async_no_input

# Windows
.\Release\requester_async_no_input.exe
```

In your requesters' terminals, you should expect an output similar to this one,
showing that you have received a response:

```{.sh}
./requester_no_input

Press <CTRL-C> to exit
Response: [This is it! This is the answer. It says here...that a bolt of
lightning is going to strike the clock tower at precisely 10:04pm, next
Saturday night! If...If we could somehow...harness this lightning...channel
it...into the flux capacitor...it just might work. Next Saturday night,
we're sending you back to the future!]
```
