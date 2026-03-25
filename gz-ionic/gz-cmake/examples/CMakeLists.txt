include(ExternalProject)
include(GNUInstallDirs)

# test multiple build types
set(build_types Release RelWithDebInfo Debug)
function(shorten_build_type build_type output_var)
  if (${build_type} STREQUAL "Release")
      set(short_bt "R")
    elseif (${build_type} STREQUAL "RelWithDebInfo")
      set(short_bt "RWD")
    elseif (${build_type} STREQUAL "Debug")
      set(short_bt "D")
    elseif (${build_type} STREQUAL "Coverage")
      set(short_bt "C")
    else()
      message(FATAL_ERROR "Unknown build_type ${build_type}")
    endif()
    set (${output_var} ${short_bt} PARENT_SCOPE)
endfunction()

find_program(EXAMPLE_LCOV_PATH lcov)
if (NOT WIN32 AND EXAMPLE_LCOV_PATH)
  list(APPEND build_types Coverage)
endif()

set(example_directories
  gz_conf
  no_gz_prefix
  prerelease
  c_nodep
  c_nodep_static
  c_child
  c_child_private
  c_static_child
  comp_deps
  config_ifp
  version_from_package_xml
  )
if (NOT CMAKE_GENERATOR MATCHES "Visual Studio")
  list(APPEND example_directories
    comp_depsA
    comp_depsB
    comp_depsC
  )
endif()

foreach(example ${example_directories})
  set(run_codecheck false)
  if (${example} STREQUAL "gz_conf")
    set(example_tarball_name gz-minimal-0.1.0.tar.bz2)
  elseif (${example} STREQUAL "no_gz_prefix")
    set(example_tarball_name no_gz_prefix-0.1.0.tar.bz2)
  elseif (${example} STREQUAL "prerelease")
    set(example_tarball_name gz-minimal-1.0.0~pre1.tar.bz2)
  elseif (${example} STREQUAL "c_nodep")
    set(example_tarball_name gz-c_no_deps-0.1.0.tar.bz2)
    set(run_codecheck true)
  elseif (${example} STREQUAL "c_nodep_static")
    set(example_tarball_name gz-c_no_deps_static-0.1.0.tar.bz2)
    set(run_codecheck true)
  elseif (${example} STREQUAL "c_child")
    set(example_tarball_name gz-c_child-0.1.0.tar.bz2)
    set(run_codecheck true)
  elseif (${example} STREQUAL "c_child_private")
    set(example_tarball_name gz-c_child_private-0.1.0.tar.bz2)
    set(run_codecheck true)
  elseif (${example} STREQUAL "c_static_child")
    set(example_tarball_name gz-c_static_child-0.1.0.tar.bz2)
    set(run_codecheck true)
  elseif (${example} STREQUAL "comp_deps")
    set(example_tarball_name gz-comp_deps-0.1.0.tar.bz2)
  elseif (${example} STREQUAL "comp_depsA")
    set(example_tarball_name gz-comp_depsa-0.1.0.tar.bz2)
  elseif (${example} STREQUAL "comp_depsB")
    set(example_tarball_name gz-comp_depsb-0.1.0.tar.bz2)
  elseif (${example} STREQUAL "comp_depsC")
    set(example_tarball_name gz-comp_depsc-0.1.0.tar.bz2)
  elseif (${example} STREQUAL "config_ifp")
    set(example_tarball_name  gz-find_config-0.1.0.tar.bz2)
  elseif (${example} STREQUAL "version_from_package_xml")
    set(example_tarball_name  gz-version_from_package_xml-8.21.65.tar.bz2)

  else()
    set(example_tarball_name)
  endif()

  foreach (build_type ${build_types})
    shorten_build_type("${build_type}" short_bt)
    set(TEST_NAME ${example}_${short_bt})
    string(TIMESTAMP TEST_TIME)
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/junit_pass.xml.in"
      "${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}_pass.xml"
      @ONLY)
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/junit_fail.xml.in"
      "${CMAKE_CURRENT_BINARY_DIR}/test_results/${TEST_NAME}.xml"
      @ONLY)
    set(example_INSTALL_DIR ${CMAKE_BINARY_DIR}/install/${short_bt})
    ExternalProject_Add(
      ${TEST_NAME}

      DEPENDS FAKE_INSTALL
      SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${example}"
      # BUILD_ALWAYS needed since cmake doesn't notice when
      # example files change.
      # See alternate approach in a2113e0997c9 if this becomes too slow
      BUILD_ALWAYS 1
      CMAKE_ARGS
        "-DCMAKE_PREFIX_PATH=${FAKE_INSTALL_PREFIX}"
        "-DCMAKE_BUILD_TYPE=${build_type}"
        "-DCMAKE_INSTALL_PREFIX=${example_INSTALL_DIR}"
      TEST_COMMAND
        ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}_pass.xml"
        "${CMAKE_CURRENT_BINARY_DIR}/test_results/${TEST_NAME}.xml"
    )
    if (${run_codecheck} AND TARGET codecheck)
      if(CMAKE_GENERATOR MATCHES "Unix Makefiles")
        ExternalProject_Add_Step(
          ${TEST_NAME}
          codecheck
          COMMAND
            $(MAKE) -C <BINARY_DIR> codecheck
          DEPENDEES
            configure
        )
      else()
        ExternalProject_Add_Step(
          ${TEST_NAME}
          codecheck
          COMMAND
            ${CMAKE_COMMAND} --build <BINARY_DIR> --target codecheck
          DEPENDEES
            configure
        )
      endif()
    endif()
    if (CPACK_GENERATOR)
      if(CMAKE_GENERATOR MATCHES "Unix Makefiles")
        ExternalProject_Add_Step(
          ${TEST_NAME}
          package_source
          COMMAND
            $(MAKE) -C <BINARY_DIR> package_source
          DEPENDEES
            configure
        )
      else()
        ExternalProject_Add_Step(
          ${TEST_NAME}
          package_source
          COMMAND
            ${CMAKE_COMMAND} --build <BINARY_DIR> --target package_source
          DEPENDEES
            configure
        )
      endif()

      ExternalProject_Add_Step(
        ${TEST_NAME}
        test_tarball_name
        COMMAND
          ${CMAKE_COMMAND} -E tar tf <BINARY_DIR>/${example_tarball_name}
        DEPENDEES
          package_source
        DEPENDERS
          test
      )
    endif()
    add_test(
      ${TEST_NAME}
      ${CMAKE_COMMAND} -E copy
      "${CMAKE_CURRENT_BINARY_DIR}/test_results/${TEST_NAME}.xml"
      "${CMAKE_BINARY_DIR}/test_results/${TEST_NAME}.xml"
    )
  endforeach()

