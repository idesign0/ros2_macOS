import argparse
import logging
from typing import Optional, List, Type, Tuple

import rclpy
from rclpy.executors import MultiThreadedExecutor
from rclpy.utilities import try_shutdown


def _parse_args() -> Tuple[argparse.Namespace, List[str]]:
    """Parse arguments."""
    parser = argparse.ArgumentParser(description="Component Executable Config")
    parser.add_argument(
        "--config_type", type=str, help="Component configuration class name"
    )
    parser.add_argument("--component_type", type=str, help="Component class name")
    parser.add_argument(
        "--node_name",
        type=str,
        help="Component ROS2 node name",
    )
    parser.add_argument("--config", type=str, help="Component configuration object")
    parser.add_argument(
        "--inputs",
        type=str,
        help="Component input topics",
    )
    parser.add_argument(
        "--outputs",
        type=str,
        help="Component output topics",
    )

    parser.add_argument(
        "--config_file", type=str, help="Path to configuration file (yaml, json, toml)"
    )
    parser.add_argument(
        "--events", type=str, help="Events to be monitored by the component"
    )
    parser.add_argument(
        "--actions", type=str, help="Actions associated with the component Events"
    )
    parser.add_argument(
        "--fallbacks", type=str, help="Fallbacks to be executed on component Failure"
    )
    parser.add_argument(
        "--algorithms_config",
        type=str,
        help="User defined configuration for component algorithms",
    )
    parser.add_argument(
        "--external_processors",
        type=str,
        help="External processors associated with the component input and output topics",
    )
    parser.add_argument(
        "--additional_types",
        type=str,
        help="Additional type modules from derived packages",
    )
    return parser.parse_known_args()


def _parse_component_config(
    args: argparse.Namespace, config_classes: List[Type]
) -> Optional[object]:
    """Parse the component config object

    :param args: Command line arguments
    :type args: argparse.Namespace

    :return: Component config object
    :rtype: object
    """
    config_type = args.config_type or None
    config_class = None
    if not config_type:
        logging.warning(
            f"Unknown config_type '{config_type}'. Proceeding with default config for component"
        )
        return None

    # Get config type and update from json arg
    config_class = next(
        (conf_cls for conf_cls in config_classes if conf_cls.__name__ == config_type),
        None,
    )
    if not config_class:
        logging.warning(
            f"Unknown config_type '{config_type}'. Proceeding with default config for component"
        )
        return None

    config = config_class()

    config_json = args.config

    if config_json and config:
        config.from_json(config_json)

    return config


def _parse_ros_args(args_names: List[str]) -> List[str]:
    """Parse ROS arguments from command line arguments

    :param args_names: List of all parsed arguments
    :type args_names: list[str]

    :return: List ROS parsed arguments
    :rtype: list[str]
    """
    # Look for --ros-args in ros_args
    ros_args_start = None
    if "--ros-args" in args_names:
        ros_args_start = args_names.index("--ros-args")

    if ros_args_start is not None:
        ros_specific_args = args_names[ros_args_start:]
    else:
        ros_specific_args = []
    return ros_specific_args


def _parse_component_class(
    list_of_components: List[Type], component_type, component_name
):
    if not component_type:
        raise ValueError("Cannot launch without providing a component_type")

    comp_class = next(
        (
            comp_cls
            for comp_cls in list_of_components
            if comp_cls.__name__ == component_type
        ),
        None,
    )

    if not comp_class:
        raise ValueError(
            f"Cannot launch unknown component type '{component_type}'. Known types are: '{list_of_components}'"
        )

    if not component_name:
        raise ValueError("Cannot launch component without specifying a name")

    # SET PROCESS NAME (if setproctitle is available)
    try:
        import setproctitle

        setproctitle.setproctitle(component_name)
    except ImportError:
        pass
    return comp_class


def setup_component(
    *, list_of_components: List[Type], list_of_configs: List[Type]
) -> object:
    args, args_names = _parse_args()

    # Initialize rclpy with the ros-specific arguments
    rclpy.init(args=_parse_ros_args(args_names))

    # Get type and name
    component_type = args.component_type or None
    component_name = args.node_name or None

    # Parse the component class
    comp_class = _parse_component_class(
        list_of_components, component_type, component_name
    )

    # Parse the config class
    config = _parse_component_config(args, list_of_configs)

    # Get config file if provided
    config_file = args.config_file or None

    # Init the component
    component = comp_class(
        config=config, component_name=component_name, config_file=config_file
    )

    # Init the node with rclpy
    component.rclpy_init_node()

    # Add any passed additional types
    if args.additional_types:
        component.set_additional_types(args.additional_types)

    # Set inputs/outputs
    try:
        if inputs_json := args.inputs:
            component._inputs_json = inputs_json

        if outputs_json := args.outputs:
            component._outputs_json = outputs_json
    except (ValueError, TypeError) as e:
        logging.warning(
            f"Passed Invalid inputs and/or outputs -> continue with component default values. Error: '{e}'"
        )

    # Set events/actions
    events_json = args.events or None
    actions_json = args.actions or None

    if events_json and actions_json:
        component._events_json = events_json
        component._actions_json = actions_json

    # Set algorithms configuration
    if algorithms_json := args.algorithms_config:
        component._algorithms_json = algorithms_json

    # Set fallbacks
    if fallbacks_json := args.fallbacks:
        component._fallbacks_json = fallbacks_json

    # Set external processors
    if args.external_processors:
        component._external_processors_json = args.external_processors

    return component


def run_component(component: object):
    """Run the component using a MultiThreadedExecutor

    :param component: Component to run
    :type component: object
    """
    executor = MultiThreadedExecutor()

    executor.add_node(component)

    try:
        executor.spin()

    except KeyboardInterrupt:
        pass

    finally:
        executor.remove_node(component)
        try_shutdown()


def executable_main(*, list_of_components: List[Type], list_of_configs: List[Type]):
    """Executable main function to run a component as a ROS2 node in a new process.
    Used to start a node using Launcher

    To use in your custom package based on ros_sugar:
    - Create an executable.py with a main and import your custom components and their config classes in it:
        ```python
            #!/usr/bin/env python3
            from ros_sugar import executable_main

            my_components_list = ...
            my_configs_list = ...

            def main(args=None):
                executable_main(list_of_components=my_components_list, list_of_configs=my_configs_list)
        ```
    - Add your executable as an entry point in your custom package setup.py:
        ```python
            from setuptools import find_packages, setup

            package_name = "my_awesome_pkg"

            console_scripts = [
                "executable = my_awesome_pkg.executable:main",
            ]

            setup(
                name=package_name,
                version="1",
                packages=find_packages(),
                install_requires=["setuptools"],
                zip_safe=True,
                maintainer="Cyberdyne Systems",
                maintainer_email="contact@cyberdynesystems.com",
                description="My awesome ROS2 sugar package",
                entry_points={
                    "console_scripts": console_scripts,
                },
            )
        ```
    - Provide your package name and your entry point name to ros_sugar launcher in your script.

    :param list_of_components: List of all known Component classes in the package
    :type list_of_components: List[Type]
    :param list_of_configs: List of all known ComponentConfig classes in the package
    :type list_of_configs: List[Type]
    :raises ValueError: If component or component config are unknown classes
    :raises ValueError: If component cannot be started with provided arguments
    """

    component = setup_component(
        list_of_components=list_of_components, list_of_configs=list_of_configs
    )

    run_component(component)
