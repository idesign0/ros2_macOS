.. _writing_a_plugin:

Writing a Plugin for TF Monitoring
##################################

**Goal:** Implement a CLIPS Plugin to observe transforms of a turtle from turtlesim

**Tutorial level:** Advanced

**Time:** 45 minutes

.. contents:: Contents
   :depth: 2
   :local:

Background
----------

Plugins can be used to access CLIPS environments via their C++ API, as detailed in the |APG|.
This allows users to customize CLIPS to their needs.

Prerequisites
-------------

This tutorial extends the ``cx_tutorial_agents`` package created in the :doc:`previous tutorial <hello_world>`.
Additionally, it requires the :rosdoc:`turtlesim` package, which provides a minimal simulation environment to interact with and the :rosdoc:`turtle_tf2_py` package to obtain transforms from the simulation.

The introductory tutorial for :rostut:`pluginlib <Tutorials/Beginner-Client-Libraries/Pluginlib.html>` is a helpful resource to get a general understanding of ``pluginlib`` plugins, as those are not discussed in detail here.

Similarly, the introduction to :rostut:`tf2 listeners <Tutorials/Intermediate/Tf2/Writing-A-Tf2-Listener-Cpp.html>`  explains the basics for observing transforms via ``tf2``, which is utilized in this tutorial.

TF Monitoring Plugin
--------------------

The objective of this tutorial is to write a custom CLIPS plugin that asynchronously injects poses obtained from a transform tree into a running CLIPS environment.

This involves:

* Setting up a class that derives from the base plugin definition.
* Adding a TF listener to query the transform tree on demand.
* Providing a CLIPS template for storing TF data, as well as CLIPS functions for starting and stopping periodic TF lookups.
* Managing asynchronous callbacks to update the fact base as needed.
* Handling CLIPS data and garbage collection.

.. note::

    Throughout this this tutorial, the CLIPS C(++) API is used extensively.
    When working with the API you should familiarize yourself with the :rosdoc:`clips_vendor` package and the namespaced CLIPS target it provides, as well as the |APG|, which is the reference manual for all public interfaces of CLIPS.


1 Obtaining the Files
^^^^^^^^^^^^^^^^^^^^^

Navigate to the ``params`` directory of the ``cx_tutorial_agents`` package from the :doc:`Hello World tutorial <hello_world>` and download the configuration file using the following command:

.. code-block:: bash

   cd ~/ros2/cx_tutorial_ws/src/cx_tutorial_agents/params
   wget -O tf2_tracked_pose.yaml https://raw.githubusercontent.com/carologistics/clips_executive/master/cx_tutorial_agents/params/tf2_tracked_pose.yaml

This adds the file ``tf2_tracked_pose.yaml`` for setting up the |CX| for this tutorial.

Next, a CLIPS file ``tf2_tracked_pose.clp`` can be downloaded to the ``clips`` directory, providing a minimal example to test the plugin:

.. code-block:: bash

   cd ~/ros2/cx_tutorial_ws/src/cx_tutorial_agents/clips
   wget -O tf2_tracked_pose.clp https://raw.githubusercontent.com/carologistics/clips_executive/master/cx_tutorial_agents/clips/tf2_tracked_pose.clp

The plugin code is taken from the :docsite:`Tf2PoseTrackerPlugin <clips_executive/plugins/tf2_pose_tracker_plugin>`. The code can be found on :source-master:`GitHub <cx_plugins/tf2_pose_tracker_plugin>`.

2 Plugin Package Setup
^^^^^^^^^^^^^^^^^^^^^^

The plugin resides in it's own ROS package, hence it has an appropriate ``package.xml`` and ``CMakeLists.txt``. Additionally, each plugin comes with a plugin description that needs to be properly exported. See the
:rostut:`beginner tutorial on pluginlib
<Tutorials/Beginner-Client-Libraries/Pluginlib.html>` for a general introduction to ``pluginlib`` plugins.

