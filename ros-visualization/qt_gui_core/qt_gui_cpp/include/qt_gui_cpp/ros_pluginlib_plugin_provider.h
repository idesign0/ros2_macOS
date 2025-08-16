/*
 * Copyright (c) 2011, Dirk Thomas, TU Darmstadt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the TU Darmstadt nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef qt_gui_cpp__RosPluginlibPluginProvider_H
#define qt_gui_cpp__RosPluginlibPluginProvider_H

// Pluginlib has an optional dependency on boost::shared_ptr, which is not required here
// On machines without boost, including pluginlib/class_loader.hpp requires defining this flag to
// disable that dependency. Mosty notably these are the machines configured on ci.ros2.org
#define PLUGINLIB__DISABLE_BOOST_FUNCTIONS

#include "plugin.h"
#include "plugin_context.h"
#include "plugin_descriptor.h"
#include "plugin_provider.h"

#include <pluginlib/class_loader.hpp>
#include <rcpputils/filesystem_helper.hpp>
#include <tinyxml2.h>

#include <QCoreApplication>
#include <QEvent>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

#include <fstream>
#include <string>
#include <vector>

namespace qt_gui_cpp
{

template<typename T>
class RosPluginlibPluginProvider
  : public QObject
  , public PluginProvider
{

public:

  static RosPluginlibPluginProvider<T>* create_instance(const QString& export_tag, const QString& base_class_type)
  {
    return new RosPluginlibPluginProvider<T>(export_tag, base_class_type);
  }

  RosPluginlibPluginProvider(const QString& export_tag, const QString& base_class_type)
    : QObject()
    , PluginProvider()
    , export_tag_(export_tag)
    , base_class_type_(base_class_type)
    , class_loader_(0)
  {
    unload_libraries_event_ = QEvent::registerEventType();
  }

  virtual ~RosPluginlibPluginProvider()
  {
    if (class_loader_)
    {
      delete class_loader_;
    }
  }

  virtual QMap<QString, QString> discover(QObject* discovery_data)
  {
    return PluginProvider::discover(discovery_data);
  }

  virtual QList<PluginDescriptor*> discover_descriptors(QObject* discovery_data)
  {
    if (class_loader_)
    {
      delete class_loader_;
    }

    Settings discovery_settings(discovery_data);
    QString key = "qt_gui_cpp.RosPluginlibPluginProvider/" + export_tag_ + " " + base_class_type_;
    bool is_cached = discovery_settings.contains(key);

    std::vector<std::string> plugin_xml_paths;
    // reuse plugin paths from cache if available
    if (is_cached)
    {
      QStringList paths = discovery_settings.value(key).toStringList();
      for (QStringList::const_iterator it = paths.begin(); it != paths.end(); it++)
      {
        plugin_xml_paths.push_back(it->toStdString());
      }
    }
    else
    {
      qDebug("RosPluginlibPluginProvider::discover_descriptors() crawling for plugins of type '%s' and base class '%s'", export_tag_.toStdString().c_str(), base_class_type_.toStdString().c_str());
    }
    class_loader_ = new pluginlib::ClassLoader<T>(export_tag_.toStdString(), base_class_type_.toStdString(), std::string("plugin"), plugin_xml_paths);

    if (!is_cached)
    {
      // save discovered paths
      std::vector<std::string> paths = class_loader_->getPluginXmlPaths();
      QStringList qpaths;
      for (std::vector<std::string>::const_iterator it = paths.begin(); it != paths.end(); it++)
      {
        qpaths.push_back(it->c_str());
      }
      discovery_settings.setValue(key, qpaths);
    }

    QList<PluginDescriptor*> descriptors;

    std::vector<std::string> classes = class_loader_->getDeclaredClasses();
    for (std::vector<std::string>::iterator it = classes.begin(); it != classes.end(); it++)
    {
      std::string lookup_name = *it;

      std::string name = class_loader_->getName(lookup_name);
      std::string plugin_xml = class_loader_->getPluginManifestPath(lookup_name);
      rcpputils::fs::path p(plugin_xml);
      std::string plugin_path = p.parent_path().string();

      QMap<QString, QString> attributes;
      attributes["class_name"] = name.c_str();
      attributes["class_type"] = class_loader_->getClassType(lookup_name).c_str();
      attributes["class_base_class_type"] = class_loader_->getBaseClassType().c_str();
      attributes["package_name"] = class_loader_->getClassPackage(lookup_name).c_str();
      attributes["plugin_path"] = plugin_path.c_str();

      // check if plugin is available
      //std::string library_path = class_loader_->getClassLibraryPath(lookup_name);
      //attributes["not_available"] = !std::ifstream(library_path.c_str()) ? QString("library ").append(lookup_name.c_str()).append(" not found (may be it must be built?)") : "";
      attributes["not_available"] = "";

      PluginDescriptor* plugin_descriptor = new PluginDescriptor(lookup_name.c_str(), attributes);
      QString label = name.c_str();
      QString statustip = class_loader_->getClassDescription(lookup_name).c_str();
      QString icon;
      QString icontype;
      parseManifest(lookup_name, plugin_path, label, statustip, icon, icontype, plugin_descriptor);
      plugin_descriptor->setActionAttributes(label, statustip, icon, icontype);

      // add plugin descriptor
      descriptors.append(plugin_descriptor);
    }
    return descriptors;
  }

  virtual void* load(const QString& plugin_id, PluginContext* plugin_context)
  {
    return load_explicit_type(plugin_id, plugin_context);
  }

  virtual Plugin* load_plugin(const QString& plugin_id, PluginContext* plugin_context)
  {
    T* instance = load_explicit_type(plugin_id, plugin_context);
    if (instance == 0)
    {
      return 0;
    }
    Plugin* plugin = dynamic_cast<Plugin*>(instance);
    if (plugin == 0)
    {
      // TODO: garbage instance
      qWarning("RosPluginlibPluginProvider::load_plugin() called on non-plugin plugin provider");
      return 0;
    }
    return plugin;
  }

  virtual T* load_explicit_type(const QString& plugin_id, PluginContext* plugin_context)
  {
    std::string lookup_name = plugin_id.toStdString();

    if (!class_loader_->isClassAvailable(lookup_name))
    {
      qWarning("RosPluginlibPluginProvider::load_explicit_type(%s) class not available", lookup_name.c_str());
      return 0;
    }

    std::shared_ptr<T> instance;
    try
    {
      instance = create_plugin(lookup_name, plugin_context);
    }
    catch (pluginlib::LibraryLoadException& e)
    {
      qWarning("RosPluginlibPluginProvider::load_explicit_type(%s) could not load library (%s)", lookup_name.c_str(), e.what());
      return 0;
    }
    catch (pluginlib::PluginlibException& e)
    {
      qWarning("RosPluginlibPluginProvider::load_explicit_type(%s) failed creating instance (%s)", lookup_name.c_str(), e.what());
      return 0;
    }

    if (!instance)
    {
      qWarning("RosPluginlibPluginProvider::load_explicit_type(%s) failed creating instance", lookup_name.c_str());
      return 0;
    }

    // pass context to plugin
    Plugin* plugin = dynamic_cast<Plugin*>(&*instance);
    if (plugin)
    {
      try
      {
        init_plugin(plugin_id, plugin_context, plugin);
      }
      catch (std::exception& e)
      {
        // TODO: garbage instance
        qWarning("RosPluginlibPluginProvider::load_explicit_type(%s) failed initializing plugin (%s)", lookup_name.c_str(), e.what());
        return 0;
      }
    }

    //qDebug("RosPluginlibPluginProvider::load_explicit_type(%s) succeeded", lookup_name.c_str());
    instances_[&*instance] = instance;

    return &*instance;
  }

  virtual void unload(void* instance)
  {
    if (!instances_.contains(instance))
    {
      qCritical("RosPluginlibPluginProvider::unload() instance not found");
      return;
    }

    std::shared_ptr<T> pointer = instances_.take(instance);
    libraries_to_unload_.append(pointer);

    QCoreApplication::postEvent(this, new QEvent(static_cast<QEvent::Type>(unload_libraries_event_)));
  }

  bool event(QEvent* e)
  {
    if (e->type() == unload_libraries_event_)
    {
      libraries_to_unload_.clear();
      return true;
    }
    return QObject::event(e);
  }

protected:

  virtual std::shared_ptr<T> create_plugin(const std::string& lookup_name, PluginContext* /*plugin_context*/ = 0)
  {
    return class_loader_->createSharedInstance(lookup_name);
  }

  virtual void init_plugin(const QString& /*plugin_id*/, PluginContext* plugin_context, Plugin* plugin)
  {
    plugin->initPlugin(*plugin_context);
  }

