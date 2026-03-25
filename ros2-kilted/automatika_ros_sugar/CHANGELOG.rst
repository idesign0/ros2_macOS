^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package automatika_ros_sugar
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.5.0 (2026-02-14)
------------------
* (refactor) Removes callback_group from component config
* (fix) Fixes action serialization for fallbacks
* (refactor) Removes optional health monitoring and enables it for all components
* (docs) Updates events/actions docs using new theme
* (docs) Updates concepts overview
* (docs) Updates core design docs pages
* (fix) Removes outdated check in component_action decorator
* (fix) Fixes UI outputs init error
* (docs) Updates overview and install sections
* (docs) Updates docs for events/actions and removes unnecessary md copying
* (docs) Uses Shibuya theme for docs
* (feature) Adds support for Path visulization in UI outputs
* (feature) Adds UI callback for Path datatype
* (fix) Handles point frame_id for UI output display on maps
* (docs) Re-orgnize the index tree
* (fix) Moves component fallback topics blackboard to init
* (feature) Adds support for displaying point-like UI outputs on a Map element
* (fix) Fixes clicked points transformation in UI map elements
* (feature) Adds support for topic inputs in Fallback Actions
* (fix) Fixes registering actions with multiple topics to events
* (fix) Fixes parsing and clearing a component internal events/actions
* (feature) Adds method to replace an Action input topic
* (fix) Fixes checking on_any event with multi-topic condition source
* (fix) Fixes sending topics blackboard values to action on internal (monitor) events
* (fix) Fixes serialization error in base config
* (fix) Fixes getting component internal events in launcher
* (docs) Updates docs with new API updates for events and actions
* (feature) Makes all UI cards draggable and resizable
* (feature) Adds dark/light mode toggle to UI
* (feature) Adds dynamic publishers creation in UI node
* (feature) Improves launcher logging display and colors
* (reafctor) Removes 'event_name' param from Event class and uses an internal unique ID instead
* (fix) Adds a timestamp and unique ID to events topics dashboard to prevent 'stale' message processing in events
* (refactor) Uses sugarcoat logger in UI node
* (fix) Fixes launching opaque actions with new multi-condition event design
* (refactor) Updates system actions
* (feature) Adds support for complex conditions (and, or, not) in event and refactors the Event class
* (refactor) Removes 'event_parsers' from action
* (feature) Updates the point input ui element based based on a present map element
* (refactor) Converts external processor types to an enum class
* (feature) Adds style for buttons tooltip
* (feature) Adds clicked point publishing to map canvas
* (fix) Fixes error in actions and adds external processor deserialization to executable
* (feature) Adds element and websocket to handle map data in UI
* (fix) Fixes condition builder class to avoid errors when usinf deepcopy or inspect on an object
* (feature) Adds automatic action parser from ROS types to Python types
* (feature) Adds property to automatically get a component health status topic
* (docs) Updates docus with new events-actions api
* (refactor) Adds Action to core module
* (refactor) Removes old events module
* (refactor) Removes unused init args for main Event class
* (feature) Adds conditions for string values (contains, not_contains, is_in, not_in)
* (refactor) Moves event condition class to a new module
* (feature) Adds condition builders for boolean values in events
* (feature) Adds OnAny to events constructed from conditions
* (refactor) Refactors actions module and adds method to create an automatic action parser from given topic message type
* (feature) Adds internal events condition constructor and updates the event API
* (feature) Adds trigger actions for sending empty ROS2 action/service calls
* (refactor) Updates status topic name
* (feature) Adds automatic parsing from topic values in Action
* (feature) Handles parsing internal component event_actions in launcher
* (feature) Update Task UI element with server info and request pop-up
* (refactor) Moves all component and system level actions to 'actions' module
* (feature) Adds component actions with automatic event parsers
* Contributors: ahr, mkabtoul

