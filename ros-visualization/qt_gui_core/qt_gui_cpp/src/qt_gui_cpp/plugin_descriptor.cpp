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

#include <qt_gui_cpp/plugin_descriptor.h>

namespace qt_gui_cpp {

PluginDescriptor::PluginDescriptor(const QString& plugin_id, const QMap<QString, QString>& attributes)
{
  plugin_id_ = plugin_id;
  attributes_ = attributes;
}

const QString& PluginDescriptor::pluginId() const
{
  return plugin_id_;
}

const QMap<QString, QString>& PluginDescriptor::attributes() const
{
  return attributes_;
}

QMap<QString, QString>& PluginDescriptor::attributes()
{
  return attributes_;
}

const QMap<QString, QString>& PluginDescriptor::actionAttributes() const
{
  return action_attributes_;
}

void PluginDescriptor::setActionAttributes(const QString& label, const QString& statustip, const QString& icon, const QString& icontype)
{
  action_attributes_["label"] = label;
  action_attributes_["statustip"] = statustip;
  action_attributes_["icon"] = icon;
  action_attributes_["icontype"] = icontype;
}

int PluginDescriptor::countGroups() const
{
  return groups_.size();
}

QMap<QString, QString> PluginDescriptor::group(int index) const
{
  return groups_[index];
}

void PluginDescriptor::addGroupAttributes(const QString& label, const QString& statustip, const QString& icon, const QString& icontype)
{
  QMap<QString, QString> attributes;
  attributes["label"] = label;
  attributes["statustip"] = statustip;
  attributes["icon"] = icon;
  attributes["icontype"] = icontype;
  groups_.append(attributes);
}

QMap<QString, QString> PluginDescriptor::toDictionary() const
{
  QMap<QString, QString> dict;
  QString plugin_prefix = plugin_id_ + ".";
  dict[plugin_prefix + "plugin_id"] = plugin_id_;
  for (QMap<QString, QString>::const_iterator it = attributes_.constBegin(); it != attributes_.constEnd(); it++)
  {
    dict[plugin_prefix + QString("attributes.") + it.key()] = it.value();
  }
  for (QMap<QString, QString>::const_iterator it = action_attributes_.constBegin(); it != action_attributes_.constEnd(); it++)
  {
    dict[plugin_prefix + QString("action.") + it.key()] = it.value();
  }
  int group_index = 1;
  for (QVector<QMap<QString, QString> >::const_iterator it = groups_.constBegin(); it != groups_.constEnd(); it++)
  {
    QString prefix = QString("groups.") + QString::number(group_index) + QString(".");
    for (QMap<QString, QString>::const_iterator jt = it->constBegin(); jt != it->constEnd(); jt++)
    {
      dict[plugin_prefix + prefix + jt.key()] = jt.value();
    }
    group_index++;
  }
  return dict;
}

} // namespace
