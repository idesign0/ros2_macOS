^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package autoware_internal_planning_msgs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1.12.1 (2025-12-22)
-------------------
* docs: add documentation to remaining msg/srv fields (`#82 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/82>`_)
  * docs: add unit and range documentation to message fields
  Add inline comments to clarify units and valid ranges for:
  - VelocityLimit.msg: max_velocity unit [m/s], sender constants
  - VelocityLimitClearCommand.msg: sender field description
  - ControlPoint.msg: velocity [m/s], shift_length [m], distance [m]
  - SafetyFactor.msg: ttc_begin/ttc_end units [s]
  Related to https://github.com/autowarefoundation/autoware_adapi_msgs/issues/106
  * fix sender
  * docs: add unit and range documentation to message definitions
  * style(pre-commit): autofix
  * remove node_modules
  * Update autoware_internal_debug_msgs/msg/ProcessingTimeNode.msg
  ---------
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* docs: add unit and range documentation to message fields (`#81 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/81>`_)
  * docs: add unit and range documentation to message fields
  Add inline comments to clarify units and valid ranges for:
  - VelocityLimit.msg: max_velocity unit [m/s], sender constants
  - VelocityLimitClearCommand.msg: sender field description
  - ControlPoint.msg: velocity [m/s], shift_length [m], distance [m]
  - SafetyFactor.msg: ttc_begin/ttc_end units [s]
  Related to https://github.com/autowarefoundation/autoware_adapi_msgs/issues/106
  * fix sender
  ---------
* Contributors: Yutaka Kondo

1.12.0 (2025-07-25)
-------------------
* chore(autoware_internal_planning_msgs): unifiy module load srv (`#76 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/76>`_)
* Contributors: Yuki TAKAGI

1.11.0 (2025-07-23)
-------------------
* feat(autoware_internal_planning_msgs): add new message definitions for new planning framework (`#73 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/73>`_)
  * feat(msgs): add new message definitions for CandidateTrajectories, CandidateTrajectory, GeneratorInfo, ScoredCandidateTrajectories, and ScoredCandidateTrajectory
  * fix(msgs): correct message type from TrajectoryGeneratorInfo to GeneratorInfo in CandidateTrajectories.msg
  * feat(msgs): add message definitions for CandidateTrajectories and related messages in CMakeLists.txt
  * refactor(msgs): simplify ScoredCandidateTrajectory by removing unnecessary fields
  ---------
* Contributors: Yukihiro Saito

1.10.0 (2025-06-03)
-------------------

1.9.0 (2025-05-23)
------------------

1.8.1 (2025-04-07)
------------------

1.8.0 (2025-03-27)
------------------
* feat(autoware_internal_planning_msgs): port routing services from tier4_planning_msgs (`#55 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/55>`_)
* Contributors: Ryohsuke Mitsudome

1.7.0 (2025-03-06)
------------------

1.6.0 (2025-02-19)
------------------
* feat: port VelocityLimit messages from tier4_planning_msgs (`#49 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/49>`_)
* Contributors: Ryohsuke Mitsudome

1.5.0 (2025-01-23)
------------------
* feat(autoware_internal_planning_msgs): adaption to autoware_motion_utils (`#45 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/45>`_)
  * feat(autoware_internal_planning_msgs): add msg required by autoware_motion_utils
  * style(pre-commit): autofix
  ---------
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* fix(autoware_internal_planning_msgs): fix PathPoint type error cause by duplicate definition in repo autoware_msgs and autoware_internal_msgs (`#46 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/46>`_)
  * fix(autoware_internal_planning_msgs): fix PathPoint type error cause by duplicate definition in repo autoware_msgs and autoware_internal_msgs
  * style(pre-commit): autofix
  ---------
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* Contributors: NorahXiong

1.4.0 (2025-01-15)
------------------
* feat(autoware_internal_planning_msgs): add Scenario.msg (`#43 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/43>`_)
  Co-authored-by: Yutaka Kondo <yutaka.kondo@youtalk.jp>
* feat(autoware_internal_planning_msgs): add PathWithLaneId.msg (`#42 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/42>`_)
  * feat(autoware_internal_planning_msgs): add PathWithLaneId.msg
  * style(pre-commit): autofix
  ---------
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* Contributors: cyn-liu

* feat(autoware_internal_planning_msgs): add Scenario.msg (`#43 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/43>`_)
  Co-authored-by: Yutaka Kondo <yutaka.kondo@youtalk.jp>
* feat(autoware_internal_planning_msgs): add PathWithLaneId.msg (`#42 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/42>`_)
  * feat(autoware_internal_planning_msgs): add PathWithLaneId.msg
  * style(pre-commit): autofix
  ---------
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* Contributors: cyn-liu

1.3.0 (2024-12-26)
------------------

1.2.0 (2024-12-19)
------------------

1.1.0 (2024-07-01)
------------------

1.0.1 (2024-05-10)
------------------