0.4.3 (2026-01-17)
------------------
* (refactor) Organizes UI custom javascript into three functional scripts
* (docs) Improves the core concepts documentation
* (fix) Fixes check for None max_retries value in fallback
* (fix) Fixes logging colors in multi-process execution
* (feature) Adds parameter for lifecycle transition timeout
* (fix) Add error catching during all fallbacks action execution
* (fix) Fixes fallback init and exposes on_giveup fallback
* (fix) Adds fallbacks serialization to support multi-process execution
* (fix) Removes optional fallback in execute method
* (fix) Updates health status after executing the fallback in component
* (fix) Adds fallback to component lifecycle transition errors
* (feature) Adds method to add event/action pairs from inside child components
* (docs) Adds generation of llms.txt to docs
* (feature) Adds Task UI element to track action requests
* (feature) Adds option to hide the settings panel in the UI
* (feature) Adds property to get a UI input for a component's main action server
* (feature) Adds action clients to UI inputs
* (feature) Improves ActionClient with methods to cancel requests and monitor feedback
* (feature) Adds parsing ROS messages from/to dictionary
* (docs) Fixes installation instructions in readme
* (fix) Fixes external processor argument passing in the launcher
* (refactor) Adds utility function for running external processors
* (feature) Adds service clients to UINode inputs in order to send service calls directly from the UI
* (feature) Adds method to send a service request using data from a dictionary
  This method is useful to send requests without initializing the request class. Used in the UI node to send requests from an input form data
* Contributors: ahr, mkabtoul

0.4.2 (2025-11-30)
------------------
* (docs) Adds robot plugins tutorial to docs
* (feature) Adds publish pre processors to robot plugin client
* (feature) Adds robot plugin client and enable using plugins in components and launcher
* (fix) Fixes publishing occupancy grid (humble)
* (fix) Fixes conversion between Vector3 and Point
* Contributors: ahr, mkabtoul

0.4.1 (2025-11-07)
------------------
* (feature) Adds UI font files locally
* (fix) Fixes logging card scroll behavior
* (fix) Fixes drawing of output UI elements by filtering those that log
* (fix) Fixes setting id in divs added to logging card
* (fix) Clears last message when getting output for UI
* (feature) Adds method to augment exisiting text on logging card
* (fix) Fixes terminal scrolling behavior in UI
* (fix) Makes inputs/outputs displays conditional in UI
* (refactor) Moves logging card outputs to their own functions based on datatype
* Contributors: ahr, mkabtoul

0.4.0 (2025-11-04)
------------------
* (docs) Adds dynamic ui to docs and updates readme
* (feature) Adds automatika CSS custom style for UI
* (fix) Adds use of step in reading img msgs
* (fix) Fixes colors when converting image to jpeg using cv2
* (fix) Turns ComponentConfig run_type into a private parameter (_run_type) in the attrs class
* (feature) Makes streaming websocket connections more robust by recreating them after htmx events and adding reconnects for stale cache
* (fix) Handles nested settings classes in ui
* (feature) Adds UI_EXTENSIONS to add custom UI elements from derived packages
* (fix) Fixes parsing optional type for ui serialization
* (fix) Fixes getting relative path for static files
* (feature) Adds serialization of additional types
* (fix) Fixes base attrs handing of parsed types for ui serialization
* (fix) Handles literals inside generic types correctly
* (feature) Adds 'dragabble' sections
* (feature) Adds toggle button to all UI cards
* (feature) Presists updates settings on the UI
* (feature) Adds ui output for OccupancyGrid
* (fix) Fixes de-seralizing qos config in topics
* (feature) Adds ui dict converters for Point/Pose types
* (fix) Adjusts inputs/outputs grid display in UI frontend
* (feature) Adds extensibility to sending various datatypes to ROS topics
* (fix) Fixes type hint in launcher add_pkg for 3.8 compatibility
* (fix) Fixes input for 'less than' validator
* (feature) Adds responsive settings display styling
* (fix) Adds check for length of payload before putting it on the DOM
* (feature) Adds sending and logging audio msgs
* (feature) Adds audio elements to the log and adds clearing inputs after submission
* (feature) Adds header and audio message elements to frontend
* (feature) Adds dynamic creating of outputs starting with images
* (feature) Adds complete set of image and compressed image pre processing
* (feature) Adds raw data output option in extra callback functions
* (feature) Adds sending and receiving text, images and errors with websocket endpoint
* (fix) Fixes base attrs type description for Literal for python3.8+
* (fix) Moves get_field_info from BaseConfig to BaseAttrs
* (fix) Fixes transition for component config update service
* (feature) Adds publishers and subscribers to ui node for input and output topics
  - Adds method for publishing to inputs
