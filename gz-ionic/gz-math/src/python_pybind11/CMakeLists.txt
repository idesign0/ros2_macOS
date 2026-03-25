# Detect if we are doing a standalone build of the bindings, using an external gz-math
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required(VERSION 3.22.1)
  find_package(gz-cmake4 4.1.0 REQUIRED)
  gz_get_package_xml_version(${CMAKE_SOURCE_DIR}/../../package.xml GZ_MATH)
  project(gz-math${GZ_MATH_VERSION_MAJOR}-python VERSION ${GZ_MATH_VERSION})
  find_package(gz-math${PROJECT_VERSION_MAJOR} REQUIRED)
  set(PROJECT_LIBRARY_TARGET_NAME "gz-math${PROJECT_VERSION_MAJOR}::gz-math${PROJECT_VERSION_MAJOR}")
  # require python dependencies to be found
  find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
  set(CMAKE_REQUIRE_FIND_PACKAGE_pybind11 TRUE)
  include(GNUInstallDirs)
  include(CTest)
endif()

set(PYBIND11_PYTHON_VERSION 3)
find_package(pybind11 2.2 QUIET)

if (${pybind11_FOUND})
  message(STATUS "Searching for pybind11 - found version ${pybind11_VERSION}.")
else()
  message(WARNING "pybind11 is missing: Python interfaces are disabled.")
  return()
endif()

message(STATUS "Building pybind11 interfaces")
set(BINDINGS_MODULE_NAME "math${PROJECT_VERSION_MAJOR}")
# Split from main extension and converted to pybind11
pybind11_add_module(${BINDINGS_MODULE_NAME} MODULE
  src/_gz_math_pybind11.cc
  src/Angle.cc
  src/AxisAlignedBox.cc
  src/Capsule.cc
  src/Color.cc
  src/CoordinateVector3.cc
  src/DiffDriveOdometry.cc
  src/Ellipsoid.cc
  src/Filter.cc
  src/Frustum.cc
  src/GaussMarkovProcess.cc
  src/Helpers.cc
  src/Interval.cc
  src/Kmeans.cc
  src/Line2.cc
  src/Line3.cc
  src/MassMatrix3.cc
  src/Material.cc
  src/Matrix3.cc
  src/Matrix4.cc
  src/Matrix6.cc
  src/MecanumDriveOdometry.cc
  src/MovingWindowFilter.cc
  src/PID.cc
  src/Polynomial3.cc
  src/Pose3.cc
  src/Quaternion.cc
  src/Rand.cc
  src/Region3.cc
  src/RollingMean.cc
  src/RotationSpline.cc
  src/SemanticVersion.cc
  src/SignalStats.cc
  src/SphericalCoordinates.cc
  src/Spline.cc
  src/StopWatch.cc
  src/Temperature.cc
  src/Triangle.cc
  src/Triangle3.cc
  src/Vector2.cc
  src/Vector3.cc
  src/Vector4.cc
  src/Vector3Stats.cc
)

target_link_libraries(${BINDINGS_MODULE_NAME} PRIVATE
  ${PROJECT_LIBRARY_TARGET_NAME}
)

target_compile_definitions(${BINDINGS_MODULE_NAME} PRIVATE
  BINDINGS_MODULE_NAME=${BINDINGS_MODULE_NAME})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # Workaround for Clang and pybind11 on Focal
  # https://github.com/pybind/pybind11/issues/1604
  # Resolved by newer versions of pybind11
  if(${pybind11_VERSION} VERSION_LESS "2.4.4")
    target_compile_options(${BINDINGS_MODULE_NAME} PRIVATE -fsized-deallocation)
  endif()

  # Suppress warnings that clang misidentifies:
  # https://github.com/pybind/pybind11/issues/1893
  target_compile_options(${BINDINGS_MODULE_NAME} PRIVATE -Wno-self-assign-overloaded)
endif()

