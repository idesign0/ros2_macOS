^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package apriltag_detector
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

3.1.0 (2025-12-03)
------------------
* fix install destinations for libraries
* Contributors: Bernd Pfrommer

3.0.4 (2025-09-01)
------------------
* support new image transport node interface
* add isSubscribed() to detector interface and fixed bug in detect_from_bag
* added getNumTagsDetected()
* Contributors: Bernd Pfrommer

3.0.3 (2025-07-29)
------------------
* avoid deprecated image transport functions
* Fix compilation and loading of shared libraries on Windows
  * Fix compilation and running on Windows
  ---------
  Co-authored-by: Bernd Pfrommer <bernd.pfrommer@gmail.com>
* Contributors: Bernd Pfrommer, Silvio Traversaro

3.0.2 (2025-05-24)
------------------
* no longer use ament_target_dependencies
* Contributors: Bernd Pfrommer

3.0.1 (2025-02-28)
------------------

3.0.0 (2025-02-28)
------------------
* clean up apriltag_detector packages. Detector component is now in apriltag_detector.
* Contributors: Bernd Pfrommer

2.0.1 (2024-09-26)
------------------

2.0.0 (2024-05-11)
------------------
* initial release as base class of pluggable detectors
* Contributors: Bernd Pfrommer