* (feature) Adds routes for settings update and websocket connection
* (feature) Adds settings update through ui ros node
* (refactor) Breaks executable main into two functions
* (feature) Adds FastHTML based dynamically generated UI for recipe
  - Adds UI server executable that spins the UI ROS node in a thread
  - Adds dynamically generated settings UI
* (feature) Adds helper method to get fields, annotations and validators from BaseComponentConfig
* (fix) Adds minor fix to published frame id and occupancy grid sender
* Contributors: ahr, mkabtoul

0.3.2 (2025-09-03)
------------------
* (docs) Updates events docs with new classes
* (feature) Adds ros time automatically to stamped messages in publisher
* (fix) Fixes error in publishing audio msgs as byte arrays
* (fix) Fixes Pose publisher converter
* (fix) Fixes event handle once and delay options
* (fix) Fixes condition for OnChange event trigger
* (feature) Adds events for contains any/all plus change in value
* (fix) Fixes error in geomerty transformation util
* Contributors: ahr, mkabtoul

0.3.1 (2025-06-28)
------------------
* (refactor) Corrects type hint in callbacks
* (refactor) Adds type hints to validators
* (refactor) Minor improvements and typo correction
* (refactor) Resolves todo in component actions for active flag and adds default logger for module
* (chore) Updates service creation script with better error handling
* (docs) Updates installation instructions
* (refactor) Removes numpy-quaternion from dependencies and implements rotations using numpy
* (docs) Updates logo icon and adds 'config from file' page to docs
* (fix) Adds wait for node activation after restart and fixes optional arguments parsing in Component
* (docs) Updates events docs
* (docs) Adds international readmes
* (chore) Removes pip based test dependencies for ROS build farm
* (docs) Updates readme
* Contributors: ahr, mkabtoul

0.3.0 (2025-06-18)
------------------
* (chore) Updates installation instructions and CI
* (fix) Removes NoneType from types for compatibility with python < 3.10
* (fix) Add missing pyyaml dependency
* (feature) Adds config parsing from toml, json and yaml and removes Omegaconf dependency
* (fix) Add missing action decorator to methods
* (docs) Updates img names
* (docs) Updates readme
* (chore) Adds a sweeter name for ROS Sugar .. Sugarcoat
* (docs) Minor edits
* (docs) Adds documentation for creating systemd services with scripts
* (docs) Fixes size of logo
* (docs) Fixes warnings in docs
* (docs) Minor fix
* (docs) Updates supported types docs
* (fix) Fixes minor typo
* (fix) Fixes logging level parsing
* (feature) Adds node logging level and rclpy logging level to component config
* (fix) Adds BGR2RGB tranformation for yuv422_yuy2 encoding
* (docs) Removes unused extension
* (docs) Adds docs for components services and pkg advantages
* Merge branch 'feature/system_management' of github.com:automatika-robotics/ros-sugar into feature/system_management
* (fix) Fixes transformation quaternion parsing
* (feature) Adds utility method for processing all ROS image encodings
* (feature) Adds transformation/rotation propertires to TFListener
* (fix) Fixes lifecycle transitioin callback in component
* (chore) Adds msgpack-numpy and attrs as deb package dependencies
* (fix) Handles lifecycle actions start/stop/restart from the launcher
* (fix) Adds action goal lock to handle aborting ongoing goals and executing new incoming goals
* Contributors: ahr, mkabtoul

