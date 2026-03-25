cmake_minimum_required(VERSION 3.14)
project(autoware_lanelet2_extension)

find_package(autoware_cmake REQUIRED)
find_package(tf2 REQUIRED)
autoware_package()

ament_auto_add_library(${PROJECT_NAME}_lib SHARED
  lib/autoware_osm_parser.cpp
  lib/autoware_traffic_light.cpp
  lib/crosswalk.cpp
  lib/detection_area.cpp
  lib/landmark.cpp
  lib/no_parking_area.cpp
  lib/no_stopping_area.cpp
  lib/bus_stop_area.cpp
  lib/message_conversion.cpp
  lib/mgrs_projector.cpp
  lib/query.cpp
  lib/roundabout.cpp
  lib/road_marking.cpp
  lib/speed_bump.cpp
  lib/transverse_mercator_projector.cpp
  lib/utilities.cpp
  lib/deprecated.cpp
  lib/virtual_traffic_light.cpp
  lib/visualization.cpp
  lib/route_checker.cpp
)

# Suppress boost geometry uninitialized variable warnings
# This is a known issue in boost geometry library where internal template code
# may trigger maybe-uninitialized warnings that are false positives.
# The warnings occur in boost geometry's distance calculation algorithms
# and do not affect the actual functionality of the code.
# Related: https://www.boost.org/doc/libs/latest/libs/utility/doc/html/utility/utilities/value_init.html
# and https://www.boost.org/doc/libs/1_89_0/libs/optional/doc/html/boost_optional/design/gotchas/false_positive_with__wmaybe_uninitialized.html
target_compile_options(${PROJECT_NAME}_lib PRIVATE -Wno-error=maybe-uninitialized)

if(BUILD_TESTING)
  ament_add_ros_isolated_gtest(projector-test test/src/test_projector.cpp)
  target_link_libraries(projector-test ${PROJECT_NAME}_lib ${tf2_LIBRARIES})
  ament_add_ros_isolated_gtest(query-test test/src/test_query.cpp)
  target_link_libraries(query-test ${PROJECT_NAME}_lib ${tf2_LIBRARIES})
  ament_add_ros_isolated_gtest(regulatory_elements-test test/src/test_regulatory_elements.cpp)
  target_link_libraries(regulatory_elements-test ${PROJECT_NAME}_lib ${tf2_LIBRARIES})
  ament_add_ros_isolated_gtest(utilities-test test/src/test_utilities.cpp)
  target_link_libraries(utilities-test ${PROJECT_NAME}_lib ${tf2_LIBRARIES})
  ament_add_ros_isolated_gtest(route-test test/src/test_route_checker.cpp)
  target_link_libraries(route-test ${PROJECT_NAME}_lib ${tf2_LIBRARIES})
  ament_add_ros_isolated_gtest(normalize-radian test/src/test_normalize_radian.cpp)
  target_link_libraries(normalize-radian ${PROJECT_NAME}_lib ${tf2_LIBRARIES})
endif()

ament_auto_package()
