Configuration
#############

The |CX| node configuration is depending on the environments and plugins it manages.

Example
+++++++

.. code-block:: yaml

  clips_manager:
    ros__parameters:
      environments: ["main"]
      autostart_node: false
      bond_heartbeat_period: 0.0
      main:
        plugins: ["executive","files"]
        log_clips_to_file: true
        watch: ["rules", "facts"]
      # plugin parameters
      # ...

Node Parameters
+++++++++++++++

:autostart_node:

  ============== =======
  Type           Default
  -------------- -------
  bool           false
  ============== =======

  Description
    If set to *true*, automatically activate the lifecycle node on startup.

:bond_heartbeat_period:

  ============== =======
  Type           Default
  -------------- -------
  float          0.0
  ============== =======

  Description
    Heartbeat rate when used to bond with other nodes. A period of 0.0 disables bond usage.


:environments:

  ============== =======
  Type           Default
  -------------- -------
  string vector  []
  ============== =======

  Description
    Each entry should be a unique name for an environment. Each environment is then further configured.




Environment Parameters
++++++++++++++++++++++

Given an environment `env`, these are the configuration options:


:env.log_clips_to_file:

  ============== =======
  Type           Default
  -------------- -------
  bool           true
  ============== =======

  Description
    If set to *true*, each environment creates a log file <env-name>_<timestamp>.log in the ROS log directory containing all CLIPS output.

:env.watch:

  ============== =======
  Type           Default
  -------------- -------
  string vector  []
  ============== =======

  Description
    Watch items to define verbosity of the environment.
    Possible values: ["all", "facts", "instances", "slots", "rules", "activations", "messages", "message_handlers", "generic_functions", "methods", "deffunctions", "compilations", "statistics", "globals", "focus"]


:env.plugins:

  ============== =======
  Type           Default
  -------------- -------
  string vector  []
  ============== =======

  Description
    List of unique plugin names.
    Each plugin is further specified with parameters depending on its type.

:env.redirect_stdout_to_debug:

  ============== =======
  Type           Default
  -------------- -------
  bool           false
  ============== =======

  Description
    If set to true, all CLIPS output that logs to the ``clips::STDOUT="stdout"`` router is logged in ROS via ``RCLCPP_DEBUG`` instead of ``RCLCPP_INFO``. This is useful when the (often rather verbose) default CLIPS output should be captured in the log file but should not spam the main terminal. CLIPS programs that use other routers (such as ``info`` or ``t``) to print output still appear using ``RCLCPP_INFO``.


Plugin Parameters
+++++++++++++++++
Given a plugin `p` (as specified in an environments plugin list), each needs to specify the type of the plugin:


:p.plugin:

  ============== =======
  Type           Default
  -------------- -------
  string         ""
  ============== =======

  Description
    The specifier of the pluginlib plugin.


Each plugin may also have additional configuration options, which are documented in the :ref:`plugins` section.
