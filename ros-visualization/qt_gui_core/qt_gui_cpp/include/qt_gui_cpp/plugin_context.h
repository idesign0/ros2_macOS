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

#ifndef qt_gui_cpp__PluginContext_H
#define qt_gui_cpp__PluginContext_H

#include "generic_proxy.h"

#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
// Upstream issue: https://codereview.qt-project.org/c/qt/qtbase/+/272258
#if __GNUC__ >= 9
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wdeprecated-copy"
#endif
#include <QVariant>
#if __GNUC__ >= 9
# pragma GCC diagnostic pop
#endif
#include <QWidget>

namespace qt_gui_cpp
{

/**
 * PluginContext providing information to the plugin and exposing methods for the plugin to interact with the framework.
 * It relays all methods to the corresponding plugin handler.
 */
class PluginContext
  : public QObject
{

  Q_OBJECT

public:

  PluginContext(QObject* obj, int serial_number, const QStringList& argv);

  PluginContext(const PluginContext& other);

  /**
   * Return the serial number of the plugin.
   * For a specific type of plugin each instance gets a serial number (which is the first currently not used positive integer at construction time).
   * @return The serial number
   */
  int serialNumber() const;

  /**
   * Return the command line arguments of the plugin.
   * @return The arguments without a program name at the beginning
   */
  const QStringList& argv() const;

  /**
   * Add a widget to the UI.
   * The widget is embedded into a new QDockWidget which itself is added to the QMainWindow.
   * This method can be called once for each widget a plugin would like to add and at any point in time (until the calling plugin has been shutdown).
   * @note The ownership of the widget pointer is transferred to the callee which will delete it when the plugin is shut down.
   * @param widget The widget to add
   */
  void addWidget(QWidget* widget);

  /**
   * Remove a previously added widget from the UI.
   * @note The ownership of the widget pointer is transferred back to the caller which is responsible of deleting it.
   * @param widget The widget to remove
   */
  void removeWidget(QWidget* widget);

  /**
   * Close the plugin.
   * The framework will call `Plugin.shutdown_plugin()` and unload it afterwards.
   */
  void closePlugin();

  /**
   * Reload the plugin.
   */
  void reloadPlugin();

protected:

  GenericProxy proxy_;

  int serial_number_;

  QStringList argv_;

};

} // namespace

#endif // qt_gui_cpp__PluginContext_H