Each plugin of the |CX| uses  the ``cx::ClipsPlugin`` as base class, provided by the ``cx_plugin`` package. The plugin for this tutorial is defined as a class ``cx::Tf2PoseTrackerPlugin`` and resides in the ``cx_tf2_pose_tracker_plugin`` package. The description file is shown below:

.. code-block:: xml

    <class_libraries>
      <library path="cx_tf2_pose_tracker_plugin">
        <class type="cx::Tf2PoseTrackerPlugin" base_class_type="cx::ClipsPlugin">
          <description>Plugin to periodically track tf2 poses.</description>
        </class>
      </library>
    </class_libraries>

In the ``CMakeLists.txt``, the description file needs to be properly exported

.. code-block:: cmake

    pluginlib_export_plugin_description_file(cx_plugin tf2_pose_tracker_plugin.xml)

    install(
      FILES tf2_pose_tracker_plugin.xml
      DESTINATION share/${PROJECT_NAME}
    )

Also, the plugin needs to link against the namespaced CLIPS target provided by the :rosdoc:`clips_vendor` package, which is used throughout the |CX| to interface with CLIPS.

.. note::

    This target wraps the original CLIPS code with a namespace ``clips::``, hence the CLIPS features documented in the |APG| require the namespace prefix.

.. code-block:: cmake

    target_link_libraries(${PROJECT_NAME} ClipsNS::libclips_ns)


3 Plugin Class Structure
^^^^^^^^^^^^^^^^^^^^^^^^

The ``Tf2PoseTrackerPlugin`` class inherits from ``ClipsPlugin`` base class provided by the ``cx_plugin`` package.


.. code-block:: cpp

    #ifndef CX_PLUGINS__TF2POSETRACKER_PLUGIN_HPP_
    #define CX_PLUGINS__TF2POSETRACKER_PLUGIN_HPP_

    #include <string>
    #include <vector>

    #include "cx_plugin/clips_plugin.hpp"

    #include <geometry_msgs/msg/transform_stamped.hpp>
    #include <tf2_ros/buffer.h>
    #include <tf2_ros/transform_listener.h>

    namespace cx {

    class Tf2PoseTrackerPlugin : public ClipsPlugin {
    public:
      Tf2PoseTrackerPlugin();
      ~Tf2PoseTrackerPlugin();

      void initialize() override;
      void finalize() override;

      bool clips_env_init(std::shared_ptr<clips::Environment> &env) override;
      bool clips_env_destroyed(std::shared_ptr<clips::Environment> &env) override;

    private:
      struct PoseTracker {
        rclcpp::TimerBase::SharedPtr timer;
        clips::Fact *pose_fact;
        clips::Environment *env;
      };

      std::unique_ptr<rclcpp::Logger> logger_;

      std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
      std::unique_ptr<tf2_ros::TransformListener> tf_listener_;

      rclcpp::CallbackGroup::SharedPtr cb_group_;

      std::vector<std::shared_ptr<PoseTracker>> pose_trackers_;

      void start_periodic_lookup(clips::Environment *env, const std::string &parent,
                                 const std::string &child, double frequency);
      bool stop_periodic_lookup(PoseTracker *pose_tracker);
    };
    } // namespace cx

    #endif // !CX_PLUGINS__TF2POSETRACKER_PLUGIN_HPP_

As such, it generally should override the functions ``initialize`` and ``finalize``, which are invoked when the plugin is loaded, as well as the function ``clips_env_init`` and ``clips_env_destoyed``, which are called each time a CLIPS environment loads and unloads the plugin.

.. code-block:: cpp

  void initialize() override;
  void finalize() override;

  bool clips_env_init(std::shared_ptr<clips::Environment> &env) override;
  bool clips_env_destroyed(std::shared_ptr<clips::Environment> &env) override;

Also, it defines some data types and structures to access ``tf2`` transforms and ROS timers for periodic callbacks. The ``PoseTracker`` struct bundles a ROS timer with a managed CLIPS fact storing the last updated transform, as well as the belonging CLIPS environment.

