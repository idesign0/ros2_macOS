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


class PluginInstanceId():
    """Identifier of a plugin instance."""

    def __init__(self, plugin_id=None, serial_number=None, instance_id=None):
        if instance_id is not None:
            # convert from unicode
            instance_id = str(instance_id)
            parts = instance_id.rsplit('#', 1)
            self.plugin_id = parts[0]
            self.serial_number = int(parts[1])
        else:
            # convert from unicode
            self.plugin_id = str(plugin_id)
            self.serial_number = int(serial_number) if serial_number is not None else None

    def __eq__(self, other):
        return self.plugin_id == other.plugin_id and self.serial_number == other.serial_number

    def __hash__(self):
        return hash(str(self))

    def __str__(self):
        return self.plugin_id + '#' + str(self.serial_number)

    def tidy_str(self):
        return self.plugin_id.replace('/', '__') + '__' + str(self.serial_number)

    def tidy_plugin_str(self):
        return self.plugin_id.replace('/', '__')
