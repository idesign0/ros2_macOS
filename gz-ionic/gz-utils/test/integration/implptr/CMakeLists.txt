add_library(implptr_test_classes SHARED implptr_test_classes.cc)

target_link_libraries(implptr_test_classes
  ${PROJECT_LIBRARY_TARGET_NAME}
)

# Piggyback on the auto-generated dll exports for Windows
target_compile_definitions(
  implptr_test_classes
  PRIVATE
  -Dgz_utils${PROJECT_VERSION_MAJOR}_EXPORTS
)

gz_build_tests(TYPE INTEGRATION SOURCES ImplPtr_TEST.cc)

target_link_libraries(INTEGRATION_ImplPtr_TEST
  implptr_test_classes
)
