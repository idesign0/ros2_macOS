cmake_minimum_required(VERSION 3.5)
project(rqt_py_common)

# Load catkin and all dependencies required for this package
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)

ament_python_install_package(${PROJECT_NAME}
  PACKAGE_DIR src/${PROJECT_NAME})

install(DIRECTORY resource
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(rosidl_default_generators REQUIRED)
  find_package(ament_cmake_pytest REQUIRED)
  # Provides PYTHON_EXECUTABLE_DEBUG
  find_package(python_cmake_module REQUIRED)
  find_package(PythonExtra MODULE)

  if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(${PROJECT_NAME}_PYTHON_EXECUTABLE "${PYTHON_EXECUTABLE_DEBUG}")
  else()
    set(${PROJECT_NAME}_PYTHON_EXECUTABLE "${PYTHON_EXECUTABLE}")
  endif()

  # use different project name for generated test interfaces in order to make
  # them importable in the tests beside the Python module of this package
  set(_PROJECT_NAME ${PROJECT_NAME})
  set(PROJECT_NAME "rqt_py_common_test_interfaces")
  rosidl_generate_interfaces(${PROJECT_NAME}
    test/msg/Val.msg
    test/msg/ArrayVal.msg
    test/srv/AddTwoInts.srv
    SKIP_INSTALL)
  # setup fake ament index
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/ament_index/share/ament_index/resource_index/packages")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ament_index/share/ament_index/resource_index/packages/${_PROJECT_NAME}")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ament_index/share/ament_index/resource_index/packages/${PROJECT_NAME}")
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/ament_index/share/ament_index/resource_index/rosidl_interfaces")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ament_index/share/ament_index/resource_index/rosidl_interfaces/${_PROJECT_NAME}")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ament_index/share/ament_index/resource_index/rosidl_interfaces/${PROJECT_NAME}"
    "msg/Val.msg\n"
    "msg/ArrayVal.msg\n"
    "srv/AddTwoInts.srv")
  set(PROJECT_NAME ${_PROJECT_NAME})

  ament_add_pytest_test(${PROJECT_NAME} test
    ENV "AMENT_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}/ament_index"
    APPEND_ENV PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_py
    TIMEOUT 90,
    PYTHON_EXECUTABLE "${${PROJECT_NAME}_PYTHON_EXECUTABLE}")
endif()

ament_package()
