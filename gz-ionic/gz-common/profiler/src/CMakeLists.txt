option(GZ_PROFILER_REMOTERY "Enable the Remotery profiler" TRUE)

set(
  PROFILER_SRCS
  Profiler.cc
)

set(
  PROFILER_TESTS
  Profiler_Disabled_TEST.cc
)

if(GZ_PROFILER_REMOTERY)
  set(
    Remotery_SRC
    ./Remotery/lib/Remotery.c
    ./Remotery/lib/Remotery.h
  )

  # A few warnings to suppress, rather than modifying vendored code
  set_source_files_properties(./Remotery/lib/Remotery.c PROPERTIES
    COMPILE_FLAGS
    "$<$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>:-Wno-strict-prototypes -Wno-unused-but-set-variable>")

  if(APPLE)
    set(
      Remotery_SRC
      ${Remotery_SRC}
      ./Remotery/lib/RemoteryMetal.mm
    )
    find_library(FOUNDATION Foundation
      HINTS /System/Library/Frameworks
    )
  endif()

  set(RMT_ENABLED 1)
  set(RMT_USE_TINYCRT 0)
  set(RMT_USE_CUDA 0)
  set(RMT_USE_D3D11 0)
  set(RMT_USE_OPENGL 0)
  set(RMT_USE_METAL 0)

  if(UNIX OR WIN32)
    set(RMT_USE_OPENGL 1)
  endif()

  if(WIN32)
    set(RMT_USE_D3D11 1)
  endif()

  if(APPLE)
    set (RMT_USE_METAL 1)
  endif()

  configure_file(RemoteryConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/RemoteryConfig.h)

  list(APPEND PROFILER_SRCS ${Remotery_SRC} RemoteryProfilerImpl.cc)
  list(APPEND PROFILER_TESTS Profiler_Remotery_TEST.cc)

  if(NOT WIN32)
    list(APPEND PROFILER_TESTS Profiler_Error_TEST.cc)
  endif()
else()  # GZ_PROFILER_REMOTERY
  list(APPEND PROFILER_TESTS Profiler_Custom_TEST.cc)
endif()

gz_add_component(profiler SOURCES ${PROFILER_SRCS} GET_TARGET_NAME profiler_target)

# Always enable profiler so that it's built, but make it
# private so that downstream users can still disable profiling
target_compile_definitions(${profiler_target} PRIVATE "GZ_PROFILER_ENABLE=1")
target_compile_definitions(${profiler_target} PRIVATE "RMT_USE_METAL=${RMT_USE_METAL}")

if(GZ_PROFILER_REMOTERY)
  target_compile_definitions(${profiler_target} PRIVATE "GZ_PROFILER_REMOTERY=1")
  target_include_directories(
    ${profiler_target}
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Remotery/lib>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  )

  if (UNIX)
    target_link_libraries(${profiler_target} PUBLIC pthread)
  endif()

  if(APPLE)
    target_link_libraries(${profiler_target} PUBLIC ${FOUNDATION})
  endif()
endif()

gz_build_tests(
  TYPE UNIT
  SOURCES ${PROFILER_TESTS}
  LIB_DEPS
    ${profiler_target}
    gz-common${GZ_COMMON_VER}-testing
  TEST_LIST profiler_tests)

if(TARGET UNIT_Profiler_Remotery_TEST)
  target_compile_definitions(UNIT_Profiler_Remotery_TEST
    PUBLIC "GZ_PROFILER_ENABLE=1")
endif()

if(TARGET UNIT_Profiler_Error_TEST)
  target_include_directories(UNIT_Profiler_Error_TEST
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Remotery/lib>
  )
endif()

if(TARGET UNIT_Profiler_Custom_TEST)
  target_compile_definitions(UNIT_Profiler_Custom_TEST
    PUBLIC "GZ_PROFILER_ENABLE=1")
endif()

if(GZ_PROFILER_REMOTERY)
  set(GZ_PROFILER_SCRIPT_PATH ${CMAKE_INSTALL_LIBEXECDIR}/gz/gz-common${PROJECT_VERSION_MAJOR})
  set(GZ_PROFILER_VIS_PATH ${GZ_DATA_INSTALL_DIR}/profiler_vis)

  configure_file(Remotery/gz_remotery_vis.in
    ${CMAKE_CURRENT_BINARY_DIR}/gz_remotery_vis
    @ONLY)

  install(PROGRAMS
    ${CMAKE_CURRENT_BINARY_DIR}/gz_remotery_vis
    DESTINATION ${GZ_PROFILER_SCRIPT_PATH})

  install(DIRECTORY Remotery/vis/
          DESTINATION ${GZ_PROFILER_VIS_PATH})
endif()
