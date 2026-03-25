# This script is part of the unit test mechanism for gz-tools.
execute_process(COMMAND ruby ${GZ_SCRIPT}
  ${TEST_NAME}
  OUTPUT_VARIABLE TEST_OUTPUT
  ERROR_VARIABLE TEST_ERROR)

# Copy failure result every time we run test so we don't accidentally use
# previous results.
execute_process(COMMAND
  ${CMAKE_COMMAND} -E copy
  "${TEST_NAME}_fail.xml"
  "../test_results/${TEST_NAME}.xml"
)

if (${TEST_OUTPUT} MATCHES "Test executed" AND NOT
    ((${TEST_OUTPUT} MATCHES "Library error") OR
    (${TEST_ERROR} MATCHES "Library error")))
  execute_process(COMMAND
    ${CMAKE_COMMAND} -E copy
    "${TEST_NAME}_pass.xml"
    "../test_results/${TEST_NAME}.xml"
  )
else()
  message(FATAL_ERROR "Failed test.
  Output: ${TEST_OUTPUT}
  Error: ${TEST_ERROR}")
endif()
