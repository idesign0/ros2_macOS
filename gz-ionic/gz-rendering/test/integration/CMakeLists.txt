set(TEST_TYPE "INTEGRATION")

set(tests
  boundingbox_camera
  camera
  depth_camera
  gpu_rays
  heightmap
  lidar_visual
  mesh
  projector
  render_pass
  scene
  segmentation_camera
  shadows
  sky
  thermal_camera
  load_unload
  waves
  wide_angle_camera
)

foreach(test ${tests})
  gz_rendering_test(
    TYPE ${TEST_TYPE}
    SOURCE ${test}
    LIB_DEPS
      gz-plugin${GZ_PLUGIN_VER}::loader
      gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
      ${PROJECT_LIBRARY_TARGET_NAME}
  )
endforeach()

# Test symbols having the right name on linux only
if (UNIX AND NOT APPLE)
  configure_file(all_symbols_have_version.bash.in ${CMAKE_CURRENT_BINARY_DIR}/all_symbols_have_version.bash @ONLY)
  add_test(NAME INTEGRATION_versioned_symbols
    COMMAND bash ${CMAKE_CURRENT_BINARY_DIR}/all_symbols_have_version.bash $<TARGET_FILE:${PROJECT_LIBRARY_TARGET_NAME}>)

  foreach(comp ${RENDERING_COMPONENTS})
   set(comp_target_name "${PROJECT_LIBRARY_TARGET_NAME}-${comp}")
   add_test(NAME "INTEGRATION_versioned_symbols_${comp}"
    COMMAND bash ${CMAKE_CURRENT_BINARY_DIR}/all_symbols_have_version.bash $<TARGET_FILE:${comp_target_name}>)
  endforeach()
endif()
