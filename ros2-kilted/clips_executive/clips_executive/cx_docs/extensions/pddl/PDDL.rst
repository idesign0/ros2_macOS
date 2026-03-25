
PDDL Manager Plugin
###################

**Goal:** Interface with a managed PDDL domain and use a planner from CLIPS

**Tutorial Level**: Intermediate

Background
----------

This plugin provides the necessary means to integrate a CLIPS environment and a PDDL Planning Domain via the ROS2 CX. The core idea is that an external planner can be used to solve a given problem, instantiated from the world model maintained by CLIPS. This allows you to build an agent that can use the benefits of PDDL planning (including temporal planning) with its guarantees, while using the flexibility of CLIPS for execution itself.

Prerequisites
-------------

This tutorial mostly focuses on the usage of a plugin from within the |CX|. That means you should have some understanding of how the ROS2CX generally operates, what plugins do in the ROS2CX (especially file load plugin, protobuf plugin, etc), etc. It's also beneficial if you have some understanding of PDDL planning and ROS2-CLIPS integration in the ROS2CX.

### Unified Planning Framework
Under the hood, the PDDL Manager Plugin uses the UPF (Unified Planning Framework, https://github.com/aiplan4eu/unified-planning), which is a wrapper that provides tools to provide a common interface with many different PDDL planners. It also provides some tools for plan introspection, managing problems and domains, and for parsing PDDL domain files. Since we build on this framework, the support of planners is generally dictated by compatibility with the UPF. In this case, we will use the temporal planner NextFLAP (https://github.com/ossaver/NextFLAP), which is not supported by default (thus we provide some packaging around it).

Bridging CLIPS and PDDL
-----------------------

In the following, we will set up a simple project to show the steps required to incorporate PDDL planning through the PDDL Manager extension of the |CX|. We will go through all the steps from package creation to building and running a small example, in which we plan for a temporal version of BlocksWorld, and receive the completed plan on the CLIPS side. The final version of the project can be found under `extensions/pddl/pddl_agent_example`.

Setting Up a New Package
^^^^^^^^^^^^^^^^^^^^^^^^

First, we want to set up a new package for our project. You can generally set this package up wherever you desire, but in this tutorial we will work directly in the `extensions/pddl` directory. Run

.. code-block:: bash

    ros2 pkg create --build-type ament_python --license Apache-2.0 --node-name pddl_agent_example pddl_agent_example

Additionally, in your package's main directory, create the following directories: `launch`, `clips`, `pddl` and `params`, which we will use to store launch , clips, pddl, and config files respectively. Now you should have a project structure that's roughly as follows:

.. code-block:: yaml

    pddl_agent_example/
        package.xml
        resource/my_package
        setup.cfg
        setup.py
        pddl_agent_example/
            __init__.py
            pddl_agent_example.py
        launch/
        params/
        clips/
        pddl/

In Ayour `package.xml`, change the description, maintainer and version to your desired values, and add the following dependencies:

.. code-block:: xml

    <depend>rclcpp</depend>
    <depend>cx_bringup</depend>
    <depend>pddl_manager</depend>

Finally, we also need to properly configure the `setup.py` file in order to copy all the files we need to the right locations in the install directory. Add:

.. code-block:: python

    (os.path.join('share', package_name, 'launch'), glob('launch/*')),
    (os.path.join('share', package_name, 'clips'), glob('clips/*')),
    (os.path.join('share', package_name, 'pddl'), glob('pddl/*')),
    (os.path.join('share', package_name, 'params'), glob('params/*'))

into your list of `data_files` in the `setup.py`. For the PDDL files, I refer you to look up the code for this Tutorial in the repository. We just use a simple temporal blocks world problem for this example.

1 Configuring the ROS2 CLIPS-Executive
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For our planning project, we need several capabilities that come with the |CX|. First of all, we need to be able to:
- Continuously run the inference engine, which can be achieved with the :docsite:`ExecutivePlugin <clips_executive/plugins/executive_plugin>`
- Establish ROS communication to other components, which is provided by the :docsite:`RosMsgsPlugin <clips_executive/plugins/ros_msgs_plugin>`.
- Load our CLIPS files in the environment, using the :docsite:`FileLoadPlugin <clips_executive/plugins/executive_plugin>`.

Additionally, we will use need auto-generated plugins to properly interface with the PDDL manager. To achieve this, we need a configuration file that we can use to start the |CX| with exactly those capabilities:

.. code-block:: yaml

    /**:
      ros__parameters:
        autostart_node: true
        environments: ["pddl_agent"]

        pddl_agent:
          plugins: ["executive","ros_msgs","files",
                    "plan_temporal_action",
                    "timed_plan_action_msg",
                    "ament_index"]
          log_clips_to_file: true
          watch: ["facts", "rules"]

        executive:
          plugin: "cx::ExecutivePlugin"

        ros_msgs:
          plugin: "cx::RosMsgsPlugin"

        ament_index:
            plugin: "cx::AmentIndexPlugin"

        files:
          plugin: "cx::FileLoadPlugin"
          pkg_share_dirs: ["pddl_agent_example"]
          load: ["clips/agent.clp"]

        plan_temporal_action:
            plugin: "cx::CXPddlMsgsPlanTemporalPlugin"

        timed_plan_action_msg:
            plugin: "cx::CXPddlMsgsTimedPlanActionPlugin"

The first block defines that the CX node starts and activates automatically. It also sets up a CLIPS environment with the name `pddl_agent` that is managed by the CX.

For this environment, we defined the plugins `executive`, `ros_msgs`, `files`, `config`, `plan_temporal_action`, and `timed_plan_action_msg` to be loaded. The first two are required to run the executive, and to enable communication with ROS. The plugin `files` is used to load our self-written CLIPS files (here we will have only one: `agent.clp`. And the last two are auto-generated plugins that are used to interact with the PDDL manager extension, specifically to receive temporal actions from the planner, and to call the planner service.  Finally, we can use the `ament_index` plugin later to access our package directories to more easily find files like PDDL files.

Save the file as `config.yaml` in the `params` directory that we previously created.

3 Writing a Launch File
^^^^^^^^^^^^^^^^^^^^^^^

Now we need to launch all our desired nodes. To do so properly, we will write a launch file `example_launch.py` in the `launch` directory we have created before.

.. code-block:: python

       def launch_with_context(context, *args, **kwargs):
       # configure and launc the CX node
       example_agent_dir = get_package_share_directory('pddl_agent_example')
       cx_config_file = os.path.join(example_agent_dir, "params", "config.yaml")

    log_level = LaunchConfiguration('log_level')
    cx_node = Node(
    package='cx_clips_env_manager',
    executable='cx_node',
    output='screen',
    emulate_tty=True,
    parameters=[
    cx_config_file,
    ],
    arguments=['--ros-args', '--log-level', log_level]
    )

    # launch the pddl_manager
    pddl_manager_dir = get_package_share_directory('pddl_manager')
    launch_pddl_manager = os.path.join(pddl_manager_dir, 'launch', 'pddl_manager.launch.py')

    return [cx_node, IncludeLaunchDescription(
    PythonLaunchDescriptionSource(launch_pddl_manager)
    ),]

    def generate_launch_description():
    declare_log_level_ = DeclareLaunchArgument(
    "log_level",
    default_value='info',
    )
    # The lauchdescription to populate with defined CMDS
    ld = LaunchDescription()
    ld.add_action(declare_log_level_)
    ld.add_action(OpaqueFunction(function=launch_with_context))

    return ld

The launch file consists of two parts, `generate_launch_description` returns a ROS2 launch description. In it we declare the log level as a parameter, and then use the function `launch_with_context` as a parameter to launch our actual nodes. First we define the directory of our package, and of our previously written config file, and start the |CX| node with the corresponding parameters. Then we also launch the `pddl_manager` as its own node, by finding its package directory and using its pre-defined launch file.

If you now build and run using the steps described in *7*, you should see the all nodes start up and then nothing happening anymore. This means, we can now move onto the PDDL and CLIPS side.

4 Access the PDDL Manager from CLIPS
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To access the PDDL Manager from within CLIPS, we need to understand how to communicate with other ROS2 nodes from within CLIPS. For this we can use the `RosMsgs` plugin.

First, we need to create clients for the services that we need to access. To do this, we use the `ros-msgs-create-client` function. Let's write a rule in our `agent.clp` for this:

.. code-block:: lisp

    (defrule pddl-init
        (not (pddl-services-loaded))
        =>
        ; create clints for all services
        (bind ?services (create$
            add_fluents AddFluents
            add_pddl_instance AddPddlInstance
            get_fluents GetFluents
            set_goals SetGoals
        ))
        (bind ?index 1)
        (bind ?length (length$ ?services))
        (while (< ?index ?length)
            (bind ?service-name (nth$ ?index ?services))
            (bind ?service-type (nth$ (+ ?index 1) ?services))
            (ros-msgs-create-client
                (str-cat "/pddl_manager" "/" ?service-name)
                (str-cat "pddl_msgs/srv/" ?service-type)
            )
            (bind ?index (+ ?index 2))
        )
        (assert (pddl-services-loaded))
    )

We also want to create a client for Plan Temporal, corresponding to the plugin we specifically loaded before: `CXPddlMsgsPlanTemporalPlugin`. To do so we can write the following simple rule:

.. code-block:: lisp

    (defrule pddl-init-plan-client
        (not (pddl-planning-client-created))
        =>
        (pddl-msgs-plan-temporal-create-client (str-cat "/pddl_manager" "/temp_plan"))
        (assert (pddl-planning-client-created))
    )

At the end of the rule, we assert a fact `(pddl-services-loaded)`, we use this to guide the program flow, and to "forward our agenda".  Next, we need to create a PDDL instance and tell the PDDL Manager which domain and problem files to load. We can use a rule like the following one for this:

.. code-block:: lisp

    (defrule pddl-add-instance
        (ros-msgs-client (service ?service&:(eq ?service (str-cat "/pddl_manager" "/add_pddl_instance"))) (type ?type))
        (not (pddl-loaded))
        (pddl-services-loaded)
        =>
        (bind ?new-req (ros-msgs-create-request ?type))
        (ros-msgs-set-field ?new-req "name" "test") ;instance of name test
        (bind ?share-dir (ament-index-get-package-share-directory "pddl_agent_example"))
        (ros-msgs-set-field ?new-req "directory" (str-cat ?share-dir "/pddl"))
        (ros-msgs-set-field ?new-req "domain_file" "domain.pddl")
        (ros-msgs-set-field ?new-req "problem_file" "problem.pddl")
        (bind ?id (ros-msgs-async-send-request ?new-req ?service))
        (assert (pddl-loaded))
        (ros-msgs-destroy-message ?new-req)
    )

Note, that we use the fact `ros-msgs-client` in our precondition to check whether we have a client for the `add_pddl_instance` service that we can use to create our request. We will have this, because of the rule that we previously wrote. Also note, that the service is identified by the node name (here we assume the default `/pddl_manager`) and by the service name `add_pddl_instance`. On the RHS of the rule, we just create a request message using the `ros-msgs-create-request` function, and then we fill the fields as required. We call our instance `test`, we use `ament-index-get-package-share-directory` to find the path to our PDDL files, and we just give the names of the files we created. Finally, we send the request using the function `ros-msgs-async-send-request` and destroy the message for memory cleanliness ;)

We want to know what kind of response we got (i.e., whether our request was successful or not, e.g., because the domain file contained errors), so we can write another rule for that:

.. code-block:: lisp

    (defrule pddl-add-instance-result
        (ros-msgs-client (service ?s&:(eq ?s (str-cat "/pddl_manager" "/add_pddl_instance"))) (type ?type))
        ?msg-f <- (ros-msgs-response (service ?s) (msg-ptr ?ptr) (request-id ?id))
        =>
        (bind ?success (ros-msgs-get-field ?ptr "success"))
        (bind ?error (ros-msgs-get-field ?ptr "error"))
        (if ?success then
            (printout t "PDDL instance added" crlf)
        else
            (printout error "Failed to set problem instance" ?error crlf)
        )
        (ros-msgs-destroy-message ?ptr)
        (retract ?msg-f)
    )

We use `ros-msgs-client` as before, but now also check if there is a `ros-msgs-response` fact. These facts are automatically asserted if we get a response to a request. They have a pointer in memory to a message, which we use to read them. On the RHS we unpack the message behind the pointer and just add some output. Finally, we destroy the message and retract the response fact.

5 Set a Goal
^^^^^^^^^^^^

Now that we have a PDDL instance set up, we can plan for it. As a first step, let's define the goal that we want to plan for. For this, we need to things: `pddl_msgs/msg/Fluent` messages, that we can use to describe the goal fluents. And the `ros-msgs-client` we know already. In the following rule we will do the following things:

* create a request
* create two fluents that make up our planning goal, corresponding to `(and (on a b) (on b c))` (remember we're working with a simple temporal blocksworld example here)
* =end the request with the two fluents and clean up.

.. code-block:: lisp

    (defrule pddl-set-goal
        (pddl-fluents-requested)
        (not (pddl-goals-set))
        (ros-msgs-client (service ?s&:(eq ?s (str-cat "/pddl_manager" "/set_goals"))) (type ?type))
        =>
        (bind ?new-req (ros-msgs-create-request ?type))
        (bind ?fluent-goal-msgs (create$))

        (bind ?fluent-msg (ros-msgs-create-message "pddl_msgs/msg/Fluent"))
        (ros-msgs-set-field ?fluent-msg "pddl_instance" "test")
        (ros-msgs-set-field ?fluent-msg "name" "on")
        (ros-msgs-set-field ?fluent-msg "args" (create$ "a" "b"))
        (bind ?fluent-goal-msgs (create$ ?fluent-goal-msgs ?fluent-msg))
        (bind ?fluent-msg (ros-msgs-create-message "pddl_msgs/msg/Fluent"))
        (ros-msgs-set-field ?fluent-msg "pddl_instance" "test")
        (ros-msgs-set-field ?fluent-msg "name" "on")
        (ros-msgs-set-field ?fluent-msg "args" (create$ "b" "c"))
        (bind ?fluent-goal-msgs (create$ ?fluent-goal-msgs ?fluent-msg))

        (ros-msgs-set-field ?new-req "fluents" ?fluent-goal-msgs)
        (bind ?id (ros-msgs-async-send-request ?new-req ?s))
        (if ?id then
            (printout t "Requested to set goals" crlf)
        else
            (printout error "Sending of request failed, is the service " ?s " running?" crlf)
        )
        (foreach ?msg ?fluent-goal-msgs
            (ros-msgs-destroy-message ?msg)
        )
        (ros-msgs-destroy-message ?new-req)
        (assert (pddl-goals-set))
    )

As you can see by now, communicating with the PDDL Manager follows the template of communicating with any other ROS node using the |CX|.

Again, we can listen to the results of our requests using the same approach as above:

.. code-block:: lisp

    (defrule pddl-set-goal-result
    " Get response, read it and delete."
        (ros-msgs-client (service ?s&:(eq ?s (str-cat "/pddl_manager" "/set_goals"))) (type ?type))
        ?msg-f <- (ros-msgs-response (service ?s) (msg-ptr ?ptr) (request-id ?id))
        (pddl-goals-set)
        =>
        (bind ?success (ros-msgs-get-field ?ptr "success"))
        (bind ?error (ros-msgs-get-field ?ptr "error"))
        (if ?success then
            (printout t "Goals set successfully" crlf)
        else
            (printout error "Failed to set goals (" "test" "):" ?error crlf)
        )
        (ros-msgs-destroy-message ?ptr)
        (retract ?msg-f)
    )

6 Plan for the Problem
^^^^^^^^^^^^^^^^^^^^^^

Finally, we can plan for the problem. So let's do it. Planning works a bit different, as we work with the `pddl-msgs-plan-temporal-client` now. Besides the change in names, the principle is still the same:

.. code-block:: lisp

    (defrule pddl-plan
        (pddl-msgs-plan-temporal-client (server ?server&:(eq ?server "/pddl_manager/temp_plan")))
        (not (planned))
        =>
        (printout green "Start planning" crlf)
        (bind ?goal (pddl-msgs-plan-temporal-goal-create))
        (pddl-msgs-plan-temporal-goal-set-field ?goal "pddl_instance" "test")
        (pddl-msgs-plan-temporal-goal-set-field ?goal "goal_instance" "base")
        (pddl-msgs-plan-temporal-send-goal ?goal ?server)
        (assert (planned))
    )

Here we plan for the instance `test` as defined above, and its base goal instance `base`.

Once we get the callback from the planner we can check if it successfully found a plan, and if it did, we can read the resulting plan actions. Since we are planning for a temporal planning, we can also get the durations and the time of the action in the plan. In the following rule we read the response, and print the actions nicely if we got a plan. Yay, we're done!

.. code-block:: lisp

    (defrule pddl-plan-result
        ?wr-f <- (pddl-msgs-plan-temporal-wrapped-result (server "/pddl_manager/temp_plan") (code SUCCEEDED) (result-ptr ?res-ptr))
        =>
        (bind ?plan-found (pddl-msgs-plan-temporal-result-get-field ?res-ptr "success"))
        (printout green "planning done" crlf)
        (if ?plan-found then
            (bind ?plan (pddl-msgs-plan-temporal-result-get-field ?res-ptr "actions"))
            (foreach ?action ?plan
            (bind ?name (sym-cat (pddl-msgs-timed-plan-action-get-field ?action "name")))
            (bind ?args (pddl-msgs-timed-plan-action-get-field ?action "args"))
            (bind ?ps-time (pddl-msgs-timed-plan-action-get-field ?action "start_time"))
            (bind ?p-duration (pddl-msgs-timed-plan-action-get-field ?action "duration"))
            (printout t ?ps-time "(" ?p-duration ")   " ?name ?args crlf)
            )
        else
            (printout red "plan not found!" crlf)
        )
        (pddl-msgs-plan-temporal-result-destroy ?res-ptr)
    )

7 Build and Run
^^^^^^^^^^^^^^^

To build the package, go into the respective directory in your workspace and run

.. code-block:: bash

    colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=OFF


Now you need to source the package

.. code-block:: bash

    source install/setup.bash

And finally, you can start the system using the launch file we wrote. You should see the planner being initialised and triggered, and the actions in the plan being printed. Run:

.. code-block:: bash

    ros2 launch pddl_agent_example example_launch.py

to start the full system.

Summary
^^^^^^^

You now know how to get access to PDDL planners through the |CX| with the PDDL Manager Plugin. This allows you to dynamically plan for problems and use the resulting plans as guidelines for your execution. The PDDL manager provides more tools for advanced users, so you can also manage multiple instances of a planning problem with various granularities and subsets of actions, and predicates.

Addendum: Other fun things you can do
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Of course there's many more interesting things you can do with the PDDL manager:

* you can check whether preconditions of actions are satisifed
* you can add and delete fluents from the world state
* you can manage complex problems, restricting your domain to only a subset of objects, fluents, or actions, etc.
* and many more.

These functionalities are still partially under development and will be covered in different tutorials. But, due to its relevance, we will briefly cover how to access the current initial state for your problem using the PDDL manager. The principles are the same that we already know, so we won't go into too much detail.

.. code-block:: lisp

    (defrule pddl-get-fluents
      (ros-msgs-client (service ?s&:(eq ?s (str-cat "/pddl_manager" "/get_fluents"))) (type ?type))
      (pddl-loaded)
      (not (pddl-fluents-requested))
      =>
      (bind ?new-req (ros-msgs-create-request ?type))
      (ros-msgs-set-field ?new-req "pddl_instance" "test")
      (bind ?id (ros-msgs-async-send-request ?new-req ?s))
      (if ?id then
        (printout t "Requested Fluents" crlf)
        (assert (pddl-fluents-requested))
       else
        (printout error "Sending of request failed, is the service " ?s " running?" crlf)
      )
      (ros-msgs-destroy-message ?new-req)
    )

    (defrule pddl-get-fluents-result
    " Get response, read it and delete."
      (ros-msgs-client (service ?s&:(eq ?s (str-cat "/pddl_manager" "/get_fluents"))) (type ?type))
      ?msg-f <- (ros-msgs-response (service ?s) (msg-ptr ?ptr) (request-id ?id))
      (pddl-fluents-requested)
    =>
      (bind ?success (ros-msgs-get-field ?ptr "success"))
      (bind ?error (ros-msgs-get-field ?ptr "error"))
      (if ?success then
        (printout t "Got fluents from current instance" crlf)
        (bind ?fluents (ros-msgs-get-field ?ptr "fluents"))
        (foreach ?fluent ?fluents
          (bind ?instance (sym-cat (ros-msgs-get-field ?fluent "pddl_instance")))
          (bind ?name (sym-cat (ros-msgs-get-field ?fluent "name")))
          (bind ?args (ros-msgs-get-field ?fluent "args"))
          (printout t ?name ?args crlf)
        )
       else
        (printout error "Failed to get fluents (" "test" "):" ?error crlf)
      )
      (ros-msgs-destroy-message ?ptr)
    )