if(USE_SYSTEM_PATHS_FOR_PYTHON_INSTALLATION)
  if(NOT Python3_SITEARCH)
    # Get install variable from Python3 module
    find_package(Python3 COMPONENTS Interpreter)
  endif()

  if(USE_DIST_PACKAGES_FOR_PYTHON)
    string(REPLACE "site-packages" "dist-packages" GZ_PYTHON_INSTALL_PATH ${Python3_SITEARCH})
  else()
    # custom cmake command is returning dist-packages
    string(REPLACE "dist-packages" "site-packages" GZ_PYTHON_INSTALL_PATH ${Python3_SITEARCH})
  endif()
else()
  # If not a system installation, respect local paths
  set(GZ_PYTHON_INSTALL_PATH ${CMAKE_INSTALL_LIBDIR}/python)
endif()

set(GZ_PYTHON_INSTALL_PATH "${GZ_PYTHON_INSTALL_PATH}/gz")

# Set the build location and install location for a CPython extension
function(configure_build_install_location _library_name)
  # Install into test folder in build space for unit tests to import
  set_target_properties(${_library_name} PROPERTIES
    # Use generator expression to avoid prepending a build type specific directory on Windows
    LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_CURRENT_BINARY_DIR}/test/gz>
    RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_CURRENT_BINARY_DIR}/test/gz>)

  # Touch an init file to mark this directory as a usable python module
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test/gz/)
  file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/test/gz/__init__.py)

  # Install library for actual use
  install(TARGETS ${_library_name}
    DESTINATION "${GZ_PYTHON_INSTALL_PATH}/"
  )
endfunction()

configure_build_install_location(${BINDINGS_MODULE_NAME})

if (BUILD_TESTING)
  # Add the Python tests
  set(python_tests
    Angle_TEST
    AxisAlignedBox_TEST
    AxisAlignedBoxHelpers_TEST
    Box_TEST
    Capsule_TEST
    Color_TEST
    Cone_TEST
    CoordinateVector3_TEST
    Cylinder_TEST
    DiffDriveOdometry_TEST
    Ellipsoid_TEST
    Filter_TEST
    Frustum_TEST
    GaussMarkovProcess_TEST
    Helpers_TEST
    Inertial_TEST
    Interval_TEST
    Kmeans_TEST
    Line2_TEST
    Line3_TEST
    MassMatrix3_TEST
    Material_TEST
    Matrix3_TEST
    Matrix4_TEST
    Matrix6_TEST
    MecanumDriveOdometry_TEST
    MovingWindowFilter_TEST
    OrientedBox_TEST
    PID_TEST
    Plane_TEST
    Polynomial3_TEST
    Pose3_TEST
    Quaternion_TEST
    Rand_TEST
    Region3_TEST
    RollingMean_TEST
    RotationSpline_TEST
    SemanticVersion_TEST
    SignalStats_TEST
    Sphere_TEST
    SphericalCoordinates_TEST
    Spline_TEST
    StopWatch_TEST
    Temperature_TEST
    Triangle3_TEST
    Triangle_TEST
    Vector2_TEST
    Vector3_TEST
    Vector3Stats_TEST
    Vector4_TEST
  )

  execute_process(COMMAND "${Python3_EXECUTABLE}" -m pytest --version
    OUTPUT_VARIABLE PYTEST_output
    ERROR_VARIABLE  PYTEST_error
    RESULT_VARIABLE PYTEST_result)
  if(${PYTEST_result} EQUAL 0)
    set(pytest_FOUND TRUE)
  else()
    message("")
    message(WARNING "Pytest package not available: ${PYTEST_error}")
  endif()

  foreach (test ${python_tests})
    if (pytest_FOUND)
      add_test(NAME ${test}.py COMMAND
        "${Python3_EXECUTABLE}" -m pytest "${CMAKE_SOURCE_DIR}/src/python_pybind11/test/${test}.py" --junitxml "${CMAKE_BINARY_DIR}/test_results/${test}.xml")
    else()
      add_test(NAME ${test}.py COMMAND
        "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/src/python_pybind11/test/${test}.py")
    endif()

    set(_env_vars)
    list(APPEND _env_vars "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/test")
    list(APPEND _env_vars "LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}/test:$ENV{LD_LIBRARY_PATH}")
    set_tests_properties(${test}.py PROPERTIES
      ENVIRONMENT "${_env_vars}")
  endforeach()

endif()
