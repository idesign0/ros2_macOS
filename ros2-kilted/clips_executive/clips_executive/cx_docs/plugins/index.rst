.. _plugins:

CLIPS Plugins
=============

The |CX| can dynamically load `pluginlib`_ plugins, which act as an interface for users to apply the CLIPS C++ API (See the |APG|), e.g., to inject user-defined functions into CLIPS environments.

Plugins are specializations of of the :docsite:`cx_plugin` base class and are handled as follows:
 - Each plugin is initialized exactly once before it is loaded into environments by calling it's ``initialize()`` function.
 - When an environment is loaded, all specified plugins are **loaded in order** of the respective plugins parameter.
 - Whenever an environment needs to load a plugin, it's ``clips_env_init()`` function is called once. Loading the same plugin again before unloading it first, results in an error (and the function is not called again).
 - Whenever an environment needs to unload a plugin, it's ``clips_env_destroyed()`` is called once. Unloading the same plugin again before loading it first, results in an error (and the function is not called again).
 - Before an environment is destroyed, all plugins are **unloaded in reverse order** of loading.
 - On destruction of a plugin, the `finalize()` function is called exactly once.
 - Before a plugin is destroyed, it is unloaded from all environments.

See also the tutorial on :ref:`Writing a Plugin <writing_a_plugin>` to learn how to write your own plugins.

The |CX| provides several plugins out-of-the-box that are described here.

Plugin Interface
++++++++++++++++

.. toctree::
   :maxdepth: 2

   overrides.rst
   pitfalls.rst



Available Plugins
+++++++++++++++++

.. toctree::
   :maxdepth: 2

   cx_ament_index.rst
   executive_plugin.rst
   config_plugin.rst
   file_load_plugin.rst
   protobuf_plugin.rst
   ros_msgs_plugin.rst
   ros_param_plugin.rst
   tf2_pose_tracker_plugin.rst
