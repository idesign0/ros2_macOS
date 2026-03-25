^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package autoware_vehicle_msgs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1.11.0 (2025-10-23)
-------------------

1.10.0 (2025-07-18)
-------------------
* chore: update maintainer for autoware_msgs packages (`#143 <https://github.com/autowarefoundation/autoware_msgs/issues/143>`_)
* Contributors: Ryohsuke Mitsudome

1.9.0 (2025-06-18)
------------------

1.8.0 (2025-05-21)
------------------
* chore: fix email domain (`#133 <https://github.com/autowarefoundation/autoware_msgs/issues/133>`_)
* chore: update maintainers (`#132 <https://github.com/autowarefoundation/autoware_msgs/issues/132>`_)
* Contributors: Mete Fatih Cırıt

1.7.0 (2025-04-21)
------------------

1.6.0 (2025-04-07)
------------------

1.5.0 (2025-04-02)
------------------

1.4.0 (2025-02-25)
------------------
* fix(autoware_msgs): fix links to issues in CHANGELOG.rst files (`#108 <https://github.com/autowarefoundation/autoware_msgs/issues/108>`_)
* Contributors: Esteve Fernandez

1.3.0 (2024-11-25)
------------------

1.2.0 (2024-10-01)
------------------

1.1.0 (2024-05-10)
------------------
* chore: update `package.xml` and fix `CMakeLists.txt` (`#91 <https://github.com/autowarefoundation/autoware_msgs/issues/91>`_)
  update package.xml and fix cmakefiles
* feat: update msg definitions (`#87 <https://github.com/autowarefoundation/autoware_msgs/issues/87>`_)
  * feat(autoware_perception_msgs): Add DetectedObjects messages and TrackedObjects messages
  * feat!(autoware_planning_msgs): remove PathWithLaneId msgs
  * feat!(autoware_perception_msgs): rename traffic signals to traffic lights
  * remove control mode command msg
  * fix(autoware_vehicle_msgs): remove ControlModeCommands.msg in CMakeLists.txt (`#90 <https://github.com/autowarefoundation/autoware_msgs/issues/90>`_)
  * feat(autoware_vehicle_msgs)!: remove VehicleControlCommand, VehicleKinematicState, and VehicleOdometry messages (`#88 <https://github.com/autowarefoundation/autoware_msgs/issues/88>`_)
  feat!(autoware_vehicle_msgs): remove VehicleControlCommand, VehicleKinematicState, and VehicleOdometry messages
  * Revert "feat!(autoware_perception_msgs): rename traffic signals to traffic lights"
  This reverts commit 768bb854036fc94db4364168e68d48b21401c17b.
  * add TrafficLight msgs
  * add todo
  ---------
  Co-authored-by: Ryohsuke Mitsudome <ryohsuke.mitsudome@tier4.jp>
  Co-authored-by: Ryohsuke Mitsudome <43976834+mitsudome-r@users.noreply.github.com>
  Co-authored-by: Yutaka Kondo <yutaka.kondo@youtalk.jp>
* fix(autoware_vehicle_msgs): add missed vehicle msgs (`#80 <https://github.com/autowarefoundation/autoware_msgs/issues/80>`_)
  * fix(autoware_vehicle_msgs): add missed vehicle msgs
  * style(pre-commit): autofix
  ---------
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* feat(autoware_vehicle_msgs): add autoware_vehicle_msgs (`#78 <https://github.com/autowarefoundation/autoware_msgs/issues/78>`_)
  * feat(autoware_vehicle_msgs): add autoware_vehicle_msgs
  * feat(autoware_vehicle_msgs): add ControlModeCommand.srv
  * style(pre-commit): autofix
  * fix(autoware_vehicle_msgs): remove msgs unused
  * style(pre-commit): autofix
  ---------
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* Contributors: Yukihiro Saito, Yutaka Kondo, beginningfan
