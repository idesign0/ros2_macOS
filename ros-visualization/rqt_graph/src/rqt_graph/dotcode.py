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

# this is a modified version of rx/rxgraph/src/rxgraph/dotcode.py

import re
import copy

from rqt_graph import rosgraph2_impl
import math

import pydot

try:
    unicode
    # we're on python2, or the "unicode" function has already been defined elsewhere
except NameError:
    unicode = str
    # we're on python3

# node/node connectivity
NODE_NODE_GRAPH = 'node_node'
# node/topic connections where an actual network connection exists
NODE_TOPIC_GRAPH = 'node_topic'
# all node/topic connections, even if no actual network connection
NODE_TOPIC_ALL_GRAPH = 'node_topic_all'

QUIET_NAMES = ['/diag_agg', '/runtime_logger', '/pr2_dashboard', '/rviz',
               '/rosout', '/cpu_monitor', '/monitor', '/hd_monitor',
               '/rxloggerlevel', '/clock', '/rqt', '/statistics']


def _conv(n):
    """Convert a node name to a valid dot name, which can't contain the leading space"""
    if n.startswith(' '):
        return 't_' + n[1:]
    else:
        return 'n_' + n


def matches_any(name, patternlist):
    if patternlist is None or len(patternlist) == 0:
        return False
    for pattern in patternlist:
        if unicode(name).strip() == pattern:
            return True
        if re.match("^[a-zA-Z0-9_/]+$", pattern) is None:
            if re.match(unicode(pattern), name.strip()) is not None:
                return True
    return False


class NodeConnections:

    def __init__(self, incoming=None, outgoing=None):
        self.incoming = incoming or []
        self.outgoing = outgoing or []


