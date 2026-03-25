^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package camera_aravis2
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1.2.0 (2026-03-09)
------------------
* other: add miguel as maintainer
* feat: enable manual param order in BEGIN and END
  Use the prefix #NN\_ to preserve the order. For example:
  `#1 <https://github.com/FraunhoferIOSB/camera_aravis2/issues/1>`__FirstParam
  `#2 <https://github.com/FraunhoferIOSB/camera_aravis2/issues/2>`__SecondParam
  OtherParamsThatAreAlphabeticallyOrdered
* Contributors: Miguel Granero

1.1.0 (2024-12-12)
------------------
* Update maintainer -> Raphael Hagmanns
* Enhance: Improved assertions and error messages
* Fix: added failure exit code to nodes
* Enhance: changed message of acquisition start/stop to info-level
* Fix: calculation of ROS topic subscriber count -> fix start/stop acquisition
* Fix: added epsilon in the comparison between double values
* Fix: handling of missing 'file://' in camera_info_url
* Fix: Adjusted step of integer and fp range
* Feat: Added support to access GigEVision cameras via IP address.
* Feat: Support for USB3 Cameras.
* Feat: Added functionally do specify parameters that are to be made dynamically changeable.
* Added service to manually trigger computation of white balance.
* Minor changes and bug fixes
* Contributors: Boitumelo Ruf, Louis-Romain JOLY, Ralph Ursprung

1.0.0 (2024-07-15)
------------------
* Implemented GenICam compliant 'camera_driver_gv' for ROS2
	* Allowing to access a GenICam compliant via its serial number and publishing an image stream
	* Supporting multiple substreams
	* Allowing to flexibly control GenICam-Specific parameters, such as TransportLayerControl, ImageFormatControl, AcquisitionControl, AnalogControl
	* Support for PTP timestamps.
	* Support to publish diagnostic messages for camera.
* Implemented 'camera_finder' to list all available GenICam devices.
* Implemented 'camera_xml_exporter' to export camera-specific GenICam support into XML.
* CI/CD support
* Contributors: Boitumelo Ruf
