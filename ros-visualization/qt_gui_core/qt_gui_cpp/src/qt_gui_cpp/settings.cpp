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

#include <qt_gui_cpp/settings.h>

#include <stdexcept>

namespace qt_gui_cpp {

Settings::Settings(QObject* obj)
  : proxy_(obj)
{}

Settings Settings::getSettings(const QString& prefix)
{
  Settings settings(proxy_.proxiedObject());
  bool rc = proxy_.invokeMethodWithReturn("get_settings", Q_RETURN_ARG(Settings, settings), Q_ARG(QString, prefix));
  if (!rc) throw std::runtime_error("Settings::get_settings() invoke method failed");
  return settings;
}

QStringList Settings::allKeys() const
{
  QStringList list;
  bool rc = const_cast<Settings*>(this)->proxy_.invokeMethodWithReturn("all_keys", Q_RETURN_ARG(QStringList, list));
  if (!rc) throw std::runtime_error("Settings::all_keys() invoke method failed");
  return list;
}

QStringList Settings::childGroups() const
{
  QStringList list;
  bool rc = const_cast<Settings*>(this)->proxy_.invokeMethodWithReturn("child_groups", Q_RETURN_ARG(QStringList, list));
  if (!rc) throw std::runtime_error("Settings::child_groups() invoke method failed");
  return list;
}

QStringList Settings::childKeys() const
{
  QStringList list;
  bool rc = const_cast<Settings*>(this)->proxy_.invokeMethodWithReturn("child_keys", Q_RETURN_ARG(QStringList, list));
  if (!rc) throw std::runtime_error("Settings::child_keys() invoke method failed");
  return list;
}

bool Settings::contains(const QString& key) const
{
  bool flag = false;
  bool rc = const_cast<Settings*>(this)->proxy_.invokeMethodWithReturn("contains", Q_RETURN_ARG(bool, flag), Q_ARG(QString, key));
  if (!rc) throw std::runtime_error("Settings::contains() invoke method failed");
  return flag;
}

void Settings::remove(const QString& key)
{
  bool rc = proxy_.invokeMethod("remove", Q_ARG(QString, key));
  if (!rc) throw std::runtime_error("Settings::remove() invoke method failed");
}

void Settings::setValue(const QString& key, const QVariant& value)
{
  bool rc = proxy_.invokeMethod("set_value", Q_ARG(QString, key), Q_ARG(QVariant, value));
  if (!rc) throw std::runtime_error("Settings::set_value() invoke method failed");
}

QVariant Settings::value(const QString& key, const QVariant& defaultValue) const
{
  QVariant val;
  bool rc = const_cast<Settings*>(this)->proxy_.invokeMethodWithReturn("value", Q_RETURN_ARG(QVariant, val), Q_ARG(QString, key), Q_ARG(QVariant, defaultValue));
  if (!rc) throw std::runtime_error("Settings::value() invoke method failed");
  return val;
}

} // namespace
