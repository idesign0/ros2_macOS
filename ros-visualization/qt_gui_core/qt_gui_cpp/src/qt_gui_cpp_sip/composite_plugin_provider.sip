namespace qt_gui_cpp
{

class CompositePluginProvider
  : qt_gui_cpp::PluginProvider
{
%TypeHeaderCode
#include <qt_gui_cpp/composite_plugin_provider.h>
%End

public:

  CompositePluginProvider(const QList<qt_gui_cpp::PluginProvider*>& plugin_providers = QList<qt_gui_cpp::PluginProvider*>());

  virtual ~CompositePluginProvider();

  virtual void set_plugin_providers(const QList<qt_gui_cpp::PluginProvider*>& plugin_providers);

  virtual QList<qt_gui_cpp::PluginDescriptor*> discover_descriptors(QObject* discovery_data);

  virtual void* load(const QString& plugin_id, qt_gui_cpp::PluginContext* plugin_context);

  virtual qt_gui_cpp::Plugin* load_plugin(const QString& plugin_id, qt_gui_cpp::PluginContext* plugin_context);

  virtual void unload(void* plugin_instance);

  virtual void shutdown();

};

};
