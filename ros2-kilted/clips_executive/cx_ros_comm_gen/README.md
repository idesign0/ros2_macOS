# cx_ros_comm_gen
This package offers a convenient generator that allows to interface with ros nodes via messages, services and action clients/servers.

## Usage
The usage involves a few steps:
1. Generating the bindings
2. Loading the generated bindings as CLIPS Executive plugins
3. Using the generated functions and templates

### Generating Bindings
The *cx_ros_comm_gen* package provides cmake functions to generate bindings for ros messages, services and actions via the following macros:
```cmake
cx_generate_msg_bindings(package msg_name)

cx_generate_srv_bindings(package srv_name)

cx_generate_action_bindings(package action_name)
```
See the below snippet for an example (taken from the *cx_bringup* package):
```cmake
find_package(cx_ros_comm_gen REQUIRED)

# generate bindings to interface with ROS messages, such as std_msgs::msg::String
cx_generate_msg_bindings("std_msgs" "String")

# generate bindings to interface with ROS services, such as std_srvs::srv::SetBool
cx_generate_srv_bindings("std_srvs" "SetBool")

# generate bindings to interface with ROS services, such as example_interfaces::action::Fibonacci
cx_generate_action_bindings("example_interfaces" "Fibonacci")
```
Note that upon invoking these functions the following dependencies will be used, hence it makes sense to add them to the belonging *package.xml*:
```cmake
find_package(${package} REQUIRED)
find_package(pluginlib REQUIRED)
find_package(clips_vendor REQUIRED)
find_package(clips REQUIRED)
find_package(rclcpp_action REQUIRED) # only if action bindings are generated
```
Invoking these macros causes the following effects:
1. A cmake `custom_command` is created that invokes the *generator.py* script of this package
2. A shared library for the plugin is created using the generated c++ files from the custom command
3. A plugin description file is installed so the plugin becomes available via pluginlib

### Registering Bindings
Register the generated plugins with the plugin manager from the *cx_plugin_manager* package via it's configuration file.
For the example above, the configuration is depicted below (the snippet from the *cx_bringup* package):
```yaml
clips_plugins_manager:
ros__parameters:
clips_plugins_list: ["cx_std_msgs_string_plugin", "cx_std_srvs_set_bool_plugin", "cx_example_interfaces_fibonacci_plugin"]
clips_plugins:
  cx_std_msgs_string_plugin:
    plugin: "cx::CXStdMsgsStringPlugin"
  cx_std_srvs_set_bool_plugin:
    plugin: "cx::CXStdSrvsSetBoolPlugin"
  cx_example_interfaces_fibonacci_plugin:
    plugin: "cx::CXExampleInterfacesFibonacciPlugin"
```
### Using the Bindings in CLIPS
Given a package (e.g., "std_msgs") and a message (e.g., "String") all deftemplates and functions are named using Kebab-case (as expected in CLIPS) and are are prefixed by `<package-kebab>-<message-kebab>-` (e.g., "std-msgs-string-").

