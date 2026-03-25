from gz.msgs11.stringmsg_pb2 import StringMsg
from gz.transport14 import Node, AdvertiseMessageOptions, SubscribeOptions, NodeOptions

import unittest


class OptionsTEST(unittest.TestCase):
    def stringmsg_cb(self, msg: StringMsg):
        _ = msg.data

    def test_advertise_message_options(self):
        opts = AdvertiseMessageOptions()
        msgs_per_sec = 10
        self.assertFalse(opts.throttled)
        self.assertNotEqual(opts.msgs_per_sec, msgs_per_sec)
        opts.msgs_per_sec = msgs_per_sec
        self.assertEqual(opts.msgs_per_sec, msgs_per_sec)
        self.assertTrue(opts.throttled)

        node = Node()
        pub = node.advertise("/test_topic", StringMsg, opts)
        self.assertTrue(pub)

    def test_subscribe_options(self):
        opts = SubscribeOptions()
        msgs_per_sec = 10
        self.assertFalse(opts.throttled)
        self.assertNotEqual(opts.msgs_per_sec, msgs_per_sec)
        opts.msgs_per_sec = msgs_per_sec
        self.assertEqual(opts.msgs_per_sec, msgs_per_sec)
        self.assertTrue(opts.throttled)

        node = Node()
        self.assertTrue(
            node.subscribe(StringMsg, "/test_topic", self.stringmsg_cb, opts)
        )

    def test_node_options(self):
        opts = NodeOptions()

        namespace = "test_namespace"
        self.assertNotEqual(opts.namespace, namespace)
        opts.namespace = namespace
        self.assertEqual(opts.namespace, namespace)

        partition = "test_partition"
        self.assertNotEqual(opts.partition, partition)
        opts.partition = partition
        self.assertEqual(opts.partition, partition)

        from_topic = "test_topic"
        to_topic = "test_topic_remapped"

        result, topic_remapped = opts.topic_remap(from_topic)
        self.assertFalse(result)
        self.assertNotEqual(topic_remapped, to_topic)
        self.assertEqual(topic_remapped, "")
        self.assertTrue(opts.add_topic_remap(from_topic, to_topic))
        result, topic_remapped = opts.topic_remap(from_topic)
        self.assertTrue(result)
        self.assertEqual(topic_remapped, to_topic)

        node = Node(opts)
        opts_from_node = node.options
        self.assertEqual(opts.namespace, opts_from_node.namespace)
        self.assertEqual(opts.partition, opts_from_node.partition)
        result, topic_remapped = opts_from_node.topic_remap(from_topic)
        self.assertTrue(result)
        self.assertEqual(topic_remapped, to_topic)
