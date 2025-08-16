#!/usr/bin/env python

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

from __future__ import print_function

from argparse import ArgumentParser, SUPPRESS
import os
import signal
import sys

from ament_index_python.resources import get_resource, has_resource


class Main(object):

    main_filename = None

    def __init__(self, qtgui_path, invoked_filename=None, settings_filename=None):
        self._qtgui_path = qtgui_path
        if invoked_filename is None:
            invoked_filename = os.path.abspath(__file__)
        Main.main_filename = invoked_filename
        if settings_filename is None:
            settings_filename = 'qt_gui'
        self._settings_filename = settings_filename

        self.plugin_providers = []
        self._options = None

        # check if DBus is available
        self._dbus_available = False
        try:
            # use qt/glib mainloop integration to get dbus mainloop working
            from dbus.mainloop.glib import DBusGMainLoop
            DBusGMainLoop(set_as_default=True)
            import dbus
            try:
                # before being able to check if a session bus is available the dbus
                # mainloop must be set up
                dbus.SessionBus()
                self._dbus_available = True
            except dbus.exceptions.DBusException:
                pass
        except ImportError:
            pass

    def add_arguments(self, parser, standalone=False, plugin_argument_provider=None):
        common_group = parser.add_argument_group('Options for GUI instance')
        common_group.add_argument(
            '-b', '--qt-binding', dest='qt_binding', type=str, metavar='BINDING',
            help='choose Qt bindings to be used [pyqt|pyside]')
        common_group.add_argument(
            '--clear-config', dest='clear_config', default=False, action='store_true',
            help='clear the configuration (including all perspectives and plugin settings)')
        if not standalone:
            common_group.add_argument(
                '-f', '--freeze-layout', dest='freeze_layout', action='store_true',
                help='freeze the layout of the GUI (prevent rearranging widgets, disable '
                     'undock/redock)')
        common_group.add_argument(
            '--force-discover', dest='force_discover', default=False, action='store_true',
            help='force a rediscover of plugins')
        common_group.add_argument(
            '-h', '--help', action='help', help='show this help message and exit')
        common_group.add_argument(
                '-ht', '--hide-title', dest='hide_title', action='store_true',
                help='hide the title label, the icon, and the help button (combine with -l and -f '
                     'to eliminate the entire title bar and reclaim the space)')
        if not standalone:
            common_group.add_argument(
                '-l', '--lock-perspective', dest='lock_perspective', action='store_true',
                help='lock the GUI to the used perspective (hide menu bar and close buttons of '
                     'plugins)')

            common_group.add_argument(
                '-p', '--perspective', dest='perspective', type=str, metavar='PERSPECTIVE',
                help='start with this named perspective')
            common_group.add_argument(
                '--perspective-file', dest='perspective_file', type=str,
                metavar='PERSPECTIVE_FILE', help='start with a perspective loaded from a file')
        common_group.add_argument(
            '--reload-import', dest='reload_import', default=False, action='store_true',
            help='reload every imported module')
        if not standalone:
            common_group.add_argument(
                '-s', '--standalone', dest='standalone_plugin', type=str, metavar='PLUGIN',
                help='start only this plugin (implies -l). To pass arguments to the plugin use '
                     '--args')
        common_group.add_argument(
            '-t', '--on-top', dest='on_top', default=False, action='store_true',
            help='set window mode to always on top')
        common_group.add_argument(
            '-v', '--verbose', dest='verbose', default=False, action='store_true',
            help='output qDebug messages')

        if not standalone:
            common_group.add_argument(
                '--args', dest='plugin_args', nargs='*', type=str,
                help='arbitrary arguments which are passes to the plugin '
                     '(only with -s, --command-start-plugin or --embed-plugin). '
                     'It must be the last option since it collects all following options.')

            group = parser.add_argument_group(
                'Options to query information without starting a GUI instance',
                'These options can be used to query information about valid arguments for various '
                'options.')
            group.add_argument(
                '--list-perspectives', dest='list_perspectives', action='store_true',
                help='list available perspectives')
            group.add_argument(
                '--list-plugins', dest='list_plugins', action='store_true',
                help='list available plugins')
            parser.add_argument_group(group)

            group = parser.add_argument_group(
                'Options to operate on a running GUI instance',
                'These options can be used to perform actions on a running GUI instance.')
            group.add_argument(
                '--command-pid', dest='command_pid', type=int, metavar='PID',
                help='pid of the GUI instance to operate on, defaults to oldest running GUI '
                'instance')
            group.add_argument(
                '--command-start-plugin', dest='command_start_plugin', type=str, metavar='PLUGIN',
                help='start plugin')
            group.add_argument(
                '--command-switch-perspective', dest='command_switch_perspective', type=str,
                metavar='PERSPECTIVE', help='switch perspective')
            if not self._dbus_available:
                group.description = 'These options are not available since DBus is not available!'
                for o in group._group_actions:
                    o.help = SUPPRESS
            parser.add_argument_group(group)

            group = parser.add_argument_group(
                'Special options for embedding widgets from separate processes',
                'These options should never be used on the CLI but only from the GUI code itself.')
            group.add_argument(
                '--embed-plugin', dest='embed_plugin', type=str, metavar='PLUGIN',
                help='embed a plugin into an already running GUI instance (requires all other '
                     '--embed-* options)')
            group.add_argument(
                '--embed-plugin-serial', dest='embed_plugin_serial', type=int, metavar='SERIAL',
                help='serial number of plugin to be embedded '
                     '(requires all other --embed-* options)')
            group.add_argument(
                '--embed-plugin-address', dest='embed_plugin_address', type=str, metavar='ADDRESS',
                help='dbus server address of the GUI instance to embed plugin into '
                     '(requires all other --embed-* options)')
            for o in group._group_actions:
                o.help = SUPPRESS
            parser.add_argument_group(group)

        if plugin_argument_provider:
            plugin_argument_provider(parser)

        return common_group

    def _add_plugin_providers(self):
        pass

    def _add_reload_paths(self, reload_importer):
        reload_importer.add_reload_path(os.path.join(os.path.dirname(__file__), *('..',) * 4))

    def _set_theme_if_necessary(self):
        from python_qt_binding.QtGui import QIcon
        # Always prefer tango_icons_vendor
        package_path = has_resource('packages', 'tango_icons_vendor')
        if package_path:
            icon_paths = QIcon.themeSearchPaths()
            icon_paths = [
                os.path.join(package_path, 'share', 'tango_icons_vendor', 'resource', 'icons')
            ] + icon_paths
            QIcon.setThemeSearchPaths(icon_paths)

        # Use Tango if possible, fall back to system default
        original_theme = QIcon.themeName()
        if original_theme == 'Tango':
            # We don't have any back-up behaviour in case Tango is the default
            return
        # Set Tango as default and try to get an icon
        QIcon.setThemeName('Tango')
        if QIcon.fromTheme('document-save').isNull():
            # When Tango icon can't be found, use the system default theme
            QIcon.setThemeName(original_theme)
            return
        # In case we overruled the default theme to be Tango, set fallback theme to the original
        # theme. Any original fallback theme is dropped in this case.
        QIcon.setFallbackThemeName(original_theme)

    def create_application(self, argv):
        from python_qt_binding.QtCore import Qt
        from python_qt_binding.QtWidgets import QApplication
        app = QApplication(argv)
        app.setAttribute(Qt.AA_DontShowIconsInMenus, False)
        return app

    def main(self, argv=None, standalone=None, plugin_argument_provider=None,
             plugin_manager_settings_prefix='', help_text=None):
        if argv is None:
            argv = sys.argv

        # extract --args and everything behind manually since argparse can not handle that
        arguments = argv[1:]

        # extract plugin specific args when not being invoked in standalone mode programmatically
        if not standalone:
            plugin_args = []
            if '--args' in arguments:
                index = arguments.index('--args')
                plugin_args = arguments[index + 1:]
                arguments = arguments[0:index + 1]

        parser = ArgumentParser(os.path.basename(Main.main_filename), add_help=False)
        self.add_arguments(parser, standalone=bool(standalone),
                           plugin_argument_provider=plugin_argument_provider)
        self._options = parser.parse_args(arguments)

        if standalone:
            # rerun parsing to separate common arguments from plugin specific arguments
            parser = ArgumentParser(os.path.basename(Main.main_filename), add_help=False)
            self.add_arguments(parser, standalone=bool(standalone))
            self._options, plugin_args = parser.parse_known_args(arguments)
        self._options.multi_process = False  # not supported anymore
        self._options.plugin_args = plugin_args

        # set default values for options not available in standalone mode
        if standalone:
            self._options.freeze_layout = False
            self._options.lock_perspective = False
            self._options.perspective = None
            self._options.perspective_file = None
            self._options.standalone_plugin = standalone
            self._options.list_perspectives = False
            self._options.list_plugins = False
            self._options.command_pid = None
            self._options.command_start_plugin = None
            self._options.command_switch_perspective = None
            self._options.embed_plugin = None
            self._options.embed_plugin_serial = None
            self._options.embed_plugin_address = None

        # check option dependencies
        try:
            if self._options.plugin_args and \
                    not self._options.standalone_plugin and \
                    not self._options.command_start_plugin and \
                    not self._options.embed_plugin:
                raise RuntimeError(
                    'Option --args can only be used together with either --standalone, '
                    '--command-start-plugin or --embed-plugin option')

            if self._options.freeze_layout and not self._options.lock_perspective:
                raise RuntimeError(
                    'Option --freeze_layout can only be used together with the '
                    '--lock_perspective option')

            list_options = (self._options.list_perspectives, self._options.list_plugins)
            list_options_set = [opt for opt in list_options if opt is not False]
            if len(list_options_set) > 1:
                raise RuntimeError('Only one --list-* option can be used at a time')

            command_options = (
                self._options.command_start_plugin, self._options.command_switch_perspective)
            command_options_set = [opt for opt in command_options if opt is not None]
            if len(command_options_set) > 0 and not self._dbus_available:
                raise RuntimeError(
                    'Without DBus support the --command-* options are not available')
            if len(command_options_set) > 1:
                raise RuntimeError(
                    'Only one --command-* option can be used at a time (except --command-pid '
                    'which is optional)')
            if len(command_options_set) == 0 and self._options.command_pid is not None:
                raise RuntimeError(
                    'Option --command_pid can only be used together with an other --command-* '
                    'option')

            embed_options = (
                self._options.embed_plugin, self._options.embed_plugin_serial,
                self._options.embed_plugin_address)
            embed_options_set = [opt for opt in embed_options if opt is not None]
            if len(command_options_set) > 0 and not self._dbus_available:
                raise RuntimeError('Without DBus support the --embed-* options are not available')
            if len(embed_options_set) > 0 and len(embed_options_set) < len(embed_options):
                raise RuntimeError("Missing option(s) - all '--embed-*' options must be set")

            if len(embed_options_set) > 0 and self._options.clear_config:
                raise RuntimeError(
                    'Option --clear-config can only be used without any --embed-* option')

            groups = (list_options_set, command_options_set, embed_options_set)
            groups_set = [opt for opt in groups if len(opt) > 0]
            if len(groups_set) > 1:
                raise RuntimeError(
                    'Options from different groups (--list, --command, --embed) can not be used '
                    'together')

            perspective_options = (self._options.perspective, self._options.perspective_file)
            perspective_options_set = [opt for opt in perspective_options if opt is not None]
            if len(perspective_options_set) > 1:
                raise RuntimeError('Only one --perspective-* option can be used at a time')

            if self._options.perspective_file is not None and \
                    not os.path.isfile(self._options.perspective_file):
                raise RuntimeError('Option --perspective-file must reference existing file')

        except RuntimeError as e:
            print(str(e))
            # parser.parse_args(['--help'])
            # calling --help will exit
            return 1

        # set implicit option dependencies
        if self._options.standalone_plugin is not None:
            self._options.lock_perspective = True

        # create application context containing various relevant information
        from qt_gui.application_context import ApplicationContext
        context = ApplicationContext()
        context.qtgui_path = self._qtgui_path
        context.options = self._options

        if self._dbus_available:
            from dbus import DBusException, Interface, SessionBus

        # non-special applications provide various dbus interfaces
        if self._dbus_available:
            context.provide_app_dbus_interfaces = len(groups_set) == 0
            context.dbus_base_bus_name = 'org.ros.qt_gui'
            if context.provide_app_dbus_interfaces:
                context.dbus_unique_bus_name = context.dbus_base_bus_name + '.pid%d' % os.getpid()

                # provide pid of application via dbus
                from qt_gui.application_dbus_interface import ApplicationDBusInterface
                _dbus_server = ApplicationDBusInterface(context.dbus_base_bus_name)  # noqa: F841

        # determine host bus name, either based on pid given on command line or
        # via dbus application interface if any other instance is available
        if len(command_options_set) > 0 or len(embed_options_set) > 0:
            host_pid = None
            if self._options.command_pid is not None:
                host_pid = self._options.command_pid
            else:
                try:
                    remote_object = SessionBus().get_object(
                        context.dbus_base_bus_name, '/Application')
                except DBusException:
                    pass
                else:
                    remote_interface = Interface(
                        remote_object, context.dbus_base_bus_name + '.Application')
                    host_pid = remote_interface.get_pid()
            if host_pid is not None:
                context.dbus_host_bus_name = context.dbus_base_bus_name + '.pid%d' % host_pid

        # execute command on host application instance
        if len(command_options_set) > 0:
            if self._options.command_start_plugin is not None:
                try:
                    remote_object = SessionBus().get_object(
                        context.dbus_host_bus_name, '/PluginManager')
                except DBusException:
                    (rc, msg) = (1, 'unable to communicate with GUI instance "%s"' %
                                 context.dbus_host_bus_name)
                else:
                    remote_interface = Interface(
                        remote_object, context.dbus_base_bus_name + '.PluginManager')
                    (rc, msg) = remote_interface.start_plugin(
                        self._options.command_start_plugin, ' '.join(self._options.plugin_args))
                if rc == 0:
                    print('qt_gui_main() started plugin "%s" in GUI "%s"' %
                          (msg, context.dbus_host_bus_name))
                else:
                    print('qt_gui_main() could not start plugin "%s" in GUI "%s": %s' %
                          (self._options.command_start_plugin, context.dbus_host_bus_name, msg))
                return rc
            elif self._options.command_switch_perspective is not None:
                remote_object = SessionBus().get_object(
                    context.dbus_host_bus_name, '/PerspectiveManager')
                remote_interface = Interface(
                    remote_object, context.dbus_base_bus_name + '.PerspectiveManager')
                remote_interface.switch_perspective(self._options.command_switch_perspective)
                print('qt_gui_main() switched to perspective "%s" in GUI "%s"' %
                      (self._options.command_switch_perspective, context.dbus_host_bus_name))
                return 0
            raise RuntimeError('Unknown command not handled')

        # choose selected or default qt binding
        setattr(sys, 'SELECT_QT_BINDING', self._options.qt_binding)
        from python_qt_binding import QT_BINDING

        from python_qt_binding.QtCore import (qDebug, qInstallMessageHandler,
                                              QSettings, Qt, QtCriticalMsg, QtDebugMsg)
        from python_qt_binding.QtCore import QtFatalMsg, QTimer, QtWarningMsg

        from python_qt_binding.QtGui import QIcon
        from python_qt_binding.QtWidgets import QAction

        from qt_gui.about_handler import AboutHandler
        from qt_gui.composite_plugin_provider import CompositePluginProvider
        from qt_gui.container_manager import ContainerManager
        from qt_gui.help_provider import HelpProvider
        from qt_gui.icon_loader import get_icon
        from qt_gui.main_window import MainWindow
        from qt_gui.minimized_dock_widgets_toolbar import MinimizedDockWidgetsToolbar
        from qt_gui.perspective_manager import PerspectiveManager
        from qt_gui.plugin_manager import PluginManager

        # TODO PySide2 segfaults when invoking this custom message handler atm
        if QT_BINDING != 'pyside':
            def message_handler(type_, context, msg):
                colored_output = 'TERM' in os.environ and 'ANSI_COLORS_DISABLED' not in os.environ
                cyan_color = '\033[36m' if colored_output else ''
                red_color = '\033[31m' if colored_output else ''
                reset_color = '\033[0m' if colored_output else ''
                if type_ == QtDebugMsg and self._options.verbose:
                    print(msg, file=sys.stderr)
                elif type_ == QtWarningMsg:
                    print(cyan_color + msg + reset_color, file=sys.stderr)
                elif type_ == QtCriticalMsg:
                    print(red_color + msg + reset_color, file=sys.stderr)
                elif type_ == QtFatalMsg:
                    print(red_color + msg + reset_color, file=sys.stderr)
                    sys.exit(1)
            qInstallMessageHandler(message_handler)

        app = self.create_application(argv)

        self._set_theme_if_necessary()

        settings = QSettings(
            QSettings.IniFormat, QSettings.UserScope, 'ros.org', self._settings_filename)
        if len(embed_options_set) == 0:
            if self._options.clear_config:
                settings.clear()

            main_window = MainWindow(help_text)
            if self._options.on_top:
                main_window.setWindowFlags(Qt.WindowStaysOnTopHint)

            main_window.statusBar()

            def sigint_handler(*args):
                qDebug('\nsigint_handler()')
                main_window.close()
            signal.signal(signal.SIGINT, sigint_handler)
            # the timer enables triggering the sigint_handler
            timer = QTimer()
            timer.start(500)
            timer.timeout.connect(lambda: None)

            if not self._options.lock_perspective:
                menu_bar = main_window.menuBar()
                file_menu = menu_bar.addMenu(menu_bar.tr('&File'))
                action = QAction(file_menu.tr('&Quit'), file_menu)
                action.setIcon(QIcon.fromTheme('application-exit'))
                action.triggered.connect(main_window.close)
                file_menu.addAction(action)
            else:
                menu_bar = None

        else:
            app.setQuitOnLastWindowClosed(False)

            main_window = None
            menu_bar = None

        self._add_plugin_providers()

        # setup plugin manager
        plugin_provider = CompositePluginProvider(self.plugin_providers)
        plugin_manager = PluginManager(
            plugin_provider, settings, context, settings_prefix=plugin_manager_settings_prefix)

        if self._options.list_plugins:
            # output available plugins
            print('\n'.join(sorted(plugin_manager.get_plugins().values())))
            return 0

        help_provider = HelpProvider()
        plugin_manager.plugin_help_signal.connect(help_provider.plugin_help_request)

        # setup perspective manager
        if main_window is not None:
            perspective_manager = PerspectiveManager(settings, context)

            if self._options.list_perspectives:
                # output available perspectives
                print('\n'.join(sorted(perspective_manager.perspectives)))
                return 0
        else:
            perspective_manager = None

        if main_window is not None:
            container_manager = ContainerManager(main_window, plugin_manager)
            plugin_manager.set_main_window(main_window, menu_bar, container_manager)
            plugin_manager.plugin_load_unload_signal.connect(main_window.showHelpWidget)

            if not self._options.freeze_layout:
                minimized_dock_widgets_toolbar = MinimizedDockWidgetsToolbar(
                    container_manager, main_window)
                main_window.addToolBar(Qt.BottomToolBarArea, minimized_dock_widgets_toolbar)
                plugin_manager.set_minimized_dock_widgets_toolbar(minimized_dock_widgets_toolbar)

        if menu_bar is not None:
            perspective_menu = menu_bar.addMenu(menu_bar.tr('P&erspectives'))
            perspective_manager.set_menu(perspective_menu)

        # connect various signals and slots
        if perspective_manager is not None and main_window is not None:
            # signal changed perspective to update window title
            perspective_manager.perspective_changed_signal.connect(main_window.perspective_changed)
            # signal new settings due to changed perspective
            perspective_manager.save_settings_signal.connect(main_window.save_settings)
            perspective_manager.restore_settings_signal.connect(main_window.restore_settings)
            perspective_manager.restore_settings_without_plugin_changes_signal.connect(
                main_window.restore_settings)

        if perspective_manager is not None and plugin_manager is not None:
            perspective_manager.save_settings_signal.connect(plugin_manager.save_settings)
            plugin_manager.save_settings_completed_signal.connect(
                perspective_manager.save_settings_completed)
            perspective_manager.restore_settings_signal.connect(plugin_manager.restore_settings)
            perspective_manager.restore_settings_without_plugin_changes_signal.connect(
                plugin_manager.restore_settings_without_plugins)

        if plugin_manager is not None and main_window is not None:
            # signal before changing plugins to save window state
            plugin_manager.plugins_about_to_change_signal.connect(main_window.save_setup)
            # signal changed plugins to restore window state
            plugin_manager.plugins_changed_signal.connect(main_window.restore_state)
            # signal save settings to store plugin setup on close
            main_window.save_settings_before_close_signal.connect(plugin_manager.close_application)
            # signal save and shutdown called for all plugins, trigger closing main window again
            plugin_manager.close_application_signal.connect(
                main_window.close, type=Qt.QueuedConnection)

        if main_window is not None and menu_bar is not None:
            about_handler = AboutHandler(context.qtgui_path, main_window)
            help_menu = menu_bar.addMenu(menu_bar.tr('&Help'))
            action = QAction(help_menu.tr('&About'), help_menu)
            action.setIcon(QIcon.fromTheme('help-about'))
            action.triggered.connect(about_handler.show)
            help_menu.addAction(action)

        # set initial size - only used without saved configuration
        if main_window is not None:
            # Adjust size to fit the widget if standalone (e.g. single plugin)
            if standalone:
                main_window.adjustSize()
            # On "clean" startup set some size to fully display the menu bar
            else:
                main_window.resize(600, 450)
                main_window.move(100, 100)

        # ensure that qt_gui/src is in sys.path
        src_path = os.path.realpath(os.path.join(os.path.dirname(__file__), '..'))
        if src_path not in sys.path:
            sys.path.append(src_path)

        # load specific plugin
        plugin = None
        plugin_serial = None
        if self._options.embed_plugin is not None:
            plugin = self._options.embed_plugin
            plugin_serial = self._options.embed_plugin_serial
        elif self._options.standalone_plugin is not None:
            plugin = self._options.standalone_plugin
            plugin_serial = 0
        if plugin is not None:
            plugins = plugin_manager.find_plugins_by_name(plugin)
            if len(plugins) == 0:
                print('qt_gui_main() found no plugin matching "%s"' % plugin)
                print('try passing the option "--force-discover"')
                return 1
            elif len(plugins) > 1:
                print('qt_gui_main() found multiple plugins matching "%s"\n%s' %
                      (plugin, '\n'.join(plugins.values())))
                return 1
            plugin = list(plugins.keys())[0]

        qDebug('QtBindingHelper using %s' % QT_BINDING)

        plugin_manager.discover()

        if self._options.reload_import:
            qDebug('ReloadImporter() automatically reload all subsequent imports')
            from qt_gui.reload_importer import ReloadImporter
            _reload_importer = ReloadImporter()
            self._add_reload_paths(_reload_importer)
            _reload_importer.enable()

        # switch perspective
        if perspective_manager is not None:
            if plugin:
                perspective_manager.set_perspective(plugin, hide_and_without_plugin_changes=True)
            elif self._options.perspective_file:
                perspective_manager.import_perspective_from_file(
                    self._options.perspective_file,
                    perspective_manager.HIDDEN_PREFIX + os.path.basename(
                        self._options.perspective_file))
            else:
                perspective_manager.set_perspective(self._options.perspective)

        # load specific plugin
        if plugin:
            plugin_manager.load_plugin(plugin, plugin_serial, self._options.plugin_args)
            running = plugin_manager.is_plugin_running(plugin, plugin_serial)
            if not running:
                return 1
            if self._options.standalone_plugin:
                # use icon of standalone plugin (if available) for application
                plugin_descriptor = plugin_manager.get_plugin_descriptor(plugin)
                action_attributes = plugin_descriptor.action_attributes()
                if 'icon' in action_attributes and action_attributes['icon'] is not None:
                    base_path = plugin_descriptor.attributes().get('plugin_path')
                    try:
                        icon = get_icon(
                            action_attributes['icon'],
                            action_attributes.get('icontype', None),
                            base_path)
                    except UserWarning:
                        pass
                    else:
                        app.setWindowIcon(icon)

        if main_window is not None:
            main_window.show()
            if sys.platform == 'darwin':
                main_window.raise_()

        return app.exec_()


if __name__ == '__main__':
    _, qtgui_path = get_resource('packages', 'qt_gui')
    main = Main(qtgui_path)
    sys.exit(main.main())
