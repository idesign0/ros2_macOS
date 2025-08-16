# Copyright (c) 2011, Willow Garage, Inc.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#    * Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#    * Neither the name of the copyright holder nor the names of its
#      contributors may be used to endorse or promote products derived from
#      this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.


# Author: Michael Ferguson

from threading import Lock

from builtin_interfaces.msg import Time
from rclpy.duration import Duration
from rclpy.qos import QoSProfile
from std_msgs.msg import Header
from visualization_msgs.msg import InteractiveMarker
from visualization_msgs.msg import InteractiveMarkerFeedback
from visualization_msgs.msg import InteractiveMarkerPose
from visualization_msgs.msg import InteractiveMarkerUpdate
from visualization_msgs.srv import GetInteractiveMarkers


class MarkerContext:
    """Represents a single marker."""

    def __init__(self, time):
        self.last_feedback = time
        self.last_client_id = ''
        self.default_feedback_callback = None
        self.feedback_callbacks = {}
        self.int_marker = InteractiveMarker()


class UpdateContext:
    """Represents an update to a single marker."""

    FULL_UPDATE = 0
    POSE_UPDATE = 1
    ERASE = 2

    def __init__(self):
        self.update_type = self.FULL_UPDATE
        self.int_marker = InteractiveMarker()
        self.default_feedback_callback = None
        self.feedback_callbacks = {}


