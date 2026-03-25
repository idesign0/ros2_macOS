gz_get_sources(tests)

if (SKIP_av OR INTERNAL_SKIP_av)
  list(REMOVE_ITEM tests encoder_timing.cc)
  list(REMOVE_ITEM tests video_encoder.cc)
endif()

if (SKIP_graphics OR INTERNAL_SKIP_graphics)
  list(REMOVE_ITEM tests mesh.cc)
endif()

gz_build_tests(
  TYPE INTEGRATION
  SOURCES ${tests}
  LIB_DEPS gz-common${GZ_COMMON_VER}-testing
)

# Graphics specific integration tests
if(TARGET INTEGRATION_mesh)
  target_link_libraries(INTEGRATION_mesh ${PROJECT_LIBRARY_TARGET_NAME}-graphics)
endif()

# AV specific integration tests
if(TARGET INTEGRATION_encoder_timing)
  target_link_libraries(INTEGRATION_encoder_timing ${PROJECT_LIBRARY_TARGET_NAME}-av)
endif()

if(TARGET INTEGRATION_video_encoder)
  target_link_libraries(INTEGRATION_video_encoder ${PROJECT_LIBRARY_TARGET_NAME}-av)
endif()
