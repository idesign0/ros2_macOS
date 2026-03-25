.. _usage_cx_rl:

CX Reinforcement Learning
=========================

Source code on .

Reinforcement Learning can be utilized to create a policy for the selection of actions in the |CX|. For that, the :docsite:`usage_rl_interfaces_plugin` creates several ROS2 interfaces to extract information and select actions from and in the CLIPS environment.

The **cx_reinforcement_learning** package features the `CXRLGym` class utilizing the genereated interfaces to provide most basic functions according to the `Gymnasium`_ API standard for reinforcement learning.

.. toctree::
   :maxdepth: 2

   blocksworld.rst