.. code-block:: cpp

      struct PoseTracker {
        rclcpp::TimerBase::SharedPtr timer;
        clips::Fact *pose_fact;
        clips::Environment *env;
      };

The different ``PoseTracker`` instances are stored in a vector, managing the lifetime of the objects.

.. code-block:: cpp

      std::vector<std::shared_ptr<PoseTracker>> pose_trackers_;

Lastly, two helper functions are used that will be bound to CLIPS functions and will allow to create and destroy ``PoseTracker`` instances.

.. code-block:: cpp

      void start_periodic_lookup(clips::Environment *env, const std::string &parent,
                                 const std::string &child, double frequency);
      bool stop_periodic_lookup(PoseTracker *pose_tracker);

4 Opening ROS Interfaces on initialize()
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Plugin initialization involves all steps that should be done, before any CLIPS environment can utilize it's features.

Here, the transform listener is initialized with the help of the parent lifecycle node of the |CX|, provided from the base class.

.. code-block:: cpp

    void Tf2PoseTrackerPlugin::initialize() {
      logger_ = std::make_unique<rclcpp::Logger>(rclcpp::get_logger(plugin_name_));

      auto node = parent_.lock();
      // fetch plugin parameter
      cx::cx_utils::declare_parameter_if_not_declared(
          node, plugin_name_ + ".spin_thread", rclcpp::ParameterValue(true));
      bool tf_spin_thread;
      node->get_parameter(plugin_name_ + ".spin_thread", tf_spin_thread);

      // setup transform listener
      cb_group_ = node->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
      tf_buffer_ = std::make_shared<tf2_ros::Buffer>(node->get_clock());
      tf_listener_ = std::make_unique<tf2_ros::TransformListener>(
          *tf_buffer_, node->get_node_base_interface(),
          node->get_node_logging_interface(), node->get_node_parameters_interface(),
          node->get_node_topics_interface(), tf_spin_thread);
    }

Note that plugins must provide default constructors. Information is only passed to a plugin after construction.
This is why the ROS logger is wrapped in a smart pointer (so it can be default-constructed), and only instantiated on the ``initialize()`` call, at which the actual plugin name is known (provided by the base class via ``plugin_name_``).

.. code-block:: cpp

      logger_ = std::make_unique<rclcpp::Logger>(rclcpp::get_logger(plugin_name_));


Additionally, a plugin-specific parameter ``spin_thread`` is declared and retrieved, stored under the respective plugin name.

.. code-block:: cpp

      // fetch plugin parameter
      cx::cx_utils::declare_parameter_if_not_declared(
          node, plugin_name_ + ".spin_thread", rclcpp::ParameterValue(true));
      bool tf_spin_thread;
      node->get_parameter(plugin_name_ + ".spin_thread", tf_spin_thread);

5 Cleaning Up References On finalize()
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

During finalize the plugin cleans up all data structures it manages, in this cas the transform listener with the associated buffer and callback group as well as all managed ROS timers for tracking specific poses.

.. code-block:: cpp

    void Tf2PoseTrackerPlugin::finalize() {
      // release all memory and cancel all timers
      for(auto &pose_tracker: pose_trackers_) {
        pose_tracker->timer->cancel();
        clips::ReleaseFact(pose_tracker->pose_fact);
      }

      // release references
      pose_trackers_.clear();
      logger_.reset();
      tf_buffer_.reset();
      tf_listener_.reset();
      cb_group_.reset();
    }

In particular, all timers are properly cancelled and all references to CLIPS data is released for garbage collection.

.. note::

   Special care is required when handling data from CLIPS, as CLIPS manages it's memory including garbage collection as needed. ``Retain`` and ``Release`` functions are provided to safely interact with data.


6 Adding Templates and Functions in clips_env_init()
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Upon loading a plugin into an environment, the ``clips_env_init()`` function is invoked.

Here, a fact template is declared and two user-defined functions (UDFs) are provided to the environment.

