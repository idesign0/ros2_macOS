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

from python_qt_binding.QtCore import QObject
from python_qt_binding.QtWidgets import QAction, QMenu


class MenuManager(QObject):
    """Menu manager containing alphabetically ordered items and other items before and after."""

    def __init__(self, menu):
        super(MenuManager, self).__init__()
        self.setObjectName('MenuManager')

        self.menu = menu
        self._prefixes_separator = None
        self._ordered_items = []
        self._suffixes_separator = None

        # get already existing items from menu
        for action in menu.actions():
            menu = action.menu()
            if menu is not None:
                self._ordered_items.append(menu)
            else:
                self._ordered_items.append(action)

    def add_prefix(self, item):
        if self._prefixes_separator is None:
            before = self._ordered_items[0] if self._ordered_items else None
            if isinstance(before, QMenu):
                before = before.menuAction()
            self._prefixes_separator = self.menu.insertSeparator(before)
        self._insert_item(self._prefixes_separator, item)

    def add_item(self, new_item):
        for i, item in enumerate(self._ordered_items):
            if self._get_item_label(item) > self._get_item_label(new_item):
                self._insert_item(item, new_item)
                self._ordered_items.insert(i, new_item)
                return

        before = self._suffixes_separator or None
        self._insert_item(before, new_item)
        self._ordered_items.append(new_item)

    def add_suffix(self, item):
        if self._suffixes_separator is None:
            self._suffixes_separator = self.menu.addSeparator()
        self._insert_item(None, item)

    def count_items(self):
        return len(self._ordered_items)

    def contains_item(self, name):
        return self.get_item(name) is not None

    def get_item(self, name):
        if isinstance(name, QAction) or isinstance(name, QMenu):
            if name in self._ordered_items:
                return name
            return None
        for item in self._ordered_items:
            if self._get_item_label(item) == name:
                return item
        return None

    def contains_menu(self, name):
        return self.get_menu(name) is not None

    def get_menu(self, name):
        for item in self._ordered_items:
            if self._get_item_label(item) == name and isinstance(item, QMenu):
                return item
        return None

    def get_items(self):
        names = []
        for item in self._ordered_items:
            names.append(self._get_item_label(item))
        return names

    def set_item_checked(self, name, flag):
        item = self.get_item(name)
        if item is not None:
            item.setChecked(flag)

    def set_item_disabled(self, name, flag):
        item = self.get_item(name)
        if item is not None:
            item.setDisabled(flag)

    def remove_item(self, name):
        item = self.get_item(name)
        if item is not None:
            self.menu.removeAction(item)
            self._ordered_items.remove(item)
            item.deleteLater()

    def _insert_item(self, before, item):
        if isinstance(before, QMenu):
            before = before.menuAction()
        if isinstance(item, QAction):
            self.menu.insertAction(before, item)
        elif isinstance(item, QMenu):
            self.menu.insertMenu(before, item)
        elif item is None:
            self.menu.insertSeparator(before)
        else:
            raise UserWarning('unknown item type')

    def _get_item_label(self, item):
        if isinstance(item, QAction):
            return item.text()
        elif isinstance(item, QMenu):
            return item.title()
        else:
            raise UserWarning('unknown item type')
