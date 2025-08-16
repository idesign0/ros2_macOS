namespace qt_gui_cpp
{

class RosPluginlibPluginProvider_ForPluginProviders
  : QObject
  , qt_gui_cpp::PluginProvider
{
%TypeHeaderCode
#include <qt_gui_cpp/ros_pluginlib_plugin_provider_for_plugin_providers.h>
%End

public:

  static qt_gui_cpp::RosPluginlibPluginProvider_ForPluginProviders* create_instance(const QString& export_tag, const QString& base_class_type);

  RosPluginlibPluginProvider_ForPluginProviders(const QString& export_tag, const QString& base_class_type);

  virtual ~RosPluginlibPluginProvider_ForPluginProviders();

  virtual QMap<QString, QString> discover(QObject* discovery_data);

  virtual QList<qt_gui_cpp::PluginDescriptor*> discover_descriptors(QObject* discovery_data);

  virtual void* load(const QString& plugin_id, qt_gui_cpp::PluginContext* plugin_context);

  virtual qt_gui_cpp::Plugin* load_plugin(const QString& plugin_id, qt_gui_cpp::PluginContext* plugin_context);

  virtual qt_gui_cpp::PluginProvider* load_explicit_type(const QString& plugin_id, qt_gui_cpp::PluginContext* plugin_context);

  virtual void unload(void* instance);

  virtual void shutdown();

};

};
