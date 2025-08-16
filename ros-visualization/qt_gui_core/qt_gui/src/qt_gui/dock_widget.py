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

from python_qt_binding.QtCore import qDebug, QEvent, QPoint, QRect, Qt
from python_qt_binding.QtGui import QMouseEvent
from python_qt_binding.QtWidgets import QApplication, QDockWidget

from qt_gui.dockable_main_window import DockableMainWindow
from qt_gui.reparent_event import ReparentEvent


class DockWidget(QDockWidget):
    """Widget with the capability to be reparented via drag-and-drop to any other main window."""

    def __init__(self, container_manager):
        super(DockWidget, self).__init__()
        self._container_manager = container_manager
        if self._container_manager is not None:
            self.event = self._event
        self._dragging_parent = None
        self._dragging_local_pos = None
        self._releasing_and_repressing_while_dragging = False
        self._main_windows = []

    def _event(self, e):
        if e.type() == QEvent.MouseButtonPress and e.button() == Qt.LeftButton:
            qDebug('%spress, rel=%s, global=%s, diff=%s' % (
                (' - pseudo ' if self._releasing_and_repressing_while_dragging else ''),
                e.pos(), e.globalPos(), e.globalPos() - self.pos()))

        if e.type() == QEvent.MouseButtonRelease and e.button() == Qt.LeftButton:
            qDebug('%srelease, rel=%s, global=%s, diff=%s' % (
                (' - pseudo ' if self._releasing_and_repressing_while_dragging else ''),
                e.pos(), e.globalPos(), e.globalPos() - self.pos()))

        # store local position when pressing button before starting the custom drag'n'drop
        # only allow when layout is not frozen
        if self._dragging_parent is None and \
                e.type() == QEvent.MouseButtonPress and \
                e.button() == Qt.LeftButton and \
                bool(self.features() & QDockWidget.DockWidgetMovable):
            self._dragging_local_pos = e.pos()

        if self._dragging_parent is None and \
                self._dragging_local_pos is not None and \
                e.type() == QEvent.Move and \
                QApplication.mouseButtons() & Qt.LeftButton:
            if self._widget_at(e.pos()) is not None:
                qDebug(
                    'DockWidget._event() start drag, dockwidget=%s, parent=%s, '
                    'floating=%s, pos=%s' % (
                        str(self), str(self.parent()),
                        str(self.isFloating()),
                        str(self._dragging_local_pos)))
                self._dragging_parent = self.parent()
                # ignore further mouse events so that the widget behind this dock widget
                # can be determined
                self.setAttribute(Qt.WA_TransparentForMouseEvents)

                # collect all main windows (except self.main_window) to re-implement
                # QApplication.widgetAt() in self._widget_at()
                self._main_windows = [self._container_manager.get_root_main_window()]
                for container in self._container_manager.get_containers():
                    if container == self:
                        continue
                    self._main_windows.append(container.main_window)

        # unset local position when releasing button even when custom drag'n'drop
        # has not been started
        if self._dragging_local_pos is not None and \
                e.type() == QEvent.MouseButtonRelease and \
                e.button() == Qt.LeftButton \
                and not self._releasing_and_repressing_while_dragging:
            self._dragging_local_pos = None

        if self._dragging_parent is not None and \
                e.type() == QEvent.MouseButtonRelease and \
                e.button() == Qt.LeftButton and \
                not self._releasing_and_repressing_while_dragging:
            qDebug('DockWidget._event() stop drag, dockwidget=%s, parent=%s\n' %
                   (self, self.parent()))
            self._dragging_parent = None
            self.setAttribute(Qt.WA_TransparentForMouseEvents, False)
            self._main_windows = []

        if self._dragging_parent is not None and \
                e.type() == QEvent.MouseMove and \
                e.buttons() & Qt.LeftButton and \
                not self._releasing_and_repressing_while_dragging:
            widget = self._widget_at(e.globalPos())
            new_parent = self._get_new_parent(widget)
            # print 'new_parent', new_parent, (new_parent.objectName() if new_parent else '')
            if new_parent is not None and new_parent != self.parent():
                self._releasing_and_repressing_while_dragging = True

                # schedule stop of pseudo drag'n'drop and let it complete
                mouse_release_event = QMouseEvent(
                    QEvent.MouseButtonRelease, self._dragging_local_pos,
                    e.globalPos(), Qt.LeftButton, Qt.NoButton, e.modifiers())
                QApplication.instance().postEvent(self, mouse_release_event)
                QApplication.sendPostedEvents()

                # schedule reparent to hovered main window and let it complete
                reparent_event = ReparentEvent(self, new_parent)
                QApplication.instance().postEvent(self._container_manager, reparent_event)
                QApplication.sendPostedEvents()

                # reenable mouse events to be able to receive upcoming pseudo mouse events
                self.setAttribute(Qt.WA_TransparentForMouseEvents, False)

                # schedule restart of pseudo drag'n'drop and let it complete
                mouse_repress_event = QMouseEvent(
                    QEvent.MouseButtonPress, self._dragging_local_pos, e.globalPos(),
                    Qt.LeftButton, Qt.LeftButton, e.modifiers())
                QApplication.instance().postEvent(self, mouse_repress_event)
                QApplication.sendPostedEvents()

                # schedule move to trigger dock widget drag'n'drop required for snapping and
                # showing rubber band and let it complete move forth...
                mouse_move_event = QMouseEvent(
                    QEvent.MouseMove,
                    self._dragging_local_pos,
                    e.globalPos() + QPoint(QApplication.startDragDistance(), 1),
                    Qt.NoButton,
                    Qt.LeftButton,
                    e.modifiers())
                QApplication.instance().postEvent(self, mouse_move_event)
                QApplication.sendPostedEvents()
                # ...and back
                mouse_move_event = QMouseEvent(
                    QEvent.MouseMove, self._dragging_local_pos, e.globalPos(),
                    Qt.NoButton, Qt.LeftButton, e.modifiers())
                QApplication.instance().postEvent(self, mouse_move_event)
                QApplication.sendPostedEvents()

                # restore attributes after repressing the button
                self.setAttribute(Qt.WA_TransparentForMouseEvents)

                self._releasing_and_repressing_while_dragging = False

        return super(DockWidget, self).event(e)

    def _get_new_parent(self, widget):
        from .dock_widget_container import DockWidgetContainer
        if isinstance(widget, DockWidgetContainer):
            if widget.isFloating():
                return None
            widget = widget.parent()
        while widget is not None:
            if isinstance(widget, DockableMainWindow):
                break
            if not callable(widget.parent):
                return None
            widget = widget.parent()
        return widget

    def _widget_at(self, global_point):
        # print '_widget_at()', global_point#, local_point
        widget = QApplication.widgetAt(global_point)
        # print '- widget', widget, (widget.objectName() if widget is not None else '')
        root_main_window = self._container_manager.get_root_main_window()

        # work around bug where root main window is detected when point is near but not inside it
        if widget == root_main_window and \
                not self._widget_contains(root_main_window, global_point):
            # print '- work around to large root main window'
            widget = None

        # work around bug where dock widget is floating and no widget is found
        if widget is None and self.isFloating():
            # determine all main windows which match the point
            overlapping = {}
            for main_window in self._main_windows:
                if self._widget_contains(main_window, global_point):
                    parent = main_window.parent()
                    is_floating = parent is None or parent.isFloating()
                    overlapping[main_window] = is_floating
            # print 'overlapping', overlapping

            if len(overlapping) == 1:
                # only found one candidate so pick it
                widget, _ = overlapping.popitem()
            elif len(overlapping) > 1:
                # try to determine which main window is the right one
                # determined docked main windows
                overlapping_docked = [mw for mw, floating in overlapping.items() if not floating]
                # print 'overlapping_docked', overlapping_docked

                # if at max one of the main windows is floating
                if len(overlapping_docked) >= len(overlapping) - 1:
                    # the floating main window is not considered because the docked ones are
                    # children of it

                    # consider all docked main windows and remove parent if both parent and child
                    # are in the list print 'considered docked main windows', overlapping_docked
                    parents = []
                    for mw1 in overlapping_docked:
                        # parent of the main window is a dock widget container
                        parent = mw1.parent()
                        if parent is None:
                            continue
                        # parent of the dock widget container is a main window
                        parent = parent.parent()
                        if parent is None:
                            continue
                        for mw2 in overlapping_docked:
                            if mw2 == parent:
                                parents.append(mw2)
                    for parent in parents:
                        overlapping_docked.remove(parent)
                    # print 'considered non-parent main windows', overlapping_docked

                    # if only one docked main window is found then pick it
                    if len(overlapping_docked) == 1:
                        # print '- pick single remaining main window'
                        widget = overlapping_docked[0]
                    else:
                        # print '- found multiple main windows - could not determine which one is
                        # on top'
                        pass
                # TODO any more heuristic possible?
                # if all remaining docked main windows have a most common ancestor use the
                # order of children() to determine topmost
        return widget

    def _widget_contains(self, widget, point):
        topleft = widget.mapToGlobal(widget.mapFromParent(widget.geometry().topLeft()))
        rect = QRect(topleft, widget.geometry().size())
        return rect.contains(point)

    def save_settings(self, settings):
        # print 'DockWidget.save_settings()', 'parent',
        # self._parent_container_serial_number(), 'settings group',
        # settings._group
        settings.set_value('parent', self._parent_container_serial_number())

        title_bar = self.titleBarWidget()
        title_bar.save_settings(settings)

    def restore_settings(self, settings):
        serial_number = settings.value('parent', None)
        # print 'DockWidget.restore_settings()', 'parent', serial_number,
        # 'settings group', settings._group
        if serial_number is not None:
            serial_number = int(serial_number)
        if self._parent_container_serial_number() != serial_number and \
                self._container_manager is not None:
            floating = self.isFloating()
            pos = self.pos()
            new_container = self._container_manager.get_container(serial_number)
            if new_container is not None:
                new_parent = new_container.main_window
            else:
                new_parent = self._container_manager.get_root_main_window()
            area = self.parent().dockWidgetArea(self)
            new_parent.addDockWidget(area, self)
            if floating:
                self.setFloating(floating)
                self.move(pos)

        title_bar = self.titleBarWidget()
        title_bar.restore_settings(settings)

    def _parent_container(self, dock_widget):
        from .dock_widget_container import DockWidgetContainer
        parent = dock_widget.parent()
        if parent is not None:
            parent = parent.parent()
            if isinstance(parent, DockWidgetContainer):
                return parent
        return None

    def _parent_container_serial_number(self):
        serial_number = None
        parent = self._parent_container(self)
        if parent is not None:
            serial_number = parent.serial_number()
        return serial_number
