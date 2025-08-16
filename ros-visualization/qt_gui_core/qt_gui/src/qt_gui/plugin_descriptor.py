# Copyright (c) 2011, Dirk Thomas, Dorian Scholz, TU Darmstadt
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above
#     copyright notice, this list of conditions and the following
#     disclaimer in the documentation and/or other materials provided
#     with the distribution.
#   * Neither the name of the TU Darmstadt nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.


class PluginDescriptor(object):
    """Information about one Plugin."""

    def __init__(self, plugin_id, attributes=None):
        self._dict = {}
        self._dict['plugin_id'] = plugin_id
        self._dict['attributes'] = attributes or {}

    def plugin_id(self):
        return self._dict['plugin_id']

    def attributes(self):
        return self._dict['attributes']

    def action_attributes(self):
        return self._dict.get('action', {})

    def set_action_attributes(self, label, statustip=None, icon=None, icontype=None):
        self._dict['action'] = {
            'label': label,
            'statustip': statustip,
            'icon': icon,
            'icontype': icontype,
        }

    def groups(self):
        return self._dict.get('groups', [])

    def add_group_attributes(self, label, statustip=None, icon=None, icontype=None):
        if 'groups' not in self._dict:
            self._dict['groups'] = []
        self._dict['groups'].append({
            'label': label,
            'statustip': statustip,
            'icon': icon,
            'icontype': icontype,
        })
