Pitfalls and Considerations
###########################

Below are some lessons learned when developingthe core plugins of the |CX|. They might be a useful read for some.

Injected functions should not be blocking
*****************************************

In order to ensure running a responsive CLIPS application, make sure the injected functions are executing fast. Long-lasting operations should rather be dispatched by a function and then asynchronously handled once they finish. This allows the inference engine to continue operating while heavy computations or time-consuming sub-routines are processed.

Keep your locking scopes as tight as possible
*********************************************

When writing complex plugins with asynchronous operations, be sure to scope your guarded regions well.

A common pitfall may occur when plugins also need to guard data structures from concurrent access using some mutex, while also handling CLIPS access.

Consider this example from **cx_ros_msgs_plugin** which allows interactions with ROS topics:

#. The asynchronous subscription callbacks adds messages and meta-data to an unordered map, which needs to be guarded by a mutex `map_mtx_` as multiple write operations could occur at the same time when multi-threaded executors and re-entrant callback groups are used. Additionally, the messages are asserted as facts (holding a reference to the message) in the callback.
#. The **ros-msgs-get-field** UDF allows to retrieve fields of messages. As fields may contain messages, this again might need to store meta-data, hence it also needs to lock `map_mtx_`.

A bad implementation using a scoped lock for the entire scope of the callback and the entire scope of **ros-msgs-get-field** could cause a deadlock if the ros-msgs-get-field function is called on the left-hand side of a rule, e.g., like this:

.. code-block:: lisp

    (defrule deadlock-example
      (ros-msgs-subscription (topic ?sub))
      ?msg-f <- (ros-msgs-message (topic ?sub) (msg-ptr ?inc-msg))
      (test (and (= 0 (ros-msgs-get-field ?inc-msg "velocity"))))
    ...

The assertion of the fact in the callback triggers the conditional check in the rule which therefore tries to lock `map_mtx_` blocked by the callback function itself. Hence, make sure to keep the scopes of any mutex as tight as possible!

Be aware of hidden locks
************************

It can be tricky to interface between ROS callbacks and CLIPS environments, especially if locks are guarding said callbacks that are not visible to the end user.
As CLIPS environment access must also guarded by locks (as access is not thread-safe), this can easily create deadlocks in situations, where other functions can be called from clips that also try to acquire the lock held by a callback.

Example: The feedback callback for action clients is guarded by a mutex that is also used by client goal handles to access members in a thread-safe manner (such as get_status()).

A CLIPS rule that calls ClientGoalHandle::get_status() will therefore attempt to lock such a mutex while the clips lock is being held by the thread running the clips environment.
If a callback is received right before, then the clips environment will stall as the function call is stuck (mutex is held by the callback function), while the callback function is stuck because it tries to acquire the lock for the clips environment (because it wants to pass the callback content to the clips environment).

In these cases special care must be taken, e.g., by deferring CLIPS access out of scope of the mutexes guarding the callbacks.
