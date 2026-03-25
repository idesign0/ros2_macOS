# Converting a Sugarcoat Recipe into a systemd Service

:::{note}
Recipes written with Sugarcoat packages can be easily launched as `systemd` services using the `create_service` tool provided with the package.
:::

:::{tip}
This is ideal for deploying Sugarcoat components in production environments or embedded systems where automatic startup and restart behavior is critical.
:::

Once you have a Python script written for your Sugarcoat-based package (lets call it `my_awesome_system.py`), you can install it as a systemd service with the following command:

```bash
ros2 run automatika_ros_sugar create_service <path-to-python-script> <service-name>
```

## Arguments

- `<path-to-python-script>`: The full path to your Sugarcoat Python script (e.g., `/path/to/my_awesome_system.py`).
- `<service-name>`: The name of the systemd service (do **not** include the `.service` extension).

## Example

```bash
ros2 run automatika_ros_sugar create_service ~/ros2_ws/my_awesome_system.py my_awesome_service
```

This command will install and optionally enable a `systemd` service named `my_awesome_service.service`.

## Full Command Usage

```text
usage: create_service [-h] [--service-description SERVICE_DESCRIPTION]
                      [--install-path INSTALL_PATH]
                      [--source-workspace-path SOURCE_WORKSPACE_PATH]
                      [--no-enable] [--restart-time RESTART_TIME]
                      service_file_path service_name
```

Install a Python script as a systemd service.

### Positional Arguments

- **`service_file_path`**: Path to the Python script you want to install as a service.
- **`service_name`**: Name of the systemd service (without `.service` extension).

### Optional Arguments

- `-h, --help`: Show the help message and exit.
- `--service-description SERVICE_DESCRIPTION`: Human-readable description of the service. Defaults to `"Sugarcoat Description"`.
- `--install-path INSTALL_PATH`: Directory to install the systemd service file. Defaults to `/etc/systemd/system`.
- `--source-workspace-path SOURCE_WORKSPACE_PATH`: Path to the ROS workspace `setup` script. If omitted, it auto-detects the active ROS distribution.
- `--no-enable`: Skip enabling the service after installation.
- `--restart-time RESTART_TIME`: Time to wait before restarting the service if it fails (e.g., `3s`). Default is `3s`.

## What This Does

This command:

1. Creates a `.service` file for `systemd`.
2. Installs it in the specified or default location.
3. Sources the appropriate ROS environment.
4. Optionally enables and starts the service immediately.

Once installed, you can manage the service using standard `systemd` commands:

```bash
sudo systemctl start my_awesome_service
sudo systemctl status my_awesome_service
sudo systemctl stop my_awesome_service
sudo systemctl enable my_awesome_service
```
