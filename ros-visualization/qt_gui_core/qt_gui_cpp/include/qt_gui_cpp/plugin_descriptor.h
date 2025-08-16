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

#ifndef qt_gui_cpp__PluginDescriptor_H
#define qt_gui_cpp__PluginDescriptor_H

#include <QMap>
#include <QString>
#include <QVector>

namespace qt_gui_cpp
{

class PluginDescriptor
{

public:

  PluginDescriptor(const QString& plugin_id, const QMap<QString, QString>& attributes = (QMap<QString, QString>()));

  const QString& pluginId() const;

  const QMap<QString, QString>& attributes() const;

  QMap<QString, QString>& attributes();

  const QMap<QString, QString>& actionAttributes() const;

  void setActionAttributes(const QString& label, const QString& statustip = QString(), const QString& icon = QString(), const QString& icontype = QString());

  int countGroups() const;

  QMap<QString, QString> group(int index) const;

  void addGroupAttributes(const QString& label, const QString& statustip = QString(), const QString& icon = QString(), const QString& icontype = QString());

  QMap<QString, QString> toDictionary() const;

protected:

  QString plugin_id_;

  QMap<QString, QString> attributes_;

  QMap<QString, QString> action_attributes_;

  QVector<QMap<QString, QString> > groups_;

};

} // namespace

#endif // qt_gui_cpp__PluginDescriptor_H
