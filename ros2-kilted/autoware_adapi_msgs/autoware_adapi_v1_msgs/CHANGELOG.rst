^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package autoware_adapi_v1_msgs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1.9.1 (2025-11-14)
------------------
* feat(autoware_adapi_v1_msgs): add ROUNDABOUT support (`#102 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/102>`_)
  feat(autoware_adapi_v1_msgs): add ROUNDABOUT constant to PlanningBehavior message
* Contributors: Sho Iwasawa

1.9.0 (2025-06-19)
------------------
* feat(autoware_adapi_v1_msgs): add parent field to diag leaf message (`#97 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/97>`_)
* fix(autoware_adapi_v1_msgs): fix description response (`#96 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/96>`_)
* feat(autoware_adapi_v1_msgs): update diag messages (`#92 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/92>`_)
* feat(autoware_adapi_v1_msgs): add mrm description (`#94 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/94>`_)
* feat(autoware_adapi_v1_msgs): rename mrm request sender (`#91 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/91>`_)
* feat(autoware_adapi_v1_msgs): add mrm request messages (`#83 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/83>`_)
* feat(autoware_adapi_v1_msgs): add vehicle spec message (`#85 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/85>`_)
* feat(autoware_adapi_v1_msgs): add metrics message (`#84 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/84>`_)
* feat(autoware_adapi_v1_msgs): update manual control messages (`#86 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/86>`_)
* feat(autoware_adapi_v1_msgs): add RTI message (`#47 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/47>`_)
* feat(autoware_adapi_v1_msgs): add manual control messages (`#82 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/82>`_)
* Contributors: Takagi, Isamu

1.7.0 (2025-02-18)
------------------
* feat: drop route following check option (`#71 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/71>`_)
* fix(autoware_adapi_msgs): fix links to issues in CHANGELOG.rst files (`#72 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/72>`_)
* feat(planning): add new planning behavior (adaptive-cruise) (`#67 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/67>`_)
* feat(autoware_adapi_v1_msgs): add new planning behavior "run-out" (`#65 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/65>`_)
* feat: update v1.6.0 (`#63 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/63>`_)
* feat(autoware_adapi_v1_msgs): add route following check option (`#61 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/61>`_)
* Contributors: Esteve Fernandez, Kyoichi Sugahara, Satoshi OTA, Takagi, Isamu

1.3.0 (2024-07-03)
------------------
* feat(autoware_adapi_v1_msgs): remove energy status (`#58 <https://github.com/youtalk/autoware_adapi_msgs/issues/58>`_)
* feat(autoware_adapi_v1_msgs): add diagnostics (`#54 <https://github.com/youtalk/autoware_adapi_msgs/issues/54>`_)
* Contributors: Takagi, Isamu

1.2.1 (2024-04-30)
------------------
* build: set package versions to the package.xml files (`#55 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/55>`_)
  * version 1.1.0
  * separate to build\_ and exec\_ depends
  * update description
  * style(pre-commit): autofix
  * Update autoware_adapi_v1_msgs/package.xml
  Co-authored-by: Takagi, Isamu <43976882+isamu-takagi@users.noreply.github.com>
  * Update autoware_adapi_v1_msgs/package.xml
  Co-authored-by: Takagi, Isamu <43976882+isamu-takagi@users.noreply.github.com>
  * Update autoware_adapi_version_msgs/package.xml
  Co-authored-by: Takagi, Isamu <43976882+isamu-takagi@users.noreply.github.com>
  * version 1.2.0
  * Revert "separate to build\_ and exec\_ depends"
  This reverts commit 6b657cf84a674a8272fbc6571a0f3dffa0a1dce8.
  ---------
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
  Co-authored-by: Takagi, Isamu <43976882+isamu-takagi@users.noreply.github.com>
* feat(autoware_adapi_v1_msgs): add heartbeat (`#50 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/50>`_)
  * feat(autoware_adapi_v1_msgs): add heartbeat
  * feat(autoware_adapi_v1_msgs): add message counter
  * Update autoware_adapi_v1_msgs/system/msg/Heartbeat.msg
  Co-authored-by: M. Fatih Cırıt <xmfcx@users.noreply.github.com>
  * feat: add comment
  ---------
  Co-authored-by: M. Fatih Cırıt <xmfcx@users.noreply.github.com>
* feat(autoware_adapi_v1_msgs): remove planning factor type (`#48 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/48>`_)
* feat(autoware_adapi_v1_msgs): add planning behavior type (`#45 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/45>`_)
* feat(autoware_adapi_v1_msgs): add MRM pull over behavior for mrm state (`#43 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/43>`_)
  * add pull over behavior to mrm state
  * Update autoware_adapi_v1_msgs/system/msg/MrmState.msg
  Co-authored-by: Kotaro Yoshimoto <pythagora.yoshimoto@gmail.com>
  ---------
  Co-authored-by: Kotaro Yoshimoto <pythagora.yoshimoto@gmail.com>
* feat(autoware_adapi_v1_msgs): add cooperation (`#41 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/41>`_)
  * feat: add cooperation messages
  * feat: add cooperation service
  * feat: merge planning factors
  * feat: modify constant names
  * feat: add cooperation default
  * update fields
  * upfate default decision service
  * rename service
  * update message name
  * fix constants
  * add constants for behavior and sequence
  * update field
  * fix order
  * add comment
  ---------
* feat(autoware_adapi_v1_msgs): add door command (`#40 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/40>`_)
  * door command
  * remove unused constant
  * modify command
  ---------
