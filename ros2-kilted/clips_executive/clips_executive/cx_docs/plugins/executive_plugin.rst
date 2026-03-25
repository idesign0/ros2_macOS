.. _usage_executive_plugin:

Continuous Execution Plugin
###########################

Source code on :source-master:`GitHub <cx_plugins/executive_plugin>`.

.. admonition:: Plugin Class

  cx::ExecutivePlugin

This plugin provides continuous execution of CLIPS environments.
Additionally, it enables reasoning about the current ROS and system time.

Configuration
*************

.. _refresh_rate:

:`refresh_rate`:

  ============ =======
  Type         Default
  ------------ -------
  int          10
  ============ =======

  Description
    Target rate (in Hz) with which agendas are refreshed and run is called.
    This is done sequentially for the registered environments.

:`publish_on_refresh`:

  ============ =======
  Type         Default
  ------------ -------
  bool         false
  ============ =======

  Description
    Whether to publish an empty message with each agenda refresh on the `clips_executive/refresh_agenda` topic.
    Mainly useful for debug purposes to measure the actual refresh rate.

.. _assert_time:

:`assert_time`:

  ============ =======
  Type         Default
  ------------ -------
  bool         true
  ============ =======

  Description
    Whether the latest ROS time as fact into the CLIPS environment on each iteration.


Features
********

With the set :ref:`refresh_rate` the executive plugin refreshes all agendas and then runs the loaded CLIPS environments.

Facts
~~~~~

If :ref:`assert_time` is set to ``true``, it asserts the ordered fact `time` with the current ROS time as float.

.. code-block:: lisp

  (time ?ros-time-float)

Functions
~~~~~~~~~

This plugin adds deffunctions to retrieve the current time.

.. code-block:: lisp

  (bind ?ros-time (now))         ; returns a FLOAT holding get_clock()->now().seconds()
  (bind ?sys-time (now-systime)) ; returns a FLOAT of system time


Rules
~~~~~

If :ref:`assert_time` is set to ``true``, it defines a rule to clean up the time fact at the end of agenda execution.

.. code-block:: lisp

  (defglobal
    ?*PRIORITY-TIME-RETRACT*    = -10000
  )

  (defrule time-retract
    (declare (salience ?*PRIORITY-TIME-RETRACT*))
    ?f <- (time $?)
    =>
    (retract ?f)
  )

Other
~~~~~

Lastly, the ``time`` facts and  ``time-retract`` rule are unwatched.

.. code-block:: lisp

  (unwatch facts time)
  (unwatch rules time-retract)


Usage Example
*************

A minimal working example is provided by the :docsite:`cx_bringup` package. Run it via:

.. code-block:: bash

    ros2 launch cx_bringup cx_launch.py manager_config:=plugin_examples/executive.yaml

It prints the current ROS and system time in each iteration and compares the time at the start of the iteration with the time at the time the rule is fired.

Configuration
~~~~~~~~~~~~~

File :source-master:`cx_bringup/params/plugin_examples/executive.yaml`.

.. code-block:: yaml

  clips_manager:
    ros__parameters:
      environments: ["cx_executive"]
      cx_executive:
        plugins: ["executive", "files"]
        log_clips_to_file: true
        watch: ["facts", "rules"]

      executive:
        plugin: "cx::ExecutivePlugin"
        publish_on_refresh: false
        assert_time: true
        refresh_rate: 10
      files:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_bringup"]
        load: [
          "clips/plugin_examples/executive.clp"]

Code
~~~~

File :source-master:`cx_bringup/clips/plugin_examples/executive.clp`.

.. code-block:: lisp

  (defrule print-time
    (time ?now)
    =>
    (printout info "time between agenda refresh and rule fire: " (- (now) ?now) crlf)
    (printout info "ROS time: " (now) crlf)
    (printout info "sys time: " (now-systime) crlf)
  )
