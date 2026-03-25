install(PROGRAMS gz_msgs_generate.py
  RENAME ${PROJECT_NAME}_generate.py
  DESTINATION ${GZ_BIN_INSTALL_DIR})

install(PROGRAMS gz_msgs_generate_factory.py
  RENAME ${PROJECT_NAME}_generate_factory.py
  DESTINATION ${GZ_BIN_INSTALL_DIR})
