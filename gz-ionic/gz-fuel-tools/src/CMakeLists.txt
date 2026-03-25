set (sources
  ClientConfig.cc
  CollectionIdentifier.cc
  FuelClient.cc
  Helpers.cc
  gz.cc
  Interface.cc
  JSONParser.cc
  LocalCache.cc
  Model.cc
  ModelIdentifier.cc
  ModelIter.cc
  RestClient.cc
  Result.cc
  ServerConfig.cc
  Zip.cc
  WorldIdentifier.cc
  WorldIter.cc
)

set (gtest_sources
  ClientConfig_TEST.cc
  CollectionIdentifier_TEST.cc
  FuelClient_TEST.cc
  gz_src_TEST.cc
  Interface_TEST.cc
  Helpers_TEST.cc
  JSONParser_TEST.cc
  LocalCache_TEST.cc
  ModelIdentifier_TEST.cc
  ModelIter_TEST.cc
  Model_TEST.cc
  RestClient_TEST.cc
  Result_TEST.cc
  ServerConfig_TEST.cc
  WorldIdentifier_TEST.cc
  WorldIter_TEST.cc
  Zip_TEST.cc
)

if (HAVE_GZ_TOOLS)
  set (gtest_sources ${gtest_sources}
    gz_TEST.cc
  )
endif()

# Create the library target.
gz_create_core_library(SOURCES ${sources} CXX_STANDARD 17)

# Link the libraries that we always need.
target_link_libraries(${PROJECT_LIBRARY_TARGET_NAME}
  PUBLIC
    gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
  PRIVATE
    gz-msgs${GZ_MSGS_VER}::gz-msgs${GZ_MSGS_VER}
    curl::curl
    ${JSONCPP_TARGET}
    TINYXML2::TINYXML2
    ${YAML_TARGET}
    ZIP::ZIP
)

gz_target_interface_include_directories(${PROJECT_LIBRARY_TARGET_NAME}
  gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
  gz-msgs${GZ_MSGS_VER}::gz-msgs${GZ_MSGS_VER}
)


# Build the unit tests.
gz_build_tests(TYPE UNIT
                SOURCES ${gtest_sources}
                TEST_LIST test_targets
                LIB_DEPS
                  gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
                  gz-common${GZ_COMMON_VER}::testing
                  TINYXML2::TINYXML2
)

# Some tests download simulation assets, which can take longer than the
# default time limit.
if (TARGET UNIT_gz_src_TEST)
  set_tests_properties(UNIT_gz_src_TEST PROPERTIES TIMEOUT 1000)
endif()
if (TARGET UNIT_FuelClient_TEST)
  set_tests_properties(UNIT_FuelClient_TEST PROPERTIES TIMEOUT 1000)
endif()

foreach(test ${test_targets})

  target_compile_definitions(${test} PRIVATE
    "GZ_PATH=\"${HAVE_GZ_TOOLS}\"")

endforeach()

# Command line support.
add_subdirectory(cmd)
