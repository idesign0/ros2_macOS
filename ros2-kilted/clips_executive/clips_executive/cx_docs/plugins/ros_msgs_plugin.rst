.. _usage_ros_msgs_plugin:

Generic ROS Msg Plugin
######################

Source code on :source-master:`GitHub <cx_plugins/ros_msgs_plugin>`.

.. admonition:: Plugin Class

  cx::RosMsgsPlugin

This plugin provides the ability to interface with ROS topics of any type using introspection.

.. attention::

   ROS introspection used to be limited to topics.
   Service introspection was started in Jazzy and finished in Kilted.
   Action introspection was started in Kilted and is not completed yet.
   The features of this plugin vary, depending on the ROS version it is compiled with.

Configuration
*************

This plugin has no specific configuration options.

Features
********

Facts
~~~~~

.. code-block:: lisp

  ; Asserted by the create-subscription function.
  ; Retracted by the destroy-subscription function.
  (deftemplate ros-msgs-subscription
    (slot topic (type STRING)) ; example: "/cx_string_in"
    (slot type (type STRING))  ; example: "std_msgs/msg/String"
  )

  ; Asserted by the ros-msgs-create-publisher function.
  ; Retracted by the respective ros-msgs-destroy-publisher function.
  (deftemplate-ros-msgs-publisher
    (slot topic (type STRING)) ; example: "/cx_string_out"
    (slot type (type STRING))  ; example: "std_msgs/msg/String"
  )

  ; Asserted by the callback of a subscription whenever a message arrives.
  ; Process the message and then call ros-msgs-destroy-message before retracting!
  (deftemplate ros-msgs-message
    (slot topic (type STRING))             ; example: "/cx_string_in"
    (slot msg-ptr (type EXTERNAL-ADDRESS)) ; example: <Pointer-C-0x7f1550001d20>
  )

Supported since Jazzy:

.. code-block:: lisp

  ; Asserted by the ros-msgs-create-client function.
  ; Retracted by the respective ros-msgs-destroy-client function.
  (deftemplate-ros-msgs-client
    (slot topic (type STRING)) ; example: "/ros_cx_client"
    (slot type (type STRING))  ; example: "std_srvs/srv/SetBool"
  )

  ; Asserted by the callback of ros-msgs-async-send-request.
  ; Process the response like a normal message and then call
  ; ros-msgs-destroy-message before retracting!
  (deftemplate ros-msgs-response
    (slot service (type STRING))            ; example: "/ros_cx_client"
    (slot request-id (type INTEGER))        ; example: 1
    (slot msg-ptr (type EXTERNAL-ADDRESS))  ; example: <Pointer-C-0x7f1550001d20>
  )

Supported since Kilted:

.. code-block:: lisp

  ; Asserted by the ros-msgs-create-service function.
  ; Retracted by the respective ros-msgs-destroy-service function.
  (deftemplate ros-msgs-service
    (slot service (type STRING)) ; example: "/ros_cx_service"
    (slot type (type STRING))    ; example: "std_srvs/srv/SetBool"
  )

  ; Asserted by the ros-msgs-create-action-client function.
  ; Retracted by the respective ros-msgs-destroy-action-client function.
  (deftemplate ros-msgs-action-client
    (slot server (type STRING))
    (slot type (type STRING)))
  )

  ; Asserted by the goal response callback of ros-msgs-async-send-goal function.
  ; Process the response by using the client goal handle functions:
  ; - ros-msgs-client-goal-handle-get-goal-id
  ; - ros-msgs-client-goal-handle-get-goal-stamp
  ; - ros-msgs-client-goal-handle-get-status
  ; Call ros-msgs-destroy-client-goal-handle before retracting.
  ; Clean up only after goal is completely handled by the server.
  (deftemplate ros-msgs-goal-response
    (slot server (type STRING))
    (slot client-goal-handle-ptr (type EXTERNAL-ADDRESS))
  )

  ; Asserted by the goal result callback of ros-msgs-async-send-goal function.
  ; The result is a ROS message and can be processed using ros-msgs-get-field.
  (deftemplate ros-msgs-wrapped-result
    (slot server (type STRING))
    (slot goal-id (type SYMBOL))
    (slot code (type SYMBOL) (allowed-values UNKNOWN SUCCEEDED CANCELED  ABORTED))
    (slot result-ptr (type EXTERNAL-ADDRESS))
  )

  ; Asserted by the goal feedback callback of ros-msgs-async-send-goal function.
  ; The feedback is a ROS message and can be processed using ros-msgs-get-field.
  (deftemplate ros-msgs-feedback
    (slot server (type STRING))
    (slot client-goal-handle-ptr (type EXTERNAL-ADDRESS))
    (slot feedback-ptr (type EXTERNAL-ADDRESS))
  )


