set(TEST_TYPE "INTEGRATION")

set(tests
  authPubSub.cc
  scopedTopic.cc
  callback_scope_TEST.cc
  statistics.cc
  twoProcsPubSub.cc
  twoProcsSrvCall.cc
  twoProcsSrvCallStress.cc
  twoProcsSrvCallSync1.cc
  twoProcsSrvCallWithoutInput.cc
  twoProcsSrvCallWithoutInputStress.cc
  twoProcsSrvCallWithoutInputSync1.cc
  twoProcsSrvCallWithoutOutput.cc
  twoProcsSrvCallWithoutOutputStress.cc
)

# Test symbols having the right name on linux only.
if (UNIX AND NOT APPLE)
  configure_file(all_symbols_have_version.bash.in
    ${CMAKE_CURRENT_BINARY_DIR}/all_symbols_have_version.bash @ONLY)
  add_test(NAME INTEGRATION_versioned_symbols
    COMMAND bash ${CMAKE_CURRENT_BINARY_DIR}/all_symbols_have_version.bash $<TARGET_FILE:${PROJECT_LIBRARY_TARGET_NAME}>)
endif()

gz_build_tests(TYPE INTEGRATION SOURCES ${tests}
  TEST_LIST test_list
  LIB_DEPS ${EXTRA_TEST_LIB_DEPS} test_config)

foreach(test ${test_list})
  set(_env_vars)
  list(APPEND _env_vars "GZ_IP=127.0.0.1")
  set_property(TEST ${test} PROPERTY ENVIRONMENT "${_env_vars}")
endforeach()

set(auxiliary_files
  authPubSubSubscriberInvalid_aux
  fastPub_aux
  pub_aux
  pub_aux_throttled
  scopedTopicSubscriber_aux
  twoProcsPublisher_aux
  twoProcsPubSubMixedSubscribers_aux
  twoProcsPubSubSingleSubscriber_aux
  twoProcsPubSubSubscriber_aux
  twoProcsSrvCallReplier_aux
  twoProcsSrvCallReplierInc_aux
  twoProcsSrvCallWithoutInputReplier_aux
  twoProcsSrvCallWithoutInputReplierInc_aux
  twoProcsSrvCallWithoutOutputReplier_aux
  twoProcsSrvCallWithoutOutputReplierInc_aux
)

# Build the auxiliary files.
foreach(AUX_EXECUTABLE ${auxiliary_files})
  gz_add_executable(${AUX_EXECUTABLE} test_executables/${AUX_EXECUTABLE}.cc)

  # Link the libraries that we always need.
  target_link_libraries(${AUX_EXECUTABLE}
    PRIVATE
      ${PROJECT_LIBRARY_TARGET_NAME}
      gtest
      ${EXTRA_TEST_LIB_DEPS}
      test_config
  )
endforeach(AUX_EXECUTABLE)
