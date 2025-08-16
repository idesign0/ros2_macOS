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

#ifndef qt_gui_cpp__CompositePluginProvider_H
#define qt_gui_cpp__CompositePluginProvider_H

#include "plugin_descriptor.h"
#include "plugin_provider.h"

#include <QList>
#include <QMap>
#include <QSet>
#include <QString>

namespace qt_gui_cpp
{

class CompositePluginProvider
  : public PluginProvider
{

public:

  CompositePluginProvider(const QList<PluginProvider*>& plugin_providers = QList<PluginProvider*>());

  virtual ~CompositePluginProvider();

  /**
   * @note The ownership of the plugin providers is transferred to the callee.
   */
  virtual void set_plugin_providers(const QList<PluginProvider*>& plugin_providers);

  virtual QList<PluginDescriptor*> discover_descriptors(QObject* discovery_data);

  virtual void* load(const QString& plugin_id, PluginContext* plugin_context);

  virtual Plugin* load_plugin(const QString& plugin_id, PluginContext* plugin_context);

  virtual void unload(void* plugin_instance);

  virtual void shutdown();

private:

  QList<PluginProvider*> plugin_providers_;

  QMap<PluginProvider*, QSet<QString> > discovered_plugins_;

  QMap<void*, PluginProvider*> running_plugins_;

};

} // namespace

#endif // qt_gui_cpp__CompositePluginProvider_H
