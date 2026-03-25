Logging and Routing
###################

The CLIPS Environment manager provides a custom CLIPS logger that logs CLIPS output to ROS and also saves CLIPS output of each environment to files if configured so.
The log files are stored at the ROS logging directory ( typically ``~/.ros/log/``, see also the :rostut:`ROS logging tutorial <Tutorials/Demos/Logging-and-logger-configuration.html#logging-directory-configuration>`) and are named using the name of the environment followed by a timestamp.

For log routing inside CLIPS, the custom loggers accepts the following logical names that log to the file and additionally also in some cases to ROS:

 - ``l``, ``t``, ``info``, ``loginfo`` log via ``RCLCPP_INFO``
 - ``green``, ``blue``, ``yellow``, ``magenta``, ``cyan``, ``white``, ``bold`` log via ``RCLCPP_INFO`` with additional ANSI escape codes for color output (for terminals that support them)
 -  ``debug``, ``logdebug`` log via ``RCLCPP_DEBUG``
 - ``warn``, ``logwarn``, ``stdwrn`` log via ``RCLCPP_WARN``
 - ``error``, ``logerror``, ``stderr`` log via ``RCLCPP_ERROR``
 - ``stdout`` logs via ``RCLPP_INFO`` unless the environment has the configuration ``redirect_stdout_to_debug`` set to ``true``, then it logs via ``RCLCPP_DEBUG``
