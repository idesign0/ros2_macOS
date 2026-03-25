Plugin Generation
#################

The ``cx_ros_comm_gen`` package provides cmake functions to generate bindings for ROS messages, services and actions via the following macros:

.. code-block:: cmake

  cx_generate_msg_bindings(package msg_name)

  cx_generate_srv_bindings(package srv_name)

  cx_generate_action_bindings(package action_name)

These macros can be used in any existing package that uses :rostut:`ament_cmake <How-To-Guides/Ament-CMake-Documentation.html>` as build tool. See the below snippet for an example:

.. code-block:: cmake

  find_package(cx_ros_comm_gen REQUIRED)

  # generates the cx::CXStdMsgsStringPlugin
  cx_generate_msg_bindings("std_msgs" "String")

  # generates the cx::CXStdSrvsSetBoolPlugin
  cx_generate_srv_bindings("std_srvs" "SetBool")

  # generates the cx::CXExampleInterfacesFibonacciPlugin
  cx_generate_action_bindings("example_interfaces" "Fibonacci")

Note that upon invoking these functions the following dependencies will be used, hence they should be added to the belonging packages *package.xml*:

.. code-block:: cmake

  find_package(${package} REQUIRED)
  find_package(pluginlib REQUIRED)
  find_package(clips_vendor REQUIRED)
  find_package(clips REQUIRED)
  find_package(rclcpp_action REQUIRED) # only if action bindings are generated

Pitfalls and Limitations
************************

Nested Messages
~~~~~~~~~~~~~~~

It is quite common to have nested messages in ROS.
Just creating the binding to the top-level message does not give access to the functions required to create or read the message fields that are of a different message type.
In that case it is necessary to also generate bindings for the nested messages.

Object Lifetimes and CLIPS
~~~~~~~~~~~~~~~~~~~~~~~~~~

Since clips stores objects via void pointers, dynamic object lifetime management via `std::shared_ptr` does not work directly from within CLIPS.
Instead, object lifetimes need to be managed more explicitly through the usage of `create` and `destroy` functions.

It is advised to clean up all objects as soon as they are not needed anymore in order to free up memory.

This is mostly straight-forward in case of short lifetimes (e.g., creating a message and immediately sending it), but can be a bit tricky in case of long-lasting references, such as goals and goal handles in action clients/servers. In particular, it is advised to clean up resources belonging to a goal once the goal is fully processed and either rejected or finished.