private:

  template<typename TVersion>
  struct TinyXMLAPIChoice
  {
    template<
      // the function signature must use a template argument to trigger SFINAE
      typename TDoc,
      // T needs to be an explicit argument for std::enable_if to have a type
      typename TType = TVersion,
      // only enable for TinyXML versions >= 6
      typename = typename std::enable_if<std::is_same<TType, std::true_type>::value>::type
    >
    static void warningWithErrorStr(const std::string & manifest_path, const TDoc & doc, std::true_type * = nullptr)
    {
      qWarning("RosPluginlibPluginProvider::parseManifest() could not load manifest \"%s\" (%s)", manifest_path.c_str(), doc.ErrorStr());
    }
    template<
      typename TDoc,
      typename TType = TVersion,
      typename = typename std::enable_if<std::is_same<TType, std::false_type>::value>::type
    >
    static void warningWithErrorStr(const std::string & manifest_path, const TDoc & doc, std::false_type * = nullptr)
    {
      qWarning("RosPluginlibPluginProvider::parseManifest() could not load manifest \"%s\" (%s, %s)", manifest_path.c_str(), doc.GetErrorStr1(), doc.GetErrorStr2());
    }
  };

  bool parseManifest(const std::string& lookup_name, const std::string& plugin_path, QString& label, QString& statustip, QString& icon, QString& icontype, PluginDescriptor* plugin_descriptor)
  {
    //qDebug("RosPluginlibPluginProvider::parseManifest()");

    std::string manifest_path = class_loader_->getPluginManifestPath(lookup_name);
    //qDebug("RosPluginlibPluginProvider::parseManifest() manifest_path \"%s\"", manifest_path.c_str());
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError result = doc.LoadFile(manifest_path.c_str());
    if (result != tinyxml2::XML_SUCCESS)
    {
      TinyXMLAPIChoice<std::integral_constant<bool, (TIXML2_MAJOR_VERSION >= 6)>>::warningWithErrorStr(manifest_path, doc);
      return false;
    }

    // search library-tag with specific path-attribute
    std::string class_type = class_loader_->getClassType(lookup_name);
    tinyxml2::XMLElement* library_element = doc.FirstChildElement("library");
    while (library_element)
    {
        // search class-tag with specific type- and base_class_type-attribute
        tinyxml2::XMLElement* class_element = library_element->FirstChildElement("class");
        while (class_element)
        {
          if (class_type.compare(class_element->Attribute("type")) == 0 && base_class_type_.compare(class_element->Attribute("base_class_type")) == 0)
          {
            tinyxml2::XMLElement* qtgui_element = class_element->FirstChildElement("qtgui");
            if (qtgui_element)
            {
              // extract meta information
              parseActionAttributes(qtgui_element, plugin_path, label, statustip, icon, icontype);

              // extract grouping information
              tinyxml2::XMLElement* group_element = qtgui_element->FirstChildElement("group");
              while (group_element)
              {
                QString group_label;
                QString group_statustip;
                QString group_icon;
                QString group_icontype;
                parseActionAttributes(group_element, plugin_path, group_label, group_statustip, group_icon, group_icontype);
                plugin_descriptor->addGroupAttributes(group_label, group_statustip, group_icon, group_icontype);

                group_element = group_element->NextSiblingElement("group");
              }
            }
            return true;
          }
          class_element = class_element->NextSiblingElement("class");
        }
        break;

      library_element = library_element->NextSiblingElement("library");
    }

    qWarning("RosPluginlibPluginProvider::parseManifest() could not handle manifest \"%s\"", manifest_path.c_str());
    return false;
  }

  void parseActionAttributes(tinyxml2::XMLElement* element, const std::string& plugin_path, QString& label, QString& statustip, QString& icon, QString& icontype)
  {
    tinyxml2::XMLElement* child_element;
    if ((child_element = element->FirstChildElement("label")) != 0)
    {
      label = child_element->GetText();
    }
    if ((child_element = element->FirstChildElement("icon")) != 0)
    {
      icontype = child_element->Attribute("type");
      if (icontype == "file")
      {
        // prepend base path
        icon = plugin_path.c_str();
        icon += "/";
        icon += child_element->GetText();
      }
      else
      {
        icon = child_element->GetText();
      }
    }
    if ((child_element = element->FirstChildElement("statustip")) != 0)
    {
      statustip = child_element->GetText();
    }
  }

  void unload_pending_libraries()
  {
  }

  QString export_tag_;

  QString base_class_type_;

  int unload_libraries_event_;

  pluginlib::ClassLoader<T>* class_loader_;

  QMap<void*, std::shared_ptr<T> > instances_;

  QList<std::shared_ptr<T> > libraries_to_unload_;

};

} // namespace

#endif // qt_gui_cpp__RosPluginlibPluginProvider_H
