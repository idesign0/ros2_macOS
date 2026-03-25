# cx_config_plugin
This package offers the `cx::ConfigPlugin` CLIPS plugin to read config values from yaml.

## Usage
Register this plugin with the plugin manager. It requires no additional configuration, an example setup is shown below:

```yaml
clips_manager:
  ros__parameters:
    environments: ["main"]
    main:
      plugins: ["config"]

    config:
      plugin: "cx::ConfigPlugin"
```
## CLIPS Features
This plugin adds deftemplates and deffunctions as listed below:

### Deftemplates
```lisp
; Asserted by the config-load function for each configuration value in the parsed yaml file.
(deftemplate confval
  (slot path (type STRING))
  (slot type (type SYMBOL) (allowed-values FLOAT UINT INT BOOL STRING))
  (slot value)
  (slot is-list (type SYMBOL) (allowed-values TRUE FALSE) (default FALSE))
  (multislot list-value)
)
```

### Functions
```lisp
; Load all config values from a file (absolute path) given a prefix and store tjhem into (confval) facts.
; The prefix determines tha resulting path in the confval facts.
(config-load ?file ?prefix)
```
## Example
Assuming a yaml file at path `/home/<user>/test.yaml`:
```yaml
root:
  branch:
        val1: true
        val2: [test1, test2]
  branch2:
        val1: false
```
Calling the function with different prefixes yields the following results:
### Parsing Everyting
```lisp
(config-load "/home/<user>/test.yaml" "/")
(confval
  (path "/root/branch/val1")
  (type BOOL)
  (value TRUE)
  (is-list FALSE)
  (list-value)
)
(confval
  (path "/root/branch/val2")
  (type STRING)
  (value nil)
  (is-list TRUE)
  (list-value "test1" "test2")
)
(confval
  (path "/root/branch2/val1")
  (type BOOL)
  (value FALSE)
  (is-list FLASE)
  (list-value)
)
```
### Parsing Only "branch"
```lisp
(config-load "/home/<user>/test.yaml" "/root/branch")
(confval
  (path "/root/branch/val1")
  (type BOOL)
  (value TRUE)
  (is-list FALSE)
  (list-value)
)
(confval
  (path "/root/branch/val2")
  (type STRING)
  (value nil)
  (is-list TRUE)
  (list-value "test1" "test2")
)
```