.. code-block:: cpp

    bool Tf2PoseTrackerPlugin::clips_env_init(
        std::shared_ptr<clips::Environment> &env) {
      auto context = CLIPSEnvContext::get_context(env.get());
      RCLCPP_DEBUG(*logger_, "Initializing plugin for environment %s",
                   context->env_name_.c_str());

      // define fact template
      clips::Build(env.get(), "(deftemplate tf2-tracked-pose \
                (slot parent (type STRING)) \
                (slot child (type STRING)) \
                (slot stamp (type FLOAT)) \
                (multislot translation (type FLOAT) (cardinality 3 3)) \
                (multislot rotation (type FLOAT) (cardinality 4 4)) \
                (slot timer (type EXTERNAL-ADDRESS)) \
    )");

      // user defined functions
      clips::AddUDF(
          env.get(), "tf2-start-periodic-lookup", "b", 3, 3, ";sy;sy;d",
          [](clips::Environment *env, clips::UDFContext *udfc,
             clips::UDFValue *out) {
            auto *instance = static_cast<Tf2PoseTrackerPlugin *>(udfc->context);
            clips::UDFValue parent, child, freq;
            using namespace clips;
            clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &parent);
            clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &child);
            clips::UDFNthArgument(udfc, 3, NUMBER_BITS, &freq);

            try {
              instance->start_periodic_lookup(env, parent.lexemeValue->contents,
                                              child.lexemeValue->contents,
                                              freq.floatValue->contents);
              out->lexemeValue = clips::CreateBoolean(env, true);
            } catch (std::exception &e) {
              RCLCPP_ERROR(*instance->logger_, "Failed to create pose updater: %s",
                           e.what());
              out->lexemeValue = clips::CreateBoolean(env, false);
            }
          },
          "tf2_start_periodic_lookup", this);

      clips::AddUDF(
          env.get(), "tf2-stop-periodic-lookup", "b", 1, 1, ";e",
          [](clips::Environment *env, clips::UDFContext *udfc,
             clips::UDFValue *out) {
            auto *instance = static_cast<Tf2PoseTrackerPlugin *>(udfc->context);
            clips::UDFValue pose_tracker;
            using namespace clips;
            clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &pose_tracker);
            PoseTracker *typed_pose_tracker = static_cast<PoseTracker *>(
                pose_tracker.externalAddressValue->contents);
            out->lexemeValue = clips::CreateBoolean(
                env, instance->stop_periodic_lookup(typed_pose_tracker));
          },
          "tf2_stop_periodic_lookup", this);

      return true;
    }

6.1 Access to Environment Context
.................................

The function starts with a simple debugging statement that utilizes the plugins ROS logger to print for which environment the plugin is initialized. This requires accessing the context stored in each environment managed by the |CX|, as this is where custom data, such as the user-assigned name of each environment, is stored.

.. code-block::

      auto context = CLIPSEnvContext::get_context(env.get());
      RCLCPP_DEBUG(*logger_, "Initializing plugin for environment %s",
                   context->env_name_.c_str());

6.1 CLIPS Environments and Threading
....................................

CLIPS is not thread-safe, hence interactions with CLIPS need to be guarded from concurrent access.

The environment is already guarded by the mutex when entering ``clips_env_init()`` (invoked by the environment manager node), and it is safe to directly interact with the provided environment in this scope. The same holds true for ``clips_env_destroyed()``.

The mutex belonging to a CLIPS environment is stored on the heap inside of the environment context (via the member ``context->env_mtx_``).


6.3 Constructs via Build Function
.................................

The ``Build`` function is used to construct a deftemplate for the environment from a string representation. This template is used to store the information obtained from transform lookups.

.. code-block:: cpp

  // define fact template
  clips::Build(env.get(), "(deftemplate tf2-tracked-pose \
            (slot parent (type STRING)) \
            (slot child (type STRING)) \
            (slot stamp (type FLOAT)) \
            (multislot translation (type FLOAT) (cardinality 3 3)) \
            (multislot rotation (type FLOAT) (cardinality 4 4)) \
            (slot timer (type EXTERNAL-ADDRESS)) \
  )");


6.4 User-Defined Functions
..........................