0.2.9 (2025-02-18)
------------------
* (docs) Updates supported types docs and adds docstrings
* (feature) Adds a script to  make any python script a systemd service
* (fix) Minor fix to check for action server creation before destruction
* (fix) Adds algorithm config from yaml if available
* (fix) Removes setproctitle as a hard dependency
* (fix) Checks for subscription in got_inputs method
* (fix) Fixes type hints for python3.8 compatibility
* Contributors: ahr, mkabtoul

0.2.8 (2025-01-28)
------------------
* (fix) Removes testing to keep build stable until dependencies are merged in rosdistro
* Contributors: ahr

0.2.7 (2025-01-28)
------------------
* (fix) Fixes ChildComponent class in tests
* (fix) Minor fix in component execution_step
* (feature) Adds pytests to package.xml and CMakeLists
* (feature) Adds tests for Actios and ComponentRunType
* (fix) Fixes health_status publishing for non timed components
* (feature) Adds pytest for all event classes and fixes existing errors
* (feature) Adds support for std_msgs MultiArray messages
* (fix) Fixes Event trigger value check and OnGreater event serialization
* Contributors: Maria Kabtoul, mkabtoul

0.2.6 (2025-01-17)
------------------
* (fix) Fixes type hint
* (fix) Fixes getting available events
* (feature) checks for components and events duplicate names
* (fix) Changes type of monitor components to activate
* (chore) Fixes OS versions in CI
* (chore) Adds arms builds to debian packaging
* (refactor) Changes the fuction to create events from jsons
* (fix) Fixes events parsing using serialized events as dictionary keys
* (docs) Adds verification tag
* (docs) Adds external links to docs
* (docs) Adds source link to docs
* Contributors: ahr, mkabtoul

0.2.5 (2025-01-07)
------------------
* (fix) Gets imports and default values based on installed distro
* (fix) Fix launch and launch_ros imports based on ros distro
* Contributors: ahr, mkabtoul

0.2.4 (2024-12-27)
------------------
* (fix) Adds algorithm auto re-configuration from YAML file
* (fix) Fixes callback got_msg property
* (feature) Adds topics callbacks/conversions reparsing to component
  Supports running components from different packages in one script and each component uses its own package callbacks/conversions
* (fix) Updates AllowedTopics config and its validator
* (refactor) Removes PIL as a dependancy
* (fix) Fixes component state transition logging
* (fix) Fixes order to custom method execution in component lifecycle transition methods
* (refactor) Removes BaseNode class
* (fix) Fixes packaging workflow formatting
* (fix) Removes redundant methods from components
* (chore) Increments release action version
* (chore) Adds new action in debs creation workflow
* (refactor) Formats utils
* (refactor) Minor refactoring in utils
* (fix) Removes fix for color correction as the transformation is now applied at the time of visualization
* (fix) Adds color transformation when reading images of yuv encoding
* (chore) Changes name of release action
* (feature) Adds component algorithm config management to the api
* (fix) fixes datatypes update method for using multiple packages
* (chore) Cleans up cmake and packaging
* (refactor) Improves error message when a topic of unsupported type is created
* (refactor) Handles additional datatypes provided by user packages
* (fix) Pins release mirror workflow to run only on release publishing
* (fix) Adds branch name to release workflow
* (fix) Fixes name of action
* (feature) Adds release mirror action
* (docs) Removes autogenerated docs
* (docs) Adds minor modification to readme
* (docs) Changes package description
* (feature) Adds ExecuteMethod service to BaseComponent
* (fix) Minor fix in conversion method
* (refactor) Makes compressed image a realization of image
* (fix) Fixes ros compressed image conversion util
* (feature) Adds support for CompressedImage msg
* (feature) Adds ros_log_level option to each added package
* (feature) Adds additional supported types argument to BaseComponent and Topic validators
* (fix) Adds algorithm auto re-configuration from YAML file
* (fix) Fixes callback got_msg property
* (feature) Adds topics callbacks/conversions reparsing to component
  Supports running components from different packages in one script and each component uses its own package callbacks/conversions
