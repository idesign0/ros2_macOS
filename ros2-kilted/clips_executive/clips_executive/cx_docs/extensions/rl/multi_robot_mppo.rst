.. _cx_rl_mppo:

CXRLMaskablePPONode: RL Model Multi-Robot Applications
======================================================

The ``CXRLMaskablePPONode`` class provides a ROS 2 node integrated with a
maskable, multi-robot Proximal Policy Optimization (PPO) agent
(``MultiRobotMaskablePPO``). This allows training and executing reinforcement
learning policies in environments with multiple robots while automatically
handling invalid action masking.

It extends the ``CXRLBaseNode`` and integrates a custom maskable PPO
implementation compatible with Stable-Baselines3 and SB3-Contrib.

Key features:

- Multi-robot support with configurable number of parallel agents.
- Maskable PPO allowing invalid actions to be ignored during training.
- Time-based or step-based rollout collection.
- Integration with ROS 2 for lifecycle, parameter management, and execution.
- Automatic logging, checkpointing, and training termination.

MultiRobotMaskablePPO
*********************

The ``MultiRobotMaskablePPO`` class is a custom extension of
`MaskableActorCriticPolicy`_ from `sb3_contrib`_. It supports multi-robot environments and
invalid action masking.

Key points:

- Rollout collection uses multithreading to simulate multiple robots in
  parallel. Each thread executes actions for a specific robot and steps the environment concurrently, enabling faster collection of experiences while respecting per-robot action masks.
- Supports ``time_based`` or ``step_based`` rollout collection.
- Integrates with ``MultiRobotMaskableRolloutBuffer`` and
  ``MultiRobotMaskableDictRolloutBuffer`` for action masking.
- Computes returns and advantages, accounting for terminal states
  even if episodes are truncated.


Configuration
*************

The ``CXRLMaskablePPONode`` exposes the following ROS 2 parameters for configuring the RL agent and training:

Base PPO Parameters (forwarded from MaskablePPO)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These parameters are standard PPO hyperparameters, forwarded directly to the underlying ``MultiRobotMaskablePPO`` implementation:

============================= =============== =========================================================
Parameter                     Type             Description
============================= =============== =========================================================
model.learning_rate            double          Learning rate of the PPO agent
model.gamma                    double          Discount factor for future rewards
model.gae_lambda               double          Factor for Generalized Advantage Estimation
model.ent_coef                 double          Entropy coefficient for the PPO loss
model.vf_coef                  double          Value function coefficient for the PPO loss
model.max_grad_norm             double          Maximum gradient norm for gradient clipping
model.batch_size               integer         Minibatch size for PPO updates
model.n_steps                  integer         Number of steps per environment for rollout
model.seed                     integer         Random seed for reproducibility
model.verbose                  integer         Verbosity level for logging (0=none, 1=info, 2=debug)
============================= =============== =========================================================

.. attention::

   Not all parameters of the underlying PPO/MPPO model are exposed as ROS parameters.
   To access advanced settings (such as custom network architectures, `policy_kwargs`, or other
   Stable Baselines3 options), create a class that derives from ``CXRLMaskablePPONode`` override the ``create_new_model()`` method (see the :ref:`Interfaces <cxrlmppo_interfaces>` section for more information).

Multi-Robot / Custom Parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These parameters are specific to the multi-robot maskable variant:

The ``CXRLMaskablePPONode`` utilizes the following parameters:

:model_name:

  ============= =======
  Type          Default
  ------------- -------
  string        "default_agent"
  ============= =======

  Description
    Name of the RL model for saving or loading.

:training.retraining:

  ============= =======
  Type          Default
  ------------- -------
  boolean       False
  ============= =======

  Description
    Whether to retrain an existing model.

:training.max_episodes:

  ============= =======
  Type          Default
  ------------- -------
  integer       1000
  ============= =======

  Description
    Maximum number of training episodes.

:training.timesteps:

  ============= =======
  Type          Default
  ------------- -------
  integer       100000
  ============= =======

  Description
    Total number of timesteps for the learning procedure.

:model.n_robots:

  ============= =======
  Type          Default
  ------------- -------
  integer       3
  ============= =======

  Description
    Number of robots to simulate in parallel during rollout collection.

:model.wait_for_all_robots:

  ============= =======
  Type          Default
  ------------- -------
  boolean       True
  ============= =======

  Description
    Whether to wait for all robot threads to finish before computing returns. Ensures rollouts are fully collected for all robots.

:model.time_based:

  ============= =======
  Type          Default
  ------------- -------
  boolean       False
  ============= =======

  Description
    If True, rollouts are collected for a fixed duration instead of a fixed number of steps.

:model.n_time:

  ============= =======
  Type          Default
  ------------- -------
  integer       450
  ============= =======

  Description
    Maximum duration (in seconds) of a time-based rollout when ``time_based=True``.

:model.deadzone:

  ============= =======
  Type          Default
  ------------- -------
  integer       10
  ============= =======

  Description
    Time buffer (in seconds) to prevent starting new threads near the end of a time-based rollout.

.. _cxrlmppo_interfaces:
Interfaces
**********


The ``CXRLMaskablePPONode`` provides the following methods:

``set_model() → MultiRobotMaskablePPO``
  Populate ``self.model`` with an RL agent depending on the current ``rl_mode`` and links it with the ``CXRLGym`` environemnt.

  Behavior:

  - **TRAINING**: creates a new agent or loads an existing one if ``training.retraining=True``.
  - **EXECUTION**: loads a previously trained agent.

  Returns the loaded or newly created ``MultiRobotMaskablePPO`` instance.

``create_new_model() → MultiRobotMaskablePPO``
  Helper method to ``set_model``.
  Creates a new MPPO agent from scratch with parameters read from ROS parameters.

``load_model() → MultiRobotMaskablePPO``
  Helper method to ``set_model``.
  Loads a previously trained MPPO agent from disk.
  Updates ``self.model`` and links it with ``self.env``.
  Logs a message once loading is complete.

``run_training()``
  Starts the training loop of the RL agent according to the parameters
  ``training.max_episodes`` and ``training.timesteps``.
  Handles callbacks for stopping and checkpointing.
  Saves the trained agent to disk and calls ``self.env.env.on_training_end()``.
  Performs node shutdown after training completes.
