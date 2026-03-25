initialize()
############

This function is called exactly once when a plugin is loaded, before it actually provides it's features to CLIPS environments.

Typical uses include
1) Reading of parameters from specified in the parent node.
2) Initialization of environment-agnostic class members.

clips_env_init()
################

Called once for every environment the plugin is loaded into.

Typically this is used to inject user-defined functions, define templates etc.

It should return `true`, if the initialization of the environment was successful. If it returns `false`, then the plugin manager will call `clips_env_destroyed` to allow proper cleanup.

Environment Reset
*****************

However, be aware that each environment is reset afterwards on startup.

This in particular means that all asserted facts and instances are deleted and it makes no sense to directly assert facts in this function.

If your plugin should provide initial facts, it should therefore use `deffacts` instead, which would assert the facts on reset.

Environment Context and Multithreading
**************************************

Each environment also holds an instance of the `CLIPSEnvContext` class from the **cx_utils** package that can be retrieved via static functions:

.. code-block:: c++

    cx::CLIPSEnvContext::get_context(clips::Environment *env)
    cx::CLIPSEnvContext::get_context(std::shared_ptr<clips::Environment> env)

This instance contains the name and the environment as well as a mutex to guard the environment.

**Operations on CLIPS environments are not thread-safe**, hence each environment interaction needs to be guarded by this mutex.
This is mainly relevant for plugins handling asynchronous operations.
Directly accessing the environment in **clips_env_init** or **clips_env_destroyed** is safe, because the plugin manager already guards the environments with the mutex (do **not** attempt to lock the mutex in this context or it will block).

Similarly, if a plugin provides a C++ function to a CLIPS environment, it's body will be scoped through whatever context that invoked the function (e.g., the CLIPS environment manager through invoking the inference engine via ``(run)``).

clips_env_destroyed()
#####################

Called once for every environment that needs to unload a previously loaded plugin's feature.
Also is called when a `clips_env_init` call returns `false`.

Typically this is used to undefine user-defined functions, templates, etc.

finalize()
##########

This function is called exactly once when a plugin is finally unloaded again, hence all resources should be freed for a graceful destruction of the object.
