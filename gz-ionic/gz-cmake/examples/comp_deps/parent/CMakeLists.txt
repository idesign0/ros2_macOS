
gz_add_component(parent INTERFACE
  GET_TARGET_NAME parent)

install(
  DIRECTORY include/
  DESTINATION "${GZ_INCLUDE_INSTALL_DIR_FULL}")
