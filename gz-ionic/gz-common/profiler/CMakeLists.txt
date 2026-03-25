add_subdirectory(src)

install(
  DIRECTORY include/gz
  DESTINATION ${GZ_INCLUDE_INSTALL_DIR_FULL})