#### Message Bindings
##### Deftemplates
```lisp
; Asserted by the respective create-subscription function.
; Retracted by the respective destroy-subscription function.
(<package-kebab>-<message-kebab>-subscription (topic ?topic-name-string))

; Asserted by the respective create-publisher function.
; Retracted by the respective destroy-publisher function.
(<package-kebab>-<message-kebab>-publisher (topic ?topic-name-string))

; Asserted by the callback of a subscriber whenever a message arrives.
; Process the message and then call destroy-message before retracting!
(<package-kebab>-<message-kebab>-message (topic ?topic-name-string) (msg-ptr ?msg-ptr))
```
##### Functions
```lisp
; Create and destroy publishers and subscribers.
(<package-kebab>-<message-kebab>-create-publisher ?topic-name)
(<package-kebab>-<message-kebab>-destroy-publisher ?topic-name)
(<package-kebab>-<message-kebab>-create-subscription ?topic-name)
(<package-kebab>-<message-kebab>-destroy-subscription ?topic-name)

; Publish a given message over a topic.
; Requires the publisher to be created first using create-publisher.
(<package-kebab>-<message-kebab>-publish ?msg-ptr ?topic-name)

; Create a message and return a pointer to it
(bind ?msg-ptr (<package-kebab>-<message-kebab>-create-message))
; Destroy a message, call this after publishing a message or processing an incoming message to prevent it from staying in memory.
(<package-kebab>-<message-kebab>-destroy-message ?msg-ptr)

; Populate the field of a message.
; If the field is a message, then pass a pointer to that message (by using that respective messages bindings).
(<package-kebab>-<message-kebab>-set-field ?msg-ptr ?field-name ?field-value)

; Retrieve a field of a message.
; If the field is a message, then a pointer is returned that can only be processed by using that respective messages bindings.
(<package-kebab>-<message-kebab>-get-field ?msg-ptr ?field-name)
```
#### Service Bindings
Given a package (e.g., "std_srvs") and a service (e.g., "SetBool") all deftemplates and functions are named using Kebab-case (as expected in CLIPS) and are are prefixed by `<package-kebab>-<service-kebab>-` (e.g., "std-srvs-set-bool-").
##### Deftemplates
```lisp
; Asserted by the respective creat-client function.
; Retracted by the respective destroy-client function.
(<package-kebab>-<service-kebab>-client (service ?service-name-string))

; Asserted by the respective create-service function.
; Retracted by the respective destroy-service function.
(<package-kebab>-<service-kebab>-service (name ?service-name-string))

; Asserted by the callback of a client once a response arrives.
; Process the response and then call response-destroy before retracting!
(<package-kebab>-<service-kebab>-response (service ?service-name-string) (msg-ptr ?msg-ptr))
```
##### Functions
```lisp
; Create and destroy clients and services.
; After creating a service, a user-defined function is called whenever a request arrives
(<package-kebab>-<service-kebab>-create-service ?service-name)
(<package-kebab>-<service-kebab>-destroy-service ?service-name)
(<package-kebab>-<service-kebab>-create-client ?service-name)
(<package-kebab>-<service-kebab>-destroy-client ?service-name)

; Send a request to a service provider
; Requires the client to be created first using create-client.
; Callback will assert a response fact once received.
(<package-kebab>-<service-kebab>-send-request ?msg-ptr ?topic-name)

; Creating, destroying and processing of requests
(bind ?msg-ptr (<package-kebab>-<service-kebab>-request-create))
(<package-kebab>-<service-kebab>-request-destroy ?msg-ptr)
(<package-kebab>-<service-kebab>-request-set-field ?msg-ptr ?field-name ?field-value)
(<package-kebab>-<service-kebab>-request-get-field ?msg-ptr ?field-name)

; Creating, destroying and processing of responses
(bind ?msg-ptr (<package-kebab>-<service-kebab>-response-create))
(<package-kebab>-<service-kebab>-response-destroy ?msg-ptr)
(<package-kebab>-<service-kebab>-response-set-field ?msg-ptr ?field-name ?field-value)
(<package-kebab>-<service-kebab>-response-get-field ?msg-ptr ?field-name)
```
##### Functions Defined by User
```lisp
; Define this to react to service calls.
; Read the request and populate the response using appropriate functions above.
; Do not destroy request or response within this function, they are cleaned up upon exiting the function.
(<package-kebab>-<service-kebab>-service-callback ?service-name ?const-request-ptr ?response-ptr)
```

