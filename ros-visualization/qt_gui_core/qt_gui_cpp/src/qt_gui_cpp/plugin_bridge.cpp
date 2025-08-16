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

#include <qt_gui_cpp/plugin_bridge.h>

#include <qt_gui_cpp/plugin.h>
#include <qt_gui_cpp/plugin_context.h>
#include <qt_gui_cpp/plugin_provider.h>

#include <QEvent>

namespace qt_gui_cpp {

PluginBridge::PluginBridge()
  : QObject()
  , provider_(0)
  , plugin_(0)
{
  setObjectName("PluginBridge");
}

bool PluginBridge::load_plugin(PluginProvider* provider, const QString& plugin_id, PluginContext* plugin_context)
{
  qDebug("PluginBridge::load_plugin() %s", plugin_id.toStdString().c_str());
  provider_ = provider;
  plugin_ = provider_->load_plugin(plugin_id, plugin_context);
  if (plugin_)
  {
    plugin_->installEventFilter(this);
  }
  return plugin_ != 0;
}

void PluginBridge::unload_plugin()
{
  qDebug("PluginBridge::unload_plugin()");
  provider_->unload_plugin(plugin_);
  plugin_ = 0;
}

bool PluginBridge::has_configuration() const
{
  if (plugin_)
  {
    return plugin_->hasConfiguration();
  }
  return false;
}

void PluginBridge::trigger_configuration()
{
  if (plugin_)
  {
    plugin_->triggerConfiguration();
  }
}

void PluginBridge::shutdown_plugin()
{
  if (plugin_)
  {
    plugin_->removeEventFilter(this);
    plugin_->shutdownPlugin();
  }
}

void PluginBridge::save_settings(QObject* plugin_settings, QObject* instance_settings)
{
  if (plugin_)
  {
    Settings plugin(plugin_settings);
    Settings instance(instance_settings);
    plugin_->saveSettings(plugin, instance);
  }
}

void PluginBridge::restore_settings(QObject* plugin_settings, QObject* instance_settings)
{
  if (plugin_)
  {
    Settings plugin(plugin_settings);
    Settings instance(instance_settings);
    plugin_->restoreSettings(plugin, instance);
  }
}

} // namespace