Functions
~~~~~~~~~

.. code-block:: lisp

  ; Create and destroy publishers, subscribers
  (ros-msgs-create-publisher ?topic-name ?msg-type)    ; example args: "/cx_string_out" "std_msgs/msg/String"
  (ros-msgs-destroy-publisher ?topic-name)             ; example args: "/cx_string_out"
  (ros-msgs-create-subscription ?topic-name ?msg-type) ; example args: "/cx_string_in" "std_msgs/msg/String"
  (ros-msgs-destroy-subscription ?topic-name)          ; example args: "/cx_string_in"

  ; Publish a given message over a topic.
  ; Requires the publisher to be created first using ros-msgs-create-publisher.
  (ros-msgs-publish ?msg-ptr ?topic-name)

  ; Create a message and return a pointer to it
  (bind ?msg-ptr (ros-msgs-create-message ?msg-type)) ; example args: "std_msgs/msg/String"
                                                      ; example ret: <Pointer-C-0x7f1550001d20>

  ; Destroy a message, call this after publishing a message or processing an incoming message to prevent it from staying in memory.
  (ros-msgs-destroy-message ?msg-ptr) ; example args: <Pointer-C-0x7f1550001d20>

  ; Populate the field of a message.
  ; If the field is a message, then pass a pointer to that message obtained from ros-msgs-create-message.
  (ros-msgs-set-field ?msg-ptr ?field-name ?field-value) ; example args: <Pointer-C-0x7f1550001d20> "data" "Hello World"

  ; Retrieve a field of a message.
  ; If the field is a message, then a pointer is returned that can be further inspected by passing it to ros-msgs-get-field.
  (ros-msgs-get-field ?msg-ptr ?field-name)

  ; Convert an array representation of a goal uuid to a symbol using rclcpp_action::to_string.
  (bind ?uuid-sym (ros-msgs-goal-uuid-to-string ?uuid-multifield)) ; example arg: (43 47 40 182 6 191 57 8 170 98 1 22 43 138 62 142)
                                                                   ; example ret: 2b2f28b6-06bf-3908-aa62-01162b8a3e8e


Supported since Jazzy:

.. code-block:: lisp

  ; Create and destroy service clients.
  (ros-msgs-create-client ?service-name ?service-type) ; example args: "/ros_cx_client" "std_srvs/srv/SetBool"
  (ros-msgs-destroy-client ?service-name)              ; example args: "/ros_cx_client"

  ; Create a request message and return a pointer to it.
  ; It is a regular message and hence can be pouplated with ros-msgs-set-field
  ; and destroyed with ros-msgs-destroy-message.
  (bind ?msg-ptr (ros-msgs-create-request ?msg-type)) ; example args: "std_srvs/srv/SetBool"
                                                      ; example ret: <Pointer-C-0x7f1550001d20>

  ; Send a given request message to a service.
  ; Requires the service client to be created first using ros-msgs-create-client.
  (bind ?request-id (ros-msgs-async-send-request ?msg-ptr ?service-name)) ; example args: "std_srvs/srv/SetBool"
                                                                          ; example ret: 1 (or FALSE if sending the request failed)


Supported since Kilted:

