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

#include <qt_gui_cpp/recursive_plugin_provider.h>

#include <stdexcept>

namespace qt_gui_cpp {

RecursivePluginProvider::RecursivePluginProvider(RosPluginlibPluginProvider_ForPluginProviders* plugin_provider)
  : CompositePluginProvider()
  , plugin_provider_(plugin_provider)
{}

RecursivePluginProvider::~RecursivePluginProvider()
{
  delete plugin_provider_;
}

QMap<QString, QString> RecursivePluginProvider::discover(QObject* discovery_data)
{
  // discover plugins, which are providers themselves
  QList<PluginDescriptor*> descriptors = plugin_provider_->discover_descriptors(discovery_data);
  QList<QString> plugin_ids;
  for (QList<PluginDescriptor*>::iterator it = descriptors.begin(); it != descriptors.end(); it++)
  {
    PluginDescriptor* descriptor = *it;
    plugin_ids.append(descriptor->pluginId());
    delete descriptor;
  }

  // instantiate plugins
  QList<PluginProvider*> providers;
  for (QList<QString>::iterator it = plugin_ids.begin(); it != plugin_ids.end(); it++)
  {
    try
    {
      // pass NULL as PluginContext for PluginProviders
      PluginProvider* instance = plugin_provider_->load_explicit_type(*it, 0);
      if (instance == 0)
      {
        throw std::runtime_error("load returned None");
      }
      providers.append(instance);
    }
    catch (...)
    {
      qCritical("RecursivePluginProvider.discover() loading plugin '%s' failed", it->toStdString().c_str());
    }
  }

  // delegate discovery through instantiated plugin providers to base class
  set_plugin_providers(providers);
  return CompositePluginProvider::discover(discovery_data);
}

} // namespace
