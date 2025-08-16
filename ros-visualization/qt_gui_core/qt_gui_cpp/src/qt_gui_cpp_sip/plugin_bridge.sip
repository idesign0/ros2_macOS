namespace qt_gui_cpp
{

class PluginBridge
  : QObject
{
%TypeHeaderCode
#include <qt_gui_cpp/plugin_bridge.h>
%End

public:

  PluginBridge();

  virtual bool load_plugin(qt_gui_cpp::PluginProvider* provider, const QString& plugin_id, qt_gui_cpp::PluginContext* plugin_context);

  virtual void unload_plugin();

  virtual bool has_configuration() const;

  virtual void trigger_configuration();

public slots:

  virtual void shutdown_plugin();

  virtual void save_settings(QObject* plugin_settings, QObject* instance_settings);

  virtual void restore_settings(QObject* plugin_settings, QObject* instance_settings);

};

};
