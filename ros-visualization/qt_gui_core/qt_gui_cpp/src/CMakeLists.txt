add_subdirectory(qt_gui_cpp)

set(qt_gui_cpp_BINDINGS "")

add_subdirectory(qt_gui_cpp_shiboken)
add_subdirectory(qt_gui_cpp_sip)

message(STATUS "Python binding generators: ${qt_gui_cpp_BINDINGS}")
if(NOT qt_gui_cpp_BINDINGS)
  message(FATAL_ERROR "No Python binding generator found.")
endif()
