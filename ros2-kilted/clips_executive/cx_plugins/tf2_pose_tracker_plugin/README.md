# cx_tf2_pose_tracker_plugin
This package offers the `cx::Tf2PoseTrackerPlugin' that allows to perform periodic lookups to update poses obtained from a transform tree.

## Usage
Register this plugin with the plugin manager.
It's configuration parameters are depicted in this example setup below.

```yaml
clips_manager:
  ros__parameters:
    environments: ["main"]
    main:
      plugins: ["tf2_pose_tracker"]

    tf2_pose_tracker:
      plugin: "cx::Tf2PoseTrackerPlugin"
      # Whether to spin up a new node for tf lookups or use the main CX node
      # Defaults to true
      spin_thread: true
```

## CLIPS Features
This plugin adds deftemplates and deffunctions as listed below:

### Deftemplates
```lisp
; Asserted and modified by the periodic lookup started with tf2-start-periodic-lookup.
(deftemplate tf2-tracked-pose
  (slot parent (type STRING))
  (slot child (type STRING))
  (slot stamp (type FLOAT))
  (multislot translation (type FLOAT) (cardinality 3 3))
  (multislot rotation (type FLOAT) (cardinality 4 4))
  (slot timer (type EXTERNAL-ADDRESS))
)
```

### Functions
```lisp
; Start a ROS timer to periodically lookup the pose of ?child-frame relative to ?parent-frame.
; Returns TRUE if timer was created successfully, FALSE otherwise.
(tf2-start-periodic-lookup ?parent-frame ?child-frame ?frequency-hz)   ; example-args: "map" "base_link" 2.0
; Stop a started ROS timer using the timer of the associated tf2-tracked-pose fact.
(tf2-stop-periodic-lookup ?timer-ptr) ; example args: <Pointer-C-0x7f1550001d20>
```
