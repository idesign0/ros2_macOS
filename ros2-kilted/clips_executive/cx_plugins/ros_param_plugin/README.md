# ros_param_plugin
This package offers the `cx::RosParamPlugin' that fetches parameters from the
ROS node running the CLIPS manager..

## Usage
Register this plugin with the plugin manager. It requires no additional configuration, an example setup is shown below:

```yaml
clips_manager:
  ros__parameters:
    environments: ["main"]
    main:
      plugins: ["ros_param"]

    ros_param:
      plugin: "cx::RosParamPlugin"
```

## CLIPS Features
This plugin defines user-defined functions that are described below.

### Functions
```lisp
(?bind ?val (ros-param-get-value ?param-name ?default-value))
; example args: "environments" (create$ not-found)
; example ret:  ("cx_ros_param")
```
