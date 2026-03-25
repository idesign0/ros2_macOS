# Modifying Lanelet2 format for Autoware

## Overview

About the basics of the default format, please refer to main [Lanelet2 repository](https://github.com/fzi-forschungszentrum-informatik/Lanelet2). (see [here](https://github.com/fzi-forschungszentrum-informatik/Lanelet2/blob/master/lanelet2_core/doc/LaneletPrimitives.md) about primitives)

In addition to default Lanelet2 Format, users should add following mandatory/optional tags to their osm lanelet files as explained in reset of this document.
Users may use `autoware_lanelet2_validation` [node](../README.md#nodes) to check if their maps are valid.

The following is the extra format added for Autoware:

- [extra regulatory elements](extra_regulatory_elements.md)
  - Right Of Way
  - Detection Area
  - Road Marking
  - Speed Bump
  - Crosswalk
  - No Stopping Area
  - No Parking Area
  - Bus Stop Area
- [extra lanelet subtype](extra_lanelet_subtypes.md)
  - Roadside Lane
  - Road Shoulder
  - Pedestrian Lane
  - Bicycle Lane

Note that each `extra_regulatory_elements` and `extra_lanelet_subtypes` should faithfully represent the real world map primitives as-is for realizing appropriate behavior on public roads. Although they are not necessarily mandatory for running minimal Autoware, each ODD requires different set of `extra_regulatory_elements` and `extra_lanelet_subtypes` respectively.

[Autoware Vector Map Requirements](https://autowarefoundation.github.io/autoware-documentation/main/design/autoware-architecture/map/map-requirements/vector-map-requirements-overview/) provides more details and guidelines for how to create the Lanelet2 maps without ambiguity while avoiding unintended behaviors due to invalid Lanelet2 map settings.

## Mandatory Tags

### Meta Info

Users may add the `MetaInfo` element to their OSM file to indicate format version and map version of their OSM file. This information is not meant to influence Autoware vehicle's behavior, but is published as ROS message so that developers could know which map was used from ROSBAG log files. MetaInfo elements exists in the same hierarchy with `node`, `way`, and `relation` elements, otherwise JOSM wouldn't be able to load the file correctly.

Here is an example of MetaInfo in osm file:

```xml
<?xml version='1.0' encoding='UTF-8'?>
<osm version='0.6' generator='JOSM'>
  <MetaInfo format_version='1.0' map_version='1.0'/>
  <node>...</node>
  <way>...</way>
  <relation>...</relation>
</osm>
```

### Elevation Tags

Elevation("ele") information for points(`node`) is optional in default Lanelet2 format.
However, some of Autoware packages(e.g. trafficlight_recognizer) need elevation to be included in HD map. Therefore, users must make sure that all points in their osm maps contain elevation tags.

Here is an example osm syntax for node object.

```xml
<node id='1' visible='true' version='1' lat='49.00501435943' lon='8.41687458512'>
  <tag k='ele' v='3.0'/> <!-- this tag is mandatory for Autoware!! -->
</node>
```

### TrafficLights

Default Lanelet2 format uses LineString(`way`) or Polygon class to represent the shape of a traffic light. For Autoware, traffic light objects must be represented only by LineString to avoid confusion, where start point is at bottom left edge and end point is at bottom right edge. Also, "height" tag must be added in order to represent the size in vertical direction (not the position).

The Following image illustrates how LineString is used to represent shape of Traffic Light in Autoware.
![How LineString is used to represent shape of Traffic Light in Autoware](traffic_light.png)

Here is an example osm syntax for traffic light object.

```xml
<way id='13' visible='true' version='1'>
  <nd ref='6' />
  <nd ref='5' />
  <tag k='type' v='traffic_light' />
  <tag k='subtype' v='red_yellow_green' />
  <tag k='height' v='0.5'/> <!-- this tag is mandatory for Autoware!! -->
</way>
```

### Turn Directions

Users must add "turn_direction" tags to lanelets within intersections to indicate vehicle's turning direction. You do not need this tags for lanelets that are not in intersections. If you do not have this tag, Autoware will not be able to light up turning indicators.
This tags only take following values:

- left
- right
- straight

Following image illustrates how lanelets should be tagged.

![Turn Directions: How lanelets should be tagged](turn_direction.png)

Here is an example of osm syntax for lanelets in intersections.

```xml
<relation id='1' visible='true' version='1'>
  <member type='way' ref='2' role='left' />
  <member type='way' ref='3' role='right' />
  <member type='relation' ref='4' role='regulatory_element' />
  <tag k='location' v='urban' />
  <tag k='one_way' v='yes' />
  <tag k='subtype' v='road' />
  <tag k='type' v='lanelet' />
  <tag k='turn_direction' v='left' /> <!-- this tag is mandatory for lanelets at intersections!! -->
</relation>
```

## Optional Taggings

Following tags are optional tags that you may want to add depending on how you want to use your map in Autoware.

### Local Coordinate Expression

Sometimes users might want to create Lanelet2 maps that are not georeferenced.
In such a case, users may use "local_x", "local_y" taggings to express local positions instead of latitude and longitude.
You will need to `lanelet2_map_projector_type` to `Local`, then [autoware map loader](https://github.com/autowarefoundation/autoware_core/tree/main/map/autoware_map_loader#lanelet2_map_loader) will overwrite x,y positions with these tags when they are present.
For z values, use "ele" tags as default Lanelet2 Format.
You would still need to fill in lat and lon attributes so that parser does not crush, but their values could be anything.

Here is example `node` element in osm with "local_x", "local_y" taggings:

```xml
<!-- lat/lon attributes are required, but their values can be anything -->
<node id='40648' visible='true' version='1' lat='0' lon='0'>
  <tag k='local_x' v=2.54'/>
  <tag k='local_y' v=4.38'/>
  <tag k='ele' v='3.0'/>
</node>
```

### Light Bulbs in Traffic Lights

Default Lanelet format can only express shape (base + height) of traffic lights.
However, region_tlr node in Autoware uses positions of each light bulbs to recognize color of traffic light. If users may wish to use this node, "light_bulbs"(`way`) element must be registered to traffic_light regulatory_element object define position and color of each light bulb in traffic lights. If you are using other trafficlight_recognizer nodes(e.g. tlr_mxnet), which only uses bounding box of traffic light, then you do not need to add this object.

"light_bulbs" object is defined using LineString(`way`), and each node of line string is placed at the center of each light bulb. Also, each node should have "color" and optionally "arrow" tags to describe its type. Also, "traffic_light_id" tag is used to indicate which ID of relevant traffic_light element.

"color" tag is used to express the color of the light bulb. Currently only three values are used:

- red
- yellow
- green

"arrow" tag is used to express the direction of the arrow of light bulb:

- up
- right
- left
- up_right
- up_left

Following image illustrates how "light_bulbs" LineString should be created.

![How "light_bulbs" LineString should be created](light_bulbs.png)

Here is an example of osm syntax for light_bulb object:

```xml
<node id=1 version='1' lat='49.00541994701' lon='8.41565013855'>
  <tag k='ele' v='5'/>
  <tag k='color' v='red'/>
</node>
<node id=2 version='1' lat='49.00542091657' lon='8.4156469497'>
  <tag k='ele' v='5'/>
  <tag k='color' v='yellow'/>
</node>
<node id=3 version='1' lat='49.00542180052' lon='8.41564400223'>
  <tag k='ele' v='5'/>
  <tag k='color' v='green'/>
</node>
<node id=3 version='1' lat='49.00542180052' lon='8.41564400223'>
  <tag k='ele' v='4.6'/>
  <tag k='color' v='green'/>
  <tag k=arrow v='right'/>
</node>
<way id=11 version='1'>
  <nd ref=1 />
  <nd ref=2 />
  <nd ref=3 />
  <tag k='traffic_light_id' v='10'/> <!-- id of linestring with type="traffic_light" -->
  <tag k='type' v='light_bulbs' />
</way>
```

After creating "light_bulbs" elements, you have to register them to traffic_light regulatory element as role "light_bulbs".
The following illustrates how light_bulbs are registered to traffic_light regulatory elements.

![How light_bulbs are registered to traffic_light regulatory elements](traffic_light_regulatory_element.png)

```xml
<relation id='8' visible='true' version='1'>
  <tag k='type' v='regulatory_element' />
  <tag k='subtype' v='traffic_light' />
  <member type='way' ref='9' role='ref_line' />
  <member type='way' ref='10' role='refers' /> <!-- refers to the traffic light line string -->
  <member type='way' ref='11' role='light_bulbs'/> <!-- refers to the light_bulb line string -->
</relation>
```

### Centerline

Note that the following explanation is not related to the Lanelet2 map format but how the Autoware handles the centerline in the Lanelet2 map.

Centerline is defined in Lanelet2 to guide the vehicle. By explicitly setting the centerline in the Lanelet2 map, the ego's planned path follows the centerline.
However, based on the current Autoware's usage of the centerline, there are several limitations.

- The object's predicted path also follows the centerline.
  - This may adversely affect the decision making of planning modules since the centerline is supposed to be used only by the ego's path planning.
- The coordinate transformation on the lane's frenet frame cannot be calculated correctly.
- For example, when the lateral distance between the actual road's centerline and a parked vehicle is calculated, actually the result will be the lateral distance between the (explicit) centerline and the vehicle.

To solve above limitations, the `overwriteLaneletsCenterlineWithWaypoints` was implemented in addition to `overwriteLaneletsCenterline` where the centerline in all the lanes is calculated.

- `overwriteLaneletsCenterlineWithWaypoints`
  - The (explicit) centerline in the Lanelet2 map is converted to the new `waypoints` tag. This `waypoints` is only applied to the ego's path planning.
  - Therefore, the above limitations can be solved, but the Autoware's usage of the centerline may be hard to understand.
- `overwriteLaneletsCenterline`
  - The (explicit) centerline in the Lanelet2 map is used as it is. Easy to understand the Autoware's usage of the centerline, but we still have above limitations.

### No Obstacle Segmentation Area

If there is a polygon area that has `no_obstacle_segmentation_area` tag, the obstacle points in this area are removed.
If you want to ignore points for a certain module, you have to define another tag and specify it in the parameter of vector_map_inside_area_filter.
Currently, following tags are defined other than `no_obstacle_segmentation_area`.

- `no_obstacle_segmentation_area_for_run_out`
  - remove points for run out module

_An example:_

```xml
  <way id="1658">
    <nd ref="1653"/>
    <nd ref="1654"/>
    <nd ref="1656"/>
    <nd ref="1657"/>
    <tag k="type" v="no_obstacle_segmentation_area"/>
    <tag k="area" v="yes"/>
  </way>
```

### Hatched Road Markings Area

The area with `hatched_road_markings` tag can be used for avoiding obstacles when there is not enough space to avoid.
Note that in some countries, it is forbidden for vehicles to go inside the area.

_An example:_

```xml
  <way id="9933">
    <nd ref="2058"/>
    <nd ref="2059"/>
    <nd ref="1581"/>
    <nd ref="2057"/>
    <nd ref="4925"/>
    <nd ref="4923"/>
    <nd ref="4921"/>
    <nd ref="4920"/>
    <tag k="type" v="hatched_road_markings"/>
    <tag k="area" v="yes"/>
  </way>
```

### No Drivable Lane

A no drivable lane is a lanelet or more that are out of operation design domain (ODD), i.e., the vehicle **must not** drive autonomously in this/these lanelet/s.
A lanelet becomes no drivable by adding an optional tag under the relevant lanelet in the map file `<tag k="no_drivable_lane" v="yes"/>`.

_An example:_

```xml
<relation id="2621">
    <member type="way" role="left" ref="2593"/>
    <member type="way" role="right" ref="2620"/>
    <tag k="type" v="lanelet"/>
    <tag k="subtype" v="road"/>
    <tag k="speed_limit" v="30.00"/>
    <tag k="location" v="urban"/>
    <tag k="one_way" v="yes"/>
    <tag k="participant:vehicle" v="yes"/>
    <tag k="no_drivable_lane" v="yes"/>
  </relation>
```

For more details about the `no_drivable_lane` concept and design, please refer to the [**_no-drivable-lane-design_**](https://github.com/autowarefoundation/autoware.universe/blob/main/planning/behavior_velocity_planner/autoware_behavior_velocity_no_drivable_lane_module/README.md) document.

### Map Filter Area

If there is a polygon area that has `obstacle_removal_area` tag, the pointcloud and objects in this area are removed.
For more details, please refer <https://github.com/tier4/l4_toolkit/tree/main/obstacle_removal_area_filter>

### Localization Landmarks

Landmarks, such as AR-Tags, can be defined into the lanelet map to aid localization module.

Landmark Specifications:

<!-- cspell:ignore apiltag -->

- **Shape**: Landmarks must be flat and defined as a polygon with exactly four vertices.
- **Vertex Definition**: Vertices must be defined in a counter-clockwise order.
- **Coordinate System** and Vertex Order:
  - The x-axis must be parallel to the vector extending from the first vertex to the second vertex.
  - The y-axis must be parallel to the vector extending from the second vertex to the third vertex.
- **Required Attributes**:
  - Specify pose_marker for type `<tag k="type" v="pose_marker"/>`
  - Specify the type of the landmark for subtype
    - [AR-Tag](https://autowarefoundation.github.io/autoware.universe/main/localization/autoware_landmark_based_localizer/autoware_ar_tag_based_localizer/): `<tag k="subtype" v="apriltag_16h5"/>`
    - [Reflector](https://autowarefoundation.github.io/autoware.universe/main/localization/autoware_landmark_based_localizer/autoware_lidar_marker_localizer/): `<tag k="subtype" v="reflector"/>`
  - Specify the ID of the landmark for marker_id `<tag k="marker_id" v="0"/>`
    - ID can be assigned arbitrarily as a string, but the same ID must be assigned for the same marker type
    - For example, apiltag_16h5 has 30 different IDs from 0 to 29. If multiple tags of the same type are to be placed in one environment, they should be assigned the same ID.

![localization_landmark](./localization_landmark.drawio.svg)

_An example:_

```xml
...

  <node id="1" lat="35.8xxxxx" lon="139.6xxxxx">
    <tag k="mgrs_code" v="99XXX000000"/>
    <tag k="local_x" v="22.2356"/>
    <tag k="local_y" v="87.4506"/>
    <tag k="ele" v="2.1725"/>
  </node>
  <node id="2" lat="35.8xxxxx" lon="139.6xxxxx">
    <tag k="mgrs_code" v="99XXX000000"/>
    <tag k="local_x" v="22.639"/>
    <tag k="local_y" v="87.5886"/>
    <tag k="ele" v="2.5947"/>
  </node>
  <node id="3" lat="35.8xxxxx" lon="139.6xxxxx">
    <tag k="mgrs_code" v="99XXX000000"/>
    <tag k="local_x" v="22.2331"/>
    <tag k="local_y" v="87.4713"/>
    <tag k="ele" v="3.0208"/>
  </node>
  <node id="4" lat="35.8xxxxx" lon="139.6xxxxx">
    <tag k="mgrs_code" v="99XXX000000"/>
    <tag k="local_x" v="21.8298"/>
    <tag k="local_y" v="87.3332"/>
    <tag k="ele" v="2.5985"/>
  </node>

...

  <way id="5">
    <nd ref="1"/>
    <nd ref="2"/>
    <nd ref="3"/>
    <nd ref="4"/>
    <tag k="type" v="pose_marker"/>
    <tag k="subtype" v="apriltag_16h5"/>
    <tag k="area" v="yes"/>
    <tag k="marker_id" v="0"/>
  </way>

...

```
