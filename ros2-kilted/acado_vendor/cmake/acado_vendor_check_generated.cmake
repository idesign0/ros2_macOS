# Checks if a given path exists
# This is a separate file so it can be run AFTER an executable
# has been built (and run)
if(NOT (EXISTS ${_OUTPUT_PATH}))
  message(FATAL_ERROR
    "${_OUTPUT_PATH} does not exist! What did you call exportCode with?"
  )
endif()