class InteractiveMarkerServer:
    """
    A server to one or many clients (e.g. rviz) displaying a set of interactive markers.

    Note: Keep in mind that changes made by calling insert(), erase(), setCallback() etc.
          are not applied until calling applyChanges().
    """

    DEFAULT_FEEDBACK_CALLBACK = 255

    def __init__(
        self,
        node,
        namespace,
        *,
        update_pub_qos=QoSProfile(depth=100),
        feedback_sub_qos=QoSProfile(depth=1)
    ):
        """
        Create an InteractiveMarkerServer and associated ROS connections.

        :param node: The node to attach this interactive marker server to.
        :param namespace: The communication namespace of the interactie marker server.
            Clients that want to interact should connect with the same namespace.
        :param update_pub_qos: QoS settings for the update publisher.
        :param feedback_sub_qos: QoS settings for the feedback subscription.
        """
        self.node = node
        self.namespace = namespace
        self.seq_num = 0
        self.mutex = Lock()

        # contains the current state of all markers
        # string : MarkerContext
        self.marker_contexts = {}

        # updates that have to be sent on the next publish
        # string : UpdateContext
        self.pending_updates = {}

        get_interactive_markers_service_name = namespace + '/get_interactive_markers'
        update_topic = namespace + '/update'
        feedback_topic = namespace + '/feedback'

        self.get_interactive_markers_srv = self.node.create_service(
            GetInteractiveMarkers,
            get_interactive_markers_service_name,
            self.getInteractiveMarkersCallback
        )

        self.update_pub = self.node.create_publisher(
            InteractiveMarkerUpdate,
            update_topic,
            update_pub_qos
        )

        self.feedback_sub = self.node.create_subscription(
            InteractiveMarkerFeedback,
            feedback_topic,
            self.processFeedback,
            feedback_sub_qos
        )

    def shutdown(self):
        """
        Shutdown the interactive marker server.

        This should be called before the node is destroyed so that the internal ROS entities
        can be destroyed.
        """
        # It only makes sense to publish changes if the ROS context is still valid
        if self.node.context.ok():
            self.clear()
            self.applyChanges()
        if self.get_interactive_markers_srv is not None:
            self.node.destroy_service(self.get_interactive_markers_srv)
            self.get_interactive_markers_srv = None
        if self.update_pub is not None:
            self.node.destroy_publisher(self.update_pub)
            self.update_pub = None
        if self.feedback_sub is not None:
            self.node.destroy_subscription(self.feedback_sub)
            self.feedback_sub = None

    def __del__(self):
        """Destruction of the interface will lead to all managed markers being cleared."""
        self.shutdown()

    def insert(self, marker, *, feedback_callback=None, feedback_type=DEFAULT_FEEDBACK_CALLBACK):
        """
        Add or replace a marker.

        Note: Changes to the marker will not take effect until you call applyChanges().
        The callback changes immediately.

        :param marker: The marker to be added or replaced.
        :param feedback_callback: Function to call on the arrival of a feedback message.
        :param feedback_type: Type of feedback for which to call the feedback.
        """
        with self.mutex:
            if marker.name in self.pending_updates:
                update = self.pending_updates[marker.name]
            else:
                update = UpdateContext()
                self.pending_updates[marker.name] = update
            update.update_type = UpdateContext.FULL_UPDATE
            update.int_marker = marker
        if feedback_callback is not None:
            self.setCallback(marker.name, feedback_callback, feedback_type)

    def setPose(self, name, pose, header=Header()):
        """
        Update the pose of a marker with the specified name.

        Note: This change will not take effect until you call applyChanges()

        :param name: Name of the interactive marker.
        :param pose: The new pose.
        :param header: Header replacement. Leave this empty to use the previous one.
        :return: True if a marker with that name exists, False otherwise.
        """
        with self.mutex:
            marker_context = self.marker_contexts.get(name, None)
            update = self.pending_updates.get(name, None)
            # if there's no marker and no pending addition for it, we can't update the pose
            if marker_context is None and update is None:
                return False
            if update is not None and update.update_type == UpdateContext.FULL_UPDATE:
                return False

            if header.frame_id is None or header.frame_id == '':
                # keep the old header
                self.doSetPose(update, name, pose, marker_context.int_marker.header)
            else:
                self.doSetPose(update, name, pose, header)
            return True

    def erase(self, name):
        """
        Erase the marker with the specified name.

        Note: This change will not take effect until you call applyChanges().

        :param name: Name of the interactive marker.
        :return: True if a marker with that name exists, False otherwise.
        """
        with self.mutex:
            if name in self.pending_updates:
                self.pending_updates[name].update_type = UpdateContext.ERASE
                return True
            if name in self.marker_contexts:
                update = UpdateContext()
                update.update_type = UpdateContext.ERASE
                self.pending_updates[name] = update
                return True
            return False

    def clear(self):
        """
        Clear all markers.

        Note: This change will not take effect until you call applyChanges().
        """
        self.pending_updates = {}
        for marker_name in self.marker_contexts.keys():
            self.erase(marker_name)

    def setCallback(self, name, feedback_callback, feedback_type=DEFAULT_FEEDBACK_CALLBACK):
        """
        Add or replace a callback function for the specified marker.

        Note: This change will not take effect until you call applyChanges().
        The server will try to call any type-specific callback first.
        If a callback for the given type already exists, it will be replaced.
        To unset a callback, pass a value of None.

        :param name: Name of the interactive marker
        :param feedback_callback: Function to call on the arrival of a feedback message.
        :param feedback_type: Type of feedback for which to call the feedback.
            Leave this empty to make this the default callback.
        """
        with self.mutex:
            marker_context = self.marker_contexts.get(name, None)
            update = self.pending_updates.get(name, None)
            if marker_context is None and update is None:
                return False

            # we need to overwrite both the callbacks for the actual marker
            # and the update, if there's any
            if marker_context:
                # the marker exists, so we can just overwrite the existing callbacks
                if feedback_type == self.DEFAULT_FEEDBACK_CALLBACK:
                    marker_context.default_feedback_callback = feedback_callback
                else:
                    if feedback_callback:
                        marker_context.feedback_callbacks[feedback_type] = feedback_callback
                    elif feedback_type in marker_context.feedback_callbacks:
                        del marker_context.feedback_callbacks[feedback_type]
            if update:
                if feedback_type == self.DEFAULT_FEEDBACK_CALLBACK:
                    update.default_feedback_callback = feedback_callback
                else:
                    if feedback_callback:
                        update.feedback_callbacks[feedback_type] = feedback_callback
                    elif feedback_type in update.feedback_callbacks:
                        del update.feedback_callbacks[feedback_type]
            return True

    def applyChanges(self):
        """Apply changes made since the last call to this method and broadcast to clients."""
        with self.mutex:
            if len(self.pending_updates.keys()) == 0:
                return

            update_msg = InteractiveMarkerUpdate()
            update_msg.type = InteractiveMarkerUpdate.UPDATE

            for name, update in self.pending_updates.items():
                if update.update_type == UpdateContext.FULL_UPDATE:
                    if name in self.marker_contexts:
                        marker_context = self.marker_contexts[name]
                    else:
                        self.node.get_logger().debug('Creating new context for ' + name)
                        # create a new int_marker context
                        marker_context = MarkerContext(self.node.get_clock().now())
                        marker_context.default_feedback_callback = update.default_feedback_callback
                        marker_context.feedback_callbacks = update.feedback_callbacks
                        self.marker_contexts[name] = marker_context

                    marker_context.int_marker = update.int_marker
                    update_msg.markers.append(marker_context.int_marker)

                elif update.update_type == UpdateContext.POSE_UPDATE:
                    if name not in self.marker_contexts:
                        self.node.get_logger().error(
                            'Pending pose update for non-existing marker found. '
                            'This is a bug in InteractiveMarkerServer.')
                        continue

                    marker_context = self.marker_contexts[name]
                    marker_context.int_marker.pose = update.int_marker.pose
                    marker_context.int_marker.header = update.int_marker.header

                    pose_update = InteractiveMarkerPose()
                    pose_update.header = marker_context.int_marker.header
                    pose_update.pose = marker_context.int_marker.pose
                    pose_update.name = marker_context.int_marker.name
                    update_msg.poses.append(pose_update)

                elif update.update_type == UpdateContext.ERASE:
                    if name in self.marker_contexts:
                        marker_context = self.marker_contexts[name]
                        del self.marker_contexts[name]
                        update_msg.erases.append(name)
            self.pending_updates = {}

        self.seq_num += 1
        self.publish(update_msg)

    def get(self, name):
        """
        Get marker by name.

        :param name: Name of the interactive marker.
        :return: Marker if exists, None otherwise.
        """
        if name in self.pending_updates:
            update = self.pending_updates[name]
        elif name in self.marker_contexts:
            return self.marker_contexts[name].int_marker
        else:
            return None

        # if there's an update pending, we'll have to account for that
        if update.update_type == UpdateContext.ERASE:
            return None
        elif update.update_type == UpdateContext.POSE_UPDATE:
            if name not in self.marker_contexts:
                return None
            marker_context = self.marker_contexts[name]
            int_marker = marker_context.int_marker
            int_marker.pose = update.int_marker.pose
            return int_marker
        elif update.update_type == UpdateContext.FULL_UPDATE:
            return update.int_marker
        return None

    def processFeedback(self, feedback):
        """Update marker pose and call user callback."""
        with self.mutex:
            # ignore feedback for non-existing markers
            if feedback.marker_name not in self.marker_contexts:
                return

            marker_context = self.marker_contexts[feedback.marker_name]

            # if two callers try to modify the same marker, reject (timeout= 1 sec)
            time_since_last_feedback = self.node.get_clock().now() - marker_context.last_feedback
            if (marker_context.last_client_id != feedback.client_id and
                    time_since_last_feedback < Duration(seconds=1.0)):
                self.node.get_logger().debug(
                    "Rejecting feedback for '{}': conflicting feedback from separate clients"
                    .format(feedback.marker_name)
                )
                return

            marker_context.last_feedback = self.node.get_clock().now()
            marker_context.last_client_id = feedback.client_id

            if feedback.event_type == feedback.POSE_UPDATE:
                if (marker_context.int_marker.header.stamp == Time()):
                    # keep the old header
                    header = marker_context.int_marker.header
                else:
                    header = feedback.header

                if feedback.marker_name in self.pending_updates:
                    self.doSetPose(
                        self.pending_updates[feedback.marker_name],
                        feedback.marker_name,
                        feedback.pose,
                        header
                    )
                else:
                    self.doSetPose(None, feedback.marker_name, feedback.pose, header)

        # call feedback handler
        feedback_callback = marker_context.feedback_callbacks.get(
            feedback.event_type, marker_context.default_feedback_callback)
        if feedback_callback is not None:
            feedback_callback(feedback)

        # apply any pose updates
        self.applyChanges()

    def publish(self, update):
        """Increase the sequence number and publish an update."""
        update.seq_num = self.seq_num
        self.update_pub.publish(update)

    def getInteractiveMarkersCallback(self, request, response):
        """Process a service request to get the current interactive markers."""
        with self.mutex:
            response.sequence_number = self.seq_num

            self.node.get_logger().debug(
                'Markers requested. Responding with the following markers:'
            )
            for name, marker_context in self.marker_contexts.items():
                self.node.get_logger().debug('    ' + name)
                response.markers.append(marker_context.int_marker)

            return response

    def doSetPose(self, update, name, pose, header):
        """Schedule a pose update pose without locking."""
        if update is None:
            update = UpdateContext()
            update.update_type = UpdateContext.POSE_UPDATE
            self.pending_updates[name] = update
        elif update.update_type != UpdateContext.FULL_UPDATE:
            update.update_type = UpdateContext.POSE_UPDATE

        update.int_marker.pose = pose
        update.int_marker.header = header
