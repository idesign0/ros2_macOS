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

#include <qt_gui_cpp/plugin_provider.h>

namespace qt_gui_cpp {

PluginProvider::PluginProvider()
{}

PluginProvider::~PluginProvider()
{}

QMap<QString, QString> PluginProvider::discover(QObject* discovery_data)
{
  QMultiMap<QString, QString> plugins;
  QList<PluginDescriptor*> descriptors = discover_descriptors(discovery_data);
  for (QList<PluginDescriptor*>::iterator it = descriptors.begin(); it != descriptors.end(); it++)
  {
    // extract plugin descriptor dictionary
    PluginDescriptor* descriptor = *it;
    QMap<QString, QString> plugin = descriptor->toDictionary();
    plugins.unite(plugin);
    delete descriptor;
  }
  return plugins;
}

QList<PluginDescriptor*> PluginProvider::discover_descriptors(QObject* discovery_data)
{
  return QList<PluginDescriptor*>();
}

void* PluginProvider::load(const QString& plugin_id, PluginContext* plugin_context)
{
  return load_plugin(plugin_id, plugin_context);
}

Plugin* PluginProvider::load_plugin(const QString& plugin_id, PluginContext* plugin_context)
{
  return 0;
}

void PluginProvider::unload(void* plugin_instance)
{}

void PluginProvider::unload_plugin(Plugin* plugin_instance)
{
  unload(plugin_instance);
}

void PluginProvider::shutdown()
{}

} // namespace
