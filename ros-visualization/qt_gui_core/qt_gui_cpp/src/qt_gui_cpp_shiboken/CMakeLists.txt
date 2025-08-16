find_package(Qt5Widgets REQUIRED)
set(qt_gui_cpp_shiboken_QT_COMPONENTS
  QtCore
  QtGui
  QtWidgets
)

set(qt_gui_cpp_shiboken_SRCS
  libqt_gui_cpp_shiboken/libqt_gui_cpp_shiboken_module_wrapper.cpp
  libqt_gui_cpp_shiboken/qt_gui_cpp_compositepluginprovider_wrapper.cpp
  libqt_gui_cpp_shiboken/qt_gui_cpp_genericproxy_wrapper.cpp
  libqt_gui_cpp_shiboken/qt_gui_cpp_plugin_wrapper.cpp
  libqt_gui_cpp_shiboken/qt_gui_cpp_pluginbridge_wrapper.cpp
  libqt_gui_cpp_shiboken/qt_gui_cpp_plugincontext_wrapper.cpp
  libqt_gui_cpp_shiboken/qt_gui_cpp_plugindescriptor_wrapper.cpp
  libqt_gui_cpp_shiboken/qt_gui_cpp_pluginprovider_wrapper.cpp
  libqt_gui_cpp_shiboken/qt_gui_cpp_recursivepluginprovider_wrapper.cpp
  libqt_gui_cpp_shiboken/qt_gui_cpp_rospluginlibpluginprovider_forpluginproviders_wrapper.cpp
  libqt_gui_cpp_shiboken/qt_gui_cpp_rospluginlibpluginprovider_forplugins_wrapper.cpp
  libqt_gui_cpp_shiboken/qt_gui_cpp_settings_wrapper.cpp
  libqt_gui_cpp_shiboken/qt_gui_cpp_wrapper.cpp
)

set(qt_gui_cpp_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
set(qt_gui_cpp_HDRS
  ${qt_gui_cpp_INCLUDE_PATH}/qt_gui_cpp/composite_plugin_provider.h
  ${qt_gui_cpp_INCLUDE_PATH}/qt_gui_cpp/generic_proxy.h
  ${qt_gui_cpp_INCLUDE_PATH}/qt_gui_cpp/plugin.h
  ${qt_gui_cpp_INCLUDE_PATH}/qt_gui_cpp/plugin_bridge.h
  ${qt_gui_cpp_INCLUDE_PATH}/qt_gui_cpp/plugin_context.h
  ${qt_gui_cpp_INCLUDE_PATH}/qt_gui_cpp/plugin_descriptor.h
  ${qt_gui_cpp_INCLUDE_PATH}/qt_gui_cpp/plugin_provider.h
  ${qt_gui_cpp_INCLUDE_PATH}/qt_gui_cpp/recursive_plugin_provider.h
  ${qt_gui_cpp_INCLUDE_PATH}/qt_gui_cpp/ros_pluginlib_plugin_provider.h
  ${qt_gui_cpp_INCLUDE_PATH}/qt_gui_cpp/ros_pluginlib_plugin_provider_for_plugin_providers.h
  ${qt_gui_cpp_INCLUDE_PATH}/qt_gui_cpp/ros_pluginlib_plugin_provider_for_plugins.h
  ${qt_gui_cpp_INCLUDE_PATH}/qt_gui_cpp/settings.h
)
ament_get_recursive_properties(deps_include_dirs _ ${TinyXML2_TARGETS} ${pluginlib_TARGETS})
list(APPEND qt_gui_cpp_INCLUDE_PATH ${deps_include_dirs})

find_package(python_qt_binding REQUIRED)
include(${python_qt_binding_DIR}/shiboken_helper.cmake)
ament_export_dependencies(pluginlib)
if(shiboken_helper_FOUND)
  if(Shiboken_VERSION VERSION_GREATER "1.1.1")
    # shiboken 1.1.2 and higher will segfault until https://bugreports.qt-project.org/browse/PYSIDE-218 is fixed
    message(WARNING "Shiboken version ${Shiboken_VERSION} would segfault when trying to process qt_gui_cpp (see https://bugreports.qt-project.org/browse/PYSIDE-218). Therefore shiboken bindings are being skipped.")
  else()
    list(APPEND qt_gui_cpp_BINDINGS "shiboken")
    set(qt_gui_cpp_BINDINGS "${qt_gui_cpp_BINDINGS}" PARENT_SCOPE)

    set(QT_INCLUDE_DIR "${Qt5Widgets_INCLUDE_DIRS}")
    shiboken_generator(libqt_gui_cpp global.h typesystem.xml ${PROJECT_SOURCE_DIR}/src/qt_gui_cpp_shiboken "${qt_gui_cpp_shiboken_SRCS}" "${qt_gui_cpp_HDRS}" "${qt_gui_cpp_INCLUDE_PATH}" "${CMAKE_CURRENT_BINARY_DIR}")

    shiboken_include_directories(qt_gui_cpp_shiboken "${qt_gui_cpp_shiboken_QT_COMPONENTS}")

    add_library(qt_gui_cpp_shiboken SHARED ${qt_gui_cpp_shiboken_SRCS})
    target_include_directories(qt_gui_cpp_shiboken PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
    target_link_libraries(qt_gui_cpp_shiboken ${PROJECT_NAME})
    ament_target_dependencies(qt_gui_cpp_shiboken pluginlib TinyXML2)
    shiboken_target_link_libraries(qt_gui_cpp_shiboken "${qt_gui_cpp_shiboken_QT_COMPONENTS}")

    install(TARGETS qt_gui_cpp_shiboken
      DESTINATION ${PYTHON_INSTALL_DIR}/${PROJECT_NAME})
  endif()
endif()
