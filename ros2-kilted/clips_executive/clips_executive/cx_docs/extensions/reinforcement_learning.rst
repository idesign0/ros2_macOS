Reinforcement Learning Integration for CLIPS
============================================

.. attention::

   This extension targets ROS kilted or later. While technically possible to use with earlier versions of ROS, the provided example and supporting CLIPS code require kilted or later.

This extension provides an implementation for using Reinforcement Learning (RL) alongside CLIPS.

If you need one of the following features, you may find this useful:

- Adaptive decision-making through trial and error in dynamic environments
- Integration of RL-based policies with CLIPS rules and reasoning
- A modular interface between symbolic reasoning (CLIPS) and sub-symbolic learning (RL)
- Seamless experimentation with Gymnasium-compatible environments in ROS

Background: Reinforcement Learning
++++++++++++++++++++++++++++++++++

Reinforcement Learning (RL) is a paradigm of machine learning in which an agent learns to make decisions by interacting with its environment.
The agent observes the current state, takes actions, and receives rewards that guide its behavior over time.

An RL setup typically consists of:

 - an *environment*, which defines the state space, action space, and transition dynamics,
 - an *agent*, which selects actions based on its current policy,
 - and a *reward function*, which provides feedback on the desirability of each action outcome.

Through repeated interaction, the agent refines its policy to maximize cumulative reward.


Overview
++++++++

The Reinforcement Learning (RL) integration is provided through a **ROS 2 node** that manages both a Gym environment and a corresponding RL model.

- The **Gym environment** is implemented via the **CXRLGym** class. It sets up the environment, training routines, and execution workflows using ROS interfaces, allowing seamless information exchange with the |CX| framework via ROS.

- The **RL model**, **MultiRobotMaskablePPO**, implements a multi-robot **Maskable Proximal Policy Optimization (MPPO)** algorithm. This model supports environments with multiple robots acting asynchronously and allows for masking of unavailable actions, improving training efficiency in complex robotic tasks.

A **node skeleton**, **CXRLBaseNode**, is provided to simplify node development. It implements a **ROS 2 lifecycle node** with optional **bond capabilities**, and manages both the Gym environment (`CXRLGym`) and the RL model.

**CXRLBaseNode** can be extended to easily integrate other RL models, allowing developers to reuse the lifecycle and environment management infrastructure without re-implementing boilerplate code.

The **cx_rl_mppo_node** is a ready-to-use node that extends `CXRLBaseNode` and comes preconfigured to use the **CXRLGym** environment and the **MultiRobotMaskablePPO** model.

To reduce manual ROS overhead, the **cx_rl_clips** package provides a **CLIPS-based interface** for interacting with the PDDL Manager node. This allows users to interact with the `CXRLGym` manager by asserting and monitoring **CLIPS facts**, without requiring direct ROS communication (e.g., publishing messages or waiting for service feedback).

This architecture enables users to:

* **Train RL agents** within ROS 2 environments.
* **Combine symbolic reasoning and learning-based control** by integrating CLIPS with the Gym-based RL node.
* **Easily extend to other RL models** by leveraging the `CXRLBaseNode` skeleton.
* **Evaluate trained models** directly on real or simulated robots, with minimal boilerplate.

Content
+++++++

.. toctree::
   :maxdepth: 2

   rl/usage.rst
   rl/cx_rl_gym.rst
   rl/multi_robot_mppo.rst
   rl/rl_clips.rst
   rl/blocksworld_tutorial.rst