A common motivation for writing plugins is to provide more functions that can be called in CLIPS. In the following, the definition for the first function "tf2-start-periodic-lookup" is examined more closely.
The corresponding ``AddUDF`` call needs the following arguments:

* A raw pointer to the ``Environment`` object that should register the user-defined function.
* The name of the function in CLIPS.
* The return specifier (here ``b`` for a boolean)
* Min and max number of arguments (exactly 3 in this case)
* The types of the function arguments, separated by ``;`` and starting with a fallback type, in this case left blank. ``sy`` indicates that both symbols and strings are accepted, ``d`` denotes floats.
* The function to invoke, which takes as arguments the environment pointer, a ``UDFContext`` for passing more data into the function and an output parameter storing the return value of the function. The function itself has no return value (void).
* An internal name for storing the function in the backend.
* Context that can be accessed when the function is invoked via the ``UDFContext`` argument (here, the reference to the plugin itself is passed as context to invoke some helper functions of the plugin).

.. code-block:: cpp

  // user defined functions
  clips::AddUDF(
      env.get(), "tf2-start-periodic-lookup", "b", 3, 3, ";sy;sy;d",
      [](clips::Environment *env, clips::UDFContext *udfc,
         clips::UDFValue *out) {
        auto *instance = static_cast<Tf2PoseTrackerPlugin *>(udfc->context);
        clips::UDFValue parent, child, freq;
        using namespace clips;
        clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &parent);
        clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &child);
        clips::UDFNthArgument(udfc, 3, NUMBER_BITS, &freq);

        try {
          instance->start_periodic_lookup(env, parent.lexemeValue->contents,
                                          child.lexemeValue->contents,
                                          freq.floatValue->contents);
          out->lexemeValue = clips::CreateBoolean(env, true);
        } catch (std::exception &e) {
          RCLCPP_ERROR(*instance->logger_, "Failed to create pose updater: %s",
                       e.what());
          out->lexemeValue = clips::CreateBoolean(env, false);
        }
      },
      "tf2_start_periodic_lookup", this);

Inside of the lambda function, the first step is to reconstruct the passed context via casting the held void reference to the appropriate type:

.. code-block:: cpp

            auto *instance = static_cast<Tf2PoseTrackerPlugin *>(udfc->context);

Next, the function arguments are retrieved. Since the number of arguments is fixed, this can be achieved using the ``UDFNthArgument`` function.

.. code-block:: cpp

            clips::UDFValue parent, child, freq;
            using namespace clips;
            clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &parent);
            clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &child);
            clips::UDFNthArgument(udfc, 3, NUMBER_BITS, &freq);

However, this snippet also showcases an unfortunate drawback when using a namespaced version of the CLIPS library, which is also mentioned in the known issues of the :rosdoc:`clips_vendor` package: The ``LEXEME_BITS`` and ``NUMBER_BITS`` statements are macros that extend to a disjunction of enum types, which are not properly namespaced.
Hence, the ``using namespace clips;`` directive is necessary here to properly use the macros.

Lastly, the helper function ``start_periodic_lookup`` is called using the context.
The CLIPS arguments are converted to their native C++ types, before they are passed at arguments to the helper function.
``STRING`` and ``SYMBOL`` types are stored a C-style strings (via ``lexemeValue``), while ``FLOAT`` values are mapped to ``double`` (via ``floatValue``).

.. code-block:: cpp

            try {
              instance->start_periodic_lookup(env, parent.lexemeValue->contents,
                                              child.lexemeValue->contents,
                                              freq.floatValue->contents);
              out->lexemeValue = clips::CreateBoolean(env, true);
            } catch (std::exception &e) {
              RCLCPP_ERROR(*instance->logger_, "Failed to create pose updater: %s",
                           e.what());
              out->lexemeValue = clips::CreateBoolean(env, false);
            }
          },

The output parameter is populated by creating a boolean indicating the success of the attempted helper function call.

The second UDF is populated in much of the same way, this time taking an external address (``void *``) as argument, which needs to be cast to it's expected type.

