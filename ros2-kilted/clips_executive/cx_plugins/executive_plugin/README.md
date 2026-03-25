# cx_example_plugin
This package provides continuous execution of CLIPS environments.
With a set frequency, this plugin refreshes all agendas and then runs the loaded CLIPS environments.

## Usage
Register this plugin with the plugin manager.
It's configuration parameters are depicted in this example setup below.

```yaml
clips_manager:
  ros__parameters:
    environments: ["main"]
    main:
      plugins: ["executive"]

    executive:
      plugin: "cx::ExecutivePlugin"
      # Publish an empty message with each agenda refresh.
      # Mainly useful for debug purposes to observe true refresh rate.
      # Defaults to false
      publish_on_refresh: false
      # With each iteration, insert the latest ros time into the CLIPS environment.
      # Defaults to true
      assert_time: true
      # Rate with which the environments should run in hz.
      # Note that this is only an upper bound, as it lets all environment
      # run until the Agenda is empty.
      # Defaults to 10
      refresh_rate: 10
```
## CLIPS Features
This plugin adds deffunctions to retrieve the current time.
```lisp
(bind ?ros-time (now))         ; returns a FLOAT holding get_clock()->now().seconds()
(bind ?sys-time (now-systime)) ; returns a FLOAT of system time
```

It additionally provides additional changes, if **assert_time** is `true`.
Then it asserts an unordered fact `time` with the current ROS time, which looks like this:
```lisp
(time 1729689959.63119)
```

It then also defines a defrule and defglobal to clean up the fact once the end of the agenda is reached:

```
(defglobal
  ?*PRIORITY-TIME-RETRACT*    = -10000
)

(defrule time-retract
  (declare (salience ?*PRIORITY-TIME-RETRACT*))
  ?f <- (time $?)
  =>
  (retract ?f)
)
```
Lastly, it unwatches both the `time` fact and `time-retract` rule as they otherwise would spam the log.
```lisp
(unwatch facts time)
(unwatch rules time-retract)
```
