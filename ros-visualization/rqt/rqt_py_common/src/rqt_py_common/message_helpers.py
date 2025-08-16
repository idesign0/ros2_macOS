# Copyright (c) 2018, PickNik Robotics
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
#   * Neither the name of the PickNik Robotics nor the names of its
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

# Author: Michael Lautman
import importlib

from ament_index_python import get_resource
from ament_index_python import get_resources

from rclpy import logging

MSG_MODE = 'msg'
SRV_MODE = 'srv'
ACTION_MODE = 'action'

ROSIDL_FILTERS = {
    MSG_MODE: lambda n: n.startswith('msg/') and n.endswith('.msg'),
    SRV_MODE: lambda n: n.startswith('srv/') and n.endswith('.srv'),
    ACTION_MODE: lambda n: n.startswith('action/') and n.endswith('.idl')
}


def _strip_type_name(type_name):
    prefixes = ['msg/', 'srv/', 'action/']
    suffixes = ['.msg', '.srv', '.idl']
    for prefix, suffix in zip(prefixes, suffixes):
        found_match = False
        if type_name.startswith(prefix):
            type_name = type_name[len(prefix):]
            found_match = True
        if type_name.endswith(suffix):
            type_name = type_name[:len(type_name) - len(suffix)]
            found_match = True
        if found_match:
            break
    return type_name


def _filter_rosidl_types(rosidl_type, interface_names):
    """
    Filter a list of rosidl interfaces with a specific type.

    :param rosidl_type: rosidl type to filter by, one of ['msg', 'srv', 'action']
    :type rosidl_type: str
    :param interface_names: list of interface names return from ament_index_python.get_resource
    :type interface_names: list(str)
    :returns: a list of the filtered rosidl types
    """
    if rosidl_type not in ROSIDL_FILTERS:
        raise ValueError('Invalid rosidl_type type "{}". Needs to be one of {}'.format(
            rosidl_type, ROSIDL_FILTERS.keys()))

    filter_fn = ROSIDL_FILTERS[rosidl_type]
    filtered = filter(filter_fn, interface_names)
    stripped = map(_strip_type_name, filtered)
    return list(stripped)


def get_rosidl_types_of_type(package_name, rosidl_type):
    """
    Retrieve a list of rosidl interfaces of a specific type provided by a ROS2 package.

    :param package_name: ROS2 package name
    :type package_name: str
    :param rosidl_type: rosidl type to filter by, one of ['msg', 'srv', 'action']
    :type rosidl_type: str
    :returns: a list of the filtered rosidl types
    """
    try:
        content, _ = get_resource('rosidl_interfaces', package_name)
    except LookupError:
        return []
    interface_names = content.splitlines()
    return _filter_rosidl_types(rosidl_type, interface_names)


def get_rosidl_types(package_name):
    """
    Retrieve a dictionary of lists for all rosidl interface types provided by a ROS2 package.

    :param package_name: ROS2 package name
    :type package_name: str
    :returns: a dictionary mapping rosidl type to a list {'msg': [...], 'srv': ... }
    """
    rosidl_types = {}
    try:
        content, _ = get_resource('rosidl_interfaces', package_name)
    except LookupError:
        return []
    interface_names = content.splitlines()
    for filter_type in ROSIDL_FILTERS.keys():
        rosidl_types[filter_type] = _filter_rosidl_types(filter_type, interface_names)
    return rosidl_types


def get_all_rosidl_types():
    """
    Retrieve a dictionary mapping packages to their provided rosidl interfaces.

    :returns: a dictionary mapping packages to their types
    """
    all_rosidl_types = {}
    for package_name in get_resources('rosidl_interfaces').keys():
        all_rosidl_types[package_name] = get_rosidl_types(package_name)
    return all_rosidl_types


def get_all_rosidl_types_of_type(rosidl_type):
    """
    Retrieve a dictionary mapping packages to a specific rosidl type.

    :returns: a dictionary mapping packages to the rosidl interfaces of a specific type
    """
    if rosidl_type not in ROSIDL_FILTERS:
        raise ValueError('Invalid rosidl_type type "{}". Needs to be one of {}'.format(
            filter_type, ROSIDL_FILTERS.keys()))

    all_rosidl_types = get_all_rosidl_types()
    return {
        package_name:
            d[rosidl_type] for package_name, d in all_rosidl_types.items() if d[rosidl_type]
    }


def get_all_service_types():
    """
    Uses the ament index to iterate through packages and calls get_service_types on each one.

    :returns: a dictionary of the form {'package_name', ['srv1', 'srv2', ...]}
    """
    return get_all_rosidl_types_of_type(SRV_MODE)


def get_service_types(package_name):
    """
    Uses the ament index gind all services avialable in the package.

    :param package_name: a string eg 'std_srvs'
    :returns: a dictionary of the form {'package_name', ['srv1', 'srv2', ...]}
    """
    return get_rosidl_types_of_type(package_name, SRV_MODE)


def get_all_message_types():
    """
    Uses the ament index to iterate through packages and calls get_message_types on each one.

    :returns: a dictionary of the form {'package_name', ['msg1', 'msg2', ...]}
    """
    return get_all_rosidl_types_of_type(MSG_MODE)


def get_message_types(package_name):
    """
    Uses the ament index to find all messages avialable in the package.

    :param package_name: a string eg 'std_msgs'
    :returns: a dictionary of the form {'std_msgs', ['Bool', 'String', ...]}
    """
    return get_rosidl_types_of_type(package_name, MSG_MODE)