class RosGraphDotcodeGenerator:

    # topic/topic -> graph.edge object
    edges = dict([])

    # ROS node name -> graph.node object
    nodes = dict([])

    def __init__(self, node):
        self._node = node

    def _get_max_traffic(self):
        traffic = 10000  # start at 10kb
        for sub in self.edges:
            for topic in self.edges[sub]:
                for pub in self.edges[sub][topic]:
                    traffic = max(traffic, self.edges[sub][topic][pub].traffic)
        return traffic

    def _get_max_age(self):
        age = 0.1  # start at 100ms
        for sub in self.edges:
            for topic in self.edges[sub]:
                for pub in self.edges[sub][topic]:
                    age = max(age, self.edges[sub][topic][pub].stamp_age_mean.to_sec())
        return age

    def _get_max_age_on_topic(self, sub, topic):
        age = 0.0
        for pub in self.edges[sub][topic]:
            age = max(age, self.edges[sub][topic][pub].stamp_age_mean.to_sec())
        return age

    def _calc_edge_color(self, sub, topic, pub=None):

        age = 0.0

        if pub is None:
            age = self._get_max_age_on_topic(sub, topic)
        elif sub in self.edges and topic in self.edges[sub] and pub in self.edges[sub][topic]:
            age = self.edges[sub][topic][pub].stamp_age_mean.to_sec()

        if age == 0.0:
            return [0, 0, 0]

        # calc coloring using the age
        heat = max(age, 0) / self._get_max_age()

        # we assume that heat is normalized between 0.0 (green) and 1.0 (red)
        # 0.0->green(0,255,0) to 0.5->yellow (255,255,0) to red 1.0(255,0,0)
        if heat < 0:
            red = 0
            green = 0
        elif heat <= 0.5:
            red = int(heat * 255 * 2)
            green = 255
        elif heat > 0.5:
            red = 255
            green = 255 - int((heat - 0.5) * 255 * 2)
        else:
            red = 0
            green = 0
        return [red, green, 0]

    def _calc_edge_penwidth(self, sub, topic, pub=None):
        if pub is None and sub in self.edges and topic in self.edges[sub]:
            traffic = 0
            for p in self.edges[sub][topic]:
                if pub is None or p == pub:
                    traffic += self.edges[sub][topic][p].traffic

            # calc penwidth using the traffic in kb/s
            return int(traffic / self._get_max_traffic() * 5)
        else:
            return 1

    def _calc_statistic_info(self, sub, topic, pub=None):
        if pub is None and sub in self.edges and topic in self.edges[sub]:
            conns = len(self.edges[sub][topic])
            if conns == 1:
                pub = next(iter(self.edges[sub][topic].keys()))
            else:
                penwidth = self._calc_edge_penwidth(sub, topic)
                color = self._calc_edge_color(sub, topic)
                label = "(" + unicode(conns) + " connections)"
                return [label, penwidth, color]

        if sub in self.edges and topic in self.edges[sub] and pub in self.edges[sub][topic]:
            penwidth = self._calc_edge_penwidth(sub, topic, pub)
            color = self._calc_edge_color(sub, topic, pub)
            period = self.edges[sub][topic][pub].period_mean.to_sec()
            if period > 0.0:
                freq = unicode(round(1.0 / period, 1))
            else:
                freq = "?"
            age = self.edges[sub][topic][pub].stamp_age_mean.to_sec()
            age_string = ""
            if age > 0.0:
                age_string = " // " + unicode(round(age, 2) * 1000) + " ms"
            label = freq + " Hz" + age_string
            return [label, penwidth, color]
        else:
            return [None, None, None]

    def _add_edge(self, edge, dotcode_factory, dotgraph, is_topic=False):
        if is_topic:
            sub = edge.end
            topic = edge.label
            pub = edge.start
            [stat_label, penwidth, color] = self._calc_statistic_info(sub, topic, pub)
            if stat_label is not None:
                temp_label = edge.label + "\\n" + stat_label
                dotcode_factory.add_edge_to_graph(
                    dotgraph,
                    _conv(edge.start),
                    _conv(edge.end),
                    label=temp_label,
                    url='topic:%s' % edge.label,
                    penwidth=penwidth,
                    color=color)
            else:
                dotcode_factory.add_edge_to_graph(
                    dotgraph,
                    _conv(edge.start),
                    _conv(edge.end),
                    label=edge.label,
                    url='topic:%s' % edge.label)
        else:
            sub = edge.end.strip()
            topic = edge.start.strip()
            [stat_label, penwidth, color] = self._calc_statistic_info(sub, topic)
            if stat_label is not None:
                temp_label = edge.label + "\\n" + stat_label
                dotcode_factory.add_edge_to_graph(
                    dotgraph,
                    _conv(edge.start),
                    _conv(edge.end),
                    label=temp_label,
                    penwidth=penwidth,
                    color=color,
                    edgetooltip=self._qos_to_string(edge.start, edge.end, edge.qos))
            else:
                dotcode_factory.add_edge_to_graph(
                    dotgraph,
                    _conv(edge.start),
                    _conv(edge.end),
                    label=edge.label,
                    edgetooltip=self._qos_to_string(edge.start, edge.end, edge.qos))

    def _qos_to_string(self, start, end, qos):
        if qos is None:
            return None
        # topic names are prefixed with an space, unlike node names
        # see rosgraph2_impl.topic_node()
        if start[0] == ' ':
            topic_name = start[1:]
            node_name = end
            endpoint = 'Subscription'
        else:
            topic_name = end[1:]
            node_name = start
            endpoint = 'Publisher'
        s = f'{endpoint}\nnode_name: {node_name}\ntopic_name: {topic_name}\nQoS settings'
        for slot_name in qos.__slots__:
            property_name = slot_name[1:]
            # ignore values currently not introspectable
            if property_name in ('history', 'depth'):
                continue
            if not hasattr(qos, property_name):
                continue
            value = getattr(qos, property_name)
            if hasattr(value, 'short_key'):
                value = value.short_key
            elif hasattr(value, 'nanoseconds'):
                value = str(value.nanoseconds) + ' ns'
            s += f'\n- {property_name}: {value}'
        return s

    def _add_node(self, node, rosgraphinst, dotcode_factory, dotgraph, unreachable):
        if node in rosgraphinst.bad_nodes:
            if unreachable:
                return ''
            bn = rosgraphinst.bad_nodes[node]
            if bn.type == rosgraph2_impl.BadNode.DEAD:
                dotcode_factory.add_node_to_graph(
                    dotgraph,
                    nodename=_conv(node),
                    nodelabel=node,
                    shape="ellipse",
                    url=node + " (DEAD)",
                    color="red")
            elif bn.type == rosgraph2_impl.BadNode.WONKY:
                dotcode_factory.add_node_to_graph(
                    dotgraph,
                    nodename=_conv(node),
                    nodelabel=node,
                    shape="ellipse",
                    url=node + " (WONKY)",
                    color="orange")
            else:
                dotcode_factory.add_node_to_graph(
                    dotgraph,
                    nodename=_conv(node),
                    nodelabel=node,
                    shape="ellipse",
                    url=node + " (UNKNOWN)",
                    color="red")
        else:
            dotcode_factory.add_node_to_graph(
                dotgraph,
                nodename=_conv(node),
                nodelabel=node,
                shape='ellipse',
                url=node)

    def _add_topic_node(self, node, rosgraphinst, dotcode_factory, dotgraph, quiet):
        label = rosgraph2_impl.node_topic(node)
        color = None
        tooltip = None
        if label in rosgraphinst.topic_with_qos_incompatibility:
            color = 'red'
            tooltip_split = ['Found qos incompatibilities:', '']
            tooltip_split.extend([
                f'- Publisher of node `{pub_node}` '
                f'incompatible with subscriptions of nodes `{", ".join(sub_node_list)}`'
                for pub_node, sub_node_list in
                rosgraphinst.topic_with_qos_incompatibility[label].items()])
            tooltip_split.extend(['', f'Use `ros2 topic info -v {label}` to check the qos profiles'])
            tooltip = '<br/>'.join(tooltip_split)
        dotcode_factory.add_node_to_graph(
            dotgraph,
            nodename=_conv(node),
            nodelabel=label,
            shape='box',
            url="topic:%s" % label,
            color=color,
            tooltip=tooltip)

    def _add_topic_node_group(self, node, dotcode_factory, dotgraph, quiet):
        label = rosgraph2_impl.node_topic(node)
        dotcode_factory.add_node_to_graph(
            dotgraph,
            nodename=_conv(node),
            nodelabel=label,
            shape='box3d',
            url='topic:%s' % label)

    def _quiet_filter(self, name):
        # ignore viewers
        for n in QUIET_NAMES:
            if n in name:
                return False
        if name.rsplit('/', 1)[-1].startswith('_'):
            return False
        return True

    def quiet_filter_topic_edge(self, edge):
        for quiet_label in ['/time', '/clock', '/rosout', '/statistics']:
            if quiet_label == edge.label:
                return False
        return self._quiet_filter(edge.start) and self._quiet_filter(edge.end)

    def generate_namespaces(self, graph, graph_mode, quiet=False):
        """
        Determine the namespaces of the nodes being displayed
        """
        namespaces = []
        nodes_and_namespaces = dict(self._node.get_node_names_and_namespace())
        if graph_mode == NODE_NODE_GRAPH:
            nodes = graph.nn_nodes
            if quiet:
                nodes = [n for n in nodes if n not in QUIET_NAMES]
            namespaces = list(set([nodes_and_namespaces[n] for n in nodes]))

        elif graph_mode == NODE_TOPIC_GRAPH or \
                graph_mode == NODE_TOPIC_ALL_GRAPH:
            nn_nodes = graph.nn_nodes
            nt_nodes = graph.nt_nodes
            if quiet:
                nn_nodes = [n for n in nn_nodes if n not in QUIET_NAMES]
                nt_nodes = [n for n in nt_nodes if n not in QUIET_NAMES]
            if nn_nodes or nt_nodes:
                namespaces = [nodes_and_namespaces[n] for n in nn_nodes]
            # an annoyance with the rosgraph library is that it
            # prepends a space to topic names as they have to have
            # different graph node namees from nodes. we have to strip here
            namespaces.extend([nodes_and_namespaces[n[1:]] for n in nt_nodes])

        return list(set(namespaces))

    def _filter_orphaned_edges(self, edges, nodes):
        nodenames = [unicode(n).strip() for n in nodes]
        # currently using and rule as the or rule generates orphan nodes with the current logic
        return [e for e in edges if e.start.strip() in nodenames and e.end.strip() in nodenames]

    def _filter_orphaned_topics(self, nt_nodes, edges):
        '''remove topic graphnodes without connected ROS nodes'''
        removal_nodes = []
        for n in nt_nodes:
            keep = False
            for e in edges:
                if e.start.strip() == unicode(n).strip() or e.end.strip() == unicode(n).strip():
                    keep = True
                    break
            if not keep:
                removal_nodes.append(n)
        for n in removal_nodes:
            nt_nodes.remove(n)
        return nt_nodes

    def _split_filter_string(self, ns_filter):
        '''splits a string after each comma, and treats tokens with leading dash as exclusions.
        Adds .* as inclusion if no other inclusion option was given'''
        includes = []
        excludes = []
        for name in ns_filter.split(','):
            if name.strip().startswith('-'):
                excludes.append(name.strip()[1:])
            else:
                includes.append(name.strip())
        if includes == [] or includes == ['/'] or includes == ['']:
            includes = ['.*']
        return includes, excludes

    def _get_node_edge_map(self, edges):
        '''
        returns a dict mapping node name to edge objects partitioned in incoming and outgoing edges
        '''
        node_connections = {}
        for edge in edges:
            if edge.start not in node_connections:
                node_connections[edge.start] = NodeConnections()
            if edge.end not in node_connections:
                node_connections[edge.end] = NodeConnections()
            node_connections[edge.start].outgoing.append(edge)
            node_connections[edge.end].incoming.append(edge)
        return node_connections

    def _filter_leaf_topics(
            self,
            nodes_in,
            edges_in,
            node_connections,
            hide_single_connection_topics,
            hide_dead_end_topics):
        '''
        removes certain ending topic nodes and their edges from list of nodes and edges

        @param hide_single_connection_topics:
            if true removes topics that are only published/subscribed by one node
        @param hide_dead_end_topics: if true removes topics having only publishers
        '''
        if not hide_dead_end_topics and not hide_single_connection_topics:
            return nodes_in, edges_in
        # do not manipulate incoming structures
        nodes = copy.copy(nodes_in)
        edges = copy.copy(edges_in)
        removal_nodes = []
        for n in nodes:
            if n in node_connections:
                node_edges = []
                has_out_edges = False
                node_edges.extend(node_connections[n].outgoing)
                if len(node_connections[n].outgoing) > 0:
                    has_out_edges = True
                node_edges.extend(node_connections[n].incoming)
                if (hide_single_connection_topics and len(node_edges) < 2) or \
                        (hide_dead_end_topics and not has_out_edges):
                    removal_nodes.append(n)
                    for e in node_edges:
                        if e in edges:
                            edges.remove(e)
        for n in removal_nodes:
            nodes.remove(n)
        return nodes, edges

    def _accumulate_action_topics(self, nodes_in, edges_in, node_connections):
        '''takes topic nodes, edges and node connections.
        Returns topic nodes where action topics have been removed,
        edges where the edges to action topics have been removed, and
        a map with the connection to each virtual action topic node'''
        removal_nodes = []
        action_nodes = {}
        # do not manipulate incoming structures
        nodes = copy.copy(nodes_in)
        edges = copy.copy(edges_in)
        for n in nodes:
            if unicode(n).endswith('/feedback'):
                prefix = unicode(n)[:-len('/feedback')].strip()
                action_topic_nodes = []
                action_topic_edges_out = list()
                action_topic_edges_in = list()
                for suffix in ['/status', '/result', '/goal', '/cancel', '/feedback']:
                    for n2 in nodes:
                        if unicode(n2).strip() == prefix + suffix:
                            action_topic_nodes.append(n2)
                            if n2 in node_connections:
                                action_topic_edges_out.extend(node_connections[n2].outgoing)
                                action_topic_edges_in.extend(node_connections[n2].incoming)
                if len(action_topic_nodes) == 5:
                    # found action
                    removal_nodes.extend(action_topic_nodes)
                    for e in action_topic_edges_out:
                        if e in edges:
                            edges.remove(e)
                    for e in action_topic_edges_in:
                        if e in edges:
                            edges.remove(e)
                    action_nodes[prefix] = {'topics': action_topic_nodes,
                                            'outgoing': action_topic_edges_out,
                                            'incoming': action_topic_edges_in}
        for n in removal_nodes:
            nodes.remove(n)
        return nodes, edges, action_nodes

    def _populate_node_graph(
            self,
            cluster_namespaces_level,
            node_list,
            dotcode_factory,
            dotgraph,
            rank,
            orientation,
            simplify):
        namespace_clusters = {}
        if cluster_namespaces_level > 0:
            for node in node_list:
                if unicode(node.strip()).count('/') > 2:
                    for i in range(
                            2, min(2 + cluster_namespaces_level, len(node.strip().split('/')))):
                        namespace = '/'.join(node.strip().split('/')[:i])
                        parent_namespace = '/'.join(node.strip().split('/')[:i - 1])
                        if namespace not in namespace_clusters:
                            if parent_namespace == '':
                                namespace_clusters[namespace] = \
                                    dotcode_factory.add_subgraph_to_graph(
                                        dotgraph,
                                        namespace,
                                        rank=rank,
                                        rankdir=orientation,
                                        simplify=simplify)
                            elif parent_namespace in namespace_clusters:
                                namespace_clusters[namespace] = \
                                    dotcode_factory.add_subgraph_to_graph(
                                        namespace_clusters[parent_namespace],
                                        namespace,
                                        rank=rank,
                                        rankdir=orientation,
                                        simplify=simplify)
                elif unicode(node.strip()).count('/') == 2:
                    namespace = '/'.join(node.strip().split('/')[0:2])
                    if namespace not in namespace_clusters:
                        namespace_clusters[namespace] = dotcode_factory.add_subgraph_to_graph(
                            dotgraph,
                            namespace,
                            rank=rank,
                            rankdir=orientation,
                            simplify=simplify)
        return namespace_clusters

    def _group_tf_nodes(self, nodes_in, edges_in, node_connections):
        '''takes topic nodes, edges and node connections.
        Returns topic nodes where tf topics have been removed,
        edges where the edges to tf topics have been removed, and
        a map with all the connections to the resulting tf group node'''
        removal_nodes = []
        tf_topic_edges_in = []
        tf_topic_edges_out = []
        # do not manipulate incoming structures
        nodes = copy.copy(nodes_in)
        edges = copy.copy(edges_in)
        for n in nodes:
            if unicode(n).strip() in ['/tf', '/tf_static'] and n in node_connections:
                tf_topic_edges_in.extend(
                    [x for x in node_connections[n].incoming if x in edges and x.end in nodes])
                tf_topic_edges_out.extend(
                    [x for x in node_connections[n].outgoing if x in edges and x.start in nodes])
                removal_nodes.append(n)

                for e in tf_topic_edges_out:
                    if e in edges:
                        edges.remove(e)
                for e in tf_topic_edges_in:
                    if e in edges:
                        edges.remove(e)
        for n in removal_nodes:
            if n in nodes:
                nodes.remove(n)
        if not tf_topic_edges_in and not tf_topic_edges_out:
            return nodes, edges, None

        return nodes, edges, {'outgoing': tf_topic_edges_out, 'incoming': tf_topic_edges_in}

    def _accumulate_image_topics(self, nodes_in, edges_in, node_connections):
        '''takes topic nodes, edges and node connections.
        Returns topic nodes where image topics have been removed,
        edges where the edges to image topics have been removed, and
        a map with the connection to each virtual image topic node'''
        removal_nodes = []
        image_nodes = {}
        # do not manipulate incoming structures
        nodes = copy.copy(nodes_in)
        edges = copy.copy(edges_in)
        for n in nodes:
            if unicode(n).endswith('/compressed'):
                prefix = unicode(n)[:-len('/compressed')].strip()
                image_topic_nodes = []
                image_topic_edges_out = list()
                image_topic_edges_in = list()
                for suffix in ['/compressed', '/compressedDepth', '/theora', '']:
                    for n2 in nodes:
                        if unicode(n2).strip() == prefix + suffix:
                            image_topic_nodes.append(n2)
                            if n2 in node_connections:
                                image_topic_edges_out.extend(node_connections[n2].outgoing)
                                image_topic_edges_in.extend(node_connections[n2].incoming)
                if len(image_topic_nodes) >= 3:
                    # found action
                    removal_nodes.extend(image_topic_nodes)
                    for e in image_topic_edges_out:
                        if e in edges:
                            edges.remove(e)
                    for e in image_topic_edges_in:
                        if e in edges:
                            edges.remove(e)
                    image_nodes[prefix] = {'topics': image_topic_nodes,
                                           'outgoing': image_topic_edges_out,
                                           'incoming': image_topic_edges_in}
        for n in removal_nodes:
            nodes.remove(n)
        return nodes, edges, image_nodes

    def _filter_hidden_topics(
        self,
        nodes_in,
        edges_in,
        node_connections,
        hide_tf_nodes,
            hide_dynamic_reconfigure):
        if not hide_tf_nodes and not hide_dynamic_reconfigure:
            return nodes_in, edges_in
        # do not manipulate incoming structures
        nodes = copy.copy(nodes_in)
        edges = copy.copy(edges_in)
        removal_nodes = []
        for n in nodes:
            # parameter_updates is the ROS 1 dynamic reconfigure topic
            # parameter_events is the ROS 2 parameter event topic
            if hide_dynamic_reconfigure:
                suffix = unicode(n).rsplit('/', 1)[-1]
                if suffix in ('parameter_updates', 'parameter_events'):
                    prefix = unicode(n).rsplit('/', 1)[0].strip()
                    if prefix == 'parameter_updates':
                        suffixes = ['/parameter_updates', '/parameter_descriptions']
                    else:
                        suffixes = ['/parameter_events']
                    dynamic_reconfigure_topic_nodes = []
                    for suffix in suffixes:
                        for n2 in nodes:
                            if unicode(n2).strip() == prefix + suffix:
                                dynamic_reconfigure_topic_nodes.append(n2)
                    if len(dynamic_reconfigure_topic_nodes) == len(suffixes):
                        for n1 in dynamic_reconfigure_topic_nodes:
                            if n1 in node_connections:
                                for e in node_connections[n1].outgoing + node_connections[n1].incoming:
                                    if e in edges:
                                        edges.remove(e)
                            removal_nodes.append(n1)
                        continue
            if hide_tf_nodes and unicode(n).strip() in ['/tf', '/tf_static']:
                if n in node_connections:
                    for e in node_connections[n].outgoing + node_connections[n].incoming:
                        if e in edges:
                            edges.remove(e)
                removal_nodes.append(n)
                continue
        for n in removal_nodes:
            if n in nodes:
                nodes.remove(n)
        return nodes, edges

    def generate_dotgraph(
        self,
        rosgraphinst,
        ns_filter,
        topic_filter,
        graph_mode,
        dotcode_factory,
        hide_single_connection_topics=False,
        hide_dead_end_topics=False,
        cluster_namespaces_level=0,
        accumulate_actions=True,
        orientation='LR',
        rank='same',  # None, same, min, max, source, sink
        ranksep=0.2,  # vertical distance between layers
        rankdir='TB',  # direction of layout (TB top > bottom, LR left > right)
        simplify=False,  # do not remove double edges
        quiet=False,
        unreachable=False,
        group_tf_nodes=False,
        hide_tf_nodes=False,
        group_image_nodes=False,
            hide_dynamic_reconfigure=False):
        """
        See generate_dotcode
        """
        includes, excludes = self._split_filter_string(ns_filter)
        topic_includes, topic_excludes = self._split_filter_string(topic_filter)

        # create the node definitions
        nn_nodes = [
            n for n in rosgraphinst.nn_nodes
            if matches_any(n, includes) and not matches_any(n, excludes)
        ]
        nt_nodes = [
            n for n in rosgraphinst.nt_nodes
            if matches_any(n, topic_includes) and not matches_any(n, topic_excludes)
        ]

        # create the edge definitions, unwrap EdgeList objects into python lists
        if graph_mode == NODE_TOPIC_GRAPH:
            edges = [e for e in rosgraphinst.nt_edges]
        else:
            edges = [e for e in rosgraphinst.nt_all_edges]

        # for accumulating actions topics
        action_nodes = {}
        # for accumulating image topics
        image_nodes = {}
        # for accumulating tf node connections
        tf_connections = None

        if (hide_single_connection_topics or
            hide_dead_end_topics or accumulate_actions or
            group_tf_nodes or hide_tf_nodes or
            group_image_nodes or hide_dynamic_reconfigure):
            # maps outgoing and incoming edges to nodes
            node_connections = self._get_node_edge_map(edges)

            nt_nodes, edges = self._filter_leaf_topics(
                nt_nodes,
                edges,
                node_connections,
                hide_single_connection_topics,
                hide_dead_end_topics)

            nt_nodes, edges = self._filter_hidden_topics(
                nt_nodes,
                edges,
                node_connections,
                hide_tf_nodes,
                hide_dynamic_reconfigure)

            if graph_mode != NODE_NODE_GRAPH:
                if accumulate_actions:
                    nt_nodes, edges, action_nodes = self._accumulate_action_topics(
                        nt_nodes, edges, node_connections)
                if group_image_nodes:
                    nt_nodes, edges, image_nodes = self._accumulate_image_topics(
                        nt_nodes, edges, node_connections)
                if group_tf_nodes and not hide_tf_nodes:
                    nt_nodes, edges, tf_connections = self._group_tf_nodes(
                        nt_nodes, edges, node_connections)

        edges = self._filter_orphaned_edges(edges, nn_nodes + nt_nodes)
        nt_nodes = self._filter_orphaned_topics(nt_nodes, edges)

        if graph_mode == NODE_NODE_GRAPH:
            edges = [
                e for e in rosgraphinst.nn_edges if rosgraph2_impl.topic_node(e.label) in nt_nodes
            ]
            nt_nodes = []

        if quiet:
            nn_nodes = list(filter(self._quiet_filter, nn_nodes))
            nt_nodes = list(filter(self._quiet_filter, nt_nodes))
            edges = list(filter(self.quiet_filter_topic_edge, edges))

        # create the graph
        # result = "digraph G {\n
        # rankdir=%(orientation)s;\n%(nodes_str)s\n%(edges_str)s}\n" % vars()
        dotgraph = dotcode_factory.get_graph(
            rank=rank,
            ranksep=ranksep,
            simplify=simplify,
            rankdir=orientation)

        ACTION_TOPICS_SUFFIX = '/action_topics'
        IMAGE_TOPICS_SUFFIX = '/image_topics'

        node_list = (nt_nodes or []) + \
            [act_prefix + ACTION_TOPICS_SUFFIX for act_prefix, _ in action_nodes.items()] + \
            [img_prefix + IMAGE_TOPICS_SUFFIX for img_prefix, _ in image_nodes.items()] + \
            nn_nodes if nn_nodes is not None else []

        namespace_clusters = self._populate_node_graph(
            cluster_namespaces_level,
            node_list,
            dotcode_factory,
            dotgraph,
            rank,
            orientation,
            simplify)

        for n in nt_nodes or []:
            # cluster topics with same namespace
            if cluster_namespaces_level > 0 and \
                    unicode(n).strip().count('/') > 1 and \
                    len(n.strip().split('/')[1]) > 0:
                if n.count('/') <= cluster_namespaces_level:
                    namespace = unicode('/'.join(n.strip().split('/')[:-1]))
                else:
                    namespace = '/'.join(n.strip().split('/')[:cluster_namespaces_level + 1])
                self._add_topic_node(
                    n,
                    rosgraphinst=rosgraphinst,
                    dotcode_factory=dotcode_factory, dotgraph=namespace_clusters[namespace],
                    quiet=quiet)
            else:
                self._add_topic_node(
                    n,
                    rosgraphinst=rosgraphinst,
                    dotcode_factory=dotcode_factory, dotgraph=dotgraph, quiet=quiet)

        for n in [act_prefix + ACTION_TOPICS_SUFFIX for act_prefix, _ in action_nodes.items()] + \
                [img_prefix + IMAGE_TOPICS_SUFFIX for img_prefix, _ in image_nodes.items()]:
            # cluster topics with same namespace
            if cluster_namespaces_level > 0 and \
                    unicode(n).strip().count('/') > 1 and \
                    len(unicode(n).strip().split('/')[1]) > 0:
                if n.strip().count('/') <= cluster_namespaces_level:
                    namespace = unicode('/'.join(n.strip().split('/')[:-1]))
                else:
                    namespace = '/'.join(n.strip().split('/')[:cluster_namespaces_level + 1])
                self._add_topic_node_group(
                    'n' + n, dotcode_factory=dotcode_factory,
                    dotgraph=namespace_clusters[namespace], quiet=quiet)
            else:
                self._add_topic_node_group(
                    'n' + n, dotcode_factory=dotcode_factory, dotgraph=dotgraph, quiet=quiet)

        if tf_connections is not None:
            # render tf nodes as a single node
            self._add_topic_node_group(
                'n/tf', dotcode_factory=dotcode_factory, dotgraph=dotgraph, quiet=quiet)
            for out_edge in tf_connections.get('outgoing', []):
                dotcode_factory.add_edge_to_graph(dotgraph, _conv('n/tf'), _conv(out_edge.end))
            for in_edge in tf_connections.get('incoming', []):
                dotcode_factory.add_edge_to_graph(dotgraph, _conv(in_edge.start), _conv('n/tf'))

        # for ROS node, if we have created a namespace clusters for
        # one of its peer topics, drop it into that cluster
        for n in nn_nodes or []:
            if cluster_namespaces_level > 0 and \
                    n.strip().count('/') > 1 and \
                    len(n.strip().split('/')[1]) > 0:
                if n.count('/') <= cluster_namespaces_level:
                    namespace = unicode('/'.join(n.strip().split('/')[:-1]))
                else:
                    namespace = '/'.join(n.strip().split('/')[:cluster_namespaces_level + 1])
                self._add_node(
                    n,
                    rosgraphinst=rosgraphinst,
                    dotcode_factory=dotcode_factory,
                    dotgraph=namespace_clusters[namespace],
                    unreachable=unreachable)
            else:
                self._add_node(
                    n,
                    rosgraphinst=rosgraphinst,
                    dotcode_factory=dotcode_factory,
                    dotgraph=dotgraph,
                    unreachable=unreachable)

        for e in edges:
            self._add_edge(
                e, dotcode_factory, dotgraph=dotgraph, is_topic=(graph_mode == NODE_NODE_GRAPH))

        for (action_prefix, node_connections) in action_nodes.items():
            for out_edge in node_connections.get('outgoing', []):
                dotcode_factory.add_edge_to_graph(
                    dotgraph,
                    _conv('n' + action_prefix + ACTION_TOPICS_SUFFIX),
                    _conv(out_edge.end))
            for in_edge in node_connections.get('incoming', []):
                dotcode_factory.add_edge_to_graph(
                    dotgraph,
                    _conv(in_edge.start),
                    _conv('n' + action_prefix + ACTION_TOPICS_SUFFIX))
        for (image_prefix, node_connections) in image_nodes.items():
            for out_edge in node_connections.get('outgoing', []):
                dotcode_factory.add_edge_to_graph(
                    dotgraph,
                    _conv('n' + image_prefix + IMAGE_TOPICS_SUFFIX),
                    _conv(out_edge.end))
            for in_edge in node_connections.get('incoming', []):
                dotcode_factory.add_edge_to_graph(
                    dotgraph,
                    _conv(in_edge.start),
                    _conv('n' + image_prefix + IMAGE_TOPICS_SUFFIX))

        return dotgraph

    def generate_dotcode(
        self,
        rosgraphinst,
        ns_filter,
        topic_filter,
        graph_mode,
        dotcode_factory,
        hide_single_connection_topics=False,
        hide_dead_end_topics=False,
        cluster_namespaces_level=0,
        accumulate_actions=True,
        orientation='LR',
        rank='same',  # None, same, min, max, source, sink
        ranksep=0.2,  # vertical distance between layers
        rankdir='TB',  # direction of layout (TB top > bottom, LR left > right)
        simplify=False,  # do not remove double edges
        quiet=False,
        unreachable=False,
        hide_tf_nodes=False,
        group_tf_nodes=False,
        group_image_nodes=False,
            hide_dynamic_reconfigure=False):
        """
        @param rosgraphinst: RosGraph instance
        @param ns_filter: nodename filter
        @type  ns_filter: string
        @param topic_filter: topicname filter
        @type  ns_filter: string
        @param graph_mode str: NODE_NODE_GRAPH | NODE_TOPIC_GRAPH | NODE_TOPIC_ALL_GRAPH
        @type  graph_mode: str
        @param orientation: rankdir value (see ORIENTATIONS dict)
        @type  dotcode_factory: object
        @param dotcode_factory: abstract factory manipulating dot language objects
        @param hide_single_connection_topics: if true remove topics with just one connection
        @param hide_dead_end_topics: if true remove topics with publishers only
        @param cluster_namespaces_level: if > 0 places box around members of same namespace
               (TODO: multiple namespace layers)
        @param accumulate_actions: if true each 5 action topic graph nodes are shown as single
            graph node
        @return: dotcode generated from graph singleton
        @rtype: str
        """
        dotgraph = self.generate_dotgraph(
            rosgraphinst=rosgraphinst,
            ns_filter=ns_filter,
            topic_filter=topic_filter,
            graph_mode=graph_mode,
            dotcode_factory=dotcode_factory,
            hide_single_connection_topics=hide_single_connection_topics,
            hide_dead_end_topics=hide_dead_end_topics,
            cluster_namespaces_level=cluster_namespaces_level,
            accumulate_actions=accumulate_actions,
            orientation=orientation,
            rank=rank,
            ranksep=ranksep,
            rankdir=rankdir,
            simplify=simplify,
            quiet=quiet,
            unreachable=unreachable,
            hide_tf_nodes=hide_tf_nodes,
            group_tf_nodes=group_tf_nodes,
            group_image_nodes=group_image_nodes,
            hide_dynamic_reconfigure=hide_dynamic_reconfigure)
        dotcode = dotcode_factory.create_dot(dotgraph)
        return dotcode
