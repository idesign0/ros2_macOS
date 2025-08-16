namespace qt_gui_cpp
{

class PluginDescriptor
{
%TypeHeaderCode
#include <qt_gui_cpp/plugin_descriptor.h>
%End

public:

  PluginDescriptor(const QString& plugin_id, const QMap<QString, QString>& attributes = QMap<QString, QString>());

};

};