def get_all_action_types():
    """
    Get all the packages that support actions and their action types.

    :returns: a dictionary of the form {'package_name', ['action1', 'action2', ...]}
    """
    return get_all_rosidl_types_of_type(ACTION_MODE)


def get_action_types(package_name):
    """
    Find all actions available in the package.

    :param package_name: a string eg 'test_msgs'
    :returns: a list of action names ['Fibonacci', ...]
    """
    return get_rosidl_types_of_type(package_name, ACTION_MODE)


def _get_rosidl_class_helper(message_type, mode, logger=None):  # noqa: C901
    """
    A helper function for common logic to be used by get_message_class and get_service_class.

    :param message_type: name of the message or service class in the form
      'package_name/MessageName' or 'package_name/msg/MessageName'
    :type message_type: str
    :param mode: one of MSG_MODE, SRV_MODE or ACTION_MODE
    :type mode: str
    :param logger: The logger to be used for warnings and info
    :type logger: either rclpy.impl.rcutils_logger.RcutilsLogger or None

    :returns: The message or service class or None
    """
    if logger is None:
        logger = logging.get_logger('_get_message_service_class_helper')

    if mode not in ROSIDL_FILTERS.keys():
        logger.warn('invalid mode {}'.format(mode))
        return None

    message_info = message_type.split('/')
    if len(message_info) not in (2, 3):
        logger.error('Malformed message_type: {}'.format(message_type))
        return None
    if len(message_info) == 3 and message_info[1] != mode:
        logger.error('Malformed {} message_type: {}'.format(mode, message_type))
        return None

    package = message_info[0]
    base_type = message_info[-1]

    try:
        _, resource_path = get_resource('rosidl_interfaces', package)
    except LookupError:
        return None
    python_pkg = None
    class_val = None

    try:
        # import the package
        python_pkg = importlib.import_module('%s.%s' % (package, mode))
    except ImportError:
        logger.info('Failed to import class: {} as {}.{}'.format(message_type, package, mode))
        return None

    try:
        class_val = getattr(python_pkg, base_type)
        return class_val

    except AttributeError:
        logger.info('Failed to load class: {}'.format(message_type))
        return None


_srv_class_cache = {}


def get_service_class(srv_type):
    """
    Gets the service class from a string representation.

    :param srv_type: the type of service in the form
      `package_name/ServiceName` or `package_name/srv/ServiceName`
    :type srv_type: str

    :returns: None or the Class
    """
    if srv_type in _srv_class_cache:
        return _srv_class_cache[srv_type]

    logger = logging.get_logger('get_service_class')
    class_val = _get_rosidl_class_helper(srv_type, SRV_MODE, logger)

    if class_val is not None:
        _srv_class_cache[srv_type] = class_val

    return class_val


_message_class_cache = {}


def get_message_class(message_type):
    """
    Gets the message class from a string representation.

    :param message_type: the type of message in the form `msg_pkg/Message`
    :type message_type: str

    :returns: None or the Class
    """
    if message_type in _message_class_cache:
        return _message_class_cache[message_type]

    logger = logging.get_logger('get_message_class')

    class_val = _get_rosidl_class_helper(message_type, MSG_MODE, logger)

    if class_val is not None:
        _message_class_cache[message_type] = class_val

    return class_val


_action_class_cache = {}


def get_action_class(action_type):
    """
    Gets the action class from a string representation.

    :param action_type: the type of action in the form `action_pkg/Action`
    :type action_type: str

    :returns: None or the Class
    """
    if action_type in _action_class_cache:
        return _action_class_cache[action_type]

    logger = logging.get_logger('get_action_class')
    class_val = _get_rosidl_class_helper(action_type, ACTION_MODE, logger)

    if class_val is not None:
        _action_class_cache[action_type] = class_val

    return class_val


def get_message_text_from_class(msg_class):
    """Get a string representation of the message class."""
    msg_slot_dict = msg_class.get_fields_and_field_types()
    return ''.join(
        ['{0} {1}\n'.format(slot_type, slot_name) for
            slot_name, slot_type in msg_slot_dict.items()])


def get_service_text_from_class(srv_class):
    """Get a string representation of the message class."""
    srv_slot_dict_req = srv_class.Request.get_fields_and_field_types()
    srv_slot_dict_res = srv_class.Response.get_fields_and_field_types()
    srv_txt = [
        '{0} {1}\n'.format(slot_type, slot_name) for
        slot_name, slot_type in srv_slot_dict_req.items()]
    srv_txt.extend(['---\n'])
    srv_txt.extend([
        '{0} {1}\n'.format(slot_type, slot_name) for
        slot_name, slot_type in srv_slot_dict_res.items()])
    return ''.join(srv_txt)


def get_action_text_from_class(action_class):
    """Get a string representation of the message class."""
    action_slot_dict_goal = action_class.Goal.get_fields_and_field_types()
    action_slot_dict_res = action_class.Result.get_fields_and_field_types()
    action_slot_dict_feedback = action_class.Feedback.get_fields_and_field_types()
    action_txt = [
        '{0} {1}\n'.format(slot_type, slot_name) for
        slot_name, slot_type in action_slot_dict_goal.items()]
    action_txt.extend(['---\n'])
    action_txt.extend([
        '{0} {1}\n'.format(slot_type, slot_name) for
        slot_name, slot_type in action_slot_dict_res.items()])
    action_txt.extend(['---\n'])
    action_txt.extend([
        '{0} {1}\n'.format(slot_type, slot_name) for
        slot_name, slot_type in action_slot_dict_feedback.items()])
    return ''.join(action_txt)
