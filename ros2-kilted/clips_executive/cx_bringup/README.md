# cx_bringup

Provides an example bringup mechanism for CX applications.

This package includes configuration files to showcase the different plugins `params` folder.

The CLIPS-Executive environment (CLIPS Environment Manager node) is configurable to specify which CLIPS environments should be loaded at startup and which plugins should be loaded in those environments.

It provides a dedicated node (`cx_node`) inside the `src` directory, which configures the lifecycle manager node per default.

Lastly, it provides a convenient launch file that can be used in order to launch the provided node including common configuration options.

## Launching the CLIPS-Executive
Make sure to source the CLIPS-Executive workspace and run the following command to select a configuration to launch:
```
ros2 launch cx_bringup cx_launch.py manager_config:=<config-file>
```
where config-file is one of:
 - *plugin_examples/ament_index.yaml*
 - *plugin_examples/config.yaml*
 - *plugin_examples/executive.yaml*
 - *plugin_examples/fibonacci_action.yaml*
 - *plugin_examples/file_load.yaml*
 - *plugin_examples/pddl_parser.yaml*
 - *plugin_examples/protobuf_linked.yaml*
 - *plugin_examples/protobuf.yaml*
 - *plugin_examples/ros_msgs.yaml*
 - *plugin_examples/set_bool_srv.yaml*
 - *plugin_examples/string_msg.yaml*

It defaults to `plugin_examples/file_load.yaml`.
