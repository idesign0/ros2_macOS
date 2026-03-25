import autoware_lanelet2_extension_python._autoware_lanelet2_extension_python_boost_python_utility as _utility_cpp
from geometry_msgs.msg import Point
from geometry_msgs.msg import Pose
from geometry_msgs.msg import Quaternion
import lanelet2
from rclpy.serialization import serialize_message

combineLaneletsShape = _utility_cpp.combineLaneletsShape
generateFineCenterline = _utility_cpp.generateFineCenterline
getCenterlineWithOffset = _utility_cpp.getCenterlineWithOffset
getRightBoundWithOffset = _utility_cpp.getRightBoundWithOffset
getLeftBoundWithOffset = _utility_cpp.getLeftBoundWithOffset
getExpandedLanelet = _utility_cpp.getExpandedLanelet
getExpandedLanelets = _utility_cpp.getExpandedLanelets
overwriteLaneletsCenterline = _utility_cpp.overwriteLaneletsCenterline
getConflictingLanelets = _utility_cpp.getConflictingLanelets
lineStringWithWidthToPolygon = _utility_cpp.lineStringWithWidthToPolygon
lineStringToPolygon = _utility_cpp.lineStringToPolygon


def getLaneletLength2d(*args):
    if len(args) == 1 and isinstance(args[0], lanelet2.core.Lanelet):
        return _utility_cpp.getLaneletLength2d(args[0])
    if len(args) == 1 and isinstance(args[0], list):
        return _utility_cpp.getLaneletLength2d(args[0])
    raise TypeError(
        "argument number does not match or 1st argument is not Lanelet or [Lanelet] type"
    )


def getLaneletLength3d(*args):
    if len(args) == 1 and isinstance(args[0], lanelet2.core.Lanelet):
        return _utility_cpp.getLaneletLength3d(args[0])
    if len(args) == 1 and isinstance(args[0], list):
        return _utility_cpp.getLaneletLength3d(args[0])
    raise TypeError(
        "argument number does not match or 1st argument is not Lanelet or [Lanelet] type"
    )


def getArcCoordinates(lanelet_sequence, pose: Pose):
    pose_byte = serialize_message(pose)
    return _utility_cpp.getArcCoordinates(lanelet_sequence, pose_byte)


getClosestSegment = _utility_cpp.getClosestSegment
getPolygonFromArcLength = _utility_cpp.getPolygonFromArcLength


def getLaneletAngle(lanelet, point: Point):
    point_byte = serialize_message(point)
    return _utility_cpp.getLaneletAngle(lanelet, point_byte)


def isInLanelet(pose: Pose, lanelet, radius=0.0):
    pose_byte = serialize_message(pose)
    return _utility_cpp.isInLanelet(pose_byte, lanelet, radius)


def getClosestCenterPose(lanelet, point: Point):
    point_byte = serialize_message(point)
    pose_array = _utility_cpp.getClosestCenterPose(lanelet, point_byte)
    pos = Point(x=pose_array[0], y=pose_array[1], z=pose_array[2])
    quat = Quaternion(x=pose_array[3], y=pose_array[4], z=pose_array[5], w=pose_array[6])
    return Pose(position=pos, orientation=quat)


def getLateralDistanceToCenterline(lanelet, pose: Pose):
    pose_byte = serialize_message(pose)
    return _utility_cpp.getLateralDistanceToCenterline(lanelet, pose_byte)


def getLateralDistanceToClosestLanelet(lanelet_sequence, pose: Pose):
    pose_byte = serialize_message(pose)
    return _utility_cpp.getLateralDistanceToClosestLanelet(lanelet_sequence, pose_byte)
