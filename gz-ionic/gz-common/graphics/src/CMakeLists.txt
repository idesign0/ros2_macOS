gz_get_libsources_and_unittests(sources gtest_sources)

gz_add_component(graphics SOURCES ${sources} GET_TARGET_NAME graphics_target)

target_link_libraries(${graphics_target}
  PUBLIC
    gz-math${GZ_MATH_VER}::gz-math${GZ_MATH_VER}
    gz-utils${GZ_UTILS_VER}::gz-utils${GZ_UTILS_VER}
  PRIVATE
    ${GzAssimp_LIBRARIES}
    FreeImage::FreeImage
    TINYXML2::TINYXML2
)

# CDT does a few float comparisons that cause warnings
target_compile_options(${graphics_target} PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:   -Wno-switch-default -Wno-float-equal>   # GCC
    $<$<CXX_COMPILER_ID:Clang>: -Wno-switch-default -Wno-float-equal>   # Clang
)

gz_build_tests(
  TYPE UNIT
  SOURCES ${gtest_sources}
  LIB_DEPS
    ${graphics_target}
    gz-common${GZ_COMMON_VER}-testing
    TINYXML2::TINYXML2
)

# Assimp doesn't offer preprocessor version, use cmake to set a compatibility
# mode for versions below 5.2.0 and 5.1.0
if(${GzAssimp_VERSION} STRLESS "5.2.0")
  message("Warning, assimp below 5.2.0 detected, setting compatibility mode")
  target_compile_definitions(${graphics_target} PRIVATE GZ_ASSIMP_PRE_5_2_0)
  if(TARGET UNIT_AssimpLoader_TEST)
    target_compile_definitions(UNIT_AssimpLoader_TEST PRIVATE GZ_ASSIMP_PRE_5_2_0)
  endif()
  if(${GzAssimp_VERSION} STRLESS "5.1.0")
    message("Warning, assimp below 5.1.0 detected, setting compatibility mode")
    target_compile_definitions(${graphics_target} PRIVATE GZ_ASSIMP_PRE_5_1_0)
    if(TARGET UNIT_AssimpLoader_TEST)
      target_compile_definitions(UNIT_AssimpLoader_TEST PRIVATE GZ_ASSIMP_PRE_5_1_0)
    endif()
  endif()
endif()
