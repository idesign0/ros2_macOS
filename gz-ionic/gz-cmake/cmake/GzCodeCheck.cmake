# Setup the codecheck target, which will run cppcheck and cppplint.
# This function is private to gz-cmake.
function(_gz_setup_target_for_codecheck)
  if(NOT Python3_Interpreter_FOUND)
    find_package(Python3 COMPONENTS Interpreter)
  endif()

  find_program(CPPCHECK_PATH cppcheck)
  find_program(FIND_PATH find)

  if(NOT CPPCHECK_PATH)
    message(STATUS "The program [cppcheck] was not found! Skipping codecheck setup")
    return()
  endif()

  if(NOT FIND_PATH)
    message(STATUS "The program [find] was not found! Skipping codecheck setup.")
    return()
  endif()

  # Base set of cppcheck option
  set (CPPCHECK_BASE -q --inline-suppr -j 4 --language=c++ --std=c++17 --force)
  if (EXISTS "${PROJECT_BINARY_DIR}/cppcheck.suppress")
    set (CPPCHECK_BASE ${CPPCHECK_BASE} --suppressions-list=${PROJECT_BINARY_DIR}/cppcheck.suppress)
  endif()

  # Extra cppcheck option
  set (CPPCHECK_EXTRA --enable=style,performance,portability,information)

  # Rules for cppcheck
  set (CPPCHECK_RULES "-UM_PI --rule-file=${GZ_CMAKE_CODECHECK_DIR}/header_guard.rule --rule-file=${GZ_CMAKE_CODECHECK_DIR}/namespace_AZ.rule")

  # The find command
  set (CPPCHECK_FIND ${FIND_PATH} ${CPPCHECK_DIRS} -name '*.cc' -o -name '*.hh' -o -name '*.c' -o -name '*.h')

  message(STATUS "Adding codecheck target")

  # Each include directory needs an -I flag
  set(CPPCHECK_INCLUDE_DIRS_FLAGS)
  foreach(dir ${CPPCHECK_INCLUDE_DIRS})
    list(APPEND CPPCHECK_INCLUDE_DIRS_FLAGS "-I${dir}")
  endforeach()

  add_custom_target(cppcheck
    # First cppcheck
    COMMAND ${CPPCHECK_PATH} ${CPPCHECK_BASE} ${CPPCHECK_EXTRA} ${CPPCHECK_INCLUDE_DIRS_FLAGS} ${CPPCHECK_RULES} `${CPPCHECK_FIND}`

    # Second cppcheck
    COMMAND ${CPPCHECK_PATH} ${CPPCHECK_BASE} --enable=missingInclude `${CPPCHECK_FIND}`
  )

  add_custom_target(codecheck
    DEPENDS cppcheck
  )

  if(Python3_Interpreter_FOUND)
    add_custom_target(cpplint
      COMMAND ${Python3_EXECUTABLE} ${GZ_CMAKE_CODECHECK_DIR}/cpplint.py --extensions=cc,hh --quiet `${CPPCHECK_FIND}`
    )

    add_dependencies(codecheck cpplint)
  endif()
endfunction()
