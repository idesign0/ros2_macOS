# Software License Agreement (BSD License)
#
# Copyright (c) 2012, Willow Garage, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of Willow Garage, Inc. nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import os

from ament_index_python.resources import get_resource, get_resources

from python_qt_binding import loadUi
from python_qt_binding.QtCore import Qt
from python_qt_binding.QtGui import QIcon
from python_qt_binding.QtWidgets import (QAction, QMenu,
                                         QTreeView, QWidget)

from rclpy import logging

from rosidl_runtime_py.utilities import get_action, get_service, get_message
from rosidl_runtime_py import get_action_interfaces, get_service_interfaces, get_message_interfaces

from rqt_console.text_browse_dialog import TextBrowseDialog

from rqt_msg.messages_tree_view import MessagesTreeView

# from rqt_py_common import rosaction

from rqt_py_common import message_helpers
from rqt_py_common.rqt_roscomm_util import RqtRoscommUtil
from rqt_py_common.message_helpers import \
    get_action_text_from_class, get_message_text_from_class, get_service_text_from_class


class MessagesWidget(QWidget):
    """
    This class is intended to be able to handle msg, srv & action (actionlib).
    The name of the class is kept to use message, by following the habit of
    rosmsg (a script that can handle both msg & srv).
    """

    def __init__(self, mode=message_helpers.MSG_MODE,
                 pkg_name='rqt_msg',
                 ui_filename='messages.ui'):
        """
        :param ui_filename: This Qt-based .ui file must have elements that are
                            referred from this class. Otherwise unexpected
                            errors are likely to happen. Best way to avoid that
                            situation when you want to give your own .ui file
                            is to implement all Qt components in
                            rqt_msg/resource/message.ui file.
        """
        super(MessagesWidget, self).__init__()

        self._logger = logging.get_logger("MessagesWidget")

        _, package_path = get_resource('packages', pkg_name)
        ui_file = os.path.join(
            package_path, 'share', pkg_name, 'resource', ui_filename)

        loadUi(ui_file, self, {'MessagesTreeView': MessagesTreeView})
        self.setObjectName(ui_filename)
        self._mode = mode

        self._add_button.setIcon(QIcon.fromTheme('list-add'))
        self._add_button.clicked.connect(self._add_message)
        self._refresh_packages(mode)
        self._refresh_msgs(self._package_combo.itemText(0))
        self._package_combo.currentIndexChanged[str].connect(self._refresh_msgs)
        self._messages_tree.mousePressEvent = self._handle_mouse_press

        self._browsers = []

    def _refresh_packages(self, mode=message_helpers.MSG_MODE):
        packages = sorted(
            [pkg_tuple[0] for pkg_tuple in RqtRoscommUtil.iterate_packages(self._mode)])
        self._package_list = packages
        self._logger.debug('pkgs={}'.format(self._package_list))
        self._package_combo.clear()
        self._package_combo.addItems(self._package_list)
        self._package_combo.setCurrentIndex(0)

    def _refresh_msgs(self, package=None):
        if package is None or len(package) == 0:
            return
        self._msgs = []
        if self._mode == message_helpers.MSG_MODE:
            msg_list = [f'{package}/{name}' for name in get_message_interfaces([package])[package]]
        elif self._mode == message_helpers.SRV_MODE:
            msg_list = [f'{package}/{name}' for name in get_service_interfaces([package])[package]]
        elif self._mode == message_helpers.ACTION_MODE:
            msg_list = [f'{package}/{name}' for name in get_action_interfaces([package])[package]]
        self._logger.debug(
            '_refresh_msgs package={} msg_list={}'.format(package, msg_list))
        for msg in msg_list:
            if (self._mode == message_helpers.MSG_MODE):
                msg_class = get_message(msg)
            elif self._mode == message_helpers.SRV_MODE:
                msg_class = get_service(msg)
            elif self._mode == message_helpers.ACTION_MODE:
                msg_class = get_action(msg)

            self._logger.debug('_refresh_msgs msg_class={}'.format(msg_class))

            if msg_class is not None:
                self._msgs.append(msg)

        self._msgs = [x.split('/')[2] for x in self._msgs]

        self._msgs_combo.clear()
        self._msgs_combo.addItems(self._msgs)

    def _add_message(self):
        if self._msgs_combo.count() == 0:
            return
        msg = (self._package_combo.currentText() +
               '/' + self._msgs_combo.currentText())

        self._logger.debug('_add_message msg={}'.format(msg))

        if self._mode == message_helpers.MSG_MODE:
            msg_class = get_message(msg)()
            text_tree_root = 'Msg Root'
            self._messages_tree.model().add_message(msg_class,
                                                    self.tr(text_tree_root), msg, msg)
        elif self._mode == message_helpers.SRV_MODE:
            msg_class = get_service(msg)
            self._messages_tree.model().add_message(msg_class.Request(),
                                                    self.tr('Service Request'),
                                                    msg + '/Request',
                                                    msg + '/Request')
            self._messages_tree.model().add_message(msg_class.Response(),
                                                    self.tr('Service Response'),
                                                    msg + '/Response',
                                                    msg + '/Response')
        elif self._mode == message_helpers.ACTION_MODE:
            action_class = get_action(msg)
            text_tree_root = 'Action Root'
            self._messages_tree.model().add_message(action_class.Goal(),
                                                    self.tr('Action Goal'),
                                                    msg + '/Goal',
                                                    msg + '/Goal')
            self._messages_tree.model().add_message(action_class.Result(),
                                                    self.tr('Action Result'),
                                                    msg + '/Result',
                                                    msg + '/Result')
            self._messages_tree.model().add_message(action_class.Feedback(),
                                                    self.tr('Action Feedback'),
                                                    msg + '/Feedback',
                                                    msg + '/Feedback')

        self._messages_tree._recursive_set_editable(
            self._messages_tree.model().invisibleRootItem(), False)

    def _handle_mouse_press(self, event,
                            old_pressEvent=QTreeView.mousePressEvent):
        if (event.buttons() & Qt.RightButton and
                event.modifiers() == Qt.NoModifier):
            self._rightclick_menu(event)
            event.accept()
        return old_pressEvent(self._messages_tree, event)

    def _rightclick_menu(self, event):
        """
        :type event: QEvent
        """
        # QTreeview.selectedIndexes() returns 0 when no node is selected.
        # This can happen when after booting no left-click has been made yet
        # (ie. looks like right-click doesn't count). These lines are the
        # workaround for that problem.
        selected = self._messages_tree.selectedIndexes()
        if len(selected) == 0:
            return

        menu = QMenu()
        text_action = QAction(self.tr('View Text'), menu)
        menu.addAction(text_action)
        remove_action = QAction(self.tr('Remove message'), menu)
        menu.addAction(remove_action)

        action = menu.exec_(event.globalPos())

        if action == text_action:
            self._logger.debug('_rightclick_menu selected={}'.format(selected))
            selected_type = selected[1].data()

            # We strip any array information for loading the python classes
            selected_type_bare = selected_type
            if selected_type_bare.find('[') >= 0:
                selected_type_bare = selected_type_bare[:selected_type_bare.find('[')]
            # We use the number of '/' to determine of the selected type is a msg, action, srv,
            # or primitive type.
            # NOTE (mlautman - 2/4/19) this heuristic seems brittle and should be removed
            selected_type_bare_tokens_len = len(selected_type_bare.split('/'))

            # We only want the base class so we transform eg. pkg1/my_srv/Request -> pkg1/my_srv
            if selected_type_bare_tokens_len > 2:
                selected_type_bare = "/".join(selected_type_bare.split('/')[:2])

            browsetext = None

            # If the type does not have '/'s then we treat it as a primitive type
            if selected_type_bare_tokens_len == 1:
                browsetext = selected_type

            # if the type has a single '/' then we treat it as a msg type
            elif selected_type_bare_tokens_len == 2:
                msg_class = get_message(selected_type_bare)
                browsetext = get_message_text_from_class(msg_class)

            # If the type has two '/'s then we treat it as a srv or action type
            elif selected_type_bare_tokens_len == 3:
                if self._mode == message_helpers.SRV_MODE:
                        msg_class = get_service(selected_type_bare)
                        browsetext = get_service_text_from_class(msg_class)

                elif self._mode == message_helpers.ACTION_MODE:
                    msg_class = get_action(selected_type_bare)
                    browsetext = get_action_text_from_class(msg_class)

                else:
                    self._logger.warn("Unrecognized value for self._mode: {} "
                                      "for selected_type: {}".format(self._mode, selected_type))
            else:
                self._logger.warn("Invalid selected_type: {}".format(selected_type))

            if browsetext is not None:
                self._browsers.append(TextBrowseDialog(browsetext))
                self._browsers[-1].show()

        if action == remove_action:
            self._messages_tree.model().removeRow(selected[0].row())

    def cleanup_browsers_on_close(self):
        for browser in self._browsers:
            browser.close()
