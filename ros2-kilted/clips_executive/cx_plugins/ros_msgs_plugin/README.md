# cx_ros_msgs_plugin
This package offers the `cx::RosMsgsPlugin' that provides functions to interface with ros topics of any type.
It makes use of the ROS introspection API to create generic publishers and subscriptions on-the-fly and to serialize and deserialize ROS messages.

## Usage
Register this plugin with the plugin manager. It requires no additional configuration, an example setup is shown below:

```yaml
clips_manager:
  ros__parameters:
    environments: ["main"]
    main:
      plugins: ["ros_msgs"]

    ros_msgs:
      plugin: "cx::RosMsgsPlugin"
```

## CLIPS Features
This plugin defines deftemplates and user-defined functions that are described below.

### Deftemplates
```lisp
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

; Asserted by the callback of a subscriber whenever a message arrives.
; Process the message and then call ros-msgs-destroy-msgs before retracting!
(deftemplate ros-msgs-message
  (slot topic (type STRING)) ; example          : "/cx_string_in"
  (slot msg-ptr (type EXTERNAL-ADDRESS)) example: <Pointer-C-0x7f1550001d20>
)
```
### Functions
```lisp
; Create and destroy publishers and subscribers.
(ros-msgs-create-publisher ?topic-name ?msg-type)    ; example args: "/cx_string_out" "std_msgs/msg/String"
(ros-msgs-destroy-publisher ?topic-name)             ; example args: "/cx_string_out"
(ros-msgs-create-subscription ?topic-name ?msg-type) ; example args: "/cx_string_in" "std_msgs/msg/String"
(ros-msgs-destroy-subscription ?topic-name)          ; example args: "/cx_string_in"

; Publish a given message over a topic.
; Requires the publisher to be created first using create-publisher.
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
```

## Object Lifetimes and CLIPS
Since clips stores objects via void pointers, dynamic object lifetime management via `std::shared_ptr` does not work direcly from within CLIPS.
Instead, object lifetimes need to be managed more explicitly through the usage of `create` and `destroy` functions.

It is advised to clean up all objects as soon as they are not needed anymore in order to free up memory.

Note that when processing nested messages, the message obtained via **ros-msgs-get-field** is not allocating new memory, but rather points to the memory of the parent message.
Calling **ros-msgs-destroy-message** is not necessary, as sub-messages retrieved via **ros-msgs-get-field** only hold a shallow reference and are cleaned up when the parent message is destroyed. Calling it anyways, will only invalidate this shallow reference.

When creating a new message via **ros-msgs-create-message**, new memory is allocated.
When using **ros-msgs-set-field** to set a nested message, dynamic memory from the sub-message is moved to the parent message, hence the nested message loses all dynamicly allocated data (e.g., unbound arrays, strings).
See the example below:
```lisp
(bind ?new-msg (ros-msgs-create-message "geometry_msgs/msg/Twist"))
(bind ?sub-msg (ros-msgs-create-message "geometry_msgs/msg/Vector3"))
(ros-msgs-set-field ?sub-msg "x" 5.5)
(ros-msgs-set-field ?new-msg "linear" ?sub-msg)
; now all dynamicly allocated members of ?sub-msg are reset,
; as they are moved to the parent message.
```
In particular, obtaining a nested message from one message `?source` and setting it as a member to another message `?sink` will cause `?source` to lose all dynamic data within it's sub-message, as the sub-message obtained is actually pointing to memory within `?source`:

```lisp
(bind ?source (ros-msgs-create-message "geometry_msgs/msg/Twist"))
(bind ?sub-msg (ros-msgs-create-message "geometry_msgs/msg/Vector3"))
(ros-msgs-set-field ?sub-msg "x" 5.5)
(ros-msgs-set-field ?source "linear" ?sub-msg)
(bind ?sink (ros-msgs-create-message "geometry_msgs/msg/Twist"))
(bind ?source-sub-msg (ros-msgs-get-field ?source "linear"))
(ros-msgs-set-field ?sink "linear" ?source-sub-msg)
; now all dynamicly allocated members of the sub message in ?source are reset,
; as they are moved to ?sink.
```
