namespace qt_gui_cpp
{

class RecursivePluginProvider
  : qt_gui_cpp::CompositePluginProvider
{
%TypeHeaderCode
#include <qt_gui_cpp/recursive_plugin_provider.h>
%End

public:

  RecursivePluginProvider(qt_gui_cpp::RosPluginlibPluginProvider_ForPluginProviders* plugin_provider);

  virtual ~RecursivePluginProvider();

  virtual QMap<QString, QString> discover(QObject* discovery_data);

};

};
