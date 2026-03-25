set(TEST_TYPE "REGRESSION")

gz_rendering_test(
  TYPE ${TEST_TYPE}
  SOURCE reload_engine
  LIB_DEPS
    gz-plugin${GZ_PLUGIN_VER}::loader
    gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
    ${PROJECT_LIBRARY_TARGET_NAME}
)

