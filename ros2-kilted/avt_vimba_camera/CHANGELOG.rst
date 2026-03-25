^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package avt_vimba_camera
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

2001.1.0 (2021-12-10)
---------------------
* Port to ROS2 (`#56 <https://github.com/astuff/avt_vimba_camera/issues/56>`_)
* Update README for ROS2 (`#58 <https://github.com/astuff/avt_vimba_camera/issues/58>`_)
* Contributors: icolwell-as

1.0.0 (2021-11-30)
------------------
* Code Cleanup and Autoformat (`#52 <https://github.com/astuff/avt_vimba_camera/issues/52>`_)
* Improve rosout logging (`#50 <https://github.com/astuff/avt_vimba_camera/issues/50>`_)
* Improve README (`#48 <https://github.com/astuff/avt_vimba_camera/issues/48>`_)
* Parameter Refactor (`#46 <https://github.com/astuff/avt_vimba_camera/issues/46>`_)
* Use timestamp from image capture in ROS header (`#41 <https://github.com/astuff/avt_vimba_camera/issues/41>`_)
* Trigger over Ethernet (`#39 <https://github.com/astuff/avt_vimba_camera/issues/39>`_)
* Remove stereo camera code (`#43 <https://github.com/astuff/avt_vimba_camera/issues/43>`_)
* Update CI (`#44 <https://github.com/astuff/avt_vimba_camera/issues/44>`_)
* Update to VimbaCPP Version 1.8.4 (SDK version 5.0) (`#36 <https://github.com/astuff/avt_vimba_camera/issues/36>`_)
* Fixed 'escalating to SIGTERM' when closing (`#22 <https://github.com/astuff/avt_vimba_camera/issues/22>`_)
* Contributors: icolwell-as, vbrebion

0.0.12 (2020-06-05)
-------------------
* Fix MonoCamera nodelet (`#16 <https://github.com/astuff/avt_vimba_camera/issues/16>`_)
* Launch File Updates (`#13 <https://github.com/astuff/avt_vimba_camera/issues/13>`_)
* Remove duplicated launch file installs (`#15 <https://github.com/astuff/avt_vimba_camera/issues/15>`_)
* Brief README (`#10 <https://github.com/astuff/avt_vimba_camera/issues/10>`_)
* Add more pixelformats, thanks to @jmoreau-hds (`#3 <https://github.com/astuff/avt_vimba_camera/issues/3>`_)
* Contributors: icolwell-as

0.0.11 (2019-08-19)
-------------------
* Merge pull request `#35 <https://github.com/astuff/avt_vimba_camera/issues/35>`_ from willcbaker/feature/nodelet
  Feature/nodelet
* Update nodelet launch files
* Add nodelet
* Fix `#31 <https://github.com/astuff/avt_vimba_camera/issues/31>`_
* Merge pull request `#30 <https://github.com/astuff/avt_vimba_camera/issues/30>`_ from chewwt/armv8
  add support for armv8
* add support for armv8
* Change frame observer to shared ptr
* Contributors: Miquel Massot, Will Baker, ruth

0.0.10 (2017-08-16)
-------------------
* Merge pull request `#26 <https://github.com/srv/avt_vimba_camera/issues/26>`_ from 130s/k/add_ci
  [CI] Add config for ROS Kinetic (and Lunar as an option).
* [CI] Add config for ROS Kinetic (and Lunar as an option).
* Merge pull request `#25 <https://github.com/srv/avt_vimba_camera/issues/25>`_ from mintar/fix_open_camera
  Fix opening the camera
* Simplify openCamera() logic
* Open camera on init
  Without this patch, the camera is never opened. This bug was introduced in 63f868791.
* Give more information to the user
* Merge pull request `#22 <https://github.com/srv/avt_vimba_camera/issues/22>`_ from plusone-robotics/dyn_reconfig_fix
  Dynamic reconfigure fix
* Fix `#21 <https://github.com/srv/avt_vimba_camera/issues/21>`_: Retry camera opening and handle SIGINT
* Changed file mode in order to build
* Fixed crashing of dynamic reconfigure
* Fixed dynamic reconfigure configuration that did not allow camera parameters to be updated
* Fix call QueueFrame() method
* Fix CPU overhead issue
* Fix `#18 <https://github.com/srv/avt_vimba_camera/issues/18>`_
* Merge pull request `#17 <https://github.com/srv/avt_vimba_camera/issues/17>`_ from josepqp/kinetic
  Added ARM 32 to CMakeList.txt
