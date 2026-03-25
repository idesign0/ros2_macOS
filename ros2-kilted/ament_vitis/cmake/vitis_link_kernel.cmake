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
# base macros and functions to link acceleration kernels (.xo files) into xclbin

#
# Link (place&route) two or more acceleration kernels into a single xclbin file
#
# .. note:: harcoded to "hw" type of build, extend if necessary.
#
# :param OUTPUT: Output name of the kernel to be linked (placed & routed)
# :type target: string
#
# :param CONFIG: CMAKE_SOURCE_DIR relative path to configuration file
# for the kernel e.g. src/zcu102.cfg.
# :type target: string
#
# :param DTSI (optional): CMAKE_SOURCE_DIR relative path to dtsi
# file for the resulting linked kernel e.g. src/vadd_faster.dtsi.
# :type target: string
#
# :param KERNELS: two or more CMAKE_BINARY_DIR relative paths for the 
# kernels to link.
# :type target: string
#
#
# Example:
#
# vitis_link_kernel(
#     OUTPUT image_proc
#     KERNELS resize_accel rectify_accel
#     CONFIG src/kv260.cfg
#   )
#
macro(vitis_link_kernel)
    set(options "")
    set(oneValueArgs OUTPUT CONFIG DTSI)
    set(multiValueArgs KERNELS)

    cmake_parse_arguments(VITIS_LINK "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
    
    # if (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" AND DEFINED CMAKE_SYSROOT})
    if (DEFINED CMAKE_SYSROOT)
        if(${NOKERNELS})
            message(STATUS "No kernels built")
        else()
            # list all kernels
            set(KERNELS_INCLUDE "")
            foreach(include_item ${VITIS_LINK_KERNELS})
                string(APPEND KERNELS_INCLUDE "${CMAKE_BINARY_DIR}/${include_item}.xo ")
            endforeach()
            
            # place and route
            set(ENV{PLATFORM_REPO_PATHS} ${PLATFORM_REPO_PATHS})
            set(CMD "${VPP_PATH} -l -t hw --config ${CMAKE_SOURCE_DIR}/${VITIS_LINK_CONFIG} ${KERNELS_INCLUDE} -o ${CMAKE_BINARY_DIR}/${VITIS_LINK_OUTPUT}.xclbin " )
            # message(${CMD})
            run(${CMD})
            
            # generate Vitis Accelerated Application (AA) files, if linking was successful
            if (EXISTS "${CMAKE_BINARY_DIR}/${VITIS_LINK_OUTPUT}.xclbin")
                ## extract the raw bitstream
                run("xclbinutil --dump-section BITSTREAM:RAW:${VITIS_LINK_OUTPUT}.bit \
                        --input ${CMAKE_BINARY_DIR}/${VITIS_LINK_OUTPUT}.xclbin --force")
                # generate bif file
                run("echo 'all:{${VITIS_LINK_OUTPUT}.bit}' > ${CMAKE_BINARY_DIR}/${VITIS_LINK_OUTPUT}.bif")
                # generate .bit.bin file
                run("bootgen -w -arch zynqmp -process_bitstream bin -image ${CMAKE_BINARY_DIR}/${VITIS_LINK_OUTPUT}.bif")
                # generate or copy default dtbo
                if ("${VITIS_LINK_DTSI}" STREQUAL "")
                    run("cp ${FIRMWARE_DATA}/../device_tree/kernel_default.dtbo \
                        ${CMAKE_BINARY_DIR}/${VITIS_LINK_OUTPUT}.dtbo")
                else()
                    run("dtc -I dts -O dtb -o ${CMAKE_BINARY_DIR}/${VITIS_LINK_OUTPUT}.dtbo \
                        ${CMAKE_SOURCE_DIR}/${VITIS_LINK_DTSI}")
                endif()  # dtbo generation - or copy        
                # copy default shell.json if non-existing at ${CMAKE_BINARY_DIR}/shell.json
                #   TODO: optionally pass a different shell.json as an argument
                if ("${VITIS_KERNEL_AUX_SHELLJSON}" STREQUAL "")
                    run("cp ${FIRMWARE_DATA}/../shell.json \
                        ${CMAKE_BINARY_DIR}/shell.json")
                endif()  # shell.json
                
                # install Vitis AA artifacts
                install(
                FILES                
                    "${CMAKE_BINARY_DIR}/${VITIS_LINK_OUTPUT}.xclbin"
                    "${CMAKE_BINARY_DIR}/${VITIS_LINK_OUTPUT}.dtbo"
                    "${CMAKE_BINARY_DIR}/${VITIS_LINK_OUTPUT}.bit.bin"
                    "${CMAKE_BINARY_DIR}/shell.json"
                DESTINATION
                    lib/${PROJECT_NAME}
                )
            endif()  # generate AA files
        endif()  # NOKERNELS
    endif()  # cross compilation, DEFINED CMAKE_SYSROOT
endmacro()  # vitis_acceleration_kernel
