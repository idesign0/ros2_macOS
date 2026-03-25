Hello World
###########

**Goal:** Setup a new package storing a simple CLIPS program.

**Tutorial level:** Beginner

**Time:** 20 minutes

.. contents:: Contents
   :depth: 2
   :local:

Background
----------

The basic usage of CLIPS involves having a CLIPS environment and some CLIPS code to run inside of it using the CLIPS inference engine.

Prerequisites
-------------

You will need to have the |CX| :doc:`installed <../getting_started/installation>` and sourced in your environment.


Hello World
-----------

The goal of this tutorial is to setup a CLIPS environment, such that it loads a small CLIPS program, which consists of a rule to print "hello world", using the |CX|.

1 Workspace Setup
^^^^^^^^^^^^^^^^^

For the reminder of this tutorial, it is assumed that you work in a dedicated workspace ``cx_tutorial_ws``:


.. code-block:: bash

    mkdir -p ~/ros2/cx_tutorial_ws/src

Next, you will create a package that will be populated with CLIPS code:


.. code-block:: bash

    ros2 pkg create --build-type ament_cmake --license Apache-2.0 cx_tutorial_agents --dependencies cx_bringup

In particular, let us create directories params and clips to host configuration files and CLIPS source code:

.. code-block:: bash

    mkdir -p ~/ros2/cx_tutorial_ws/src/cx_tutorial_agents/{params,clips}

2 Install Directories via CMake
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Make sure the new directories are also installed with the package by adding the following line to the ``CMakeLists.txt``:

.. code-block:: cmake

    install(DIRECTORY params clips DESTINATION share/${PROJECT_NAME})

The full ``CMakelists.txt`` should look like this (after removing some unnecessary sections):

.. code-block:: cmake

    cmake_minimum_required(VERSION 3.8)
    project(cx_tutorial_agents)

    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      add_compile_options(-Wall -Wextra -Wpedantic)
    endif()

    # find dependencies
    find_package(ament_cmake REQUIRED)

    install(DIRECTORY params clips DESTINATION share/${PROJECT_NAME})

    ament_package()

With the general setup out of the way, it is time to add some CLIPS code!

3 Adding a CLIPS File
^^^^^^^^^^^^^^^^^^^^^

Navigate to the ``clips`` directory and download the example CLIPS file using the following command:

.. code-block:: bash

   cd ~/ros2/cx_tutorial_ws/src/cx_tutorial_agents/clips
    wget -O hello_world.clp https://raw.githubusercontent.com/carologistics/clips_executive/master/cx_tutorial_agents/clips/hello_world.clp

This adds the file ``hello_world.clp``, containing a simple rule that prints out `hello world` when it is fired:

.. code-block:: lisp

    (defrule hello-world
    =>
      (println "hello world")
    )

Here, the rule condition (statements before ``=>``) is empty, meaning it can be fired in any situation.
The Rule effect (statements after ``=>``) is a function invocation for ``println``, which logs a line (including newline characters CR+LF) to the stdout router (see also :doc:`here <../getting_started/logging>`).

In order to execute the code, a suitable configuration for the |CX| node is required to load the file into a CLIPS environment and to run it.

4 Configuring the |CX|
^^^^^^^^^^^^^^^^^^^^^^

To achieve  this, navigate to the ``params`` directory and download the example configuration file using the following command:

.. code-block:: bash

   cd ~/ros2/cx_tutorial_ws/src/cx_tutorial_agents/params
   wget -O hello_world.yaml https://raw.githubusercontent.com/carologistics/clips_executive/master/cx_tutorial_agents/params/hello_world.yaml

This adds the file ``hello_world.yaml`` with the following content:

