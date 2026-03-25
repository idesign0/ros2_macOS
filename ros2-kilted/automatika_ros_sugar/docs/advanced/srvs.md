# Default Services In Sugarcoat Components

In addition to the standard [ROS2 Lifecycle Node](https://github.com/ros2/demos/blob/rolling/lifecycle/README.rst) services, Sugarcoat Components provide a powerful set of built-in services for live reconfiguration. These services allow you to dynamically adjust inputs, outputs, and parameters on-the-fly, making it easier to respond to changing runtime conditions or trigger intelligent behavior in response to events. Like any ROS2 services, they can be called from other Nodes or with the ROS2 CLI, and can also be called programmatically as part of an action sequence or event-driven workflow in the launch script.

The examples below assume a component named `awesome_component` with an output topic `/voice` of type `Audio`.

## Replacing an Input or Output with a different Topic
You can swap an existing topic connection (input or output) with a different topic online without restarting your script. The service will stop the running lifecycle node, replace the connection and restart it again.

- **Service Name: /{component_name}/change_topic**
- **Service Type: [automatika_ros_sugar/srv/ReplaceTopic](https://github.com/automatika-robotics/sugarcoat/blob/main/srv/ReplaceTopic.srv)**


### Example

To replace the output topic name of `awesome_component`, we can send the following service call to the node:

```shell
ros2 service call /awesome_component/change_topic automatika_ros_sugar/srv/ReplaceTopic "{direction: 1, old_name: '/voice', new_name: '/audio_device_0', new_msg_type: 'Audio'}"
```

## Updating a configuration parameter value
This `ChangeParameter` service allows updating a single configuration parameter at runtime. You can choose whether the component remains active during the change, or temporarily deactivates for a safe update.

- **Service Name: /{component_name}/update_config_parameter**
- **Service Type: [automatika_ros_sugar/srv/ChangeParameter](https://github.com/automatika-robotics/sugarcoat/blob/main/srv/ChangeParameter.srv)**

### Example

Let's change the `loop_rate` for `awesome_component` to `1Hz` without restarting the node:

```shell
ros2 service call /awesome_component/update_config_parameter automatika_ros_sugar/srv/ChangeParameter "{name: 'loop_rate', value: '1', keep_alive: false}"
```

## Updating a set of configuration parameters
The `ChangeParameters` service allows updating multiple parameters at once, making it ideal for switching modes, profiles, or reconfiguring components in batches. Similar to `ChangeParameter` service, you can choose whether the component stays active or temporarily deactivates during the update.

- **Service Name: /{component_name}/update_config_parameters**
- **Service Type: [automatika_ros_sugar/srv/ChangeParameters](https://github.com/automatika-robotics/sugarcoat/blob/main/srv/ChangeParameters.srv)**


### Example

Let's change multiple parameters at once for `awesome_component` without restarting the node:

```shell
ros2 service call /awesome_component/update_config_parameters automatika_ros_sugar/srv/ChangeParameters "{names: ['loop_rate', 'fallback_rate'], values: ['1', '10'], keep_alive: false}"
```

## Reconfiguring the Component from a given file
The `ConfigureFromFile` service lets you reconfigure an entire component from a YAML, JSON or TOML configuration file while the node is online. This is useful for applying scenario-specific settings, or restoring saved configurations—all in a single operation.

- **Service Name: /{component_name}/configure_from_file**
- **Service Type: [automatika_ros_sugar/srv/ConfigureFromFile](https://github.com/automatika-robotics/sugarcoat/blob/main/srv/ConfigureFromFile.srv)**


### Example

Example YAML configuration file for `awesome_component`:

```yaml
/**: # Common parameters for all components
  fallback_rate: 10.0

# Parameters specific to component under component name
awesome_component:
  loop_rate: 100.0
```

## Executing a Component's method

The `ExecuteMethod` service enables runtime invocation of any class method in the component. This is useful for triggering specific behaviors, tools, or diagnostics during runtime without writing additional interfaces.

- **Service Name: /{component_name}/execute_method**
- **Service Type: [automatika_ros_sugar/srv/ExecuteMethod](https://github.com/automatika-robotics/sugarcoat/blob/main/srv/ExecuteMethod.srv)**
