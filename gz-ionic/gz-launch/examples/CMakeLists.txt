file(GLOB files "*.gzlaunch")
install(FILES ${files}
  DESTINATION ${GZ_DATA_INSTALL_DIR}/configs)
