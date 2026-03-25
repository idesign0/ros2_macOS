#    ____  ____
#   /   /\/   /
#  /___/  \  /   Copyright (c) 2021, Xilinx®.
#  \   \   \/    Author: Víctor Mayoral Vilches <victorma@xilinx.com>
#   \   \
#   /   /
#  /___/   /\
#  \   \  /  \
#   \___\/\___\
#
# Vitis HLS macros to integrate High-Level Synthesis (HLS)
# capabilities into the  ROS ament build system
#
# See https://www.xilinx.com/html_docs/xilinx2020_2/vitis_doc/gnq1597858079367.html
# for more on Vitis HLS

#
# Generate a Vitis Tcl script from CMake
#
# .. note:: this macro will command CMake to generate
# the corresponding Tcl script according to the parameters
# designed below. To execute such script use either Vitis CLI
# (e.g. via "vitis_hls -f <tcl-script>") or colcon krs hls.
#
# .. note:: the macro will generate one solution per each clock
# option provided
#
# :param PROJECT: Name of the Vitis HLS project to be created
# :type target: string
#
# :param TOPFUNCTION: Top-level function of the acceleration kernel
# e.g. vadd
# :type target: string
#
# :param SRC: sources for the kernel can be various. Relative paths.
# :type target: string
#
# :param TESTBENCH: source file for the testbench. Relative path.
# :type target: string
#
# :param CLOCK: Clock period in nanoseconds (e.g. 4). One solution will be
# generated per each clock element provided.
# :type target: string
#
# :param HEADERS: one or muliple relative path directories whereto search for
# header files.
# :type target: string
#
# :param SYNTHESIS (optional): Run synthesis
# :type target: bool
#
# :param RTLSIMULATION (optional): Run RTL Simulation
# :type target: bool
#
# :param RTLIMPLEMENTATION (optional): Run RTL implementation
# :type target: bool
#
macro(vitis_hls_generate_tcl)
    set(options SYNTHESIS RTLSIMULATION RTLIMPLEMENTATION)
    set(oneValueArgs PROJECT TOPFUNCTION TESTBENCH)
    set(multiValueArgs SRC HEADERS CLOCK)

    cmake_parse_arguments(VITIS_HLS "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )

    # if (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" AND DEFINED CMAKE_SYSROOT})
    if (DEFINED CMAKE_SYSROOT)  # cross compilation, DEFINED CMAKE_SYSROOT

      # set some initial placeholders
      set(VITIS_HLS_PROJECT_VALUE ${VITIS_HLS_PROJECT})
      set(VITIS_HLS_TOPFUNCTION_VALUE ${VITIS_HLS_TOPFUNCTION})

      # define template, with or without include directories
      FILE(WRITE ${CMAKE_BINARY_DIR}/${VITIS_HLS_PROJECT}.tcl.in
      "open_project -reset @VITIS_HLS_PROJECT_VALUE@\n"
      "add_files @VITIS_HLS_SRC_VALUE@\n"
      )

      if(DEFINED VITIS_HLS_HEADERS)
        # convert to "value1 value2"
        foreach(VITIS_HLS_HEADERS_file ${VITIS_HLS_HEADERS})
          if(IS_ABSOLUTE ${VITIS_HLS_HEADERS_file})
            set(VITIS_HLS_HEADERS_VALUE "${VITIS_HLS_HEADERS_VALUE} -I ${VITIS_HLS_HEADERS_file}")
          else()
            set(VITIS_HLS_HEADERS_VALUE "${VITIS_HLS_HEADERS_VALUE} -I ${CMAKE_SOURCE_DIR}/${VITIS_HLS_HEADERS_file}")
          endif()
        endforeach()

        # NOTE:
        #    See https://gcc.gnu.org/onlinedocs/gcc/Directory-Options.html for the differences
        #     between -isystem and -I
        # FILE(APPEND ${CMAKE_BINARY_DIR}/${VITIS_HLS_PROJECT}.tcl.in
        # "add_files -tb @VITIS_HLS_TESTBENCH_VALUE@ -cflags \"-isystem @VITIS_HLS_HEADERS_VALUE@\"\n"
        # )
        FILE(APPEND ${CMAKE_BINARY_DIR}/${VITIS_HLS_PROJECT}.tcl.in
        "add_files -tb @VITIS_HLS_TESTBENCH_VALUE@ -cflags \" @VITIS_HLS_HEADERS_VALUE@\"\n")

      else()
        FILE(APPEND ${CMAKE_BINARY_DIR}/${VITIS_HLS_PROJECT}.tcl.in
          "add_files -tb @VITIS_HLS_TESTBENCH_VALUE@\n"
        )
      endif()  # VITIS_HLS_HEADERS, add include directories

      FILE(APPEND ${CMAKE_BINARY_DIR}/${VITIS_HLS_PROJECT}.tcl.in
      "set_top @VITIS_HLS_TOPFUNCTION_VALUE@\n"
      )

      # determine the SoC at build time and based on the select-ed firmware
      set(FIRMWARE_SOC_PATH ${CMAKE_INSTALL_PREFIX}/../acceleration/firmware/select/SOC)
      file (STRINGS ${FIRMWARE_SOC_PATH} FIRMWARE_SOC)

      foreach(clock_item ${VITIS_HLS_CLOCK})
        FILE(APPEND ${CMAKE_BINARY_DIR}/${VITIS_HLS_PROJECT}.tcl.in
        "# solution_${clock_item}ns\n"  # mark each solution with clock ns
        "open_solution -flow_target vitis solution_${clock_item}ns\n"  # mark each solution with clock ns
        "set_part {${FIRMWARE_SOC}}\n"
        "create_clock -period ${clock_item}\n"
        "csim_design -ldflags \"-lOpenCL\" -profile\n"
        )

        if (${VITIS_HLS_SYNTHESIS})
          FILE(APPEND ${CMAKE_BINARY_DIR}/${VITIS_HLS_PROJECT}.tcl.in
            "csynth_design\n"
          )
        endif()  # VITIS_HLS_SYNTHESIS

        if (${VITIS_HLS_RTLSIMULATION})
          FILE(APPEND ${CMAKE_BINARY_DIR}/${VITIS_HLS_PROJECT}.tcl.in
            "cosim_design -O -rtl verilog -ldflags \"-L/lib/x86_64-linux-gnu\"\n"
          )
        endif()  # VITIS_HLS_RTLSIMULATION

        if (${VITIS_HLS_RTLIMPLEMENTATION})
          FILE(APPEND ${CMAKE_BINARY_DIR}/${VITIS_HLS_PROJECT}.tcl.in
            "export_design\n"
          )
        endif()  # VITIS_HLS_RTLIMPLEMENTATION

        # Create RTL IP by default as well
        FILE(APPEND ${CMAKE_BINARY_DIR}/${VITIS_HLS_PROJECT}.tcl.in
          "config_interface -m_axi_alignment_byte_size 64 -m_axi_latency 64 -m_axi_max_widen_bitwidth 512 -m_axi_offset slave\n"
          "config_rtl -register_reset_num 3\n"
          "config_export -format xo -output ${CMAKE_BINARY_DIR}/${VITIS_HLS_PROJECT}/@VITIS_HLS_TOPFUNCTION_VALUE@.xo -rtl verilog\n"
          "set_directive_top -name @VITIS_HLS_TOPFUNCTION_VALUE@ @VITIS_HLS_TOPFUNCTION_VALUE@\n"
          "export_design -rtl verilog -format ip_catalog\n"
        )

      # exit at the end
      FILE(APPEND ${CMAKE_BINARY_DIR}/${VITIS_HLS_PROJECT}.tcl.in
      "\n"
      )
      endforeach()  # for each clock_item

      # exit at the end
      FILE(APPEND ${CMAKE_BINARY_DIR}/${VITIS_HLS_PROJECT}.tcl.in
      "exit\n"
      )

      # set placeholders
      # set(VITIS_HLS_SRC_VALUE ${VITIS_HLS_SRC})  # this gets a list as "value1;value2"
      # we need instead: "value1 value2"
      set(VITIS_HLS_SRC_VALUE "")  # re-initialize, needed if various calls in the same CMakeLists.txt
      foreach(VITIS_HLS_SRC_file ${VITIS_HLS_SRC})
        set(VITIS_HLS_SRC_VALUE "${VITIS_HLS_SRC_VALUE} ${CMAKE_SOURCE_DIR}/${VITIS_HLS_SRC_file}")
      endforeach()
      set(VITIS_HLS_TESTBENCH_VALUE ${CMAKE_SOURCE_DIR}/${VITIS_HLS_TESTBENCH})
      # set(VITIS_HLS_CLOCK_VALUE ${VITIS_HLS_CLOCK})  # not necessary anymore, iterating over clocks above

      # replace placeholders and produce final Tcl script
      configure_file(${CMAKE_BINARY_DIR}/${VITIS_HLS_PROJECT}.tcl.in ${CMAKE_BINARY_DIR}/${VITIS_HLS_PROJECT}.tcl)

      # install Tcl script
      ## NOTE: this would move .tcl scripts to the install-* directory. Instead
      ## it's recommended to keep things in build-*, rationale is that install-*
      ## is what we're deploying to the embedded platforms and we want this to be compact
      # install(
      #   FILES
      #     ${CMAKE_BINARY_DIR}/hls.tcl
      #   DESTINATION
      #     lib/${PROJECT_NAME}
      # )
    endif()  # cross compilation, DEFINED CMAKE_SYSROOT
endmacro()  # vitis_acceleration_kernel
