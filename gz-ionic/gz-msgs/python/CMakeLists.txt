# Append `_configured` to the file name so it doesn't interfere with tests.
# This happens because pytest will load the `gz.msgs` package from the build directory
# (because there's an __init__.py file there) instead of being redirected to
# `gz.msgs11` in the install directory, which is the intent of this `__init__.py` file.
set(python_init_file ${PROJECT_BINARY_DIR}/python/gz/${GS_DESIGNATION}/__init__.py_configured)
configure_file(${PROJECT_SOURCE_DIR}/python/src/__init__.py.in ${python_init_file})

install(FILES ${python_init_file} DESTINATION ${GZ_PYTHON_INSTALL_PATH}/gz/${GZ_DESIGNATION}${PROJECT_VERSION_MAJOR} RENAME __init__.py)

if (BUILD_TESTING AND NOT WIN32)
  set(python_tests
    basic_TEST
  )
  execute_process(COMMAND "${Python3_EXECUTABLE}" -m pytest --version
    OUTPUT_VARIABLE PYTEST_output
    ERROR_VARIABLE  PYTEST_error
    RESULT_VARIABLE PYTEST_result)
  if(${PYTEST_result} EQUAL 0)
    set(pytest_FOUND TRUE)
  else()
    message(WARNING "Pytest package not available: ${PYTEST_error}")
    message(WARNING "Output: ${PYTEST_output}")
  endif()

  foreach (test ${python_tests})
    if (pytest_FOUND)
      add_test(NAME ${test}.py COMMAND
        "${Python3_EXECUTABLE}" -m pytest "${CMAKE_SOURCE_DIR}/python/test/${test}.py" --junitxml "${CMAKE_BINARY_DIR}/test_results/${test}.xml")
    else()
      add_test(NAME ${test}.py COMMAND
        "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/python/test/${test}.py")
    endif()
    set(_env_vars "PYTHONPATH=${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/python/")
    set_tests_properties(${test}.py PROPERTIES ENVIRONMENT "${_env_vars}")
  endforeach()
endif()