.. code-block:: lisp

  ; Create and destroy service providers.
  ; In order to answer service requests, define a function with the following signature:
  ; (deffunction <service_name>-service-callback (?name ?request ?response)
  ;   <process request and populate response like regular ros messages>
  ; )
  ; Do not cleanup the request or response, their lifetime is handled automatically.
  ;
  ; Example:
  ; (deffunction /ros_cx_service-service-callback (?name ?request ?response)
  ;   (printout green "Received a request" crlf)
  ;   (bind ?data (ros-msgs-get-field ?request "data"))
  ;   (printout yellow ?data crlf)
  ;   (ros-msgs-set-field ?response "success" TRUE)
  ;   (ros-msgs-set-field ?response "message" (str-cat "Received " ?data))
  ; )
  (ros-msgs-create-service ?service-name ?service-type) ; example_args: "/ros_cx_service" "std_srvs/srv/SetBool"
  (ros-msgs-destroy-service ?service-name)              ; example_args: "/ros_cx_service"

  ; Create and destroy action clients.
  (ros-msgs-create-action-client ?action-server ?type) ; eample args: "fibonacci" "example_interfaces/action/Fibonacci"
  (ros-msgs-destroy-action-client ?action-server) ; example args: "fibonacci"

  ; Create a goal request and return a pointer to it.
  ; It is a regular message and hence can be pouplated with ros-msgs-set-field
  ; and destroyed with ros-msgs-destroy-message..
  (bind ?req-ptr (ros-msgs-create-goal-request ?type)) ; example args: "example_interfaces/action/Fibonacci"
                                                       ; example ret: <Pointer-C-0x7f1550001d20>

  ; Send a goal request to an action server.
  ; This registers three callbacks: response, feedback and result.
  ; These assert the corrending facts of the following types:
  ; - ros-msgs-goal-response
  ; - ros-msgs-wrapped-result
  ; - ros-msgs-feedback
  (ros-msgs-async-send-goal ?action-server ?req-ptr) ; example args: "fibonacci" <Pointer-C-0x7f1550001d20>

  ; Functions to inspect client goal handles that are contained in the facts asserted by action callbacks.

  ; Uses rclcpp_action::to_string to provide a formatted version of the associated goal id (as SYMBOL).
  (bind ?g-id (ros-msgs-client-goal-handle-get-goal-id ?client-goal-handle-ptr)) ; example args: <Pointer-C-0x7f1550001d20>
                                                          ; example ret: f2d1cd46-58f2-5bfe-d49f-e182c4a4eccc

  ; Provide the goal stamp using get_goal_stamp().seconds()
  (bind ?ts (ros-msgs-client-goal-handle-get-goal-stamp ?client-goal-handle-ptr)) ; example args: <Pointer-C-0x7f1550001d20>
                                                                                  ; example ret: 1698326401.123456

  ; Checks if a feedback callback is registered (should always return true, as the plugin always registers one.
  (bind ?aware (ros-msgs-client-goal-handle-is-feedback-aware ?client-goal-handle-ptr)) ; example args: <Pointer-C-0x7f1550001d20>
                                                                                        ; example ret: TRUE

  ; Checks if a result callback is registered (should always return true, as the plugin always registers one.
  (bind ?aware (ros-msgs-client-goal-handle-is-result-aware ?client-goal-handle-ptr)) ; example args: <Pointer-C-0x7f1550001d20>
                                                                                      ; example ret: TRUE

  ; Delete a client goal handle. Use this only after the associated goal is fully processed.
  (ros-msgs-destroy-client-goal-handle ?client-goal-handle-ptr) ; example args: <Pointer-C-0x7f1550001d20>


Message Lifetimes
~~~~~~~~~~~~~~~~~

Since clips stores objects via void pointers, dynamic object lifetime management via `std::shared_ptr` does not work directly from within CLIPS.
Instead, object lifetimes need to be managed more explicitly through the usage of `create` and `destroy` functions.

It is advised to clean up all objects as soon as they are not needed anymore in order to free up memory.

Note that when processing nested messages, the message obtained via **ros-msgs-get-field** is not allocating new memory, but rather points to the memory of the parent message.
Calling **ros-msgs-destroy-message** is not necessary, as sub-messages retrieved via **ros-msgs-get-field** only hold a shallow reference and are cleaned up when the parent message is destroyed. Calling it anyways, will only invalidate this shallow reference.

When creating a new message via **ros-msgs-create-message**, new memory is allocated.
When using **ros-msgs-set-field** to set a nested message, dynamic memory from the sub-message is moved to the parent message, hence the nested message loses all dynamically allocated data (e.g., unbound arrays, strings).
See the example below:

.. code-block:: lisp

  (bind ?new-msg (ros-msgs-create-message "geometry_msgs/msg/Twist"))
  (bind ?sub-msg (ros-msgs-create-message "geometry_msgs/msg/Vector3"))
  (ros-msgs-set-field ?sub-msg "x" 1.0)
  (ros-msgs-set-field ?new-msg "linear" ?sub-msg)
  ; now all dynamicly allocated members of ?sub-msg are reset,
  ; as they are moved to the parent message.

In particular, obtaining a nested message from one message ``?source`` and setting it as a member to another message ``?sink`` will cause ``?source`` to lose all dynamic data within it's sub-message, as the sub-message obtained is actually pointing to memory within ``?source``:

.. code-block:: lisp

  (bind ?source (ros-msgs-create-message "geometry_msgs/msg/Twist"))
  (bind ?sub-msg (ros-msgs-create-message "geometry_msgs/msg/Vector3"))
  (ros-msgs-set-field ?sub-msg "x" 5.5)
  (ros-msgs-set-field ?source "linear" ?sub-msg)
  (bind ?sink (ros-msgs-create-message "geometry_msgs/msg/Twist"))
  (bind ?source-sub-msg (ros-msgs-get-field ?source "linear"))
  (ros-msgs-set-field ?sink "linear" ?source-sub-msg)
  ; now all dynamicly allocated members of the sub message in ?source are reset,
  ; as they are moved to ?sink.

Action Lifetime
~~~~~~~~~~~~~~~

When dealing with actions, always make sure to keep the relevant goal handles alive for the entire duration of the associated action. Do not attempt to preemptively delete it. The same goes for goal requests.

Usage Example 1: Publishers and Subscribers
*******************************************

A minimal working example is provided by the :docsite:`cx_bringup` package. Run it via:

.. code-block:: bash

    ros2 launch cx_bringup cx_launch.py manager_config:=plugin_examples/ros_msgs.yaml

It creates a ``std_msgs/msg/String`` subscription on topic ``/ros_cx_in`` and prints out any text send over it.
Additionally, it creates a publisher on ``/ros_cx_out`` that publishes ``Hello World`` whenever something is received over the ``/ros_cx_in`` topic.

Configuration
~~~~~~~~~~~~~

File :source-master:`cx_bringup/params/plugin_examples/ros_msgs.yaml`.

.. code-block:: yaml

  clips_manager:
    ros__parameters:
      environments: ["cx_ros_msgs"]

      cx_ros_msgs:
        plugins: ["executive", "ros_msgs", "files"]
        watch: ["facts", "rules"]

      executive:
        plugin: "cx::ExecutivePlugin"

      ros_msgs:
        plugin: "cx::RosMsgsPlugin"

      files:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_bringup"]
        load: [
          "clips/plugin_examples/ros-msgs.clp"]


Code
~~~~

File :source-master:`cx_bringup/clips/plugin_examples/ros-msgs.clp`.

.. code-block:: lisp

  (defrule ros-msgs-pub-init
  " Create publisher for ros_cx_out."
    (not (ros-msgs-publisher (topic "ros_cx_out")))
    (not (executive-finalize))
  =>
    ; create the publisher
    (ros-msgs-create-publisher "ros_cx_out" "std_msgs/msg/String")
    (printout info "Publishing on /ros_cx_out" crlf)
  )

  (defrule ros-msgs-pub-hello
  " Whenever a message comes in, send out a Hello World message in response. "
    (declare (salience 1))
    (ros-msgs-publisher (topic ?topic))
    (ros-msgs-message)
    =>
    (printout yellow "Sending Hello World Message!" crlf)
    (bind ?msg (ros-msgs-create-message "std_msgs/msg/String"))
    (ros-msgs-set-field ?msg "data" "Hello world!")
    (ros-msgs-publish ?msg ?topic)
    (ros-msgs-destroy-message ?msg)
  )

  (defrule ros-msgs-sub-init
  " Create a simple subscriber using the generated bindings. "
    (not (ros-msgs-subscription (topic "ros_cx_in")))
    (not (executive-finalize))
  =>
    (ros-msgs-create-subscription "ros_cx_in" "std_msgs/msg/String")
    (printout info "Listening for String messages on /ros_cx_in" crlf)
  )

  (defrule ros-msgs-receive
  " React to incoming messages and answer (on a different topic). "
    (ros-msgs-subscription (topic ?sub))
    ?msg-f <- (ros-msgs-message (topic ?sub) (msg-ptr ?inc-msg))
    =>
    (bind ?recv (ros-msgs-get-field ?inc-msg "data"))
    (printout blue "Recieved via " ?sub ": " ?recv crlf)
    (ros-msgs-destroy-message ?inc-msg)
    (retract ?msg-f)
  )

  (defrule ros-msgs-pub-sub-finalize
  " Delete the publisher and subscription on executive finalize. "
    (executive-finalize)
    (ros-msgs-publisher (topic ?topic))
    (ros-msgs-subscription (topic ?in-topic))
  =>
    (printout info "Destroying publishers and subscriptions " crlf)
    (ros-msgs-destroy-publisher ?topic)
    (ros-msgs-destroy-subscription ?in-topic)
  )

  (defrule ros-msgs-message-cleanup
  " Delete the messages on executive finalize. "
    (executive-finalize)
    ?msg-f <- (ros-msgs-message (msg-ptr ?ptr))
  =>
    (ros-msgs-destroy-message ?ptr)
    (retract ?msg-f)
  )

Usage Example 2: Service Client
*******************************

This example requires ROS jazzy or later.

A minimal working example is provided by the :docsite:`cx_bringup` package. Run it via:

.. code-block:: bash

    ros2 launch cx_bringup cx_launch.py manager_config:=plugin_examples/ros_msgs_client.yaml

It creates a ``std_srvs/srv/SetBool`` client for ``/ros_cx_client``, sends a request and prints the response.

In order to launch a simple service to send the request to, run the following command:

.. code-block:: bash

    ros2 run cx_bringup test_service.py

Configuration
~~~~~~~~~~~~~

File :source-master:`cx_bringup/params/plugin_examples/ros_msgs_client.yaml`.

.. code-block:: yaml

  clips_manager:
    ros__parameters:
      environments: ["cx_ros_msgs_client"]

      cx_ros_msgs_client:
        plugins: ["executive", "ros_msgs", "files"]
        watch: ["facts", "rules"]

      executive:
        plugin: "cx::ExecutivePlugin"

      ros_msgs:
        plugin: "cx::RosMsgsPlugin"

      files:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_bringup"]
        load: [
          "clips/plugin_examples/ros-msgs-client.clp"]


Code
~~~~

File :source-master:`cx_bringup/clips/plugin_examples/ros-msgs.clp`.

.. code-block:: lisp

  (defrule ros-msgs-client-init
  " Create publisher for ros_cx_out."
    (not (ros-msgs-client (service "ros_cx_client")))
    (not (executive-finalize))
  =>
    ; create the publisher
    (ros-msgs-create-client "ros_cx_client" "std_srvs/srv/SetBool")
    (printout info "Opening client on /ros_cx_client" crlf)
  )

  (defrule ros-msgs-request-true
  " Attempt to request the service. "
    (ros-msgs-client (service ?service))
    (not (request ?any-id))
    (time ?any-time) ; used to continuously attempt to request the service until success
    =>
    (bind ?new-req (ros-msgs-create-request "std_srvs/srv/SetBool"))
    (ros-msgs-set-field ?new-req "data" TRUE)
    (bind ?id (ros-msgs-async-send-request ?new-req ?service))
    (if ?id then
      (printout yellow "Request sent with id " ?id crlf)
      (assert (request ?id))
     else
      (printout red "Sending of request failed, is the service running?" crlf)
      (printout red "Start it using \"ros2 run cx_bringup test_service.py\"" crlf)
    )
    (ros-msgs-destroy-message ?new-req)
  )

  (defrule set-bool-client-response-received
  " Get response, read it and delete."
    ?msg-fact <- (ros-msgs-response (service ?service) (msg-ptr ?ptr) (request-id ?id))
    (request ?id)
  =>
    (bind ?succ (ros-msgs-get-field ?ptr "success"))
    (bind ?msg (ros-msgs-get-field ?ptr "message"))
    (printout green "Received response from " ?service " with: " ?succ " (" ?msg ")" crlf)
    (ros-msgs-destroy-message ?ptr)
    (retract ?msg-fact)
  )

  (defrule ros-msgs-client-finalize
  " Delete the client on executive finalize. "
    (executive-finalize)
    (ros-msgs-client (service ?service))
  =>
    (printout info "Destroying client" crlf)
    (ros-msgs-destroy-client ?service)
  )

  (defrule ros-msgs-message-cleanup
  " Delete the messages on executive finalize. "
    (executive-finalize)
    ?msg-f <- (ros-msgs-message (msg-ptr ?ptr))
  =>
    (ros-msgs-destroy-message ?ptr)
    (retract ?msg-f)
  )

Usage Example 3: Service Provider
*********************************

This example requires ROS kilted or later.

A minimal working example is provided by the :docsite:`cx_bringup` package. Run it via:

.. code-block:: bash

    ros2 launch cx_bringup cx_launch.py manager_config:=plugin_examples/ros_msgs_service.yaml

It creates a ``std_srvs/srv/SetBool`` service ``/ros_cx_service`` and prints all requests, returning successfully and adding a message with the request.

In order to launch a simple client to send the request, run the following command:

.. code-block:: bash

    ros2 service call /ros_cx_service std_srvs/srv/SetBool "{data: true}"

Configuration
~~~~~~~~~~~~~

File :source-master:`cx_bringup/params/plugin_examples/ros_msgs_service.yaml`.

.. code-block:: yaml

  /**:
    ros__parameters:
      environments: ["cx_ros_msgs_service"]

      cx_ros_msgs_service:
        plugins: ["executive", "ros_msgs", "files"]
        log_clips_to_file: true
        watch: ["facts", "rules"]

      executive:
        plugin: "cx::ExecutivePlugin"
        assert_time: true

      ros_msgs:
        plugin: "cx::RosMsgsPlugin"

      files:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_bringup"]
        load: [
          "clips/plugin_examples/ros-msgs-service.clp"]


Code
~~~~

File :source-master:`cx_bringup/clips/plugin_examples/ros-msgs-service.clp`.

.. code-block:: lisp

  (defrule ros-msgs-service-init
  " Create a service /ros_cx_service"
    (not (ros-msgs-service (service "ros_cx_service")))
    (not (executive-finalize))
  =>
    (ros-msgs-create-service "ros_cx_service" "std_srvs/srv/SetBool")
    (printout green "Opening service on /ros_cx_service" crlf)
    (printout green "Call it via 'ros2 service call /ros_cx_service std_srvs/srv/SetBool \"{data: true}\""' crlf)
  )

  (deffunction ros_cx_service-service-callback (?name ?request ?response)
    (printout green "Received a request" crlf)
    (bind ?data (ros-msgs-get-field ?request "data"))
    (printout yellow ?data crlf)
    (ros-msgs-set-field ?response "success" TRUE)
    (ros-msgs-set-field ?response "message" (str-cat "Received " ?data))
  )

  (defrule ros-msgs-service-finalize
  " Delete the service on executive finalize. "
    (executive-finalize)
    (ros-msgs-service (service ?service))
  =>
    (printout info "Destroying service" crlf)
    (ros-msgs-destroy-service ?service)
  )

Usage Example 4: Action Client
******************************

This example requires ROS kilted or later.

A minimal working example is provided by the :docsite:`cx_bringup` package. Run it via:

.. code-block:: bash

    ros2 launch cx_bringup cx_launch.py manager_config:=plugin_examples/ros_msgs_action_client.yaml

The launch file starts two CLIPS environments:
 - One environment provides an action server that computes Fibonacci sequences. It is implemented using generated bindings for the server via :docsite:`cx_ros_comm_gen`.
 - The second environment acts as an action client. It first requests a Fibonacci sequence of order 5. After the goal completes, it sends a second request for a sequence of order 10. The second goal is canceled after the sixth element has been computed.

Configuration
~~~~~~~~~~~~~

File :source-master:`cx_bringup/params/plugin_examples/ros_msgs_action_client.yaml`.

.. code-block:: yaml

  /**:
    ros__parameters:
      environments: ["cx_ros_msgs_action_client", "cx_fibonacci_server"]

      cx_ros_msgs_action_client:
        plugins: ["executive", "ros_msgs", "files_client"]
        log_clips_to_file: true
        watch: ["facts", "rules"]
        redirect_stdout_to_debug: true

      cx_fibonacci_server:
        plugins: ["executive", "fibonacci", "files_server"]
        log_clips_to_file: true
        watch: ["facts", "rules"]
        redirect_stdout_to_debug: true

      executive:
        plugin: "cx::ExecutivePlugin"
        assert_time: true

      fibonacci:
        plugin: "cx::CXExampleInterfacesFibonacciPlugin"

      ros_msgs:
        plugin: "cx::RosMsgsPlugin"

      files_client:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_bringup"]
        load: [
          "clips/plugin_examples/ros-msgs-action-client.clp"]

      files_server:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_bringup"]
        load: [
          "clips/plugin_examples/fibonacci-action-server.clp"]



Code
~~~~

File :source-master:`cx_bringup/clips/plugin_examples/ros-msgs-action-client.clp`.

.. code-block:: lisp

  (defrule ros-msgs-action-client-init
  " Create a client for fibonacci example action server."
    (not (ros-msgs-action-client (server "ros_cx_fibonacci")))
    (not (executive-finalize))
  =>
    (ros-msgs-create-action-client "ros_cx_fibonacci" "example_interfaces/action/Fibonacci")
    (printout yellow "Created client for /ros_cx_fibonacci" crlf)
  )

  (defrule ros-msgs-action-client-send-goal
  " Send a goal request for the 10th fibonacci number. "
    (ros-msgs-action-client (server ?server))
    (not (send-request))
  =>
    (assert (send-request))
    (bind ?new-req (ros-msgs-create-goal-request "example_interfaces/action/Fibonacci"))
    (ros-msgs-set-field ?new-req "order" 5)
    (bind ?val (ros-msgs-get-field ?new-req "order"))
    (bind ?id (ros-msgs-async-send-goal ?new-req ?server))
    (assert (fibonacci-goal ?new-req))
    (printout yellow "Sending goal fibonacci(5)" crlf)
    ; do not destroy goal directly, it is kept alive
    ;(ros-msgs-destroy-message ?new-req)
  )

  (defrule ros-msgs-action-client-check-client-goal-handle
  " Process the response for the requested goal."
    (ros-msgs-action-client (server ?server))
    (ros-msgs-goal-response (server ?server) (client-goal-handle-ptr ?cgh-ptr))
  =>
    (bind ?g-id (ros-msgs-client-goal-handle-get-goal-id ?cgh-ptr))
    (bind ?stamp (ros-msgs-client-goal-handle-get-goal-stamp ?cgh-ptr))
    (printout yellow "Response: goal id " ?g-id " at stamp " ?stamp " with status " (ros-msgs-client-goal-handle-get-status ?cgh-ptr) crlf)
  )

  (defrule ros-msgs-action-client-read-feedback
    (ros-msgs-action-client (server ?server))
   ?fact <- (ros-msgs-feedback (server ?server) (client-goal-handle-ptr ?cgh-ptr) (feedback-ptr ?msg-ptr))
  =>
    (bind ?seq (ros-msgs-get-field ?msg-ptr "sequence"))
    (printout yellow "partial sequence: " ?seq crlf)
    (ros-msgs-destroy-message ?msg-ptr)
    (if (= (length$ ?seq) 7) then
       (assert (cancel-goal))
    )
    (retract ?fact)
  )

  (defrule ros-msgs-action-client-cleanup-after-wrapped-result
    ?response-f <- (ros-msgs-goal-response (server ?server) (client-goal-handle-ptr ?cgh-ptr))
    ?result-f <- (ros-msgs-wrapped-result (server ?server) (goal-id ?id) (code ?code) (result-ptr ?res-ptr))
    (test (eq ?id (ros-msgs-client-goal-handle-get-goal-id ?cgh-ptr)))
    ?req-f <- (fibonacci-goal ?req-ptr)
  =>
    (printout yellow "Action result: " ?code crlf)
    (if (eq ?code SUCCEEDED)
      then
      (bind ?seq (ros-msgs-get-field ?res-ptr "sequence"))
      (printout yellow "Final fibonacci sequence: " ?seq crlf)
    )
    (if (eq ?code CANCELED)
      then
      (bind ?seq (ros-msgs-get-field ?res-ptr "sequence"))
      (printout yellow "Canceled fibonacci sequence: " ?seq crlf)
    )
    (ros-msgs-destroy-message ?res-ptr)
    (ros-msgs-destroy-message ?req-ptr)
    (ros-msgs-destroy-client-goal-handle ?cgh-ptr)
    (retract ?response-f ?result-f ?req-f)
  )

  (defrule ros-msgs-action-client-send-goal-that-cancels-later
  " Send out a second goal for the fibonacci sequence of order 10 once the previous goal is completed. "
    (ros-msgs-action-client (server ?server))
    (send-request)
    (not (fibonacci-goal ?))
    (not (send-request-again))
  =>
    (assert (send-request-again))
    (printout yellow "Request fibonacci(10), will cancel before finish" crlf)
    (bind ?new-req (ros-msgs-create-goal-request "example_interfaces/action/Fibonacci"))
    (ros-msgs-set-field ?new-req "order" 10)
    (bind ?val (ros-msgs-get-field ?new-req "order"))
    (bind ?id (ros-msgs-async-send-goal ?new-req ?server))
    (assert (fibonacci-goal ?new-req))
    ; do not destroy goal directly, it is kept alive
    ;(ros-msgs-destroy-message ?new-req)
  )

  (defrule ros-msgs-action-client-request-cancel
  " Send out a cancel request after the partial feedback provides a sequence of length 7. "
    (cancel-goal)
    (ros-msgs-action-client (server ?server))
    (ros-msgs-goal-response (server ?server) (client-goal-handle-ptr ?ghp))
  =>
    (ros-msgs-async-cancel-goal ?server ?ghp)
    (printout yellow "Canceling current goal" crlf)
  )

  (defrule ros-msgs-action-client-cleanup-cancel-response
  " Process the cancel response and clean up afterwards. "
    ?msg-f <- (ros-msgs-cancel-response (cancel-response-ptr ?msg))
  =>
    (bind ?error-code (ros-msgs-get-field ?msg "return_code"))
    (bind ?goals-canceling (ros-msgs-get-field ?msg "goals_canceling"))
    (foreach ?g ?goals-canceling
      (bind ?goal-id-msg (ros-msgs-get-field ?g "goal_id"))
      (bind ?goal-id (ros-msgs-get-field ?goal-id-msg "uuid"))
      (bind ?goal-id-str (ros-msgs-goal-uuid-to-string ?goal-id))
      (printout yellow "Canceled goal with id: " ?goal-id-str crlf)
      (ros-msgs-destroy-message ?goal-id-msg)
    )
    (ros-msgs-destroy-message ?msg)
  )

  (defrule ros-msgs-action-client-finalize
  " Delete the client on executive finalize. "
    (executive-finalize)
    (ros-msgs-action-client (server ?server))
  =>
    (printout info "Destroying action client" crlf)
    (ros-msgs-destroy-action-client ?server)
  )
