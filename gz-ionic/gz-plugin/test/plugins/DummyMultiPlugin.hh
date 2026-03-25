#ifndef GZ_PLUGIN_TEST_PLUGINS_DUMMYMULTIPLUGIN_HH_
#define GZ_PLUGIN_TEST_PLUGINS_DUMMYMULTIPLUGIN_HH_

#include <gz/plugin/EnablePluginFromThis.hh>

#include "DummyPlugins.hh"

namespace test
{
namespace util
{
/// \brief A plugin that implements multiple interfaces
class DummyMultiPlugin
    : public DummyNameBase,
      public DummyDoubleBase,
      public DummyIntBase,
      public DummySetterBase,
      public DummyGetObjectBase,
      public DummyGetPluginInstancePtr,
      public gz::plugin::EnablePluginFromThis
{
  public: virtual std::string MyNameIs() const override;
  public: virtual double MyDoubleValueIs() const override;
  public: virtual int MyIntegerValueIs() const override;
  public: virtual DummyObject GetDummyObject() const override;

  public: virtual void SetName(const std::string &_name) override;
  public: virtual void SetDoubleValue(const double _val) override;
  public: virtual void SetIntegerValue(const int _val) override;

  public: virtual std::shared_ptr<void> PluginInstancePtr() const override;

  public: DummyMultiPlugin();

  private: std::string name;
  private: double val;
  private: int intVal;
};

}
}

#endif // DUMMYMULTIPLUGIN_HH
