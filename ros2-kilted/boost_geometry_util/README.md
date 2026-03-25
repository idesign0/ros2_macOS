# boost_geometry_util

boost geometry friendly library for ROS2.

## What we can do.

You can use boost geometry with ROS2 very easily.

```cpp
bg::model::box<geometry_msgs::msg::Point> box(
    boost_geometry_util::point_3d::construct<geometry_msgs::msg::Point>(x_min, y_min, z_min),
    boost_geometry_util::point_3d::construct<geometry_msgs::msg::Point>(x_max, y_max, z_max));
const auto ros_point = boost_geometry_util::point_3d::construct<geometry_msgs::msg::Point>(x_max, y_max, z_max);
boost::geometry::disjoint(box, ros_point);
```
