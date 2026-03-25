^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package autoware_map_msgs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
* feat(autoware_msg_msgs): add scale factor remove altitude (`#121 <https://github.com/autowarefoundation/autoware_msgs/issues/121>`_)
  * add_scale_factor_remove_altitude
  * add newline
  ---------
* Contributors: Yamato Ando

1.4.0 (2025-02-25)
------------------
* feat(MapProjectorInfo.msg): add LocalCartesian const (`#118 <https://github.com/autowarefoundation/autoware_msgs/issues/118>`_)
* fix(autoware_msgs): fix links to issues in CHANGELOG.rst files (`#108 <https://github.com/autowarefoundation/autoware_msgs/issues/108>`_)
* Contributors: Esteve Fernandez, Sebastian Zęderowski

1.3.0 (2024-11-25)
------------------
* feat(autoware_map_msgs): add MapProjectorInfo message (`#102 <https://github.com/autowarefoundation/autoware_msgs/issues/102>`_)
* Contributors: Ryohsuke Mitsudome

1.2.0 (2024-10-01)
------------------
* feat(autoware_map_msgs): add msg and srv files releated with dynamic lanelet loading (`#81 <https://github.com/autowarefoundation/autoware_msgs/issues/81>`_)
* Contributors: Barış Zeren, Ryohsuke Mitsudome, Yamato Ando

1.1.0 (2024-05-10)
------------------
* chore: update `package.xml` and fix `CMakeLists.txt` (`#91 <https://github.com/autowarefoundation/autoware_msgs/issues/91>`_)
  update package.xml and fix cmakefiles
* feat(autoware_map_msgs): support cylindrical AreaInfo (`#64 <https://github.com/autowarefoundation/autoware_msgs/issues/64>`_)
  * feat(autoware_map_msgs): support cylindrical AreaInfo
  * update attribute
  ---------
* feat(autoware_map_msgs): add selected map loading (`#57 <https://github.com/autowarefoundation/autoware_msgs/issues/57>`_)
  * feat(map_loader): add support for sequential_map_loading
  * feat(autoware_map_msgs): add PointCloudMetaData.msg
  * docs(autoware_map_msgs): add description of selected_map_loading
  * docs(autoware_map_msgs): remove gif for selected_map_loading
  * docs(autoware_map_msgs): fix typo
  * feat(autoware_map_msgs): make member of msg plural
  * docs(autoware_map_msgs): clarify the client needs to receive msg beforehand
  * docs(autoware_map_msgs): clarify IDs included in msgs are used as query for service
  ---------
* feat(autoware_map_msgs): add grid coordinates in PointCloudMapCellWithID.msg (`#52 <https://github.com/autowarefoundation/autoware_msgs/issues/52>`_)
  * feat(autoware_map_msgs): add grid coordinates in PointCloudMapCellWithID.msg
  * style(pre-commit): autofix
  * debug
  ---------
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* feat(map-messages): add LaneletMapBin.msg (`#30 <https://github.com/autowarefoundation/autoware_msgs/issues/30>`_)
* feat: add autoware_map_msgs for dynamic map loading (`#39 <https://github.com/autowarefoundation/autoware_msgs/issues/39>`_)
  Co-authored-by: M. Fatih Cırıt <xmfcx@users.noreply.github.com>
  Co-authored-by: Kenji Miyake <31987104+kenji-miyake@users.noreply.github.com>
  Co-authored-by: Takagi, Isamu <43976882+isamu-takagi@users.noreply.github.com>
* Contributors: M. Fatih Cırıt, Shintaro Tomie, Yutaka Kondo, kminoda
