# Software License Agreement (BSD License)
#
# Copyright (c) 2008, Willow Garage, Inc.
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
#
# Revision $Id$
# Adopted from the ROS1 ros_comm repository
# https://github.com/ros/ros_comm/blob/5de058ad1bbf3a525ae3f5288a76e59ec0dd62d0/tools/rosgraph/src/rosgraph/impl/graph.py

from __future__ import print_function

"""
Data structures and library for representing ROS Computation Graph state.
"""

import sys
import time
import itertools
import random
import logging
import traceback
import socket

from collections import defaultdict

from python_qt_binding.QtCore import qDebug, qWarning
from rclpy import logging
from rclpy.qos import qos_check_compatible
from rclpy.qos import QoSCompatibility


_ROS_NAME = '/rosviz'


def topic_node(topic):
    """
    In order to prevent topic/node name aliasing, we have to remap
    topic node names. Currently we just prepend a space, which is
    an illegal ROS name and thus not aliased.
    @return str: topic mapped to a graph node name.
    """
    return ' ' + topic


def node_topic(node):
    """
    Inverse of topic_node
    @return str: undo topic_node() operation
    """
    return node[1:]


class BadNode(object):
    """
    Data structure for storing info about a 'bad' node
    """

    # no connectivity
    DEAD = 0
    # intermittent connectivity
    WONKY = 1

    def __init__(self, name, node_type, reason):
        """
        @param type: DEAD | WONKY
        @type  type: int
        """
        self.name = name
        self.reason = reason
        self.type = node_type


class EdgeList(object):
    """
    Data structure for storing Edge instances
    """
    __slots__ = ['edges_by_start', 'edges_by_end']

    def __init__(self):
        # in order to make it easy to purge edges, we double-index them
        self.edges_by_start = {}
        self.edges_by_end = {}

    def __iter__(self):
        return itertools.chain(*[v for v in self.edges_by_start.values()])

    def has(self, edge):
        return edge in self

    def __contains__(self, edge):
        """
        @return: True if edge is in edge list
        @rtype: bool
        """
        key = edge.key
        return key in self.edges_by_start and edge in self.edges_by_start[key]

    def add(self, edge):
        """
        Add an edge to our internal representation. not multi-thread safe
        @param edge: edge to add
        @type  edge: Edge
        """
        # see note in __init__
        def update_map(edge_map, key, edge):
            if key in edge_map:
                edges = edge_map[key]
                if edge not in edges:
                    edges.append(edge)
                    return True
                else:
                    return False
            else:
                edge_map[key] = [edge]
                return True

        updated = update_map(self.edges_by_start, edge.key, edge)
        updated = update_map(self.edges_by_end, edge.rkey, edge) or updated
        return updated

    def add_edges(self, start, dest, direction, label='', qos=None):
        """
        Create Edge instances for args and add resulting edges to edge
        list. Convenience method to avoid repetitve logging, etc...
        @param edge_list: data structure to add edge to
        @type  edge_list: EdgeList
        @param start: name of start node. If None, warning will be logged and add fails
        @type  start: str
        @param dest: name of start node. If None, warning will be logged and add fails
        @type  dest: str
        @param direction: direction string (i/o/b)
        @type  direction: str
        @return: True if update occured
        @rtype: bool
        """

        # the warnings should generally be temporary, occuring of the
        # master/node information becomes stale while we are still
        # doing an update
        updated = False
        if not start:
            qWarning("cannot add edge: cannot map start [%s] to a node name", start)
        elif not dest:
            qWarning("cannot add edge: cannot map dest [%s] to a node name", dest)
        else:
            for args in edge_args(start, dest, direction, label, qos):
                updated = self.add(Edge(*args)) or updated
        return updated

    def delete_all(self, node):
        """
        Delete all edges that start or end at node
        @param node: name of node
        @type  node: str
        """
        def matching(map, pref):
            return [map[k] for k in map.keys() if k.startswith(pref)]

        pref = node+"|"
        edge_lists = matching(self.edges_by_start, pref) + matching(self.edges_by_end, pref)
        for el in edge_lists:
            for e in el:
                self.delete(e)

    def delete(self, edge):
        # see note in __init__
        def update_map(map, key, edge):
            if key in map:
                edges = map[key]
                if edge in edges:
                    edges.remove(edge)
                    return True
        update_map(self.edges_by_start, edge.key, edge)
        update_map(self.edges_by_end, edge.rkey, edge)


