message(STATUS "Configuring output file ${OUTPUT_FILE}")
configure_file("${INPUT_FILE}" "${OUTPUT_FILE}" @ONLY)
