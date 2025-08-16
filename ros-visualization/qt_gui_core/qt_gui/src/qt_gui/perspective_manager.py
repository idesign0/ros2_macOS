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

import json
import os

from python_qt_binding import loadUi
from python_qt_binding.QtCore import QByteArray, qDebug, QObject, QSignalMapper, Signal, Slot
from python_qt_binding.QtGui import QIcon, QValidator
from python_qt_binding.QtWidgets import QAction, QFileDialog, QInputDialog, QMessageBox

from qt_gui.menu_manager import MenuManager
from qt_gui.settings import Settings
from qt_gui.settings_proxy import SettingsProxy


def is_string(s):
    """Check if the argument is a string which works for both Python 2 and 3."""
    try:
        return isinstance(s, basestring)
    except NameError:
        return isinstance(s, str)


class PerspectiveManager(QObject):
    """Manager for perspectives associated with specific sets of `Settings`."""

    perspective_changed_signal = Signal(str)
    save_settings_signal = Signal(Settings, Settings)
    restore_settings_signal = Signal(Settings, Settings)
    restore_settings_without_plugin_changes_signal = Signal(Settings, Settings)

    HIDDEN_PREFIX = '@'

    def __init__(self, settings, application_context):
        super(PerspectiveManager, self).__init__()
        self.setObjectName('PerspectiveManager')

        self._qtgui_path = application_context.qtgui_path

        self._settings_proxy = SettingsProxy(settings)
        self._global_settings = Settings(self._settings_proxy, 'global')
        self._perspective_settings = None
        self._create_perspective_dialog = None

        self._menu_manager = None
        self._perspective_mapper = None

        # get perspective list from settings
        self.perspectives = self._settings_proxy.value('', 'perspectives', [])
        if is_string(self.perspectives):
            self.perspectives = [self.perspectives]

        self._current_perspective = None
        self._remove_action = None

        self._callback = None
        self._callback_args = []

        self._file_path = os.getcwd()

        if application_context.provide_app_dbus_interfaces:
            from qt_gui.perspective_manager_dbus_interface import PerspectiveManagerDBusInterface
            self._dbus_server = PerspectiveManagerDBusInterface(self, application_context)

    def set_menu(self, menu):
        self._menu_manager = MenuManager(menu)
        self._perspective_mapper = QSignalMapper(menu)
        self._perspective_mapper.mapped[str].connect(self.switch_perspective)

        # generate menu
        create_action = QAction('&Create perspective...', self._menu_manager.menu)
        create_action.setIcon(QIcon.fromTheme('list-add'))
        create_action.triggered.connect(self._on_create_perspective)
        self._menu_manager.add_suffix(create_action)

        self._remove_action = QAction('&Remove perspective...', self._menu_manager.menu)
        self._remove_action.setEnabled(False)
        self._remove_action.setIcon(QIcon.fromTheme('list-remove'))
        self._remove_action.triggered.connect(self._on_remove_perspective)
        self._menu_manager.add_suffix(self._remove_action)

        self._menu_manager.add_suffix(None)

        import_action = QAction('&Import...', self._menu_manager.menu)
        import_action.setIcon(QIcon.fromTheme('document-open'))
        import_action.triggered.connect(self._on_import_perspective)
        self._menu_manager.add_suffix(import_action)

        export_action = QAction('&Export...', self._menu_manager.menu)
        export_action.setIcon(QIcon.fromTheme('document-save-as'))
        export_action.triggered.connect(self._on_export_perspective)
        self._menu_manager.add_suffix(export_action)

        # add perspectives to menu
        for name in self.perspectives:
            if not name.startswith(self.HIDDEN_PREFIX):
                self._add_perspective_action(name)

    def set_perspective(self, name, hide_and_without_plugin_changes=False):
        if name is None:
            name = self._settings_proxy.value('', 'current-perspective', 'Default')
        elif hide_and_without_plugin_changes:
            name = self.HIDDEN_PREFIX + name
        self.switch_perspective(name, save_before=not hide_and_without_plugin_changes,
                                without_plugin_changes=hide_and_without_plugin_changes)

    @Slot(str)
    @Slot(str, bool)
    @Slot(str, bool, bool)
    def switch_perspective(
            self, name, settings_changed=True, save_before=True, without_plugin_changes=False):
        if save_before and \
                self._global_settings is not None and \
                self._perspective_settings is not None:
            self._callback = self._switch_perspective
            self._callback_args = [name, settings_changed, save_before]
            self.save_settings_signal.emit(self._global_settings, self._perspective_settings)
        else:
            self._switch_perspective(name, settings_changed, save_before, without_plugin_changes)

    def _switch_perspective(
            self, name, settings_changed, save_before, without_plugin_changes=False):
        # convert from unicode
        name = str(name.replace('/', '__'))

        qDebug('PerspectiveManager.switch_perspective() switching to perspective "%s"' % name)
        if self._current_perspective is not None and self._menu_manager is not None:
            self._menu_manager.set_item_checked(self._current_perspective, False)
            self._menu_manager.set_item_disabled(self._current_perspective, False)

        # create perspective if necessary
        if name not in self.perspectives:
            self._create_perspective(name, clone_perspective=False)

        # update current perspective
        self._current_perspective = name
        if self._menu_manager is not None:
            self._menu_manager.set_item_checked(self._current_perspective, True)
            self._menu_manager.set_item_disabled(self._current_perspective, True)
        if not self._current_perspective.startswith(self.HIDDEN_PREFIX):
            self._settings_proxy.set_value('', 'current-perspective', self._current_perspective)
        self._perspective_settings = self._get_perspective_settings(self._current_perspective)

        # emit signals
        self.perspective_changed_signal.emit(self._current_perspective.lstrip(self.HIDDEN_PREFIX))
        if settings_changed:
            if not without_plugin_changes:
                self.restore_settings_signal.emit(
                    self._global_settings, self._perspective_settings)
            else:
                self.restore_settings_without_plugin_changes_signal.emit(
                    self._global_settings, self._perspective_settings)

    def save_settings_completed(self):
        if self._callback is not None:
            callback = self._callback
            callback_args = self._callback_args
            self._callback = None
            self._callback_args = []
            callback(*callback_args)

    def _get_perspective_settings(self, perspective_name):
        return Settings(self._settings_proxy, 'perspective/%s' % perspective_name)

    def _on_create_perspective(self):
        name = self._choose_new_perspective_name()
        if name is not None:
            clone_perspective = self._create_perspective_dialog.clone_checkbox.isChecked()
            self._create_perspective(name, clone_perspective)
            self.switch_perspective(
                name, settings_changed=not clone_perspective, save_before=False)

    def _choose_new_perspective_name(self, show_cloning=True):
        # input dialog for new perspective name
        if self._create_perspective_dialog is None:
            ui_file = os.path.join(self._qtgui_path,
                                   'share', 'qt_gui', 'resource', 'perspective_create.ui')
            self._create_perspective_dialog = loadUi(ui_file)

            # custom validator preventing forward slashs
            class CustomValidator(QValidator):

                def __init__(self, parent=None):
                    super(CustomValidator, self).__init__(parent)

                def fixup(self, value):
                    value = value.replace('/', '')

                def validate(self, value, pos):
                    if value.find('/') != -1:
                        pos = value.find('/')
                        return (QValidator.Invalid, value, pos)
                    if value == '':
                        return (QValidator.Intermediate, value, pos)
                    return (QValidator.Acceptable, value, pos)
            self._create_perspective_dialog.perspective_name_edit.setValidator(CustomValidator())

        # set default values
        self._create_perspective_dialog.perspective_name_edit.setText('')
        self._create_perspective_dialog.clone_checkbox.setChecked(True)
        self._create_perspective_dialog.clone_checkbox.setVisible(show_cloning)

        # show dialog and wait for it's return value
        return_value = self._create_perspective_dialog.exec_()
        if return_value == self._create_perspective_dialog.Rejected:
            return

        name = str(self._create_perspective_dialog.perspective_name_edit.text()).lstrip(
            self.HIDDEN_PREFIX)
        if name == '':
            QMessageBox.warning(
                self._menu_manager.menu,
                self.tr('Empty perspective name'),
                self.tr('The name of the perspective must be non-empty.'))
            return
        if name in self.perspectives:
            QMessageBox.warning(
                self._menu_manager.menu,
                self.tr('Duplicate perspective name'),
                self.tr('A perspective with the same name already exists.'))
            return
        return name

    def _create_perspective(self, name, clone_perspective=True):
        # convert from unicode
        name = str(name)
        if name.find('/') != -1:
            raise RuntimeError(
                'PerspectiveManager._create_perspective() name cannot contain forward slashes (/)')

        qDebug('PerspectiveManager._create_perspective(%s, %s)' % (name, clone_perspective))
        # add to list of perspectives
        self.perspectives.append(name)
        self._settings_proxy.set_value('', 'perspectives', self.perspectives)

        # save current settings
        if self._global_settings is not None and self._perspective_settings is not None:
            self._callback = self._create_perspective_continued
            self._callback_args = [name, clone_perspective]
            self.save_settings_signal.emit(self._global_settings, self._perspective_settings)
        else:
            self._create_perspective_continued(name, clone_perspective)

    def _create_perspective_continued(self, name, clone_perspective):
        # clone settings
        if clone_perspective:
            new_settings = self._get_perspective_settings(name)
            keys = self._perspective_settings.all_keys()
            for key in keys:
                value = self._perspective_settings.value(key)
                new_settings.set_value(key, value)

        # add and switch to perspective
        if not name.startswith(self.HIDDEN_PREFIX):
            self._add_perspective_action(name)

    def _add_perspective_action(self, name):
        if self._menu_manager is not None:
            # create action
            action = QAction(name, self._menu_manager.menu)
            action.setCheckable(True)
            self._perspective_mapper.setMapping(action, name)
            action.triggered.connect(self._perspective_mapper.map)

            # add action to menu
            self._menu_manager.add_item(action)
            # enable remove-action
            if self._menu_manager.count_items() > 1:
                self._remove_action.setEnabled(True)

    def _on_remove_perspective(self):
        # input dialog to choose perspective to be removed
        names = list(self.perspectives)
        names.remove(self._current_perspective)
        name, return_value = QInputDialog.getItem(
            self._menu_manager.menu, self._menu_manager.tr('Remove perspective'),
            self._menu_manager.tr('Select the perspective'), names, 0, False)
        # convert from unicode
        name = str(name)
        if return_value == QInputDialog.Rejected:
            return
        self._remove_perspective(name)

    def _remove_perspective(self, name):
        if name not in self.perspectives:
            raise UserWarning('unknown perspective: %s' % name)
        qDebug('PerspectiveManager._remove_perspective(%s)' % str(name))

        # remove from list of perspectives
        self.perspectives.remove(name)
        self._settings_proxy.set_value('', 'perspectives', self.perspectives)

        # remove settings
        settings = self._get_perspective_settings(name)
        settings.remove('')

        # remove from menu
        self._menu_manager.remove_item(name)

        # disable remove-action
        if self._menu_manager.count_items() < 2:
            self._remove_action.setEnabled(False)

    def _on_import_perspective(self):
        file_name, _ = QFileDialog.getOpenFileName(
            self._menu_manager.menu, self.tr('Import perspective from file'),
            self._file_path, self.tr('Perspectives (*.perspective)'))
        if file_name is None or file_name == '':
            return

        perspective_name = os.path.basename(file_name)
        suffix = '.perspective'
        if perspective_name.endswith(suffix):
            perspective_name = perspective_name[:-len(suffix)]
        if perspective_name in self.perspectives:
            perspective_name = self._choose_new_perspective_name(False)
            if perspective_name is None:
                return

        self.import_perspective_from_file(file_name, perspective_name)

    def import_perspective_from_file(self, path, perspective_name):
        self._file_path = os.path.dirname(path)
        # create clean perspective
        if perspective_name in self.perspectives:
            self._remove_perspective(perspective_name)
        self._create_perspective(perspective_name, clone_perspective=False)

        # read perspective from file
        file_handle = open(path, 'r')
        # data = eval(file_handle.read())
        data = json.loads(file_handle.read())
        self._convert_values(data, self._import_value)

        new_settings = self._get_perspective_settings(perspective_name)
        self._set_dict_on_settings(data, new_settings)

        self.switch_perspective(perspective_name, settings_changed=True, save_before=True)

    def _set_dict_on_settings(self, data, settings):
        """Set dictionary key-value pairs on Settings instance."""
        keys = data.get('keys', {})
        for key in keys:
            settings.set_value(key, keys[key])
        groups = data.get('groups', {})
        for group in groups:
            sub = settings.get_settings(group)
            self._set_dict_on_settings(groups[group], sub)

    def _on_export_perspective(self):
        save_file_name = os.path.join(
            self._file_path, self._current_perspective.lstrip(self.HIDDEN_PREFIX))
        suffix = '.perspective'
        if not save_file_name.endswith(suffix):
            save_file_name += suffix
        file_name, _ = QFileDialog.getSaveFileName(
            self._menu_manager.menu, self.tr('Export perspective to file'),
            save_file_name, self.tr('Perspectives (*.perspective)'))
        if file_name is None or file_name == '':
            return
        self._file_path = os.path.dirname(file_name)

        # trigger save of perspective before export
        self._callback = self._on_export_perspective_continued
        self._callback_args = [file_name]
        self.save_settings_signal.emit(self._global_settings, self._perspective_settings)

    def _on_export_perspective_continued(self, file_name):
        # convert every value
        data = self._get_dict_from_settings(self._perspective_settings)
        self._convert_values(data, self._export_value)

        # write perspective data to file
        file_handle = open(file_name, 'w')
        file_handle.write(json.dumps(data, indent=2, separators=(',', ': ')))
        file_handle.close()

    def _get_dict_from_settings(self, settings):
        """Convert data of Settings instance to dictionary."""
        keys = {}
        for key in settings.child_keys():
            keys[str(key)] = settings.value(key)
        groups = {}
        for group in settings.child_groups():
            sub = settings.get_settings(group)
            groups[str(group)] = self._get_dict_from_settings(sub)
        return {'keys': keys, 'groups': groups}

    def _convert_values(self, data, convert_function):
        keys = data.get('keys', {})
        for key in keys:
            keys[key] = convert_function(keys[key])
        groups = data.get('groups', {})
        for group in groups:
            self._convert_values(groups[group], convert_function)

    def _import_value(self, value):
        import QtCore  # noqa: F401
        if value['type'] == 'repr':
            return eval(value['repr'])
        elif value['type'] == 'repr(QByteArray.hex)':
            return QByteArray.fromHex(eval(value['repr(QByteArray.hex)']))
        raise RuntimeError(
            'PerspectiveManager._import_value() unknown serialization type (%s)' % value['type'])

    def _export_value(self, value):
        data = {}
        if value.__class__.__name__ == 'QByteArray':
            hex_value = value.toHex()
            data['repr(QByteArray.hex)'] = \
                self._strip_qt_binding_prefix(hex_value, repr(hex_value))
            data['type'] = 'repr(QByteArray.hex)'

            # add pretty print for better readability
            characters = ''
            for i in range(1, value.size(), 2):
                try:
                    character = value.at(i)
                    if isinstance(character, bytes):
                        character = character.decode('utf-8')
                    # output all non-control characters
                    if character >= ' ' and character <= '~':
                        characters += character
                    else:
                        characters += ' '
                except UnicodeDecodeError:
                    characters += ' '
            data['pretty-print'] = characters

        else:
            data['repr'] = self._strip_qt_binding_prefix(value, repr(value))
            data['type'] = 'repr'

        # verify that serialized data can be deserialized correctly
        reimported = self._import_value(data)
        if reimported != value:
            raise RuntimeError(
                'PerspectiveManager._export_value() stored value can not be restored (%s)' %
                type(value))

        return data

    def _strip_qt_binding_prefix(self, obj, data):
        """Strip binding specific prefix from type string."""
        parts = obj.__class__.__module__.split('.')
        if len(parts) > 1 and parts[1] == 'QtCore':
            prefix = '.'.join(parts[:2])
            data = data.replace(prefix, 'QtCore', 1)
        return data
