# autoware_lanelet2_extension package

This package contains external library for Lanelet2 and is meant to ease the use of Lanelet2 in Autoware.

## Lanelet Format for Autoware

Autoware uses extended Lanelet2 Format for Autoware, which means you need to add some tags to default OSM file if you want to fully use Lanelet2 maps. For details about custom tags, please refer to this [document](./docs/lanelet2_format_extension.md).

## Code API

### IO

#### Autoware OSM Parser

Autoware Lanelet2 Format uses .osm extension as original Lanelet2.
However, there are some custom tags that is used by the parser.

Currently, this includes:

- reading `<MapMetaInfo>` tag which contains information about map format version and map version.

The parser is registered as "autoware_osm_handler" as lanelet parser

### Projection

#### MGRS Projector

MGRS projector projects latitude longitude into MGRS Coordinates.

#### Transverse Mercator Projector

This projector projects latitude and longitude into Transverse Mercator coordinate.
This is similar to [UTM projector](https://github.com/fzi-forschungszentrum-informatik/Lanelet2/blob/master/lanelet2_projection/src/UTM.cpp) in the Lanelet2 library, but uses a custom central meridian on a given origin.

### Regulatory Elements

#### Autoware Traffic Light

Autoware Traffic Light class allows you to retrieve information about traffic lights.
Autoware Traffic Light class contains following members:

- traffic light shape
- light bulbs information of traffic lights
- stopline associated to traffic light

#### Right Of Way

Autoware intersection module requires the information on which lanes can be ignored for object detection, like those lanes whose traffic light color is red when that of ego is green, etc. `right_of_way` tag is a regulatory element that consists of a tuple of (`right_of_way`s and `yield_lane`s), and `right_of_way`s have priority over`yield_lane`s. Although `right_of_way` tag itself is defined in Lanelet2, this package provides proper conditions on how this tag should be set.

### Utility

#### Message Conversion

This contains functions to convert lanelet map objects into ROS messages.
Currently it contains following conversions:

- lanelet::LaneletMapPtr to/from autoware_map_msgs::msg::LaneletMapBin
- lanelet::Point3d to geometry_msgs::Point
- lanelet::Point2d to geometry_msgs::Point
- lanelet::BasicPoint3d to geometry_msgs::Point

#### Query

This module contains functions to retrieve various information from maps.
e.g. crosswalks, trafficlights, stoplines

#### Utilities

This module contains other useful functions related to Lanelet.
e.g. matching waypoint with lanelets

#### Route Checker

This module contains a function to check the loading route is valid or not.
If it is invalid, puts warning without dying.
The case valid is when the route is created on the same map as the current one.

### Visualization

Visualization contains functions to convert lanelet objects into visualization marker messages.
Currently it contains following conversions:

- lanelet::Lanelet to Triangle Markers
- lanelet::LineString to LineStrip Markers
- TrafficLights to Triangle Markers

## Nodes

### autoware_lanelet2_extension_sample

Code for this explains how this autoware_lanelet2_extension library is used.
The executable is not meant to do anything.

### autoware_lanelet2_extension

This node checks if an .osm file follows the Autoware version of Lanelet2 format.
You can check by running:

```sh
ros2 run autoware_lanelet2_extension autoware_lanelet2_validation --ros-args -p map_file:=<path/to/map.osm>
```