.. code-block:: cpp

  clips::AddUDF(
      env.get(), "tf2-stop-periodic-lookup", "b", 1, 1, ";e",
      [](clips::Environment *env, clips::UDFContext *udfc,
         clips::UDFValue *out) {
        auto *instance = static_cast<Tf2PoseTrackerPlugin *>(udfc->context);
        clips::UDFValue pose_tracker;
        using namespace clips;
        clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &pose_tracker);
        PoseTracker *typed_pose_tracker = static_cast<PoseTracker *>(
            pose_tracker.externalAddressValue->contents);
        out->lexemeValue = clips::CreateBoolean(
            env, instance->stop_periodic_lookup(typed_pose_tracker));
      },
      "tf2_stop_periodic_lookup", this);

.. note::

   The body of each UDF can safely access CLIPS because the context that invokes the function should ensure that the environment is locked already, typically this is the ``ExecutivePlugin`` that handles the CLIPS inference engine runs. Do not try to lock the environment again within the execution scope of a UDF.

7 Asynchronous Handling of CLIPS facts in start_periodic_lookup()

The start_periodic_lookup function is responsible for creating a ROS timer that queries the transform tree and updates a fact to store the latest update to the retrieved pose.

.. code-block:: cpp

    void Tf2PoseTrackerPlugin::start_periodic_lookup(clips::Environment *env,
                                                     const std::string &parent,
                                                     const std::string &child,
                                                     double frequency) {
      using namespace std::chrono_literals;
      std::shared_ptr<PoseTracker> pose_tracker = std::make_shared<PoseTracker>();
      pose_tracker->env = env;
      auto node = parent_.lock();
      pose_tracker->timer = node->create_wall_timer(
          std::chrono::duration<double>(1.0 / frequency),
          [this, pose_tracker, env, parent, child]() {
            geometry_msgs::msg::TransformStamped tf;
            try {

              tf = tf_buffer_->lookupTransform(parent, child, tf2::TimePointZero);
              double stamp_sec =
                  tf.header.stamp.sec + tf.header.stamp.nanosec * 1e-9;

              // safely access CLIPS environment
              auto context = CLIPSEnvContext::get_context(env);
              std::scoped_lock clips_lock{context->env_mtx_};

              // update exisiting fact or create new one
              bool fact_exists = clips::FactExistp(pose_tracker->pose_fact);
              if (!pose_tracker->pose_fact || !fact_exists) {
                if (pose_tracker->pose_fact) {
                  // fact was retained before but did not survive the engine, this
                  // is not supposed to happen
                  RCLCPP_WARN(*logger_,
                              "TF lookup from %s to %s: fact was retained but does "
                              "not exist anymore",
                              parent.c_str(), child.c_str());
                  clips::ReleaseFact(pose_tracker->pose_fact);
                }

                // New fact needed, build and retain it
                clips::FactBuilder *fact_builder =
                    clips::CreateFactBuilder(env, "tf2-tracked-pose");
                clips::FBPutSlotCLIPSExternalAddress(
                    fact_builder, "timer",
                    clips::CreateCExternalAddress(env, pose_tracker.get()));
                clips::FBPutSlotString(fact_builder, "parent", parent.c_str());
                clips::FBPutSlotString(fact_builder, "child", child.c_str());
                clips::FBPutSlotFloat(fact_builder, "stamp", stamp_sec);
                clips::FBPutSlotMultifield(
                    fact_builder, "translation",
                    clips::StringToMultifield(
                        env, std::format("{} {} {}", tf.transform.translation.x,
                                         tf.transform.translation.y,
                                         tf.transform.translation.z)
                                 .c_str()));
                clips::FBPutSlotMultifield(
                    fact_builder, "rotation",
                    clips::StringToMultifield(
                        env, std::format("{} {} {} {}", tf.transform.rotation.x,
                                         tf.transform.rotation.y,
                                         tf.transform.rotation.z,
                                         tf.transform.rotation.w)
                                 .c_str()));
                pose_tracker->pose_fact = clips::FBAssert(fact_builder);
                clips::RetainFact(pose_tracker->pose_fact);
                clips::FBDispose(fact_builder);
              } else {
                // the fact exists and can can be modified
                clips::ReleaseFact(pose_tracker->pose_fact);
                clips::FactModifier *fact_modifier =
                    clips::CreateFactModifier(env, pose_tracker->pose_fact);
                clips::FMPutSlotFloat(fact_modifier, "stamp", stamp_sec);
                clips::FMPutSlotMultifield(
                    fact_modifier, "translation",
                    clips::StringToMultifield(
                        env, std::format("{} {} {}", tf.transform.translation.x,
                                         tf.transform.translation.y,
                                         tf.transform.translation.z)
                                 .c_str()));
                clips::FMPutSlotMultifield(
                    fact_modifier, "rotation",
                    clips::StringToMultifield(
                        env, std::format("{} {} {} {}", tf.transform.rotation.x,
                                         tf.transform.rotation.y,
                                         tf.transform.rotation.z,
                                         tf.transform.rotation.w)
                                 .c_str()));
                pose_tracker->pose_fact = clips::FMModify(fact_modifier);
                clips::RetainFact(pose_tracker->pose_fact);
                clips::FMDispose(fact_modifier);
              }
            } catch (const tf2::TransformException &e) {
              RCLCPP_WARN(*logger_, "TF lookup failed: %s", e.what());
            }
          },
          cb_group_);

      // store the pose tracker
      pose_trackers_.push_back(pose_tracker);
    }


