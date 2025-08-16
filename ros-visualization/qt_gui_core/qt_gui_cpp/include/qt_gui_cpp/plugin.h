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

#ifndef qt_gui_cpp__Plugin_H
#define qt_gui_cpp__Plugin_H

#include "plugin_bridge.h"
#include "plugin_context.h"
#include "settings.h"

#include <QObject>

namespace qt_gui_cpp
{

/**
 * The base class for C++ plugins.
 */
class Plugin
  : public QObject
{

  Q_OBJECT

public:

  /**
   * Construct the plugin.
   * All initialization should be performed in initPlugin().
   */
  Plugin()
    : QObject()
  {}

  /**
   * Instantiate the plugin.
   * @param the plugin context
   */
  virtual void initPlugin(PluginContext& /*context*/)
  {}

  /**
   * Shutdown and clean up the plugin before unloading.
   */
  virtual void shutdownPlugin()
  {}

  /**
   * Save the intrinsic state of the plugin to the plugin-specific or instance-specific settings.
   * @param the plugin-specific settings
   * @param the instance-specific settings
   */
  virtual void saveSettings(Settings& /*plugin_settings*/, Settings& /*instance_settings*/) const
  {}

  /**
   * Restore the intrinsic state of the plugin from the plugin-specific or instance-specific settings.
   * @param the plugin-specific settings
   * @param the instance-specific settings
   */
  virtual void restoreSettings(const Settings& /*plugin_settings*/, const Settings& /*instance_settings*/)
  {}

  /**
   * Indicate if the plugin has configuration dialog which could be triggered by an icon in the title bar of the dock widgets.
   * @return true if the plugin implements triggerConfiguration()
   */
  virtual bool hasConfiguration() const
  {
    return false;
  }

  /**
   * Trigger a configuration dialog.
   * If this method is reimplemented hasConfiguration() should also be reimplemented and return true.
   */
  virtual void triggerConfiguration()
  {}

};

} // namespace

#endif // qt_gui_cpp__Plugin_H
