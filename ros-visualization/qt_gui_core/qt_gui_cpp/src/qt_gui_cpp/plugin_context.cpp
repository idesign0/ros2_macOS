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

#include <qt_gui_cpp/plugin_context.h>

#include <stdexcept>

namespace qt_gui_cpp {

PluginContext::PluginContext(QObject* obj, int serial_number, const QStringList& argv)
  : QObject(obj)
  , proxy_(obj)
  , serial_number_(serial_number)
  , argv_(argv)
{}

PluginContext::PluginContext(const PluginContext& other)
  : QObject(other.parent())
  , proxy_(other.parent())
  , serial_number_(other.serial_number_)
  , argv_(other.argv_)
{}

int PluginContext::serialNumber() const
{
  return serial_number_;
}

const QStringList& PluginContext::argv() const
{
  return argv_;
}

void PluginContext::addWidget(QWidget* widget)
{
  bool rc = proxy_.invokeMethod("add_widget", Q_ARG(QWidget*, widget));
  if (!rc) throw std::runtime_error("PluginContext::addWidget() invoke method failed");
}

void PluginContext::removeWidget(QWidget* widget)
{
  bool rc = proxy_.invokeMethod("remove_widget", Q_ARG(QWidget*, widget));
  if (!rc) throw std::runtime_error("PluginContext::removeWidget() invoke method failed");
}

void PluginContext::closePlugin()
{
  bool rc = proxy_.invokeMethod("close_plugin");
  if (!rc) throw std::runtime_error("PluginContext::closePlugin() invoke method failed");
}

void PluginContext::reloadPlugin()
{
  bool rc = proxy_.invokeMethod("reload_plugin");
  if (!rc) throw std::runtime_error("PluginContext::reloadPlugin() invoke method failed");
}

} // namespace