7.1 Guarding the CLIPS Environment from Concurrent Access
.........................................................

This asynchronous task showcases the need for guarding the CLIPS environment from concurrent access.

While the UDF function body itself is guarded already, the callback of the created ROS timer is not.
Since the |CX| itself is ran via a :rostut:`MultiThreadedExecutor </Concepts/Intermediate/About-Executors.html>`, ROS callbacks are typically executed in parallel.

In order to obtain the required mutex, the environment context is retrieved. Then a scoped lock protects the remainder of this scope.

.. code-block:: cpp

          // safely access CLIPS environment
          auto context = CLIPSEnvContext::get_context(env);
          std::scoped_lock clips_lock{context->env_mtx_};

7.2 Creating and Modifying Facts
................................

The next step is to either create the initial fact for the data, or to update the previously asserted one with the new information.

For this, it is first checked, whether a new fact needs to be asserted (releasing the outdated fact reference if needed):

.. code-block:: cpp

              // update exisiting fact or create new one
              bool fact_exists = clips::FactExistp(pose_tracker->pose_fact);
              if (!pose_tracker->pose_fact || !fact_exists) {
                if (pose_tracker->pose_fact) {
                  // fact was retained before but did not survive the engine, this
                  // is not supposed to happen
                  RCLCPP_WARN(*logger_,
                              "TF lookup from %s to %s: fact was retained but does "
                              "not exist anymore",
                              parent.c_str(), child.c_str());
                  clips::ReleaseFact(pose_tracker->pose_fact);
                }

In case a new fact is needed, it can be created via the FactBuilder API. The resulting reference to the fact is retained to update it in subsequent iterations.
The ``timer`` slot is used to also hand a reference to the pose tracker object managing this timer, which can be used to stop the timer using the respective UDF.

.. code-block:: cpp

            // New fact needed, build and retain it
            clips::FactBuilder *fact_builder =
                clips::CreateFactBuilder(env, "tf2-tracked-pose");
            clips::FBPutSlotCLIPSExternalAddress(
                fact_builder, "timer",
                clips::CreateCExternalAddress(env, pose_tracker.get()));
            clips::FBPutSlotString(fact_builder, "parent", parent.c_str());
            clips::FBPutSlotString(fact_builder, "child", child.c_str());
            clips::FBPutSlotFloat(fact_builder, "stamp", stamp_sec);
            clips::FBPutSlotMultifield(
                fact_builder, "translation",
                clips::StringToMultifield(
                    env, std::format("{} {} {}", tf.transform.translation.x,
                                     tf.transform.translation.y,
                                     tf.transform.translation.z)
                             .c_str()));
            clips::FBPutSlotMultifield(
                fact_builder, "rotation",
                clips::StringToMultifield(
                    env, std::format("{} {} {} {}", tf.transform.rotation.x,
                                     tf.transform.rotation.y,
                                     tf.transform.rotation.z,
                                     tf.transform.rotation.w)
                             .c_str()));
            pose_tracker->pose_fact = clips::FBAssert(fact_builder);
            clips::RetainFact(pose_tracker->pose_fact);
            clips::FBDispose(fact_builder);


Similarly, if the last remembered fact still exists, the FactModifier API is used. Additionally, the old fact reference is released to mark it for garbage collection and the new reference is retained.

.. code-block:: cpp

          } else {
            // the fact exists and can can be modified
            clips::ReleaseFact(pose_tracker->pose_fact);
            clips::FactModifier *fact_modifier =
                clips::CreateFactModifier(env, pose_tracker->pose_fact);
            clips::FMPutSlotFloat(fact_modifier, "stamp", stamp_sec);
            clips::FMPutSlotMultifield(
                fact_modifier, "translation",
                clips::StringToMultifield(
                    env, std::format("{} {} {}", tf.transform.translation.x,
                                     tf.transform.translation.y,
                                     tf.transform.translation.z)
                             .c_str()));
            clips::FMPutSlotMultifield(
                fact_modifier, "rotation",
                clips::StringToMultifield(
                    env, std::format("{} {} {} {}", tf.transform.rotation.x,
                                     tf.transform.rotation.y,
                                     tf.transform.rotation.z,
                                     tf.transform.rotation.w)
                             .c_str()));
            pose_tracker->pose_fact = clips::FMModify(fact_modifier);
            clips::RetainFact(pose_tracker->pose_fact);
            clips::FMDispose(fact_modifier);
          }

