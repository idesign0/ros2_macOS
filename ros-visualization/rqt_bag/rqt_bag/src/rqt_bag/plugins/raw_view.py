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
"""
Defines a raw view: a TopicMessageView that displays the message contents in a tree.
"""
import codecs
import math

from rclpy.time import Time

from python_qt_binding.QtCore import Qt
from python_qt_binding.QtWidgets import \
    QApplication, QAbstractItemView, QSizePolicy, QTreeWidget, QTreeWidgetItem, QWidget
from .topic_message_view import TopicMessageView

# compatibility fix for python2/3
try:
    long
except NameError:
    long = int

class RawView(TopicMessageView):
    name = 'Raw'
    """
    Plugin to view a message in a treeview window
    The message is loaded into a custum treewidget
    """

    def __init__(self, timeline, parent, topic):
        """
        :param timeline: timeline data object, ''BagTimeline''
        :param parent: widget that will be added to the ros_gui context, ''QWidget''
        """
        super(RawView, self).__init__(timeline, parent, topic)
        self.message_tree = MessageTree(parent)

        # This will automatically resize the message_tree to the windowsize
        parent.layout().addWidget(self.message_tree)

    def message_viewed(self, *, entry, ros_message, msg_type_name, **kwargs):
        super(RawView, self).message_viewed(entry=entry, ros_message=ros_message, msg_type_name=msg_type_name)
        if ros_message is None:
            self.message_cleared()
        else:
            self.message_tree.set_message(ros_message, msg_type_name)

    def message_cleared(self):
        TopicMessageView.message_cleared(self)
        self.message_tree.set_message(None, None)


class MessageTree(QTreeWidget):

    def __init__(self, parent):
        super(MessageTree, self).__init__(parent)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.setHeaderHidden(False)
        self.setHeaderLabel('')
        self.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self._msg = None

        self._expanded_paths = None
        self.keyPressEvent = self.on_key_press

    @property
    def msg(self):
        return self._msg

    def set_message(self, msg, msg_type_name):
        """
        Clears the tree view and displays the new message
        :param msg: message object to display in the treeview, ''msg''
        """
        # Remember whether items were expanded or not before deleting
        if self._msg:
            for item in self.get_all_items():
                path = self.get_item_path(item)
                if item.isExpanded():
                    self._expanded_paths.add(path)
                elif path in self._expanded_paths:
                    self._expanded_paths.remove(path)
            self.clear()
        if msg:
            self.setHeaderLabel(msg_type_name)

            # Populate the tree
            # TODO(brawner) msg is stored in the db as a serialized byte array, needs to be
            # unserialized
            self._add_msg_object(None, '', '', msg, msg_type_name)

            if self._expanded_paths is None:
                self._expanded_paths = set()
            else:
                # Expand those that were previously expanded, and collapse any paths that
                # we've seen for the first time
                for item in self.get_all_items():
                    path = self.get_item_path(item)
                    if path in self._expanded_paths:
                        item.setExpanded(True)
                    else:
                        item.setExpanded(False)
        self._msg = msg
        QWidget.update(self)

    # Keyboard handler
    def on_key_press(self, event):
        if event.modifiers() & Qt.ControlModifier:
            key = event.key()
            if key == ord('C') or key == ord('c'):
                # Ctrl-C: copy text from selected items to clipboard
                self._copy_text_to_clipboard()
                event.accept()
            elif key == ord('A') or key == ord('a'):
                # Ctrl-A: expand the tree and select all items
                self.expandAll()
                self.selectAll()

    def _copy_text_to_clipboard(self):
        # Get tab indented text for all selected items
        def get_distance(item, ancestor, distance=0):
            parent = item.parent()
            if parent is None:
                return distance
            else:
                return get_distance(parent, ancestor, distance + 1)
        text = ''
        for i in self.get_all_items():
            if i in self.selectedItems():
                text += ('\t' * (get_distance(i, None))) + i.text(0) + '\n'
        # Copy the text to the clipboard
        clipboard = QApplication.clipboard()
        clipboard.setText(text)

    def get_item_path(self, item):
        # remove spaces that may get introduced in indexing, e.g. [  3] is [3]
        return item.data(0, Qt.UserRole)[0].replace(' ', '')

    def get_all_items(self):
        items = []
        try:
            root = self.invisibleRootItem()
            self.traverse(root, items.append)
        except Exception:
            # TODO: very large messages can cause a stack overflow due to recursion
            pass
        return items

    def traverse(self, root, function):
        for i in range(root.childCount()):
            child = root.child(i)
            function(child)
            self.traverse(child, function)

    def _add_msg_object(self, parent, path, name, obj, obj_type):
        label = name

        if hasattr(obj, '__slots__'):
            subobjs = [(slot, getattr(obj, slot)) for slot in obj.__slots__]
        elif type(obj) in [list, tuple]:
            len_obj = len(obj)
            if len_obj == 0:
                subobjs = []
            else:
                w = int(math.ceil(math.log10(len_obj)))
                subobjs = [('[%*d]' % (w, i), subobj) for (i, subobj) in enumerate(obj)]
        else:
            subobjs = []

        if type(obj) in [int, long, float]:
            if type(obj) == float:
                obj_repr = '%.6f' % obj
            else:
                obj_repr = str(obj)

            if obj_repr[0] == '-':
                label += ': %s' % obj_repr
            else:
                label += ':  %s' % obj_repr

        elif type(obj) in [str, bool, int, long, float, complex, Time]:
            # Ignore any binary data
            obj_repr = codecs.utf_8_decode(str(obj).encode(), 'ignore')[0]

            # Truncate long representations
            if len(obj_repr) >= 50:
                obj_repr = obj_repr[:50] + '...'

            label += ': ' + obj_repr
        item = QTreeWidgetItem([label])
        if name == '':
            pass
        elif path.find('.') == -1 and path.find('[') == -1:
            self.addTopLevelItem(item)
        else:
            parent.addChild(item)
        item.setData(0, Qt.UserRole, (path, obj_type))

        for subobj_name, subobj in subobjs:
            if subobj is None:
                continue

            # Strip the leading underscore for display. In ROS2, the Python IDL generator
            # adds a leading underscore for each name in __slots__, whereas the name of
            # the field does not have a leading underscore.
            if hasattr(obj, '__slots__') and subobj_name[0] == '_':
                subobj_name = subobj_name[1:]

            if path == '':
                subpath = subobj_name  # root field
            elif subobj_name.startswith('['):
                subpath = '%s%s' % (path, subobj_name)  # list, dict, or tuple
            else:
                subpath = '%s.%s' % (path, subobj_name)  # attribute (prefix with '.')

            if hasattr(subobj, '_type'):
                subobj_type = subobj._type
            else:
                subobj_type = type(subobj).__name__

            self._add_msg_object(item, subpath, subobj_name, subobj, subobj_type)
