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

#include <qt_gui_cpp/composite_plugin_provider.h>

#include <stdexcept>

namespace qt_gui_cpp {

CompositePluginProvider::CompositePluginProvider(const QList<PluginProvider*>& plugin_providers)
  : PluginProvider()
  , plugin_providers_(plugin_providers)
{}

CompositePluginProvider::~CompositePluginProvider()
{
  for (QList<PluginProvider*>::iterator it = plugin_providers_.begin(); it != plugin_providers_.end(); it++)
  {
    delete *it;
  }
}

void CompositePluginProvider::set_plugin_providers(const QList<PluginProvider*>& plugin_providers)
{
  // garbage old plugin providers
  for (QList<PluginProvider*>::iterator it = plugin_providers_.begin(); it != plugin_providers_.end(); it++)
  {
    delete *it;
  }
  plugin_providers_ = plugin_providers;
}

QList<PluginDescriptor*> CompositePluginProvider::discover_descriptors(QObject* discovery_data)
{
  // discover plugins from all providers
  QList<PluginDescriptor*> descriptors;
  for (QList<PluginProvider*>::iterator it = plugin_providers_.begin(); it != plugin_providers_.end(); it++)
  {
    QList<PluginDescriptor*> sub_descriptors;
    try
    {
      sub_descriptors = (*it)->discover_descriptors(discovery_data);
    }
    catch (std::runtime_error e)
    {
      // TODO: add name of plugin provider to error message
      qCritical("CompositePluginProvider::discover() could not discover plugins from provider - runtime_error:\n%s", e.what());
      continue;
    }
    catch (...)
    {
      // TODO: add name of plugin provider to error message
      qCritical("CompositePluginProvider::discover() could not discover plugins from provider");
      continue;
    }

    QSet<QString> plugin_ids;
    for (QList<PluginDescriptor*>::iterator jt = sub_descriptors.begin(); jt != sub_descriptors.end(); jt++)
    {
      PluginDescriptor* descriptor = *jt;
      descriptors.append(descriptor);
      plugin_ids.insert(descriptor->pluginId());
    }
    discovered_plugins_[*it] = plugin_ids;
  }
  return descriptors;
}

void* CompositePluginProvider::load(const QString& plugin_id, PluginContext* plugin_context)
{
  // dispatch load to appropriate provider
  for (QMap<PluginProvider*, QSet<QString> >::iterator it = discovered_plugins_.begin(); it != discovered_plugins_.end(); it++)
  {
    if (it.value().contains(plugin_id))
    {
      PluginProvider* plugin_provider = it.key();
      try
      {
        void* instance = plugin_provider->load(plugin_id, plugin_context);
        running_plugins_[instance] = plugin_provider;
        return instance;
      }
      catch (std::exception& e)
      {
        qWarning("CompositePluginProvider::load(%s) failed loading plugin (%s)", plugin_id.toStdString().c_str(), e.what());
        return 0;
      }
    }
  }
  return 0;
}

Plugin* CompositePluginProvider::load_plugin(const QString& plugin_id, PluginContext* plugin_context)
{
  // dispatch load to appropriate provider
  for (QMap<PluginProvider*, QSet<QString> >::iterator it = discovered_plugins_.begin(); it != discovered_plugins_.end(); it++)
  {
    if (it.value().contains(plugin_id))
    {
      PluginProvider* plugin_provider = it.key();
      try
      {
        Plugin* instance = plugin_provider->load_plugin(plugin_id, plugin_context);
        running_plugins_[instance] = plugin_provider;
        return instance;
      }
      catch (std::exception& e)
      {
        // error message will be generated by python
      }
    }
  }
  return 0;
}

void CompositePluginProvider::unload(void* plugin_instance)
{
  // dispatch unload to appropriate provider
  QMap<void*, PluginProvider*>::iterator it = running_plugins_.find(plugin_instance);
  if (it != running_plugins_.end())
  {
    (*it)->unload(plugin_instance);
    running_plugins_.remove(it.key());
    return;
  }
  throw std::runtime_error("plugin_instance not found");
}

void CompositePluginProvider::shutdown()
{
  for (QList<PluginProvider*>::iterator it = plugin_providers_.begin(); it != plugin_providers_.end(); it++)
  {
    (*it)->shutdown();
  }
}

} // namespace
