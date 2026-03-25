PDDL Integration for CLIPS
==========================

.. attention::

   This extension targets ROS jazzy or later. While technically possible to use with earlier versions of ROS, the provided example and supporting CLIPS code require jazzy or later.

This extension provides an implementation for using PDDL alongside CLIPS.

If you need one of the following features, you may find this useful:

- A structured model of action and change to progress the CLIPS fact base according to a robots actions
- Unified sanity checks to ensure actions can only be executed if conditions are currently met according to the fact base
- Automated planning to reach complex objectives by means of computing and executing plans consisting of individual robot actions

Background: PDDL
++++++++++++++++

The Planning Domain Definition Language (PDDL) is the de facto standard language for expressing planning problems in artificial intelligence.
A PDDL problem consists of

 - a domain description, which defines the available objects, predicates, actions and their preconditions and effects,
 - a problem description, which specifies the initial state and desired goals.

Given such a description, a PDDL planner computes a sequence of actions (a plan) that transforms the initial state into one that satisfies the goal conditions.

An example PDDL problem can be found `here <pddl/domain.html>`_.


Overview
++++++++

This extension leverages the `Unified Planning Framework (UPF)`_, which supports a wide range of PDDL standards.
UPF is wrapped and exposed as a ROS node, hence interfacing is done via the regular |CX| plugins.

In order to reduce the manual overhead, an optional CLIPS abstraction layer is provided, which takes care of the ROS communication.

Additionally, this extension contain the `NEXTFLAP planner`_, which can handle both classical and temporal PDDL variants.
This planner requires the `z3 constraint solver`_, which is also provided.

Content
+++++++

.. toctree::
   :maxdepth: 2

   pddl/usage.rst
   pddl/pddl_clips.rst
   pddl/domain.rst
   pddl/raw_tutorial.rst
   pddl/cx_pddl_clips_tutorial.rst