* (fix) Updates AllowedTopics config and its validator
* (refactor) Removes PIL as a dependancy
* (fix) Fixes component state transition logging
* (fix) Fixes order to custom method execution in component lifecycle transition methods
* (refactor) Removes BaseNode class
* (fix) Fixes packaging workflow formatting
* (fix) Removes redundant methods from components
* (chore) Increments release action version
* (chore) Adds new action in debs creation workflow
* (refactor) Formats utils
* (refactor) Minor refactoring in utils
* (fix) Removes fix for color correction as the transformation is now applied at the time of visualization
* (fix) Adds color transformation when reading images of yuv encoding
* (chore) Changes name of release action
* (feature) Adds component algorithm config management to the api
* (fix) fixes datatypes update method for using multiple packages
* (chore) Cleans up cmake and packaging
* (refactor) Improves error message when a topic of unsupported type is created
* (refactor) Handles additional datatypes provided by user packages
* (fix) Pins release mirror workflow to run only on release publishing
* (fix) Adds branch name to release workflow
* (fix) Fixes name of action
* (feature) Adds release mirror action
* (docs) Removes autogenerated docs
* (docs) Adds minor modification to readme
* (docs) Changes package description
* (feature) Adds ExecuteMethod service to BaseComponent
* (fix) Fixes OccupnacyGrid data publishing from numpy
* (fix) Minor fix in conversion method
* (refactor) Makes compressed image a realization of image
* (fix) Fixes ros compressed image conversion util
* (feature) Adds support for CompressedImage msg
* (feature) Adds ros_log_level option to each added package
* (feature) Adds additional supported types argument to BaseComponent and Topic validators
* (fix) Merge pull request `#14 <https://github.com/automatika-robotics/ros-sugar/issues/14>`_
* (chore) Updates package name to automatika_ros_sugar
* (fix) Checks numpy array shape in OccupancyGrid converter
* (feature) Adds stamped header and frame_id to ros publishers/callbacks
* (docs) Updates install instructions
* Contributors: ahr, mkabtoul

0.2.3 (2024-11-13)
------------------
* (chore) bump version 0.2.2 -> 0.2.3
* (chore) Adds deb packaging scripts and actions (`#13 <https://github.com/automatika-robotics/ros-sugar/issues/13>`_)
* (docs) Removes notice
* Contributors: ahr

0.2.2 (2024-11-04)
------------------
* (chore) bump version 0.2.1 -> 0.2.2
* (feature) Adds activation timeout to monitor and launcher
* (fix) Fixes publishing numpy data to ROS OcuupancyGrid
* (refactor) Updates OccupancyGrid get_output using numpy operations
* Contributors: mkabtoul

0.2.1 (2024-10-29)
------------------
* (chore) bump version 0.2.0 -> 0.2.1
* (feature) Adds support for external tool calling in multiprocessing
* Contributors: ahr

0.2.0 (2024-10-25)
------------------
* (chore) Bump version 0.1.1 -> 0.2.0
* Merge pull request `#12 <https://github.com/automatika-robotics/ros-sugar/issues/12>`_ from automatika-robotics/feature/external_processors
  Adds external processor support when running components in multiprocessing
