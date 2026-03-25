from rosbag2_py import SequentialReader, SequentialWriter
from rosbag2_py import StorageOptions, ConverterOptions, StorageFilter, TopicMetadata
from rclpy.serialization import deserialize_message, serialize_message
from rosidl_runtime_py.utilities import get_message
from rclpy.time import Time
import builtin_interfaces.msg

import time


def get_message_name(obj):
    c = obj.__class__
    pkg_name, interface_type, _ = c.__module__.split('.')
    return '/'.join([pkg_name, interface_type, c.__qualname__])


def to_int_stamp(t):
    if t is None:
        return int(time.time() * 1e9)
    elif isinstance(t, int):
        return t
    elif isinstance(t, float):
        return int(t * 1e9)
    elif isinstance(t, Time):
        return t.nanoseconds
    elif isinstance(t, builtin_interfaces.msg.Time):
        tmsg = Time.from_msg(t)
        return tmsg.nanoseconds
    else:
        raise TypeError(f'Cannot use {type(t)} as timestamp')


class GenericTimestamp:
    def __init__(self, t):
        self.int_t = to_int_stamp(t)

    def __int__(self):
        return self.int_t

    def __float__(self):
        return self.int_t / 1e9

    def __repr__(self):
        return f'{float(self):.9f}'


class Bag:
    def __init__(self, path, mode='r', serialization_format='cdr', format=''):
        self.storage_options = StorageOptions(str(path), format)
        self.converter_options = ConverterOptions(serialization_format, serialization_format)
        self.mode = mode
        self.serialization_format = serialization_format

        if mode == 'w':
            self.writer = SequentialWriter()
            self.writer.open(self.storage_options, self.converter_options)
            self.topic_metadata = {}

    def __enter__(self):
        return self

    def __exit__(self, *args, **kwargs):
        self.close()

    def read_messages(self, topics=None):
        if self.mode != 'r':
            raise RuntimeError(f'Cannot read messages from a bag in {self.mode} mode')

        reader = SequentialReader()
        reader.open(self.storage_options, self.converter_options)

        if topics:
            if isinstance(topics, str):
                topics = [topics]
            storage_filter = StorageFilter(topics=topics)
            reader.set_filter(storage_filter)

        topic_types = reader.get_all_topics_and_types()
        type_map = {tmeta.name: tmeta.type for tmeta in topic_types}

        while reader.has_next():
            (topic, rawdata, timestamp) = reader.read_next()
            msg_type = get_message(type_map[topic])
            msg = deserialize_message(rawdata, msg_type)
            yield topic, msg, GenericTimestamp(timestamp)

    def write(self, topic, msg, t=None):
        if self.mode != 'w':
            raise RuntimeError(f'Cannot write messages to a bag in {self.mode} mode')

        if topic not in self.topic_metadata:
            msgtype = get_message_name(msg)
            topic_id = len(self.topic_metadata)
            self.topic_metadata[topic] = TopicMetadata(id=topic_id, name=topic, type=msgtype,
                                                       serialization_format=self.serialization_format)
            self.writer.create_topic(self.topic_metadata[topic])

        self.writer.write(topic, serialize_message(msg), to_int_stamp(t))

    def close(self):
        self.writer = None
