option(GZ_COMMON_BUILD_HW_VIDEO
  "Build support for HW-accelerated video encoding" ON)

gz_get_libsources_and_unittests(sources gtest_sources)

if(NOT GZ_COMMON_BUILD_HW_VIDEO)
  list(REMOVE_ITEM sources HWEncoder.cc)
endif()

gz_add_component(av SOURCES ${sources} GET_TARGET_NAME av_target)

target_link_libraries(${av_target}
  PUBLIC
    SWSCALE::SWSCALE
    AVDEVICE::AVDEVICE
    AVFORMAT::AVFORMAT
    AVCODEC::AVCODEC
    AVUTIL::AVUTIL)

if(GZ_COMMON_BUILD_HW_VIDEO)
  target_compile_definitions(${av_target} PRIVATE GZ_COMMON_BUILD_HW_VIDEO)
endif()

gz_build_tests(
  TYPE UNIT
  SOURCES ${gtest_sources}
  LIB_DEPS
    ${av_target}
    gz-common${GZ_COMMON_VER}-testing
)