endforeach()

# dependencies between external projects
# hard to use DEPENDS in ExternalProject_Add because targets
# need to exist before they can be used there
foreach (build_type ${build_types})
  shorten_build_type("${build_type}" short_bt)
  add_dependencies(c_child_${short_bt} c_nodep_${short_bt})
  add_dependencies(c_child_private_${short_bt} c_nodep_${short_bt})
  add_dependencies(c_static_child_${short_bt} c_nodep_static_${short_bt})
  if (TARGET comp_depsA_${short_bt})
    add_dependencies(comp_depsA_${short_bt} comp_deps_${short_bt})
  endif()
  if (TARGET comp_depsB_${short_bt})
    add_dependencies(comp_depsB_${short_bt} comp_deps_${short_bt})
  endif()
  if (TARGET comp_depsC_${short_bt})
    add_dependencies(comp_depsC_${short_bt} comp_deps_${short_bt})
  endif()
endforeach()

# test that c_child pkg-config file requires c_nodep
# and that c_child_private pkg-config file requires c_nodep privately
if (UNIX)
  set(TEST_NAME c_child_requires_c_nodep)
  string(TIMESTAMP TEST_TIME)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/junit_pass.xml.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}_pass.xml"
    @ONLY)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/junit_fail.xml.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}_fail.xml"
    @ONLY)
  set(_env_vars)
  # On Debian and if the install prefix is /usr, the default CMAKE_INSTALL_LIBDIR of this project
  # as set by GNUInstallDirs will be different from the one of the example project,
  # so let's hardcode it in that case
  if(EXISTS "/etc/debian_version" AND "${CMAKE_INSTALL_PREFIX}" MATCHES "^/usr/?$")
    set(example_PKGCONFIG_INSTALL_LIBDIR "lib/pkgconfig")
  else()
    set(example_PKGCONFIG_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
  endif()

  list(APPEND _env_vars "PKG_CONFIG_PATH=${example_INSTALL_DIR}/${example_PKGCONFIG_INSTALL_LIBDIR}:${FAKE_INSTALL_PREFIX}/${example_PKGCONFIG_INSTALL_LIBDIR}:$PKG_CONFIG_PATH")
  add_test(${TEST_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/test_c_child_requires_c_no_deps.bash
  )
  set_tests_properties(${TEST_NAME} PROPERTIES
    ENVIRONMENT "${_env_vars}")
endif()

if(UNIX)
  # Test for the sanitizers example
  set(sanitizer_compiler_log ${CMAKE_BINARY_DIR}/examples/Sanitizers-prefix/src/Sanitizers-stamp/Sanitizers-build-out.log)
  ExternalProject_Add(
    Sanitizers

    DEPENDS FAKE_INSTALL
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sanitizers"
    LOG_BUILD ON
    LOG_CONFIGURE ON
    # BUILD_ALWAYS needed since cmake doesn't notice when
    # example files change.
    # See alternate approach in a2113e0997c9 if this becomes too slow
    BUILD_ALWAYS 1
    BUILD_COMMAND ${CMAKE_COMMAND} --build . --clean-first
    CMAKE_ARGS
      "-DCMAKE_PREFIX_PATH=${FAKE_INSTALL_PREFIX};${example_INSTALL_DIR}"
      "-DCMAKE_BUILD_TYPE=RelWithDebInfo"
      "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install/Sanitizers"
      "-DGZ_SANITIZER=Address"
      "-DCMAKE_CXX_FLAGS=-Wno-unused-parameter"
      "-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON"
    TEST_COMMAND
      # Multiplatform python equivalent to grep "fsanitize=address" "${sanitizer_compiler_log}"
      python3 -c "exec(\"import sys\\nwith open('${sanitizer_compiler_log}') as f:sys.exit(0) if 'fsanitize=address' in f.read() else sys.exit(1)\")"
  )
endif()
