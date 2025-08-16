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

from collections import OrderedDict

import os

import time

from ament_index_python import get_resource

from python_qt_binding import loadUi
from python_qt_binding.QtCore import Qt, Signal
try:
    from python_qt_binding.QtCore import (  # Qt 5
        QItemSelectionModel, QModelIndex)
except ImportError:
    from python_qt_binding.QtGui import (  # Qt 4
        QItemSelectionModel, QModelIndex)
from python_qt_binding.QtWidgets import QHeaderView, QWidget

from rqt_py_common.rqt_ros_graph import RqtRosGraph

from rqt_reconfigure import logging
from rqt_reconfigure.filter_children_model import FilterChildrenModel
from rqt_reconfigure.param_api import find_nodes_with_params
from rqt_reconfigure.param_client_widget import ParamClientWidget
from rqt_reconfigure.treenode_item_model import TreenodeItemModel
from rqt_reconfigure.treenode_qstditem import TreenodeQstdItem


class NodeSelectorWidget(QWidget):
    _COL_NAMES = ['Node']

    # public signal
    sig_node_selected = Signal(ParamClientWidget)

    def __init__(self, parent, context, signal_msg=None):
        """
        Init node selector widget.

        @param signal_msg: Signal to carries a system msg that is shown on GUI.
        @type signal_msg: QtCore.Signal
        """
        super(NodeSelectorWidget, self).__init__()
        self._parent = parent
        self.stretch = None
        self._signal_msg = signal_msg
        self._context = context

        _, package_path = get_resource('packages', 'rqt_reconfigure')
        ui_file = os.path.join(package_path, 'share', 'rqt_reconfigure', 'resource',
                               'node_selector.ui')
        loadUi(ui_file, self)

        # List of the available nodes. Since the list should be updated over
        # time and we don't want to create node instance per every update
        # cycle, This list instance should better be capable of keeping track.
        self._nodeitems = OrderedDict()
        # Dictionary. 1st elem is node's GRN name,
        # 2nd is TreenodeQstdItem instance.
        # TODO: Needs updated when nodes list updated.

        #  Setup treeview and models
        self._item_model = TreenodeItemModel()
        self._rootitem = self._item_model.invisibleRootItem()  # QStandardItem

        self._nodes_previous = None

        # Calling this method updates the list of the node.
        # Initially done only once.
        self._update_nodetree_pernode()

        # TODO(Isaac): Needs auto-update function enabled, once another
        #             function that updates node tree with maintaining
        #             collapse/expansion  state. http://goo.gl/GuwYp can be a
        #             help.

        self._collapse_button.pressed.connect(
            self._node_selector_view.collapseAll)
        self._expand_button.pressed.connect(self._node_selector_view.expandAll)
        self._refresh_button.pressed.connect(self._refresh_nodes)

        # Filtering preparation.
        self._proxy_model = FilterChildrenModel(self)
        self._proxy_model.setDynamicSortFilter(True)
        self._proxy_model.setSourceModel(self._item_model)
        self._node_selector_view.setModel(self._proxy_model)
        self._filterkey_prev = ''

        # This 1 line is needed to enable horizontal scrollbar. This setting
        # isn't available in .ui file.
        # Ref. http://stackoverflow.com/a/6648906/577001
        try:
            self._node_selector_view.header().setResizeMode(
                0, QHeaderView.ResizeToContents)  # Qt4
        except AttributeError:
            # TODO QHeaderView.setSectionResizeMode() is currently segfaulting
            # using Qt 5 with both bindings PyQt as well as PySide
            pass

        # Setting slot for when user clicks on QTreeView.
        self.selectionModel = self._node_selector_view.selectionModel()
        # Note: self.selectionModel.currentChanged doesn't work to deselect
        # a treenode as expected. Need to use selectionChanged.
        self.selectionModel.selectionChanged.connect(
            self._selection_changed_slot)

    def node_deselected(self, grn):
        """
        Deselect the index that corresponds to the given GRN.

        :type grn: str
        """
        # Obtain all indices currently selected.
        indexes_selected = self.selectionModel.selectedIndexes()
        for index in indexes_selected:
            grn_from_selectedindex = RqtRosGraph.get_upper_grn(index, '')
            logging.debug(' Compare given grn={} from selected={}'.format(
                grn, grn_from_selectedindex))
            # If GRN retrieved from selected index matches the given one.
            if grn == grn_from_selectedindex:
                # Deselect the index.
                self.selectionModel.select(index, QItemSelectionModel.Deselect)

    def node_selected(self, grn, scroll_to=False):
        """
        Select the index that corresponds to the given GRN.

        :type grn: str
        """
        # Iterate over all of the indexes
        for index in self._enumerate_indexes():
            grn_from_index = RqtRosGraph.get_upper_grn(index, '')
            logging.debug(' Compare given grn={} from selected={}'.format(
                grn, grn_from_index))
            # If GRN retrieved from selected index matches the given one.
            if grn == grn_from_index:
                # Select the index.
                self.selectionModel.select(index, QItemSelectionModel.Select)
                if scroll_to:
                    self._node_selector_view.scrollTo(index)
                break

    def _enumerate_indexes(self, parent=QModelIndex()):
        model = self.selectionModel.model()
        for row in range(0, model.rowCount(parent)):
            index = model.index(row, 0, parent)
            yield index
            if model.hasChildren(index):
                for child in self._enumerate_indexes(index):
                    yield child

    def _selection_deselected(self, index_current, rosnode_name_selected):
        # Intended to be called from _selection_changed_slot.
        self.selectionModel.select(index_current, QItemSelectionModel.Deselect)

        try:
            param_client_widget = self._nodeitems[
                rosnode_name_selected].get_param_client_widget()
        except Exception as e:
            raise e

        # Signal to notify other pane that also contains node widget.
        self.sig_node_selected.emit(param_client_widget)

    def _selection_selected(self, index_current, rosnode_name_selected):
        # Intended to be called from _selection_changed_slot.
        logging.debug('_selection_changed_slot row={} col={} data={}'.format(
            index_current.row(), index_current.column(),
            index_current.data(Qt.DisplayRole)))

        # Determine if it's terminal treenode.
        found_node = False
        for nodeitem in self._nodeitems.values():
            name_nodeitem = nodeitem.data(Qt.DisplayRole)
            name_rosnode_leaf = rosnode_name_selected[
                rosnode_name_selected.rfind(RqtRosGraph.DELIM_GRN) + 1:]

            # If name of the leaf in the given name & the name taken from
            # nodeitem list matches.
            if ((name_nodeitem == rosnode_name_selected) and
                (name_nodeitem[
                    name_nodeitem.rfind(RqtRosGraph.DELIM_GRN) + 1:] ==
                    name_rosnode_leaf)):

                logging.debug('terminal str {} MATCH {}'.format(
                    name_nodeitem, name_rosnode_leaf))
                found_node = True
                break
        if not found_node:  # Only when it's NOT a terminal we deselect it.
            self.selectionModel.select(index_current,
                                       QItemSelectionModel.Deselect)
            return

        # Only when it's a terminal we move forward.

        item_child = self._nodeitems[rosnode_name_selected]
        item_widget = None
        try:
            item_widget = item_child.get_param_client_widget()
        except Exception as e:
            raise e
        logging.debug('item_selected={} child={} widget={}'.format(
                      index_current, item_child, item_widget))
        self.sig_node_selected.emit(item_widget)

        # Show the node as selected.
        # selmodel.select(index_current, QItemSelectionModel.SelectCurrent)

    def _selection_changed_slot(self, selected, deselected):
        """
        Send "open ROS Node box" signal.

        ONLY IF the selected treenode is the
        terminal treenode.
        Receives args from signal QItemSelectionModel.selectionChanged.

        :param selected: All indexs where selected (could be multiple)
        :type selected: QItemSelection
        :type deselected: QItemSelection
        """
        # Getting the index where user just selected. Should be single.
        if not selected.indexes() and not deselected.indexes():
            logging.error('Nothing selected? Not ideal to reach here')
            return

        index_current = None
        if selected.indexes():
            index_current = selected.indexes()[0]
        elif len(deselected.indexes()) == 1:
            # Setting length criteria as 1 is only a workaround, to avoid
            # Node boxes on right-hand side disappears when filter key doesn't
            # match them.
            # Indeed this workaround leaves another issue. Question for
            # permanent solution is asked here http://goo.gl/V4DT1
            index_current = deselected.indexes()[0]

        logging.debug('  - - - index_current={}'.format(index_current))

        rosnode_name_selected = RqtRosGraph.get_upper_grn(index_current, '')

        # If retrieved node name isn't in the list of all nodes.
        if rosnode_name_selected not in self._nodeitems.keys():
            # De-select the selected item.
            self.selectionModel.select(index_current,
                                       QItemSelectionModel.Deselect)
            return

        if selected.indexes():
            try:
                self._selection_selected(index_current, rosnode_name_selected)
            except Exception as e:
                # TODO: print to sysmsg pane
                err_msg = 'Connection to node={} failed:\n{}'.format(
                    rosnode_name_selected, e
                )
                import traceback
                traceback.print_exc()
                self._signal_msg.emit(err_msg)
                logging.error(err_msg)

        elif deselected.indexes():
            try:
                self._selection_deselected(index_current,
                                           rosnode_name_selected)
            except Exception as e:
                self._signal_msg.emit(e)
                logging.error(e)

    def get_nodeitems(self):
        """
        Get node items.

        :rtype: OrderedDict 1st elem is node's GRN name,
                2nd is TreenodeQstdItem instance
        """
        return self._nodeitems

    def _update_nodetree_pernode(self):

        # TODO(Isaac): 11/25/2012 dynamic_reconfigure only returns params that
        #             are associated with nodes. In order to handle independent
        #             params, different approach needs taken.
        try:
            nodes = find_nodes_with_params(self._context.node)
        except Exception as e:
            self._logger.error(e)
            # TODO: print to sysmsg pane
            raise e  # TODO Make sure 'raise' here returns or finalizes  func.

        if not nodes == self._nodes_previous:
            i_node_curr = 1
            num_nodes = len(nodes)
            elapsedtime_overall = 0.0
            for node_name_grn in nodes:
                # Skip this grn if we already have it
                if node_name_grn in self._nodeitems:
                    i_node_curr += 1
                    continue

                time_siglenode_loop = time.time()

                # (Begin) For DEBUG ONLY; skip some dynreconf creation
                # if i_node_curr % 2 != 0:
                #     i_node_curr += 1
                #     continue
                # (End) For DEBUG ONLY. ####

                # Instantiate QStandardItem. Inside, dyn_reconf client will
                # be generated too.
                treenodeitem_toplevel = TreenodeQstdItem(
                    self._context, node_name_grn,
                    TreenodeQstdItem.NODE_FULLPATH
                )
                _treenode_names = treenodeitem_toplevel.get_treenode_names()

                # Using OrderedDict here is a workaround for StdItemModel
                # not returning corresponding item to index.
                self._nodeitems[node_name_grn] = treenodeitem_toplevel

                self._add_children_treenode(treenodeitem_toplevel,
                                            self._rootitem, _treenode_names)

                time_siglenode_loop = time.time() - time_siglenode_loop
                elapsedtime_overall += time_siglenode_loop

                _str_progress = 'reconf ' + \
                    'loading #{}/{} {} / {}sec node={}'.format(
                        i_node_curr, num_nodes, round(time_siglenode_loop, 2),
                        round(elapsedtime_overall, 2), node_name_grn
                    )

                # NOT a debug print - please DO NOT remove. This print works
                # as progress notification when loading takes long time.
                logging.debug(_str_progress)
                i_node_curr += 1

    def _add_children_treenode(self, treenodeitem_toplevel,
                               treenodeitem_parent, child_names_left):
        """
        Add childen treenode.

        Evaluate current treenode and the previous treenode at the same depth.
        If the name of both nodes is the same, current node instance is
        ignored (that means children will be added to the same parent). If not,
        the current node gets added to the same parent node. At the end, this
        function gets called recursively going 1 level deeper.

        :type treenodeitem_toplevel: TreenodeQstdItem
        :type treenodeitem_parent: TreenodeQstdItem.
        :type child_names_left: List of str
        :param child_names_left: List of strings that is sorted in hierarchical
                                 order of params.
        """
        # TODO(Isaac): Consider moving this method to rqt_py_common.

        name_currentnode = child_names_left.pop(0)
        grn_curr = treenodeitem_toplevel.get_raw_param_name()
        stditem_currentnode = TreenodeQstdItem(self._context, grn_curr,
                                               TreenodeQstdItem.NODE_FULLPATH)

        # item at the bottom is your most recent node.
        row_index_parent = treenodeitem_parent.rowCount() - 1

        # Obtain and instantiate prev node in the same depth.
        name_prev = ''
        stditem_prev = None
        if treenodeitem_parent.child(row_index_parent):
            stditem_prev = treenodeitem_parent.child(row_index_parent)
            name_prev = stditem_prev.text()

        stditem = None
        # If the name of both nodes is the same, current node instance is
        # ignored (that means children will be added to the same parent)
        if name_prev != name_currentnode:
            stditem_currentnode.setText(name_currentnode)

            # Arrange alphabetically by display name
            insert_index = 0
            while (insert_index < treenodeitem_parent.rowCount() and
                    treenodeitem_parent.child(insert_index)
                    .text() < name_currentnode):
                insert_index += 1

            treenodeitem_parent.insertRow(insert_index, stditem_currentnode)
            stditem = stditem_currentnode
        else:
            stditem = stditem_prev

        if child_names_left:
            # TODO: Model is closely bound to a certain type of view (treeview)
            # here. Ideally isolate those two. Maybe we should split into 2
            # class, 1 handles view, the other does model.
            self._add_children_treenode(treenodeitem_toplevel, stditem,
                                        child_names_left)
        else:  # Selectable ROS Node.
            # TODO: Accept even non-terminal treenode as long as it's ROS Node.
            self._item_model.set_item_from_index(grn_curr, stditem.index())

    def _prune_nodetree_pernode(self):
        try:
            nodes = find_nodes_with_params(self._context.node)
        except Exception as e:
            logging.error('Reconfigure GUI cannot connect to master.')
            raise e  # TODO Make sure 'raise' here returns or finalizes func.

        for i in reversed(range(0, self._rootitem.rowCount())):
            candidate_for_removal = \
                self._rootitem.child(i).get_raw_param_name()
            if candidate_for_removal not in nodes:
                logging.debug(
                    'Removing {} because the server is no longer available.'.
                    format(candidate_for_removal))
                self._rootitem.removeRow(i)
                self._nodeitems.pop(candidate_for_removal).reset()

    def _refresh_nodes(self):
        self._prune_nodetree_pernode()
        self._update_nodetree_pernode()

    def set_filter(self, filter_):
        """
        Pass fileter instance to the child proxymodel.

        :type filter_: BaseFilter
        """
        self._proxy_model.set_filter(filter_)

    def _test_sel_index(self, selected, deselected):
        # Method for Debug only.

        # index_current = self.selectionModel.currentIndex()
        src_model = self._item_model
        index_current = None
        index_deselected = None
        index_parent = None
        curr_qstd_item = None
        if selected.indexes():
            index_current = selected.indexes()[0]
            index_parent = index_current.parent()
            curr_qstd_item = src_model.itemFromIndex(index_current)
        elif deselected.indexes():
            index_deselected = deselected.indexes()[0]
            index_parent = index_deselected.parent()
            curr_qstd_item = src_model.itemFromIndex(index_deselected)

        if selected.indexes() > 0:
            logging.debug(
                'sel={} par={} desel={} '
                'sel.d={} par.d={} desel.d={} cur.item={}'
                .format(
                    index_current, index_parent, index_deselected,
                    index_current.data(Qt.DisplayRole),
                    index_parent.data(Qt.DisplayRole),
                    None,  # index_deselected.data(Qt.DisplayRole)
                    curr_qstd_item))
        elif deselected.indexes():
            logging.debug(
                'sel={} par={} desel={} '
                'sel.d={} par.d={} desel.d={} cur.item={}'
                .format(
                    index_current, index_parent, index_deselected,
                    None, index_parent.data(Qt.DisplayRole),
                    index_deselected.data(Qt.DisplayRole),
                    curr_qstd_item))

    def save_settings(self, instance_settings):
        expanded_nodes = []
        for index in self._enumerate_indexes():
            if self._node_selector_view.isExpanded(index):
                grn = RqtRosGraph.get_upper_grn(index, '')
                if grn:
                    expanded_nodes.append(grn)
        instance_settings.set_value('expanded_nodes', expanded_nodes)

    def restore_settings(self, instance_settings):
        expanded_nodes = instance_settings.value('expanded_nodes', [])
        if expanded_nodes:
            for index in self._enumerate_indexes():
                if RqtRosGraph.get_upper_grn(index, '') in expanded_nodes:
                    self._node_selector_view.setExpanded(index, True)