* (refactor) Makes msgpack a global dependancy
* (fix) Fixes deserialization of external processors and handling of processor result in launcher
* (fix) Corrects the serialization of numpy arrays within lists
* (feature) Changes defaults for launcher parameters when using multiprocessing
* (fix) Fixes handling composite type check for deserialization and input/output deserialization in components
* (fix) Adds node name as parameter to callbacks for init
* (fix) Adds alias to attrs private attribute in BaseComponentConfig
* (fix) Restores executable to old version
* Merge branch 'feature/external_processors' of github.com:automatika-robotics/ros-sugar into feature/external_processors
* (fix) Fixes new method name in launcher
* (fix) Moves callbackgroup to BaseComponentConfig and changes initialization of inputs/outputs in component
* (fix) Fixes serialization of callbackgroup in config
* (fix) Fixes type hints for compatibility
* (docs) Fixes ubuntu version for dependancy problems
* (refactor) Makes msgpack a functional dependency
* (refactor) Adds handling of callback group and input/output initialization to facilitate multiprocessing
* (feature) Adds handling of callback group for multiprocess launch
* (fix) Adds serialization of np arrays and tuples
* (fix) Adds converter for QoS profile for serialization
* (refactor) Changes inputs/outputs handling in executable
* (refactor) Changes name of enum convert utility function
* (fix) Fixes use of multi processors for same topic in launcher
* (fix) Fix package installation for documentation workflow
* (feature) Adds support for multiple external processors on the same topic
* (fix) Fixes visibility of external_processors to protected
* (fix) Fixes typo in attaching external preprocessors
* (feature) Adds unix socket based listener threads for using external processors with components being run in multiprocessing
  - Modifies executable to add an argument for external processors
  - Adds setting and getting for external processor json in component
  - Adds setting up of external processors on component activation and destruction on component stop
  - Adds setup of external processor sockets and thread pool in launcher
* (fix) Moves callbackgroup to BaseComponentConfig and changes initialization of inputs/outputs in component
* (fix) Fixes serialization of callbackgroup in config
* (fix) Fixes type hints for compatibility
* (docs) Fixes ubuntu version for dependancy problems
* (refactor) Makes msgpack a functional dependency
* (refactor) Adds handling of callback group and input/output initialization to facilitate multiprocessing
* (feature) Adds handling of callback group for multiprocess launch
* (fix) Adds serialization of np arrays and tuples
* (fix) Adds converter for QoS profile for serialization
* (refactor) Changes inputs/outputs handling in executable
* (refactor) Changes name of enum convert utility function
* (feature) Adds event processing options and supports lists in event values
  Adds options to handle an event once or handle with a time delay
* (fix) Uses List from typing in type hints
* (feature) Adds handle_once and event_delay options to Event
* (feature) Adds list to supported event trigger values
* (fix) Handles keep_alive in component parameter update service requests
* (fix) Passes monitor executor to service client send_req
* (fix) Fixes use of multi processors for same topic in launcher
* (fix) Fix package installation for documentation workflow
* (feature) Adds support for multiple external processors on the same topic
* (fix) Fixes visibility of external_processors to protected
* (fix) Fixes typo in attaching external preprocessors
* (feature) Adds unix socket based listener threads for using external processors with components being run in multiprocessing
  - Modifies executable to add an argument for external processors
  - Adds setting and getting for external processor json in component
  - Adds setting up of external processors on component activation and destruction on component stop
  - Adds setup of external processor sockets and thread pool in launcher
* (fix) Fixes minor bugs in base component and launcher (`#10 <https://github.com/automatika-robotics/ros-sugar/issues/10>`_)
* (fix) Fixes the handling of yuv422_yuy2 encoding in image reading util function
* (fix) Adds process id to monitor node name
* (fix) Fixes type check for callables in attaching post and pre processors
* (fix) Updates component launch arguments after parsing events_actions
* (docs) Updates docs url links in readme
* (docs) Adds github workflow for docs (`#9 <https://github.com/automatika-robotics/ros-sugar/issues/9>`_)
* (fix) Adds handling image encodings with alpha channel
* Create LICENSE
* Initial release version 0.1.1 (`#8 <https://github.com/automatika-robotics/ros-sugar/issues/8>`_)
* init commit
* Contributors: ahr, aleph-ra, mkabtoul
