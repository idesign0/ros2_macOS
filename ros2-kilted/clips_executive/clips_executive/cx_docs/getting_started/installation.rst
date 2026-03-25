Installation
############
This document assumes you have a basic ROS 2 installation. If not, refer to the official `ROS documentation`_.

Binary Installation
+++++++++++++++++++

The easiest way to install the |CX| is via your package manager:

.. note::

   The |CX| is currently in the process of being indexed, hence binary packages might not be available yet.

.. code-block:: bash

   sudo apt intall ros-<ros2-distro>-clips-executive


Installation from Source
+++++++++++++++++++++++++

The following steps will setup a workspace for the project. Adjust target locations as needed.

.. code-block:: bash

    # Workspace setup
    mkdir -p ~/ros2/clips_executive_ws/src
    git clone https://github.com/carologistics/clips_executive.git ~/ros2/clips_executive_ws/src/clips_executive


You may need to install dependencies using rosdep:


.. code-block:: bash

    rosdep install --from-paths src -y --ignore-src

Lastly, build and source the workspace:

.. code-block:: bash

    # Building and sourcing
    cd ~/ros2/clips_executive_ws/
    colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=OFF
    source ~/ros2/clips_executive_ws/install/setup.bash
