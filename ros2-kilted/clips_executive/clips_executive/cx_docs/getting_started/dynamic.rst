Dynamic Setup via Services
##########################

The |CX| provides the following services to dynamically manage environments and plugins after startup (given a namespace `ns` and node name `node`):

- */ns/node/create_env* for creating environments
- */ns/node/destroy_env* for destroying environments
- */ns/node/list_envs* for listing all environments
- */ns/node/load_plugin* for loading of plugins in environments.
- */ns/node/unload_plugin* for unloading of plugins from environments.
- */ns/node/list_plugins* for listing all plugins loaded in an environment.

Whenever an environment is created, the |CX| reads the respective configuration parameters.
Similarly, the first time a plugin is loaded (through a service call or on startup), it is instantiated using the current parameters. Subsequently loading or unloading of the plugin does not update the parameters (as it is only instantiated once).
