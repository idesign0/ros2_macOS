.. _usage_cx_ros_comm_gen:

ROS Interface Plugin Generator
==============================

Source code on :source-master:`GitHub <cx_ros_comm_gen>`.

In order to interface with ROS directly without the need for generic introspection, the |CX| offers a plugin generator for individual interfaces (messages, services or actions).

.. note::

    For ROS messages or service clients, the usage of the :ref:`Generic ROS Msg Plugin <usage_ros_msgs_plugin>` is recommended instead. The plugin generator should only be used in case ROS actions or services have to be used from within CLIPS.


.. toctree::
   :maxdepth: 2

   gen_info.rst
   gen_msgs.rst
   gen_srvs.rst
   gen_action.rst
