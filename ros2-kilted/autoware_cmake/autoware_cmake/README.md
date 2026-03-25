# autoware_cmake

This package provides CMake scripts for Autoware.

## Usage

### autoware_package.cmake

Call `autoware_package()` before defining build targets, which will set common options for Autoware.

```cmake
cmake_minimum_required(VERSION 3.5)
project(package_name)

find_package(autoware_cmake REQUIRED)
autoware_package()

ament_auto_add_library(...)
```

### autoware_ament_auto_package.cmake

Use `autoware_ament_auto_package()` as a replacement for `ament_auto_package()` to maintain Autoware's include structure across ROS 2 Humble, Jazzy, and Kilted.

This macro addresses a naming convention conflict between Autoware packages (which use `autoware_<module>` as package names) and Autoware include paths (which use `autoware/<module>/`). Starting with ROS 2 Kilted, `ament_auto_package()` will install headers to `include/${PROJECT_NAME}/`, which would create incorrect paths like `include/autoware_interpolation/autoware/interpolation/`.

#### Why use this?

- **Eliminates deprecation warnings** on ROS 2 Humble and Jazzy
- **Future-proof**: Works with ROS 2 Kilted+ without breaking changes
- **Maintains conventions**: Preserves `autoware/<module>/` include structure
- **Drop-in replacement**: Zero behavior change from `ament_auto_package()`

#### Example

```cmake
cmake_minimum_required(VERSION 3.14)
project(autoware_interpolation)

find_package(autoware_cmake REQUIRED)
autoware_package()

ament_auto_add_library(autoware_interpolation SHARED
  src/linear_interpolation.cpp
  src/spline_interpolation.cpp
)

if(BUILD_TESTING)
  # ... tests ...
endif()

# Use autoware_ament_auto_package() instead of ament_auto_package()
autoware_ament_auto_package()
```

#### Migration from ament_auto_package()

Simply replace `ament_auto_package()` with `autoware_ament_auto_package()` at the end of your `CMakeLists.txt`. All parameters supported by `ament_auto_package()` are also supported:

- `INSTALL_TO_PATH`: Install executables to `bin/` instead of `lib/${PROJECT_NAME}/`
- `INSTALL_TO_SHARE`: Install additional directories to `share/${PROJECT_NAME}/`
