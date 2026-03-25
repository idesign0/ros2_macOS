^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package ament_vitis
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.10.0 (2022-03-23)
------------------
* Prepare for release, change maintainer
* Add usage examples
* Enrich ROS_VITIS to be enabled only by VITIS hardware platforms
* Add ROS_VITIS and ROS_XRT variables
* Clarify output of message after operation

0.9.0 (2022-02-15)
------------------
* Release 0.9.0
* Account for NOKERNELS CMake var.
* Add vitis_link_kernel CMake macros

0.8.0 (2022-01-21)
------------------
* Refactor vitis_acceleration_kernel, add clock and link capabilities
* Add support for 2021.2 dfx-mgr flavour
* Add DTSI support to vitis_acceleration_kernel, refactor
* Release 0.8.0

0.7.0 (2021-11-23)
------------------
* Add ROS_ACCELERATION variable enablement
* Move ROS_ACCELERATION to ament_acceleration
* Export RTL IP for every kernel solution
* Update CHANGELOG, release 0.7.0

0.6.0 (2021-10-19)
------------------
* Make output less verbose
* CMake scripts report only when hardware acceleration disabled

0.5.0 (2021-06-03)
------------------
* Add CHANGELOG.rst
* Remove leftover from HLS support, and set as default
* Add x86 Ubuntu default libraries dir
* Minor improvements for firmware generation
* Adapt code to a non-Xilinx specific environments
* Add support to generate dfx-mgr artifacts
* Various CMake improvements
  * Support multiple include directories with either relative or absolute paths
  * Re-initialize VITIS_HLS_SRC_VALUE to allow for various HLS calls
  * Support multiple include directories
  * etc.

0.4.0 (2021-07-08)
------------------
* Add CMake macro to generate Tcl scripts
* add HLS Tcl generation capabilities

0.3.0 (2021-06-03)
------------------
* Add license, meet ROS 2 QL5, QL4
* Adapt to new folder structure to standardize across vendors

0.2.0 (2021-04-18)
------------------
* Simplify build process of kernels
* Enable hw_emu, hw and sw_emu targets
* Add first iteration on macros, support both Xilinx tools
as well as custom ROS packages.
* Macros work for all build targets
* Variables shield env. variables abstracting them to have
a coherent environment for Vitis v++.
