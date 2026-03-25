add_library(GzBadPluginAlign         SHARED BadPluginAlign.cc)
add_library(GzBadPluginAPIVersionNew SHARED BadPluginAPIVersionNew.cc)
add_library(GzBadPluginAPIVersionOld SHARED BadPluginAPIVersionOld.cc)
add_library(GzBadPluginNoInfo        SHARED BadPluginNoInfo.cc)
add_library(GzBadPluginSize          SHARED BadPluginSize.cc)
add_library(GzFactoryPlugins         SHARED FactoryPlugins.cc)
add_library(GzTemplatedPlugins       SHARED TemplatedPlugins.cc)

add_library(GzDummyPlugins SHARED
  DummyPlugins.cc
  DummyPluginsOtherTranslationUnit.cc)

add_library(GzInstanceCounter SHARED InstanceCounter.cc)

add_library(GzDummyStaticPlugin       STATIC DummyStaticPlugin.cc)

# Create a variable for the name of the header which will contain the dummy plugin path.
# This variable gets put in the cache so that it is available at generation time.
foreach(plugin_target
    GzBadPluginAlign
    GzBadPluginAPIVersionNew
    GzBadPluginAPIVersionOld
    GzBadPluginNoInfo
    GzBadPluginSize
    GzDummyPlugins
    GzFactoryPlugins
    GzTemplatedPlugins
    GzInstanceCounter
    GzDummyStaticPlugin)

  target_link_libraries(${plugin_target} PRIVATE
    ${PROJECT_LIBRARY_TARGET_NAME}-register)
endforeach()

# Need to link to the loader component to link static registry implementation.
target_link_libraries(GzDummyStaticPlugin PRIVATE
    ${PROJECT_LIBRARY_TARGET_NAME}-loader)
