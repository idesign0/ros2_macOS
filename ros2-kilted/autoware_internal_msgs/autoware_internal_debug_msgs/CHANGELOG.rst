^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package autoware_internal_debug_msgs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
* Contributors: Yutaka Kondo

1.12.0 (2025-07-25)
-------------------

1.11.0 (2025-07-23)
-------------------

1.10.0 (2025-06-03)
-------------------
* feat(autoware_internal_debug_msgs): add sericeLog msg, initial commit: v0.0 (`#67 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/67>`_)
* Contributors: 心刚

1.9.0 (2025-05-23)
------------------

1.8.1 (2025-04-07)
------------------

1.8.0 (2025-03-27)
------------------

1.7.0 (2025-03-06)
------------------

1.6.0 (2025-02-19)
------------------

1.5.0 (2025-01-23)
------------------

1.4.0 (2025-01-15)
------------------
* feat(autoware_internal_debug_msgs): add ProcessingTime msgs (`#41 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/41>`_)
* Contributors: cyn-liu

1.3.0 (2024-12-26)
------------------
* feat: add String.srv (`#37 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/37>`_)
* chore: sync files (`#25 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/25>`_)
  * chore: sync files
  * style(pre-commit): autofix
  ---------
  Co-authored-by: github-actions <github-actions@github.com>
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* Contributors: Takayuki Murooka, awf-autoware-bot[bot]

1.2.0 (2024-12-19)
------------------
* feat(autoware_internal_debug_msgs): move stamped messages (`#31 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/31>`_)
* feat: add autoware_internal_debug_msgs package (`#30 <https://github.com/autowarefoundation/autoware_internal_msgs/issues/30>`_)
* Contributors: Takayuki Murooka