#### Action Bindings
Given a package (e.g., "example_interfaces") and an action (e.g., "Fibonacci") all deftemplates and functions are named using Kebab-case (as expected in CLIPS) and are are prefixed by `<package-kebab>-<action-kebab>-` (e.g., "example-interfaces-fibonacci-").
##### Deftemplates
```lisp
; Asserted by the respective creat-client function.
; Retracted by the respective destroy-client function.
(<package-kebab>-<action-kebab>-client (server ?server-name-string))

; Asserted by the respective create-server function.
; Retracted by the respective destroy-server function.
(<package-kebab>-<action-kebab>-server (name ?server-name-string))

; Asserted by the goal response callback of a client.
; Process the response and then destroy feedback.
; The client-goal-handle should persist until the processing of the request is fully done.
(<package-kebab>-<action-kebab>-goal-response (server ?server-name-string) (client-goal-handle-ptr ?cgh-ptr))

; Asserted by the goal feedback callback of a client.
; Process the response and then destroy feedback (it is read-only, do not modify it).
; The client-goal-handle should persist (and not be destroyed) until the processing of the goal is fully done and the action is terminated.
; In particular, the client goal handle pointer here will match the one from the goal-response fact.
(<package-kebab>-<action-kebab>-goal-feedback (server ?server-name-string) (client-goal-handle-ptr ?cgh-ptr) (feedback-ptr ?f-ptr))

; Asserted by the goal result callback of a client.
; Process the result and then destroy the result pointer before retracting this fact.
; "code" is either UNKNOWN SUCCEEDED CANCELED or ABORTED.
; "goal-id" is the string representation of a uuid according to rclcpp_action::to_string().
(<package-kebab>-<action-kebab>-wrapped-result (server ?server-name-string) (goal-id ?uuid-str) (code ?code-symbol) (result-ptr ?f-ptr))

; Asserted by the goal result callback of a server.
; Process the server goal handle to retrieve important information such as the goal sent by the client.
; The server-goal-handle should persist (and not be destroyed) until the processing of the action is fully done and the action is terminated.
(<package-kebab>-<action-kebab>-accepted-goal (server ?server-name-string) (server-goal-handle-ptr ?f-ptr))
```
##### Functions
```lisp
; Create and destroy clients and servers.
; After creating a server, user-defined function is called whenever a goal request arrives.
(<package-kebab>-<action-kebab>-create-server ?server-name)
(<package-kebab>-<action-kebab>-destroy-server ?server-name)
(<package-kebab>-<action-kebab>-create-client ?server-name)
(<package-kebab>-<action-kebab>-destroy-client ?server-name)

; Send a goal to an action server
; Requires the client to be created first using create-client.
; Callbacks will assert goal-response feedback and wrapped-result facts.
; Do not destroy the ?msg-ptr immediately, keep it until the goal is fully processed.
(<package-kebab>-<action-kebab>-send-goal ?msg-ptr ?server-name)

; Creating, destroying and processing of goals
(bind ?msg-ptr (<package-kebab>-<action-kebab>-goal-create))
(<package-kebab>-<action-kebab>-goal-destroy ?msg-ptr)
(<package-kebab>-<action-kebab>-goal-set-field ?msg-ptr ?field-name ?field-value)
(<package-kebab>-<action-kebab>-goal-get-field ?msg-ptr ?field-name)

; Creating, destroying and processing of feedback
(bind ?msg-ptr (<package-kebab>-<action-kebab>-feedback-create))
(<package-kebab>-<action-kebab>-feedback-destroy ?msg-ptr)
(<package-kebab>-<action-kebab>-feedback-set-field ?msg-ptr ?field-name ?field-value)
(<package-kebab>-<action-kebab>-feedback-get-field ?msg-ptr ?field-name)

; Creating, destroying and processing of results
(bind ?msg-ptr (<package-kebab>-<action-kebab>-result-create))
(<package-kebab>-<action-kebab>-result-destroy ?msg-ptr)
(<package-kebab>-<action-kebab>-result-set-field ?msg-ptr ?field-name ?field-value)
(<package-kebab>-<action-kebab>-result-get-field ?msg-ptr ?field-name)

; Destroy server goal handle pointer.
(<package-kebab>-<action-kebab>-server-goal-handle-destroy ?handle-ptr)
; server goal handle members (see rclcpp_action documentation)
(<package-kebab>-<action-kebab>-server-goal-handle-abort ?handle-ptr ?result-ptr)
(<package-kebab>-<action-kebab>-server-goal-handle-succeed ?handle-ptr ?result-ptr)
(<package-kebab>-<action-kebab>-server-goal-handle-canceled ?handle-ptr ?result-ptr)
(bind ?goal-ptr (<package-kebab>-<action-kebab>-server-goal-handle-get-goal ?handle-ptr))
; the goal id is returned as string representation via rclcpp_action::to_string()
(bind ?uuid-str (<package-kebab>-<action-kebab>-server-goal-handle-get-goal-id ?handle-ptr))
(bind ?bool-sym (<package-kebab>-<action-kebab>-server-goal-handle-is-canceling ?handle-ptr))
(bind ?bool-sym (<package-kebab>-<action-kebab>-server-goal-handle-is-active ?handle-ptr))
(bind ?bool-sym (<package-kebab>-<action-kebab>-server-goal-handle-is-executing ?handle-ptr))
(<package-kebab>-<action-kebab>-server-goal-handle-execute ?handle-ptr)
(<package-kebab>-<action-kebab>-server-goal-handle-publish-feedback ?handle-ptr ?feedback-ptr)

; client goal handle members (see rclcpp_action documentation)
(bind ?bool-sym (<package-kebab>-<action-kebab>-client-goal-handle-is-feedback-aware ?handle-ptr))
(bind ?bool-sym (<package-kebab>-<action-kebab>-client-goal-handle-is-result-aware ?handle-ptr))
; return codes:
; 0 = STATUS_UNKNOWN
; 1 = STATUS_ACCEPTED
; 2 = STATUS_EXECUTING
; 3 = STATUS_CANCELING
; 4 = STATUS_SUCCEEDED
; 5 = STATUS_CANCELED
; 6 = STATUS_ABORTED
(bind ?status-int (<package-kebab>-<action-kebab>-client-goal-handle-get-status ?handle-ptr))
; the goal id is returned as string representation via rclcpp_action::to_string()
(bind ?uuid-str (<package-kebab>-<action-kebab>-client-goal-handle-get-goal-id ?handle-ptr))
(bind ?time-seconds-float (<package-kebab>-<action-kebab>-client-goal-handle-get-goal-stamp ?handle-ptr))
```

