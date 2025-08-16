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

import rclpy
import rosbag2_py
import time
import threading


from python_qt_binding.QtCore import qDebug, Qt, QTimer, qWarning, Signal
from python_qt_binding.QtWidgets import QGraphicsScene, QMessageBox

from rqt_bag import bag_helper
from rclpy.duration import Duration
from rclpy.time import Time

from .timeline_frame import TimelineFrame
from .message_listener_thread import MessageListenerThread
from .message_loader_thread import MessageLoaderThread
from .player import Player
from .recorder import Recorder
from .timeline_menu import TimelinePopupMenu


class BagTimeline(QGraphicsScene):

    """
    BagTimeline contains bag files, all information required to display the bag data visualization
    on the screen Also handles events
    """
    status_bar_changed_signal = Signal()
    selected_region_changed = Signal(Time, Time)

    def __init__(self, context, publish_clock):
        """
        :param context:
            plugin context hook to enable adding rqt_bag plugin widgets as ROS_GUI snapin panes,
            ''PluginContext''
        """
        super(BagTimeline, self).__init__()
        self._bags = []
        self._bag_lock = threading.RLock()

        self.background_task = None  # Display string
        self.background_task_cancel = False

        # Playing / Recording
        self._playhead_lock = threading.RLock()
        self._max_play_speed = 1024.0  # fastest X play speed
        self._min_play_speed = 1.0 / 1024.0  # slowest X play speed
        self._play_speed = 0.0
        self._play_all = False
        self._playhead_positions_cvs = {}
        self._playhead_positions = {}  # topic -> (bag, position)
        self._message_loaders = {}
        self._messages_cvs = {}
        self._messages = {}  # topic -> (bag, msg_data)
        self._message_listener_threads = {}  # listener -> MessageListenerThread
        self._player = False
        self._publish_clock = publish_clock
        self._recorder = None
        self.last_frame = None
        self.last_playhead = None
        self.desired_playhead = None
        self.wrap = True  # should the playhead wrap when it reaches the end?
        self.stick_to_end = False  # should the playhead stick to the end?
        self._play_timer = QTimer()
        self._play_timer.timeout.connect(self.on_idle)
        self._play_timer.setInterval(3)

        # Plugin popup management
        self._context = context
        self.popups = {}
        self._views = []
        self._listeners = {}

        # Initialize scene
        # the timeline renderer fixes use of black pens and fills, so ensure we fix white here for
        # contrast. Otherwise a dark qt theme will default it to black and the frame render
        # pen will be unreadable
        self.setBackgroundBrush(Qt.white)
        self._timeline_frame = TimelineFrame(self)
        self._timeline_frame.setPos(0, 0)
        self.addItem(self._timeline_frame)

        self.background_progress = 0
        self.__closed = False

        # Database settings
        self.serialization_format = 'cdr'
        self.storage_id = 'sqlite3'

    def get_context(self):
        """
        :returns: the ROS_GUI context, 'PluginContext'
        """
        return self._context

    def handle_close(self):
        """
        Cleans up the timeline, bag and any threads
        """
        if self.__closed:
            return
        else:
            self.__closed = True
        self._play_timer.stop()
        for topic in self._get_topics():
            self.stop_publishing(topic)
            self._message_loaders[topic].stop()
        if self._player:
            self._player.stop()
        if self._recorder:
            self._recorder.stop()
        if self.background_task is not None:
            self.background_task_cancel = True
        self._timeline_frame.handle_close()
        for frame in self._views:
            if frame.parent():
                self._context.remove_widget(frame)

    # Bag Management and access
    def add_bag(self, bag):
        """
        creates an indexing thread for each new topic in the bag
        fixes the boarders and notifies the indexing thread to index the new items bags
        :param bag: ros bag file, ''rosbag2.bag''
        """
        self._bags.append(bag)

        bag_topics = bag.get_topics()
        qDebug('Topics from this bag: {}'.format(bag_topics))

        new_topics = set(bag_topics) - set(self._timeline_frame.topics)

        for topic in new_topics:
            self._playhead_positions_cvs[topic] = threading.Condition()
            self._messages_cvs[topic] = threading.Condition()
            self._message_loaders[topic] = MessageLoaderThread(self, topic)

        self._timeline_frame._start_stamp = self._get_start_stamp()
        self._timeline_frame._end_stamp = self._get_end_stamp()
        self._timeline_frame.topics = self._get_topics()
        self._timeline_frame._topics_by_datatype = self._get_topics_by_datatype()
        # If this is the first bag, reset the timeline
        if self._timeline_frame._stamp_left is None:
            self._timeline_frame.reset_timeline()

        # Invalidate entire index cache for all topics in this bag
        with self._timeline_frame.index_cache_cv:
            for topic in bag_topics:
                self._timeline_frame.invalidated_caches.add(topic)
                if topic in self._timeline_frame.index_cache:
                    del self._timeline_frame.index_cache[topic]

            self._timeline_frame.index_cache_cv.notify()

    def file_size(self):
        with self._bag_lock:
            return sum(b.size() for b in self._bags)

    # TODO Rethink API and if these need to be visible
    def _get_start_stamp(self):
        """
        :return: first stamp in the bags, ''rclpy.time.Time''
        """
        with self._bag_lock:
            start_stamp = None
            for bag in self._bags:
                bag_start_stamp = bag.get_earliest_timestamp()
                if bag_start_stamp is not None and \
                        (start_stamp is None or bag_start_stamp < start_stamp):
                    start_stamp = bag_start_stamp
            return start_stamp

    def _get_end_stamp(self):
        """
        :return: last stamp in the bags, ''rclpy.time.Time''
        """
        with self._bag_lock:
            end_stamp = None
            for bag in self._bags:
                bag_end_stamp = bag.get_latest_timestamp()
                if bag_end_stamp is not None and (end_stamp is None or bag_end_stamp > end_stamp):
                    end_stamp = bag_end_stamp
            return end_stamp

    def _get_topics(self):
        """
        :return: sorted list of topic names, ''list(str)''
        """
        with self._bag_lock:
            topics = set()
            for bag in self._bags:
                for topic in bag.get_topics():
                    topics.add(topic)
            return sorted(topics)

    def _get_topics_by_datatype(self):
        """
        :return: dict of list of topics for each datatype, ''dict(datatype:list(topic))''
        """
        with self._bag_lock:
            topics_by_datatype = {}
            for bag in self._bags:
                for datatype, topics in bag.get_topics_by_type().items():
                    topics_by_datatype.setdefault(datatype, []).extend(topics)
            return topics_by_datatype

    def get_datatype(self, topic):
        """
        :return: datatype associated with a topic, ''str''
        :raises: if there are multiple datatypes assigned to a single topic, ''Exception''
        """
        with self._bag_lock:
            datatype = None
            for bag in self._bags:
                bag_datatype = bag.get_topic_type(topic)
                if datatype and bag_datatype and (bag_datatype != datatype):
                    raise Exception('topic %s has multiple datatypes: %s and %s' %
                                    (topic, datatype, bag_datatype))
                if bag_datatype:
                    datatype = bag_datatype
            return datatype

    def get_entries(self, topics, start_stamp, end_stamp):
        """
        generator function for bag entries
        :param topics: list of topics to query, ''list(str)''
        :param start_stamp: stamp to start at, ''rclpy.time.Time''
        :param end_stamp: stamp to end at, ''rclpy.time,Time''
        :returns: entries the bag file, ''msg''
        """
        with self._bag_lock:
            bag_entries = []
            for b in self._bags:
                bag_start_time = b.get_earliest_timestamp()
                if bag_start_time is not None and bag_start_time > end_stamp:
                    continue

                bag_end_time = b.get_latest_timestamp()
                if bag_end_time is not None and bag_end_time < start_stamp:
                    continue

                # Get all of the entries for each topic. When opening multiple
                # bags, the requested topic may not be in a given bag database
                for topic in topics:
                    entries = b.get_entries_in_range(start_stamp, end_stamp, topic)
                    if entries is not None:
                        bag_entries.extend(entries)

            for entry in sorted(bag_entries, key=lambda entry: entry.timestamp):
                yield entry

    def get_entries_with_bags(self, topic, start_stamp, end_stamp):
        """
        generator function for bag entries
        :param topics: list of topics to query, ''list(str)''
        :param start_stamp: stamp to start at, ''rclpy.time.Time''
        :param end_stamp: stamp to end at, ''rclpy.time,Time''
        :returns: tuple of (bag, entry) for the entries in the bag file, ''(rosbag2.bag, msg)''
        """
        with self._bag_lock:
            bag_entries = []
            bag_by_iter = {}
            for b in self._bags:
                bag_start_time = b.get_earliest_timestamp()
                if bag_start_time is not None and bag_start_time > end_stamp:
                    continue

                bag_end_time = b.get_latest_timestamp()
                if bag_end_time is not None and bag_end_time < start_stamp:
                    continue

                for entry in b.get_entries_in_range(start_stamp, end_stamp):
                    bag_entries.append((b, entry))

            for bag, entry in sorted(bag_entries, key=lambda item: item[1].timestamp):
                yield bag, entry

    def get_entry(self, t, topic):
        """
        Access a bag entry
        :param t: time, ''rclpy.time.Time''
        :param topic: the topic to be accessed, ''str''
        :return: tuple of (bag, entry) corresponding to time t and topic, ''(rosbag2.bag, msg)''
        """
        with self._bag_lock:
            entry_bag, entry = None, None
            for bag in self._bags:
                bag_entry = bag.get_entry(t, topic)
                if bag_entry and (not entry or bag_entry.timestamp > entry.timestamp):
                    entry_bag, entry = bag, bag_entry

            return entry_bag, entry

    def get_entry_before(self, t):
        """
        Access a bag entry
        :param t: time, ''rclpy.time.Time''
        :return: tuple of (bag, entry) corresponding to time t, ''(rosbag2.bag, msg)''
        """
        with self._bag_lock:
            entry_bag, entry = None, None
            for bag in self._bags:
                bag_entry = bag.get_entry(t - Duration(nanoseconds=1))
                if bag_entry and (not entry or bag_entry.timestamp < entry.timestamp):
                    entry_bag, entry = bag, bag_entry

            return entry_bag, entry

    def get_entry_after(self, t, topic=None):
        """
        Access a bag entry
        :param t: time, ''rclpy.time.Time''
        :return: tuple of (bag, entry) corresponding to time t, ''(rosbag2.bag, msg)''
        """
        with self._bag_lock:
            entry_bag, entry = None, None
            for bag in self._bags:
                bag_entry = bag.get_entry_after(t, topic)
                if bag_entry and (not entry or bag_entry.timestamp < entry.timestamp):
                    entry_bag, entry = bag, bag_entry

            return entry_bag, entry

    def get_next_message_time(self):
        """
        :return: time of the next message after the current playhead position,''rclpy.time.Time''
        """
        if self._timeline_frame.playhead is None:
            return None

        _, entry = self.get_entry_after(self._timeline_frame.playhead)
        if entry is None:
            return self._timeline_frame._start_stamp

        return Time(nanoseconds=entry.timestamp)

    def get_previous_message_time(self):
        """
        :return: time of the next message before the current playhead position,''rclpy.time.Time''
        """
        if self._timeline_frame.playhead is None:
            return None

        _, entry = self.get_entry_before(self._timeline_frame.playhead)
        if entry is None:
            return self._timeline_frame._end_stamp

        return Time(nanoseconds=entry.timestamp)

    def resume(self):
        if (self._player):
            self._player.resume()

    # Copy messages to...

    def start_background_task(self, background_task):
        """
        Verify that a background task is not currently running before starting a new one
        :param background_task: name of the background task, ''str''
        """
        if self.background_task is not None:
            QMessageBox(
                QMessageBox.Warning, 'Exclamation', 'Background operation already running:\n\n%s' %
                self.background_task, QMessageBox.Ok).exec_()
            return False

        self.background_task = background_task
        self.background_task_cancel = False
        return True

    def stop_background_task(self):
        self.background_task = None

    def copy_region_to_bag(self, filename):
        if len(self._bags) > 0:
            self._export_region(filename, self._timeline_frame.topics,
                                self._timeline_frame.play_region[0],
                                self._timeline_frame.play_region[1])

    def _export_region(self, path, topics, start_stamp, end_stamp):
        """
        Starts a thread to save the current selection to a new bag file
        :param path: filesystem path to write to, ''str''
        :param topics: topics to write to the file, ''list(str)''
        :param start_stamp: start of area to save, ''rclpy.time.Time''
        :param end_stamp: end of area to save, ''rclpy.time.Time''
        """
        if not self.start_background_task('Copying messages to "%s"' % path):
            return
        # TODO implement a status bar area with information on the current save status
        bag_entries = list(self.get_entries_with_bags(topics, start_stamp, end_stamp))

        if self.background_task_cancel:
            return

        # Get the total number of messages to copy
        total_messages = len(bag_entries)

        # If no messages, prompt the user and return
        if total_messages == 0:
            QMessageBox(QMessageBox.Warning, 'rqt_bag', 'No messages found', QMessageBox.Ok).exec_()
            self.stop_background_task()
            return

        # Export the messages in the selected region
        try:
            storage_options = rosbag2_py.StorageOptions(uri=path, storage_id=self.storage_id)

            converter_options = rosbag2_py.ConverterOptions(
                input_serialization_format=self.serialization_format,
                output_serialization_format=self.serialization_format)

            rosbag_writer = rosbag2_py.SequentialWriter()
            rosbag_writer.open(storage_options, converter_options)
        except Exception:
            QMessageBox(QMessageBox.Warning, 'rqt_bag',
                        'Error opening bag file [%s] for writing' % path, QMessageBox.Ok).exec_()
            self.stop_background_task()
            return

        # Run copying in a background thread
        self._export_thread = threading.Thread(
            target=self._run_export_region,
            args=(rosbag_writer, topics, start_stamp, end_stamp, bag_entries, path, self.serialization_format))
        self._export_thread.start()

    def _run_export_region(self, rosbag_writer, topics, start_stamp, end_stamp, bag_entries, export_filename, serialization_format):
        """
        Threaded function that saves the current selection to a new bag file
        :param export_bag: bagfile to write to, ''rosbag.bag''
        :param topics: topics to write to the file, ''list(str)''
        :param start_stamp: start of area to save, ''rclpy.time.Time''
        :param end_stamp: end of area to save, ''rclpy.time.Time''
        """
        total_messages = len(bag_entries)
        update_step = max(1, total_messages / 100)
        message_num = 1
        progress = 0
        database_topics = set()

        # Write out the messages
        for bag, entry in bag_entries:
            if self.background_task_cancel:
                break
            try:
                # Add any new topics to the database
                if entry.topic not in database_topics:
                    topic_metadata = bag.get_topic_metadata(entry.topic)
                    rosbag_writer.create_topic(topic_metadata)
                    database_topics.add(entry.topic)

                # Then write it out
                rosbag_writer.write(entry.topic, entry.data, entry.timestamp)
            except Exception as ex:
                qWarning('Error exporting message at position %s: %s' %
                         (str(entry.timestamp), str(ex)))
                self.stop_background_task()
                return

            if message_num % update_step == 0 or message_num == total_messages:
                new_progress = int(100.0 * (float(message_num) / total_messages))
                if new_progress != progress:
                    progress = new_progress
                    if not self.background_task_cancel:
                        self.background_progress = progress
                        self.status_bar_changed_signal.emit()

            message_num += 1

        # Close the bag
        try:
            self.background_progress = 0
            self.status_bar_changed_signal.emit()
        except Exception as ex:
            QMessageBox(QMessageBox.Warning, 'rqt_bag', 'Error closing bag file [%s]: %s' % (
                export_filename, str(ex)), QMessageBox.Ok).exec_()
        self.stop_background_task()

    def read_message(self, bag, position):
        with self._bag_lock:
            return bag.get_entry(Time(nanoseconds=position))

    # Mouse events
    def on_mouse_down(self, event):
        if event.buttons() == Qt.LeftButton:
            self._timeline_frame.on_left_down(event)
        elif event.buttons() == Qt.MidButton:
            self._timeline_frame.on_middle_down(event)
        elif event.buttons() == Qt.RightButton:
            topic = self._timeline_frame.map_y_to_topic(self.views()[0].mapToScene(event.pos()).y())
            TimelinePopupMenu(self, event, topic)

    def on_mouse_up(self, event):
        self._timeline_frame.on_mouse_up(event)

    def on_mouse_move(self, event):
        self._timeline_frame.on_mouse_move(event)

    def on_mousewheel(self, event):
        self._timeline_frame.on_mousewheel(event)

    # Zooming

    def zoom_in(self):
        self._timeline_frame.zoom_in()

    def zoom_out(self):
        self._timeline_frame.zoom_out()

    def reset_zoom(self):
        self._timeline_frame.reset_zoom()

    def translate_timeline_left(self):
        self._timeline_frame.translate_timeline_left()

    def translate_timeline_right(self):
        self._timeline_frame.translate_timeline_right()

    # Publishing
    def is_publishing(self, topic):
        return self._player and self._player.is_publishing(topic)

    def start_publishing(self, topic):
        if not self._player and not self._create_player():
            return False

        self._player.start_publishing(topic)
        return True

    def stop_publishing(self, topic):
        if not self._player:
            return False

        self._player.stop_publishing(topic)
        return True

    def _create_player(self):
        if not self._player:
            try:
                self._player = Player(self._context.node, self)
                if self._publish_clock:
                    self._player.start_clock_publishing()
            except Exception as ex:
                qWarning('Error starting player; aborting publish: %s' % str(ex))
                return False

        return True

    def set_publishing_state(self, start_publishing):
        if start_publishing:
            for topic in self._timeline_frame.topics:
                if not self.start_publishing(topic):
                    break
        else:
            for topic in self._timeline_frame.topics:
                self.stop_publishing(topic)

    # property: play_all
    def _get_play_all(self):
        return self._play_all

    def _set_play_all(self, play_all):
        if play_all == self._play_all:
            return

        self._play_all = not self._play_all

        self.last_frame = None
        self.last_playhead = None
        self.desired_playhead = None

    play_all = property(_get_play_all, _set_play_all)

    def toggle_play_all(self):
        self.play_all = not self.play_all

    # Playing
    def on_idle(self):
        self._step_playhead()

    def _step_playhead(self):
        """
        moves the playhead to the next position based on the desired position
        """
        # Reset when the playing mode switches. A rclpy.time.Time cannot be compared to a NoneType
        if self.last_playhead is None or self._timeline_frame.playhead != self.last_playhead:
            self.last_frame = None
            self.last_playhead = None
            self.desired_playhead = None

        if self._play_all:
            self.step_next_message()
        else:
            self.step_fixed()

    def step_fixed(self):
        """
        Moves the playhead a fixed distance into the future based on the current play speed
        """
        if self.play_speed == 0.0 or not self._timeline_frame.playhead:
            self.last_frame = None
            self.last_playhead = None
            return

        now = Time(seconds=time.time())
        if self.last_frame:
            # Get new playhead
            if self.stick_to_end:
                new_playhead = self.end_stamp
            else:
                jump = bag_helper.to_sec(now - self.last_frame) * self.play_speed
                new_playhead = self._timeline_frame.playhead + Duration(seconds=jump)

                start_stamp, end_stamp = self._timeline_frame.play_region

                if new_playhead > end_stamp:
                    if self.wrap:
                        if self.play_speed > 0.0:
                            new_playhead = start_stamp
                        else:
                            new_playhead = end_stamp
                    else:
                        new_playhead = end_stamp

                        if self.play_speed > 0.0:
                            self.stick_to_end = True

                elif new_playhead < start_stamp:
                    if self.wrap:
                        if self.play_speed < 0.0:
                            new_playhead = end_stamp
                        else:
                            new_playhead = start_stamp
                    else:
                        new_playhead = start_stamp

            # Update the playhead
            self._timeline_frame.playhead = new_playhead

        self.last_frame = now
        self.last_playhead = self._timeline_frame.playhead

    def step_next_message(self):
        """
        Move the playhead to the next message
        """
        if self.play_speed <= 0.0 or not self._timeline_frame.playhead:
            self.last_frame = None
            self.last_playhead = None
            return

        if self.last_frame:
            if not self.desired_playhead:
                self.desired_playhead = self._timeline_frame.playhead
            else:
                delta = Time(seconds=time.time()) - self.last_frame
                if delta > Duration(seconds=0.1):
                    delta = Duration(seconds=0.1)
                self.desired_playhead += delta

            # Get the occurrence of the next message
            next_message_time = self.get_next_message_time()

            if next_message_time < self.desired_playhead:
                self._timeline_frame.playhead = next_message_time
            else:
                self._timeline_frame.playhead = self.desired_playhead

        self.last_frame = Time(seconds=time.time())
        self.last_playhead = self._timeline_frame.playhead

    # Recording

    def record_bag(self, filename, all_topics=True, topics=[], regex=False, limit=0):
        try:
            self._recorder = Recorder(self._context.node, filename, bag_lock=self._bag_lock,
                                      all_topics=all_topics, topics=topics,
                                      regex=regex, limit=limit)
        except Exception as ex:
            qWarning('Error opening bag for recording [%s]: %s' % (filename, str(ex)))
            return

        self._recorder.add_listener(self._message_recorded)
        self.add_bag(self._recorder._bag)
        self._recorder.start()

        self.wrap = False
        self._timeline_frame._index_cache_thread.period = 0.1

        self.update()

    def toggle_recording(self):
        if self._recorder:
            self._recorder.toggle_paused()
            self.update()

    def _message_recorded(self, topic, msg, t):
        if self._timeline_frame._start_stamp is None:
            self._timeline_frame._start_stamp = t
            self._timeline_frame._end_stamp = t
            self._timeline_frame._playhead = t
        elif self._timeline_frame._end_stamp is None or t > self._timeline_frame._end_stamp:
            self._timeline_frame._end_stamp = t

        if not self._timeline_frame.topics or topic not in self._timeline_frame.topics:
            self._timeline_frame.topics = self._get_topics()
            self._timeline_frame._topics_by_datatype = self._get_topics_by_datatype()

            self._playhead_positions_cvs[topic] = threading.Condition()
            self._messages_cvs[topic] = threading.Condition()
            self._message_loaders[topic] = MessageLoaderThread(self, topic)

        # Notify the index caching thread that it has work to do
        with self._timeline_frame.index_cache_cv:
            self._timeline_frame.invalidated_caches.add(topic)
            self._timeline_frame.index_cache_cv.notify()

        if topic in self._listeners:
            for listener in self._listeners[topic]:
                try:
                    listener.timeline_changed()
                except Exception as ex:
                    qWarning('Error calling timeline_changed on %s: %s' % (type(listener), str(ex)))

        # Dynamically resize the timeline, if necessary, to make visible any new messages
        # that might otherwise have exceeded the bounds of the window
        self.reset_zoom()

    # Views / listeners
    def add_view(self, topic, frame):
        self._views.append(frame)

    def has_listeners(self, topic):
        return topic in self._listeners

    def add_listener(self, topic, listener):
        self._listeners.setdefault(topic, []).append(listener)

        self._message_listener_threads[(topic, listener)] = \
            MessageListenerThread(self, topic, listener)
        # Notify the message listeners
        self._message_loaders[topic].reset()
        with self._playhead_positions_cvs[topic]:
            self._playhead_positions_cvs[topic].notify_all()

        self.update()

    def remove_listener(self, topic, listener):
        topic_listeners = self._listeners.get(topic)
        if topic_listeners is not None and listener in topic_listeners:
            topic_listeners.remove(listener)

            if len(topic_listeners) == 0:
                del self._listeners[topic]

            # Stop the message listener thread
            if (topic, listener) in self._message_listener_threads:
                self._message_listener_threads[(topic, listener)].stop()
                del self._message_listener_threads[(topic, listener)]
            self.update()

    # Playhead

    # property: play_speed
    def _get_play_speed(self):
        if self._timeline_frame._paused:
            return 0.0
        return self._play_speed

    def _set_play_speed(self, play_speed):
        if play_speed == self._play_speed:
            return

        if play_speed > 0.0:
            self._play_speed = min(self._max_play_speed, max(self._min_play_speed, play_speed))
        elif play_speed < 0.0:
            self._play_speed = max(-self._max_play_speed, min(-self._min_play_speed, play_speed))
        else:
            self._play_speed = play_speed

        if self._play_speed < 1.0:
            self.stick_to_end = False

        self.update()
    play_speed = property(_get_play_speed, _set_play_speed)

    def toggle_play(self):
        if self._play_speed != 0.0:
            self.play_speed = 0.0
        else:
            self.play_speed = 1.0

    def navigate_play(self):
        self.play_speed = 1.0
        self.last_frame = Time(seconds=time.time())
        self.last_playhead = self._timeline_frame.playhead
        self._play_timer.start()

    def navigate_stop(self):
        self.play_speed = 0.0
        self._play_timer.stop()

    def navigate_previous(self):
        self.navigate_stop()
        self._timeline_frame.playhead = self.get_previous_message_time()
        self.last_playhead = self._timeline_frame.playhead

    def navigate_next(self):
        self.navigate_stop()
        self._timeline_frame.playhead = self.get_next_message_time()
        self.last_playhead = self._timeline_frame.playhead

    def navigate_rewind(self):
        if self._play_speed < 0.0:
            new_play_speed = self._play_speed * 2.0
        elif self._play_speed == 0.0:
            new_play_speed = -1.0
        else:
            new_play_speed = self._play_speed * 0.5

        self.play_speed = new_play_speed

    def navigate_fastforward(self):
        if self._play_speed > 0.0:
            new_play_speed = self._play_speed * 2.0
        elif self._play_speed == 0.0:
            new_play_speed = 2.0
        else:
            new_play_speed = self._play_speed * 0.5

        self.play_speed = new_play_speed

    def navigate_start(self):
        self._timeline_frame.playhead = self._timeline_frame.play_region[0]

    def navigate_end(self):
        self._timeline_frame.playhead = self._timeline_frame.play_region[1]