.. code-block:: yaml

  /**: # placeholder to work with any ROS node regardless of namespace
    ros__parameters:
      autostart_node: true
      environments: ["hello_world"]

      hello_world:
        plugins: ["files"]
        watch: ["facts", "rules"]

      files:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_tutorial_agents"]
        load: ["clips/hello_world.clp"]

The top of the file specifies the ROS node(s) for which the parameters below it applies. Here you can use a placeholder ``/**`` that matches to any node name regardless of the namespace.

.. code-block:: yaml

    /**: # placeholder to work with any ROS node regardless of namespace
      ros__parameters:


Then, an environment with the name ``"hello_world"`` is created and the lifecycle node is instructed to activate itself on startup.

.. code-block:: yaml

      autostart_node: true
      environments: ["hello_world"]

For this particular environment, the list of plugins is specified. In this case only a single plugin is needed (called ``files`` here).
Further, the watch level of CLIPS is configured (see |BPG|) to monitor both facts and rules.

.. code-block:: yaml

      hello_world:
        plugins: ["files"]
        watch: ["facts","rules"]

The :docsite:`FileLoadPlugin <clips_executive/plugins/file_load_plugin>`, which can load files to CLIPS environments. It is configured to look for files in the current package and to load the file created above.

.. code-block:: yaml

      files:
        plugin: "cx::FileLoadPlugin"
        pkg_share_dirs: ["cx_tutorial_agents"]
        load: ["clips/hello_world.clp"]

This concludes the setup for the example. The next step is to build and execute the code.

5 Running The Code
^^^^^^^^^^^^^^^^^^

Now it is time to build the package and to source the workspace. We recommend to use a symlink-based installation so that changes to your installed CLIPS files are applied without the need to rebuild the package.


.. code-block:: bash

   cd ~/ros2/cx_tutorial_ws/
   colcon build --symlink-install
   source install/setup.bash

In order to run the code you can run the ``cx_node`` directly and passing the parameter file to it:

.. code-block:: bash

   cd ~/ros2/cx_tutorial_ws/
   ros2 run cx_clips_env_manager cx_node  --ros-args --params-file src/cx_tutorial_agents/yaml/hello_world.yaml

Alternatively, you can use the launch file of the ``cx_bringup`` package, with the benefit of leveraging the ament index in order to lookup the location of the parameter file instead of relying on a full path:

.. code-block:: bash

   ros2 launch cx_bringup cx_launch.py manager_config:=hello_world.yaml package:=cx_tutorial_agents

In either case, you will see in the log output that the rule was indeed fired:


.. code-block:: bash

    [cx_node-1] [hello_world] [INFO] FIRE    1 hello-world: *
    [cx_node-1] hello_world] [INFO] hello world

Additionally, a log is created in the ROS logging directory (typically in ``~/.ros/log``), forwarding the respective CLIPS logs of the ``hello_world`` environment to a file ``hello_world_<timestamp>.log``. After executing the example, the log file will contain the following lines:


.. code-block:: bash

    [<timestamp>] [hello_world] [info] FIRE    1 hello-world: *
    [<timestamp>] [hello_world] [info] hello world
    [<timestamp>] [hello_world] [info] ==> f-1     (executive-finalize)

The first two lines show the output of the CLIPS inference engine run that is automatically triggered after loading all plugins.

.. code-block:: bash

    [<timestamp>] [hello_world] [info] FIRE    1 hello-world: *
    [<timestamp>] [hello_world] [info] hello world

Once the run is completed, the node idles until it is shut down, which then causes the |CX| to assert the ``executive-finalize`` fact and then to run the inference engine again (see :docsite:`Usage`).

.. code-block:: bash

    [<timestamp>] [hello_world] [info] ==> f-1     (executive-finalize)


Summary
-------

You created a package with your first custom configuration for the |CX|, including some CLIPS code. This involved preparing directories for configuration and CLIPS files via CMake, a yaml configuration to setup a CLIPS environment with the ``FileLoadPlugin`` and finally a CLIPS file defining a simple rule to print hello world.

Next Steps
----------

:doc:`Next <ros_monitoring>`, you will learn how to interface with ROS via the ``RosMsgsPlugin`` by providing continuous monitoring of a turtle in the ``turtlesim`` simulator.