* feat(autoware_adapi_v1_msgs): add vehicle status msgs (`#24 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/24>`_)
  * add vehicle status msgs
  * simplify door msg
  * add vehicle state
  * change vehicle motion
  * clean up
  * fix typo
  * change geo position to array and remove GeoPosition msg
  * update msgs
  * update msgs
  * update naming
  * add none
  * update msg
  * fix naming
  * fix typo
  * change naming
  * update geopoint to use geographic_msgs
  * add commend document
  * update message
  * Update autoware_adapi_v1_msgs/vehicle/msg/Kinematic.msg
  Fix speelling
  Co-authored-by: Takagi, Isamu <43976882+isamu-takagi@users.noreply.github.com>
  * update message name
  * update door layout
  * Update autoware_adapi_v1_msgs/vehicle/msg/VehicleKinematics.msg
  Update commend in VehicleKinematics
  Co-authored-by: Ryohsuke Mitsudome <43976834+mitsudome-r@users.noreply.github.com>
  * style(pre-commit): autofix
  ---------
  Co-authored-by: Takagi, Isamu <43976882+isamu-takagi@users.noreply.github.com>
  Co-authored-by: Takagi, Isamu <isamu.takagi@tier4.jp>
  Co-authored-by: Ryohsuke Mitsudome <43976834+mitsudome-r@users.noreply.github.com>
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* refactor(start_planner): rename pull out to start planner (`#36 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/36>`_)
* feat(autoware_adapi_v1_msgs): change door field names to suit (`#35 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/35>`_)
* feat: first draft proposal implementation for handling invalid lanelets (`#27 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/27>`_)
  * feat: first draft proposal implementation for handling invalid lanelets
  * feat: changing module name from invalid_lanelet to no_drivable_lane
  ---------
* feat(autoware_adapi_v1_msgs): add object recognition msg (`#25 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/25>`_)
  * add perception message for object recognition
  * use the correct msg
  * change naming
  * fix naming
  * change shape
  * add missing data
  * change naming
  * change naming
  * change shape type
  ---------
* feat(autoware_adapi_v1_msgs): add vehicle info msgs (`#28 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/28>`_)
  * feat(autoware_adapi_v1_msgs): add vehicle info messages
  * feat: add door group
  * feat: change door group
  * feat: add footprint to vehicle dimensions
  ---------
* refactor(behavior_path_planner): rename pull_over to goal_planner (`#33 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/33>`_)
  * refactor(behavior_path_planner): rename pull_over to goal_planner
  * Update autoware_adapi_v1_msgs/planning/msg/SteeringFactor.msg
  Co-authored-by: Takagi, Isamu <43976882+isamu-takagi@users.noreply.github.com>
  ---------
  Co-authored-by: Takagi, Isamu <43976882+isamu-takagi@users.noreply.github.com>
* feat(autoware_adapi_v1_msgs): add error code for route points (`#32 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/32>`_)
* feat(autoware_adapi_v1_msgs): add new error messages (`#31 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/31>`_)
  * feat(autoware_adapi_v1_msgs): add new error messages
  * Update autoware_adapi_v1_msgs/routing/srv/SetRoute.srv
  Co-authored-by: Takagi, Isamu <43976882+isamu-takagi@users.noreply.github.com>
  ---------
  Co-authored-by: Takagi, Isamu <43976882+isamu-takagi@users.noreply.github.com>
* feat(autoware_adapi_v1_msgs): add routing option for goal modification (`#30 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/30>`_)
  * feat(autoware_adapi_v1_msgs): add goal modification
  * feat: change goal modification option
  ---------
* feat(autoware_adapi_v1_msgs): add msgs for MRM (`#16 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/16>`_)
  * feat(autoware_adapi_v1_msgs): add mrm msgs
  * fix(autoware_adapi_v1_msgs): remove mrm behavior status msg
  * fix(autoware_adapi_v1_msgs): fix acronyms case
  Co-authored-by: Kenji Miyake <31987104+kenji-miyake@users.noreply.github.com>
  * chore(autoware_adapi_v1_msgs): add comments for states and behaviors
  Co-authored-by: Kenji Miyake <31987104+kenji-miyake@users.noreply.github.com>
  * ci(pre-commit): autofix
  * feat(autoware_adapi_v1_msgs): add operation mode api message (`#17 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/17>`_)
  * feat(autoware_adapi_v1_msgs): add operation mode api message
  * feat: use minimum size type
  * fix(autoware_adapi_v1_msgs): fix file names
  * fix(autoware_adapi_v1_msgs): fix constant values
  Co-authored-by: Takagi, Isamu <43976882+isamu-takagi@users.noreply.github.com>
  * remove(autoware_adapi_v1_msgs): remove mrm operation service
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
  Co-authored-by: Takagi, Isamu <43976882+isamu-takagi@users.noreply.github.com>
* feat(autoware_adapi_v1_msgs): add operation mode api message (`#17 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/17>`_)
  * feat(autoware_adapi_v1_msgs): add operation mode api message
  * feat: use minimum size type
* feat(autoware_adapi_v1_msgs): add velocity factor message (`#15 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/15>`_)
* feat(autoware_adapi_v1_msgs): add error code to route set service (`#14 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/14>`_)
* feat(autoware_adapi_v1_msgs): add motion api message (`#11 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/11>`_)
  feat(autoware_adapi_v1_msgs): add motion messages
* feat(autoware_adapi_v1_msgs): add planning api message (`#9 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/9>`_)
  * add planning api msgs
  * add cmake
  * fix naming
* feat(autoware_adapi_v1_msgs): add v1 messages (`#3 <https://github.com/autowarefoundation/autoware_adapi_msgs/issues/3>`_)
  Co-authored-by: Ryohsuke Mitsudome <43976834+mitsudome-r@users.noreply.github.com>
* Contributors: Ahmed Ebrahim, Akihiro Sakurai, Kah Hooi Tan, Kosuke Takeuchi, Makoto Kurihara, Takagi, Isamu, Yutaka Kondo, Yutaka Shimizu
