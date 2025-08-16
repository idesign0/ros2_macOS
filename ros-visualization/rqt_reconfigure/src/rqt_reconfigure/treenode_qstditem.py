# Copyright (c) 2012, Willow Garage, Inc.
# All rights reserved.
#
# Software License Agreement (BSD License 2.0)
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
#
# Author: Isaac Saito

from __future__ import division

from python_qt_binding.QtCore import Qt
from python_qt_binding.QtGui import QBrush, QStandardItem

from rqt_py_common.data_items import ReadonlyItem

from rqt_reconfigure import logging
from rqt_reconfigure.param_client_widget import ParamClientWidget


class TreenodeQstdItem(ReadonlyItem):
    """
    Extending ReadonlyItem.

    the display content of this item shouldn't be modified.
    """

    NODE_FULLPATH = 1

    def __init__(self, context, *args):
        """

        Tree node initialization.

        :param args[0]: str (will become 1st arg of QStandardItem)
        :param args[1]: integer value that indicates whether this class
                               is node that has GRN (Graph Resource Names, see
                               http://www.ros.org/wiki/Names). This can be None
        """
        grn_current_treenode = args[0]
        self._raw_param_name = grn_current_treenode
        self._list_treenode_names = self._raw_param_name.split('/')[1:]
        self._toplevel_treenode_name = self._list_treenode_names[0]
        super(TreenodeQstdItem, self).__init__(grn_current_treenode)

        self._context = context
        self._param_client = None
        # ParamClientWidget
        self._param_client_widget = None

    def reset(self):
        self._param_client_widget = None
        if self._param_client is not None:
            self._param_client.close()
            del self._param_client
            self._param_client = None

    def get_param_client_widget(self):
        """
        Get the param_client_widget.

        @rtype: ParamClientWidget (QWidget)
        @return: None if param_client is not yet generated.
        @raise ROSException:
        """
        if not self._param_client_widget:
            logging.debug('In get_param_client_widget 4')
            self._param_client_widget = ParamClientWidget(
                self._context, self._raw_param_name
            )
            """
            Creating the ParamClientWidget transfers ownership of the
            _param_client to it. If it is destroyed from Qt, we need to
            clear our reference to it and stop the param server thread we
            had.
            """

            self._param_client_widget.destroyed.connect(self.reset)
            logging.debug('In get_param_client_widget 5')
        return self._param_client_widget

    def enable_param_items(self):
        """
        Create QStdItem per parameter and addColumn them to myself.

        :rtype: None if _param_client is not initiated.
        """
        if not self._param_client_widget:
            return None
        param_names = self._param_client_widget.get_treenode_names()
        param_names_items = []
        brush = QBrush(Qt.lightGray)
        for param_name in param_names:
            item = ReadonlyItem(param_name)
            item.setBackground(brush)
            param_names_items.append(item)
        logging.debug('enable_param_items len of param_names={}'.format(
            len(param_names_items)
        ))
        self.appendColumn(param_names_items)

    def get_raw_param_name(self):
        return self._raw_param_name

    def get_treenode_names(self):
        """
        Get tree node names.

        :rtype: List of string. Null if param
        """
        return self._list_treenode_names

    def get_node_name(self):
        """
        Get the node name.

        :return: A value of single tree node (ie. NOT the fullpath node name).
                 Ex. suppose fullpath name is /top/sub/subsub/subsubsub and you
                     are at 2nd from top, the return value is subsub.
        """
        return self._toplevel_treenode_name

    def type(self):  # noqa: A003
        return QStandardItem.UserType
