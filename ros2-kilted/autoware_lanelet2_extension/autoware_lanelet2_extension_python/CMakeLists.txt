cmake_minimum_required(VERSION 3.14)
project(autoware_lanelet2_extension_python)

find_package(autoware_cmake REQUIRED)
find_package(python_cmake_module REQUIRED)

autoware_package()

find_package(BoostPython REQUIRED)

include_directories(
  ${BoostPython_INCLUDE_DIRS}

)
ament_python_install_package(${PROJECT_NAME})

set(CMAKE_SHARED_LIBRARY_PREFIX "")
ament_auto_add_library(_${PROJECT_NAME}_boost_python_utility SHARED
  src/utility.cpp
)
ament_auto_add_library(_${PROJECT_NAME}_boost_python_projection SHARED
  src/projection.cpp
)
ament_auto_add_library(_${PROJECT_NAME}_boost_python_regulatory_elements SHARED
  src/regulatory_elements.cpp
)
install(TARGETS _${PROJECT_NAME}_boost_python_utility _${PROJECT_NAME}_boost_python_projection _${PROJECT_NAME}_boost_python_regulatory_elements
  DESTINATION "${PYTHON_INSTALL_DIR}/${PROJECT_NAME}"
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
  find_package(ament_cmake_pytest REQUIRED)

  set(test_files
    test/test_import_projection.py
    test/test_import_utility.py
    test/test_import_regulatory_elements.py
    test/test_projection.py
  )
  foreach(test_file ${test_files})
    get_filename_component(test_name ${test_file} NAME_WE)
    ament_add_pytest_test(${test_name} ${test_file}
      APPEND_ENV AMENT_PREFIX_PATH=${ament_index_build_path}
      PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}
      TIMEOUT 120
      WERROR ON
    )
  endforeach()
endif()

ament_auto_package(INSTALL_TO_SHARE
  test_map
)