##### Functions Defined by User
```lisp
; Gets called for each server receiving a goal, needs to return one of these integers:
; (return 1) ; REJECT
; (return 2) ; ACCEPT_AND_EXECUTE
; (return 3) ; ACCEPT_AND_DEFER
; If the function does not exist, every goal is automatically accepted (ACCEPT_AND_EXECUTE)
(bind ?response-int (<package-kebab>-<action-kebab>-handle-goal-callback ?server-name-str ?goal-ptr ?uuid-str))

; Gets called for each server receiving a cancelation request, needs to return one of these integers:
; (return 0) ; REJECT
; (return 1) ; ACCEPT
; If the function does not exist, every request is automatically accepted (ACCEPT)
(bind ?response-int (<package-kebab>-<action-kebab>-cancel-goal-callback ?server-name-str ?server-goal-handle-ptr))
```

### Pitfalls and Limitations
#### Nested Messages
It is quite common to have nested messages in ROS.
Just creating the binding to the top-level message does not give access to the functions required to create or read the message fields that are of a different message type.
In that case it is necessary to also generate bindings for the nested messages.

#### Object Lifetimes and CLIPS
Since clips stores objects via void pointers, dynamic object lifetime management via `std::shared_ptr` does not work direcly from within CLIPS.
Instead, object lifetimes need to be managed more explicitly through the usage of `create` and `destroy` functions.

It is advised to clean up all objects as soon as they are not needed anymore in order to free up memory.

This is mostly straight-forward in case of short lifetimes (e.g., creating a message and immidiately sending it), but can be a bit tricky in case of long-lasting references, such as goals and goal handles in action clients/servers. In particular, it is adviced to clean up resources belonging to a goal to only be cleaned up once the goal is fully processed and either rejected or finished.

#### Callbacks and Mutexes
It can be tricky to interface between ROS callbacks and CLIPS environments, especially if locks are guarding said callbacks that are not visible to the end user.
As CLIPS environment access must also guarded by locks (as access is not thread-safe), this can easily create deadlocks in situations, where other functions can be called from clips that also try to acquire the lock held by a callback.

Example: The feedback callback for action clients is guarded by a mutex that is also used by client goal handles to access members in a thread-safe manner (such as get_status()).

A CLIPS rule that calls ClientGoalHandle::get_status() will therefore attempt to lock such a mutex while the clips lock is being held by the thread running the clips environment.
If a callback is received right before, then the clips environment will stall as the function call is stuck (mutex is held by the callback function), while the callback function is stuck because it tries to acquire the lock for the clips environment (because it wants to pass the callback content to the clips environment).

In these cases special care must be taken, e.g., by deferring CLIPS access out of scope of the mutexes guarding the callbacks.
