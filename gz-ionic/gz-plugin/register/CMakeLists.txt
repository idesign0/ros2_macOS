gz_add_component(register INTERFACE)

install(
  DIRECTORY include/
  DESTINATION ${GZ_INCLUDE_INSTALL_DIR_FULL}
  PATTERN "CMakeLists.txt" EXCLUDE)
