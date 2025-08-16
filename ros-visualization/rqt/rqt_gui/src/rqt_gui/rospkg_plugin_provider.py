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

import os

from ament_index_python.packages import get_packages_with_prefixes
from catkin_pkg.package import InvalidPackage, PACKAGE_MANIFEST_FILENAME, parse_package
from python_qt_binding.QtCore import qDebug, qWarning

from .ros_plugin_provider import RosPluginProvider


class RospkgPluginProvider(RosPluginProvider):
    """`RosPluginProvider` using rospkg."""

    def __init__(self, export_tag, base_class_type):
        super(RospkgPluginProvider, self).__init__(export_tag, base_class_type)
        self.setObjectName('RospkgPluginProvider')

    def _find_plugins(self, export_tag, discovery_data):
        crawl = True
        if discovery_data:
            data = discovery_data.get_settings('rqt_gui.RospkgPluginProvider')
            export_data = data.get_settings(export_tag)
            crawl = export_tag not in data.child_groups()

        plugins = []
        if crawl:
            qDebug("RospkgPluginProvider._find_plugins() crawling for plugins of type '%s'" %
                   export_tag)
            for package_name, package_path in get_packages_with_prefixes().items():
                package_share_path = os.path.join(package_path, 'share', package_name)
                package_file_path = os.path.join(
                    package_share_path, PACKAGE_MANIFEST_FILENAME)
                if os.path.isfile(package_file_path):
                    # only try to import catkin if a PACKAGE_FILE is found
                    try:
                        package = parse_package(package_file_path)
                    except InvalidPackage as e:
                        qWarning('Could not parse package file "%s":\n%s' % (package_file_path, e))
                        continue
                    for export in package.exports:
                        if export.tagname != export_tag or 'plugin' not in export.attributes:
                            continue
                        plugin_xml_path = export.attributes['plugin']
                        plugin_xml_path = plugin_xml_path.replace('${prefix}', package_share_path)
                        plugins.append([package_name, plugin_xml_path])
                    continue
            # write crawling information to cache
            if discovery_data:
                plugins_by_package = {}
                for (package_name, export) in plugins:
                    if package_name not in plugins_by_package:
                        plugins_by_package[package_name] = []
                    plugins_by_package[package_name].append(export)
                for package_name, exports in plugins_by_package.items():
                    export_data.set_value(package_name, os.pathsep.join([str(e) for e in exports]))

        else:
            # use cached information
            for package_name in export_data.all_keys():
                exports = export_data.value(package_name)
                if exports:
                    for export in exports.split(os.pathsep):
                        plugins.append([package_name, export])

        return plugins
