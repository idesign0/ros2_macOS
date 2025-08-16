namespace qt_gui_cpp
{

class PluginProvider
{
%TypeHeaderCode
#include <qt_gui_cpp/plugin_provider.h>
%End

public:

  PluginProvider();

  virtual ~PluginProvider();

  virtual QMap<QString, QString> discover(QObject* discovery_data);

  /**
   * @attention Ownership of returned PluginDescriptor's is transfered to the caller
   */
  virtual QList<qt_gui_cpp::PluginDescriptor*> discover_descriptors(QObject* discovery_data);

  virtual void* load(const QString& plugin_id, qt_gui_cpp::PluginContext* plugin_context);

  virtual qt_gui_cpp::Plugin* load_plugin(const QString& plugin_id, qt_gui_cpp::PluginContext* plugin_context);

  virtual void unload(void* plugin_instance);

  virtual void unload_plugin(qt_gui_cpp::Plugin* plugin_instance);

  virtual void shutdown();

};

};
