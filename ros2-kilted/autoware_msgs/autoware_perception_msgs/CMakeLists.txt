cmake_minimum_required(VERSION 3.14)
project(autoware_perception_msgs)

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/DetectedObject.msg"
  "msg/DetectedObjectKinematics.msg"
  "msg/DetectedObjects.msg"
  "msg/ObjectClassification.msg"
  "msg/PredictedObject.msg"
  "msg/PredictedObjectKinematics.msg"
  "msg/PredictedObjects.msg"
  "msg/PredictedPath.msg"
  "msg/PredictedTrafficLightState.msg"
  "msg/Shape.msg"
  "msg/TrackedObject.msg"
  "msg/TrackedObjectKinematics.msg"
  "msg/TrackedObjects.msg"
  "msg/TrafficLightElement.msg"
  "msg/TrafficLightGroup.msg"
  "msg/TrafficLightGroupArray.msg"
  # TODO(youtalk): Remove below after migration to TrafficLight
  "msg/TrafficSignalElement.msg"
  "msg/TrafficSignal.msg"
  "msg/TrafficSignalArray.msg"

  DEPENDENCIES
    std_msgs
    geometry_msgs
    builtin_interfaces
    unique_identifier_msgs

  ADD_LINTER_TESTS
)

ament_auto_package()