class Edge(object):
    """
    Data structure for representing ROS node graph edge
    """

    __slots__ = ['start', 'end', 'label', 'key', 'rkey', 'qos']

    def __init__(self, start, end, label='', qos=None):
        self.start = start
        self.end = end
        self.label = label
        self.qos = qos
        self.key = "%s|%s" % (self.start, self.label)
        # reverse key, indexed from end
        self.rkey = "%s|%s" % (self.end, self.label)

    def __ne__(self, other):
        return self.start != other.start or self.end != other.end

    def __str__(self):
        return "%s->%s" % (self.start, self.end)

    def __eq__(self, other):
        return self.start == other.start and self.end == other.end and \
            self.label == other.label and self.qos == other.qos


def edge_args(start, dest, direction, label, qos):
    """
    compute argument ordering for Edge constructor based on direction flag
    @param direction str: 'i', 'o', or 'b' (in/out/bidir) relative to \a start
    @param start str: name of starting node
    @param start dest: name of destination node
    """
    edge_args = []
    if direction in ['o', 'b']:
        edge_args.append((start, dest, label, qos))
    if direction in ['i', 'b']:
        edge_args.append((dest, start, label, qos))
    return edge_args


class Graph(object):
    """
    Utility class for polling ROS statistics from running ROS graph.
    Not multi-thread-safe
    """

    def __init__(self, node, node_ns='/', topic_ns='/'):
        self._node = node
        self.node_ns = node_ns or '/'
        self.topic_ns = topic_ns or '/'

        # ROS nodes
        self.nn_nodes = set([])
        # ROS topic nodes
        self.nt_nodes = set([])

        # ROS nodes that aren't responding quickly
        self.bad_nodes = {}
        import threading
        self.bad_nodes_lock = threading.Lock()

        # ROS services
        self.srvs = set([])
        # ROS node->node transport connections
        self.nn_edges = EdgeList()
        # ROS node->topic connections
        self.nt_edges = EdgeList()
        # ROS all node->topic connections, including empty
        self.nt_all_edges = EdgeList()

        # node names to URI map
        self.node_uri_map = {}  # { node_name_str : uri_str }
        # reverse map URIs to node names
        self.uri_node_map = {}  # { uri_str : node_name_str }

        # time we last updated the graph
        self.last_graph_refresh = 0
        self.last_node_refresh = {}

        self.graph_stale = 5.0
        # time we last communicated with node
        # seconds until node data is considered stale
        self.node_stale = 5.0  # seconds

        # dictionary with qos incompatibilities found
        # {topic_name: {publisher_node_name: [subscription_node_name, ...], ...}, ...}
        self.topic_with_qos_incompatibility = defaultdict(lambda: defaultdict(list))

    def set_node_stale(self, stale_secs):
        """
        @param stale_secs: seconds that data is considered fresh
        @type  stale_secs: double
        """
        self.node_stale = stale_secs

    def _graph_refresh(self):
        """
        @return: True if nodes information was updated
        @rtype: bool
        """
        updated = False
        publishers = defaultdict(list)
        subscriptions = defaultdict(list)
        servers = defaultdict(list)

        publisher_topic_names = set()
        subscriber_topic_names = set()

        for name, namespace in self._node.get_node_names_and_namespaces():
            node_name = namespace + name if namespace.endswith('/') else namespace + '/' + name

            for topic_name, topic_type in \
                    self._node.get_publisher_names_and_types_by_node(name, namespace):
                publishers[topic_name].append(node_name)
                publisher_topic_names.add(topic_name)

            for topic_name, topic_type in \
                    self._node.get_subscriber_names_and_types_by_node(name, namespace):
                subscriptions[topic_name].append(node_name)
                subscriber_topic_names.add(topic_name)

            for service_name, service_type in \
                    self._node.get_service_names_and_types_by_node(name, namespace):
                servers[service_name].append(node_name)

        publisher_qos_lists = defaultdict(lambda: defaultdict(list))
        for topic_name in publisher_topic_names:
            for topic_endpoint_info in self._node.get_publishers_info_by_topic(topic_name):
                node_name = topic_endpoint_info.node_namespace
                if not node_name.endswith('/'):
                    node_name += '/'
                node_name += topic_endpoint_info.node_name
                publisher_qos_lists[topic_name][node_name].append(topic_endpoint_info.qos_profile)
        subscriber_qos_lists = defaultdict(lambda: defaultdict(list))
        for topic_name in subscriber_topic_names:
            for topic_endpoint_info in self._node.get_subscriptions_info_by_topic(topic_name):
                node_name = topic_endpoint_info.node_namespace
                if not node_name.endswith('/'):
                    node_name += '/'
                node_name += topic_endpoint_info.node_name
                subscriber_qos_lists[topic_name][node_name].append(topic_endpoint_info.qos_profile)

        for topic, node_qos_profiles_dict in publisher_qos_lists.items():
            for pub_node, pub_qos_list in node_qos_profiles_dict.items():
                for sub_node, sub_qos_list in subscriber_qos_lists[topic].items():
                    if any(
                        qos_check_compatible(pub_qos, sub_qos)[0] == QoSCompatibility.ERROR
                        for pub_qos in pub_qos_list
                        for sub_qos in sub_qos_list
                    ):
                        self.topic_with_qos_incompatibility[topic][pub_node].append(sub_node)

        pubs = list(publishers.items())
        subs = list(subscriptions.items())
        srvs = list(servers.items())

        nodes = []
        nt_all_edges = EdgeList()
        nt_nodes = self.nt_nodes
        for state, direction in ((pubs, 'o'), (subs, 'i')):
            for topic, l in state:
                if topic.startswith(self.topic_ns):
                    nodes.extend([n for n in l if n.startswith(self.node_ns)])
                    nt_nodes.add(topic_node(topic))
                    for node in l:
                        if direction == 'o':
                            qos_list = publisher_qos_lists[topic][node]
                        elif direction == 'i':
                            qos_list = subscriber_qos_lists[topic][node]
                        else:
                            qos_list = {None}
                        for qos in qos_list:
                            updated |= nt_all_edges.add_edges(
                                node, topic_node(topic), direction, qos=qos)
        self.nt_nodes = nt_nodes
        self.nt_all_edges = nt_all_edges
        self.nt_edges = nt_all_edges

        nn_edges = EdgeList()
        for topic, pub_nodes in publishers.items():
            for pub_node, sub_node in itertools.product(pub_nodes, subscriptions.get(topic, [])):
                updated = nn_edges.add_edges(pub_node, sub_node, 'o', topic) or updated
        self.nn_edges = nn_edges

        nodes = set(nodes)

        srvs = set([s for s, _ in srvs])
        purge = None
        if nodes ^ self.nn_nodes:
            purge = self.nn_nodes - nodes
            self.nn_nodes = nodes
            updated = True
        if srvs ^ self.srvs:
            self.srvs = srvs
            updated = True

        if purge:
            qDebug("following nodes and related edges will be purged: %s", ','.join(purge))
            for p in purge:
                self.nn_edges.delete_all(p)
                self.nt_edges.delete_all(p)
                self.nt_all_edges.delete_all(p)

        qDebug("Graph refresh: done, updated[%s]" % updated)
        return updated

    def _mark_bad_node(self, node, reason):
        try:
            # bad nodes are updated in a separate thread, so lock
            self.bad_nodes_lock.acquire()
            if node in self.bad_nodes:
                self.bad_nodes[node].type = BadNode.DEAD
            else:
                self.bad_nodes[node] = (BadNode(node, BadNode.DEAD, reason))
        finally:
            self.bad_nodes_lock.release()

    def _unmark_bad_node(self, node, reason):
        """
        Promotes bad node to 'wonky' status.
        """
        try:
            # bad nodes are updated in a separate thread, so lock
            self.bad_nodes_lock.acquire()
            bad = self.bad_nodes[node]
            bad.type = BadNode.WONKY
        finally:
            self.bad_nodes_lock.release()

    def _node_refresh_businfo(self, node, bad_node=False):
        """
        Retrieve bus info from the node and update nodes and edges as appropriate
        @param node: node name
        @type  node: str
        @param api: XML-RPC proxy
        @type  api: ServerProxy
        @param bad_node: If True, node has connectivity issues and
        should be treated differently
        @type  bad_node: bool
        """
        if bad_node:
            self._mark_bad_node(node, 'Not found during discovery')
        return True

    def _node_refresh(self, node, bad_node=False):
        """
        Contact node for stats/connectivity information
        @param node: name of node to contact
        @type  node: str
        @param bad_node: if True, node has connectivity issues
        @type  bad_node: bool
        @return: True if node was successfully contacted
        @rtype  bool
        """
        # TODO: I'd like for master to provide this information in
        # getSystemState() instead to prevent the extra connection per node
        updated = False
        uri = self._node_uri_refresh(node)

        bad_node = (uri is None)
        updated = self._node_refresh_businfo(node, bad_node)
        return updated

    def _node_uri_refresh(self, node):
        current_nodes = {
            namespace + ('' if namespace.endswith('/') else '/') + name
            for name, namespace in self._node.get_node_names_and_namespaces()}
        if node not in current_nodes:
            qWarning('Node "{}" does not exist'.format(node))
            return None
        return node

    def bad_update(self):
        """
        Update loop for nodes with bad connectivity. We box them separately
        so that we can maintain the good performance of the normal update loop.
        Once a node is on the bad list it stays there.
        """
        last_node_refresh = self.last_node_refresh

        # nodes left to check
        try:
            self.bad_nodes_lock.acquire()
            # make copy due to multithreading
            update_queue = self.bad_nodes.values()[:]
        finally:
            self.bad_nodes_lock.release()

        # return value. True if new data differs from old
        updated = False
        # number of nodes we checked
        num_nodes = 0

        start_time = time.time()
        while update_queue:
            # figure out the next node to contact
            next = update_queue.pop()
            # rate limit talking to any particular node
            if time.time() > (last_node_refresh.get(next, 0.0) + self.node_stale):
                updated = self._node_refresh(next.name, True) or updated
                # include in random offset (max 1/5th normal update interval)
                # to help spread out updates
                last_node_refresh[next] = time.time() + (random.random() * self.node_stale / 5.0)
                num_nodes += 1

            # small yield to keep from torquing the processor
            time.sleep(0.01)
        end_time = time.time()
        qDebug("ROS stats (bad nodes) update took %ss" % (end_time - start_time))
        return updated

    def update(self):
        """
        Update all the stats. This method may take awhile to complete as it will
        communicate with all nodes + master.
        """

        last_node_refresh = self.last_node_refresh

        # nodes left to check
        update_queue = None
        # True if there are still more stats to fetch this cycle
        work_to_do = True
        # return value. True if new data differs from old
        updated = False
        # number of nodes we checked
        num_nodes = 0

        start_time = time.time()
        while work_to_do:

            # each time through the loop try to talk to either the master
            # or a node. stop when we have talked to everybody.

            # get a new node list from the master
            if time.time() > (self.last_graph_refresh + self.graph_stale):
                updated = self._graph_refresh()
                self.last_graph_refresh = time.time()
            # contact the nodes for stats
            else:
                # initialize update_queue based on most current nodes list
                if update_queue is None:
                    update_queue = list(self.nn_nodes)
                # no nodes left to contact
                elif not update_queue:
                    work_to_do = False
                # contact next node
                else:
                    # figure out the next node to contact
                    next = update_queue.pop()
                    # rate limit talking to any particular node
                    if time.time() > (last_node_refresh.get(next, 0.0) + self.node_stale):
                        updated = self._node_refresh(next) or updated
                        # include in random offset (max 1/5th normal update interval)
                        # to help spread out updates
                        last_node_refresh[next] = \
                            time.time() + (random.random() * self.node_stale / 5.0)
                        num_nodes += 1

            # small yield to keep from torquing the processor
            time.sleep(0.01)
        end_time = time.time()
        qDebug("ROS stats update took %ss" % (end_time - start_time))
        return updated
