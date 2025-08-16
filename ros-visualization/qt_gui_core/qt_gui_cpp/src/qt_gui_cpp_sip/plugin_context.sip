namespace qt_gui_cpp
{

class PluginContext
{
%TypeHeaderCode
#include <qt_gui_cpp/plugin_context.h>
%End

public:

  PluginContext(QObject* obj, int serial_number, const QStringList& argv);

  PluginContext(const qt_gui_cpp::PluginContext& other);

  int serialNumber() const;

  const QStringList& argv() const;

  void addWidget(QWidget* widget);

  void removeWidget(QWidget* widget);

  void closePlugin();

  void reloadPlugin();

private:

  PluginContext& operator=(const PluginContext&);

};

};
