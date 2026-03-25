Usage
#####

This document assumes you have the |CX| installed and sourced in your environment.

Aside from the regular |CX| node, the RL extension additionally requires a node for the reinforcement learning agent.
We provide the **CXRLNode** node for this purpose which  deploys a variant of multi-robot proximal policy optimization with invalid action masking via the :docsite:`cx_rl_multi_robot_mppo` package.

Starting the CX RL Node
************************
Aside from the regular |CX| node, the RL extension additionally requires a node that provides a policy and environment based on the ``CXRLBaseNode`` class (see :ref:`cx_rl_base_node`).

This package includes a reference implementation that can be started directly:

.. code-block:: bash

  ros2 run cx_rl_multi_robot_mppo cx_rl_mppo_node

For typical usage, it is recommended to start the system via the launch file provided by the ``cx_rl_bringup`` package.
This launch file wraps the |CX| launch configuration and starts the RL node alongside it:

.. code-block:: bash

  ros2 launch cx_rl_bringup cx_rl_launch.py

.. hint::

  Use the `--show-args` option to display all available launch parameters.

Example
*******

The following example showcases how an RL agent can be trained and utilzied with CLIPS-based execution. Here, a simple blocksworld domain is considered, where four blocks need to be stacked using the actions pickup (picking up a block) and stack (placing one block on top of another block).

.. code-block:: bash

  ros2 launch cx_rl_bringup cx_rl_launch.py

.. hint::

  Use the `--show-args` option to learn about all parameters that the launch file provides.


Check out the :ref:`Intefacing Guide <reinforcement_learning/rl_clips.html>` page to learn how to write your own CLIPS applications while leveraging reinforcement learning.