This concludes the callback function of the pose tracker, which then is stored to a vector to manage it's lifetime, completing the task to to create a pose tracker.

.. code-block:: cpp

      // store the pose tracker
      pose_trackers_.push_back(pose_tracker);

8 Cleaning Up with the Help of stop_periodic_lookup()
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The last thing that is left is to stop the pose tracker on demand, given the raw reference to it (as stored previously in the ``timer`` slot).
When locating the stored ``PoseTracker``, it's timer is cancelled and the associated fact is released for garbage collection (but not retracted), before it is removed from the vector, which cleans up the object.

.. code-block:: cpp

    bool Tf2PoseTrackerPlugin::stop_periodic_lookup(PoseTracker *pose_tracker) {
      // lookup active updater, cancel the timer and release the fact address
      auto it = std::find_if(pose_trackers_.begin(), pose_trackers_.end(),
                             [pose_tracker](const std::shared_ptr<PoseTracker> &p) {
                               return p.get() == pose_tracker;
                             });

      if (it != pose_trackers_.end()) {
        clips::ReleaseFact(it->get()->pose_fact);
        it->get()->timer->cancel();
        pose_trackers_.erase(it);
        return true;
      } else {
        RCLCPP_WARN(*logger_, "tf2-stop-periodic-lookup: failed to stop periodic "
                              "lookup, invalid pointer!");
        return false;
      }
    }

9 Running The Code
^^^^^^^^^^^^^^^^^^

Open a terminal and start the tf2 turtlesim demo:

.. code-block:: bash

    ros2 launch turtle_tf2_py turtle_tf2_demo.launch.py

In a second terminal run the example setup for the plugin:

.. code-block:: bash

    ros2 launch cx_bringup cx_launch.py manager_config:=tf2_tracked_pose.yaml package:=cx_tutorial_agents

It will track the pose of turtle1 with a frequency of 0.2 hz and stop the tracking after 5 updates.

Optionally, open a third terminal to control the turtle:

.. code-block:: bash

    ros2 run turtlesim turtle_teleop_key
