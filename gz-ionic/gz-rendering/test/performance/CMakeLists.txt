set(TEST_TYPE "PERFORMANCE")

set(tests
  scene_factory
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