* Stop camera before destroy it
* Added ARM 32
* Merge branch 'kinetic' of github.com:srv/avt_vimba_camera into kinetic
* Change variable scope
* Fix `#15 <https://github.com/srv/avt_vimba_camera/issues/15>`_: do not depend on turbot_configurations
* Merge pull request `#14 <https://github.com/srv/avt_vimba_camera/issues/14>`_ from josepqp/kinetic

  * Added ARM 32 bits libraries
  * Modified CMakeList.txt to compile with ARM 32 bits
  * Added Iris Parameter

* kinetization
* Fix sync problems after camera tests
* Add a sync timer
* Try stereo image sync
* Add a check timer
* Fix `#12 <https://github.com/srv/avt_vimba_camera/issues/12>`_: allow bigger resolutions
* Fix camera info
* Fix camera config
* Fix camera info when decimation
* Make sync node acts as stereo sync checker
* Include a check timer on stereo_camera
* Perform the stereo_sync in a separate node
* Publish camera temperatures
* Change the way of reset
* Increase the initial wait time before checking sync
* Add a sync watcher node
* Fix branch mix
* Remove unused variables
* Left and right callback in a separate thread
* Change default sync time
* change logging messages
* fix binning
* add stereo launchfiles
* removed prints
* set stereo launchfiles
* removed unused params
* calibration epi. 4
* improvements to stereo node
* merge with v2.0 SDK
* upgrade to VIMBA SDK 2.0
* upgrade to 1.4
* changed ros prints from info to debug
* removed comment
* changed stereo camera launchfile
* Merge pull request `#11 <https://github.com/srv/avt_vimba_camera/issues/11>`_ from lucasb-eyer/indigo
  Set the frame_id of the image header, too.
* Set the frame_id of the image header, too.
* Contributors: Isaac I.Y. Saito, Martin Günther, Miquel Massot, Shaun Edwards, SparusII, agoins, josep, lucasb-eyer, plnegre, shaun-edwards

0.0.9 (2014-11-17)
------------------
* Fix `#8 <https://github.com/srv/avt_vimba_camera/issues/8>`_: Constructor delegation and typo in assignment
* added mono camera name
* corrected diagnostics
* fixed sync diagnostic
* improved diagnostics
* better timestamp management
* added command error check
* cleaning stereo prints
* removed old cpp
* fixed merging conflict
* update updater
* added time to tick function
* added getTimestamp
* added reset timestamp command
* changed errors to warnings
* added open/close msgs to diagnostics
* added diagnostics. wip
* bugfixes
* full operative stereo camera
* prepared launchfile for stereo
* auto set packet size
* stereo sync
* preparing for stereo
* added launchfile
* hide first run
* set auto configuration by default
* fix with ptp mode
* Fix dynamic reconfigure error with PTP
* mono camera compiles
* Fix interface type
* Merge pull request `#5 <https://github.com/srv/avt_vimba_camera/issues/5>`_ from lucasb-eyer/auto
  Fix names/values of auto settings.
* Fix names/values of auto settings.
* Fix `#2 <https://github.com/srv/avt_vimba_camera/issues/2>`_: Set the highest GeV packet size
* Merge pull request `#3 <https://github.com/srv/avt_vimba_camera/issues/3>`_ from pkok/single_identifier
  Allow user to connect by specifying either GUID or IP address.
* Allow user to connect by specifying either GUID or IP address.
* wip
* added testing launchfiles
* added parameters for sync
* Contributors: Miquel Massot, Patrick de Kok, SPENCER-Freiburg Laptop

0.0.8 (2014-09-05)
------------------
* readdition of vimba
* Contributors: Miquel Massot

0.0.7 (2014-09-04)
------------------
* removed vimba headers
* Contributors: Miquel Massot

0.0.6 (2014-09-03)
------------------
* change to libvimba package
* Contributors: Miquel Massot

0.0.5 (2014-09-03)
------------------
* add shared library as imported target
* Contributors: Miquel Massot

0.0.4 (2014-09-01)
------------------
* absolute path for libvimbacpp
* changed version
* bugfix re-release
* Contributors: Miquel Massot

0.0.2 (2014-03-24)
------------------
* test on polled camera
* formatting
* added packages
* added GPIO params
* added params and launchfile
* added launchfile
* added camera calibration and fixed reconfiguration issues
* first images in ROS
* first tests with Manta G-504C
* added tags to gitignore
* develop in progress
* added gitignore
* changed package name and pushed some devel
* added config file
* prepared and tested Vimba library
* first commit
* Contributors: Miquel Massot
