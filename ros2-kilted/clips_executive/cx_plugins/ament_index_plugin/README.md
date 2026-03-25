# cx_ament_index_plugin
This package offers the `cx::AmentIndexPlugin' that provides the functions from ament_index_cpp to CLIPS.

## Usage
Register this plugin with the plugin manager. It requires no additional configuration, an example setup is shown below:

```yaml
clips_manager:
  ros__parameters:
    environments: ["main"]
    main:
      plugins: ["ament_index"]

    ament_index:
      plugin: "cx::AmentIndexPlugin"
```

## CLIPS Features
This plugin defines user-defined functions that are described below.
Refer to the [ament_index_cpp API](https://docs.ros.org/en/rolling/p/ament_index_cpp/generated/index.html) for detailed information.

Note that all functions catch potential exceptions thrown by `ament_index_cpp` and return `FALSE` in that case.
### Functions
```lisp
(bind ?ret (ament-index-get-package-prefix ?package-name))
; example args: "cx_bringup"
; example ret:  "/opt/ros/<ros-distro>

(bind ?ret (ament-index-get-package-share-directory ?package-name))
; example args: "cx_bringup"
; example ret:  "/opt/ros/<ros-distro>/share/cx_bringup"

(bind ?ret (ament-index-get-packages-with-prefixes))
; example ret:  ("rclcpp" "/opt/ros/<ros-distro>" "cx_bringup" "/opt/ros/<ros-distro>" ...)

(bind ?ret (ament-index-has-resource ?res-type ?res-name))
; example args: "vendor_packages" "clips_vendor"
; example ret:  TRUE

(bind ?ret (ament-index-get-resource ?res-type ?res-name))
; example args: "vendor_packages" "clips_vendor"
; example ret: ("opt/clips_vendor" "/opt/ros/<ros-distro>")

(bind ?ret (ament-index-get-resources ?res-type))
; example args: "vendor_packages"
; example ret: ("clips_vendor" "/opt/ros/<ros-distro>" "<other-vendor>" "<path-to-other-vendor"> ...)

(bind ?prefix (ament-index-get-search-paths))
; example ret: ("/opt/ros/<ros-distro>" "<path-to-other-prefix>" ...)
```
