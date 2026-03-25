# ament_vitis

CMake macros and utilities to include Vitis platform into the ROS 2 build system (ament) and its development flows.

**NOTE**: `ament_vitis` has a direct runtime Non-ROS dependency with [`Vitis`](https://www.xilinx.com/products/design-tools/vitis.html) unified software platform. Download and install Vitis [here](https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vitis.html).

### Usage examples


<details><summary>Synthesize C++ with Vitis HLS into an acceleration kernel</summary>

```cmake
# example 1: resize_accel kernel
vitis_acceleration_kernel(
    NAME resize_accel
    FILE src/image_proc/xf_resize_accel.cpp
    CONFIG cfg/kv260.cfg
    INCLUDE
    include/image_proc
    ${CMAKE_INSTALL_PREFIX}/include
    TYPE hw
)

# example 2: rectify_accel kernel
vitis_acceleration_kernel(
    NAME rectify_accel
    FILE src/image_proc/xf_rectify_accel.cpp
    CONFIG cfg/kv260.cfg
    INCLUDE
    include/image_proc
    ${CMAKE_INSTALL_PREFIX}/include
    TYPE hw
)
```

</details>


<details><summary>Link and implement (place & route) a previously synthesized acceleration kernel with Vitis</summary>

```cmake
# example 3: image_proc kernel
vitis_link_kernel(
    OUTPUT image_proc
    KERNELS resize_accel rectify_accel
    CONFIG cfg/kv260_image_proc.cfg
)
```

</details>

<details><summary>Synthesize, place & route an acceleration kernel with Vitis</summary>

```cmake
# example 4: vadd kernel
vitis_acceleration_kernel(
    NAME vadd_faster
    FILE src/vadd.cpp
    CONFIG src/kv260.cfg
    CLOCK 100000000:vadd_faster
    DTSI src/vadd_faster.dtsi
    INCLUDE
    include
    TYPE
    hw
    LINK
    PACKAGE
)
```

</details>


<details><summary><b>Advanced</b>: generate TCL scripts for C simulation and synthesis customizing the Vitis or Vivado flows</summary>

```cmake
# C simulation and synthesis
vitis_hls_generate_tcl(
    PROJECT
    project_faster_doublevadd_publisher
    SRC
    src/vadd.cpp
    HEADERS
    include
    TESTBENCH
    src/testbench.cpp
    TOPFUNCTION
    vadd
    CLOCK
    4
    SYNTHESIS
)
```

</details>


### Quality Declaration

This package claims to be in the **Quality Level 4** category, see the [Quality Declaration](./QUALITY_DECLARATION.md) for more details.
