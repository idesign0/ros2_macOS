# cx_file_load_plugin
This package offers the `cx::FileLoadPlugin' that allows to load files into CLIPS using batch* and load*.

When just defining constructs, it is recommended to use load* over batch*, as it provides better error output.
When arbitrary commands should be executed, batch* should be used.

## Usage
Register this plugin with the plugin manager.
It's configuration parameters are depicted in this example setup below.

```yaml
clips_manager:
  ros__parameters:
    environments: ["main"]
    main:
      plugins: ["files"]

    files:
      plugin: "cx::FileLoadPlugin"
      # When specifying relative paths, look at the share directories of the listed packages to resolve them.
      # Attempts to resolve the relative paths in order of the listed packages
      # Defaults to an empty list
      pkg_share_dirs: ["cx_bringup"]
      # Specify files to load using CLIPS load* command when the plugin is loaded.
      # Supports absolute paths or relative paths using the share directories specified above.
      # Defaults to an empty list
      load: ["example.clp"]
      # Specify files to load using CLIPS batch* command when the plugin is loaded.
      # Supports absolute paths or relative paths using the share directories specified above.
      # Defaults to an empty list
      batch: ["example.clp"]
      # Specify files to load using CLIPS batch* command when the plugin is unloaded.
      # This may be used to clean up all loaded content to enable dynamic reloading of plugins at runtime.
      # Supports absolute paths or relative paths using the share directories specified above.
      # Defaults to an empty list
      cleanup_batch: ["example.clp"]
```
