# camera_aravis2

An actively maintained ROS 2 camera driver for [GenICam](https://www.emva.org/standards-technology/genicam/)-based (GigEVision and USB3Vision) cameras. 
It is a subsequent development of [camera_aravis](https://github.com/FraunhoferIOSB/camera_aravis), but in order to clean up some legacy code and, in turn, support new features mor easily, we opted to implement it with a new code-base. 
It is open sourced under the 3-clause BSD license.

It relies on the [Aravis](http://live.gnome.org/Aravis) library to access the GigEVision and USB3Vision cameras. 
Aravis is a glib/gobject based library for video acquisition using GenICam cameras. 
It currently implements the gigabit ethernet and USB3 protocols used by industrial cameras.

**Acknowledgement**: This software was developed as part of the project [ROBDEKON – Robotic Systems for Decontamination in Hazardous Environments](https://robdekon.de/), funded by the Federal Ministry of Education and Research (BMBF) under the German Federal Government’s Research for Civil Security program.

*camera_aravis for ROS 1 can be found here: https://github.com/FraunhoferIOSB/camera_aravis*

------------------------

### Continuous Integration:

| Service    | devel  | main |
| ---------- | ------- | ------ |
| GitHub     | [![Humble Hawksbill](https://github.com/FraunhoferIOSB/camera_aravis2/actions/workflows/build_and_test_humble.yml/badge.svg?branch=devel)](https://github.com/FraunhoferIOSB/camera_aravis2/actions/workflows/build_and_test_humble.yml)<br>[![Iron Irwini](https://github.com/FraunhoferIOSB/camera_aravis2/actions/workflows/build_and_test_iron.yml/badge.svg?branch=devel)](https://github.com/FraunhoferIOSB/camera_aravis2/actions/workflows/build_and_test_iron.yml)<br>[![Jazzy Jalisco](https://github.com/FraunhoferIOSB/camera_aravis2/actions/workflows/build_and_test_jazzy.yml/badge.svg?branch=devel)](https://github.com/FraunhoferIOSB/camera_aravis2/actions/workflows/build_and_test_jazzy.yml)| [![Humble Hawksbill](https://github.com/FraunhoferIOSB/camera_aravis2/actions/workflows/build_and_test_humble.yml/badge.svg?branch=main)](https://github.com/FraunhoferIOSB/camera_aravis2/actions/workflows/build_and_test_humble.yml)<br>[![Iron Irwini](https://github.com/FraunhoferIOSB/camera_aravis2/actions/workflows/build_and_test_iron.yml/badge.svg?branch=main)](https://github.com/FraunhoferIOSB/camera_aravis2/actions/workflows/build_and_test_iron.yml)<br>[![Jazzy Jalisco](https://github.com/FraunhoferIOSB/camera_aravis2/actions/workflows/build_and_test_jazzy.yml/badge.svg?branch=main)](https://github.com/FraunhoferIOSB/camera_aravis2/actions/workflows/build_and_test_jazzy.yml)|

------------------------

### Contents:

- [Running the Camera Driver](#running-the-camera-driver)
    - [Configuration](#configuration)
    - [Example Launch Files](#example-launch-files)
- [Finding Available Cameras](#finding-available-cameras)
- [Extracting Camera-Specific GenICam XML](#extracting-camera-specific-genicam-xml)
- [Building from Source](#building-from-source)
    - [Requirements](#requirements)
    - [Build](#build)
    - [Test](#test)
- [FAQ](#faq)
    - [How to use PTP](#how-to-use-ptp)
    - [How to set specific analog control values (e.g. balance ratios)](#how-to-set-specific-analog-control-values-eg-balance-ratios)
    - [How to manually trigger calculation of white balance ratios](#how-to-manually-trigger-calculation-of-white-balance-ratios)
    - [How to dynamically change camera parameters](#how-to-dynamically-change-camera-parameters)
    - [How to publish camera diagnostics / status](#how-to-publish-camera-diagnostics--status)
    - [How to access GigEVision camera using the IP address](#how-to-access-gigevision-camera-using-the-ip-address)
- [Known Issues](#known-issues)
- [Contributing](CONTRIBUTING.md)

------------------------

## Running the Camera Driver

In camera_aravis2 the actual camera driver is implemented in two nodes:
- `camera_driver_gv` for GigEVision cameras, and
- `camera_driver_uv`, for USB3Vision cameras.

### Configuration

The configuration of the camera driver is divided into a driver-specific and GenICam-specific parameterization as described below.

#### Driver-specific Parameters

- `guid`: Serial number or IP address *(GigEVision Cameras only)* of the camera that is to be opened. GUIDs and IP addresses of available cameras can be discovered with [`camera_finder`](#finding-available-cameras) node.
    - Type: String
    - Default: ""
    - Optional. If omitted a random camera is picked from the list of connected cameras and will be opened.
- `stream_names`: String list of names that are to be associated with each stream. If multiple streams are available, these names will be appended to the topic names in order to distinguish the different image streams.
    - Type: String-Array
    - Default: {}
    - Optional. If omitted or less names are given than streams available, each stream will get given a name based on its ID, starting with 0.
- `camera_info_urls`: String list of urls to camera_info files associated with each stream. List should have the same length as the number of streams provided by the camera. If the number of URLs does not correspond to number of streams available, the minimum of both is used to set the number of streams that are to be established.
    - Type: String-Array
    - Default: {}
    - Optional. If omitted, it is constructed from the camera GUID located within the current working directory, with the stream name separated by '_' appended to the file name, if more than one streams are instantiated.
- `frame_id`: Frame ID that is to be associated with the sensor and, in turn,  with the image data. If multiple streams are supported by the camera, the given ID serves as a base string to which the stream name is appended, together with '_' as separator. 
    - Type: String
    - Default: ""
    - Optional. If no frame ID is specified, the name of the node will be used.
- ```dynamic_parameters_yaml_url```: URL to yaml file specifying camera parameters that are to be made dynamically changeable.  See '[How to dynamically change camera parameters](#how-to-dynamically-change-camera-parameters)' for more info.
	- Type: String
	- Default: ""
    - Optional. If left empty no dynamic parameters, apart from the camera_aravis-specific parameters will be available
- ```diagnostic_yaml_url```: URL to yaml file specifying the camera features which are to be 
monitored. See '[How to publish camera diagnostics / status](#how-to-publish-camera-diagnostics--status)' for more info.
	- Type: String
	- Default: ""
    - Optional. If left empty no diagnostic features will be read and published.
- ```diagnostic_publish_rate```: Rate (in Hz) at which to read and publish the diagnostic data.
	- Type: double
	- Default: 0.1 (every 10 seconds)

#### GenICam-Specific Parameters

To configure the camera, camera_aravis2 relies on the GenICam Standard Feature Naming Convention (SNFC) which can be found [here](https://www.emva.org/standards-technology/genicam/genicam-downloads/). 
The SNFC groups the individual features into numerous catagories (e.g. Image Format Control, Acquisition Control, Analog Control, ...).
Camera_aravis2 explicitly looks for a number of features in a couple of categories to be specified and tries to set the features accordingly. 
If specified as launch parameter, camera_aravis2 will set the following features in the same order as listed below:

- `DeviceControl` (List): GenICam parameters of the 'DeviceControl' category that are to be set. Here, no specific order is implemented. Nested parameter list is evaluated in alphabetical order.
- `TransportLayerControl`
    - `BEGIN` (List): Additional GenICam parameters that are to be set at the beginning of the 'TransportLayerControl' section. Nested parameter list is evaluated in alphabetical order.
    - `GevSCPSPacketSize` (Int) - *GigEVision Cameras only* : Specifies the packet size, in bytes, which are to be send. This should correspond 'DeviceStreamChannelPacketSize' and the maximum transport unit (MTU) of the interface.
    - `GevSCPD` (Int) - *GigEVision Cameras only* : Controls the delay (in GEV timestamp counter unit) to insert between
each packet for this stream channel.
    - `PtpEnable` (Bool) - *GigEVision Cameras only* : Enables the Precision Time Protocol (PTP).
    - `END` (List): Additional GenICam parameters that are to be set at the end of the 'TransportLayerControl' section. Nested parameter list is evaluated in alphabetical order.
- `ImageFormatControl`
    - `BEGIN` (List): Additional GenICam parameters that are to be set at the beginning of the 'ImageFormatControl' section. Nested parameter list is evaluated in alphabetical order.
    - `PixelFormat`* (String): Format of the pixels provided by the device.
    - `ReverseX`* (Bool): Flip horizontally the image sent by the device.
    - `ReverseY`* (Bool): Flip vertically the image sent by the device.
    - `BinningHorizontal`* (Int): Number of pixels to horizontally combine together.
    - `BinningHorizontalMode`* (String): Mode to use when BinningHorizontal is used. Values possible: 'Sum', or 'Average'.
    - `BinningVertical`* (Int): Number of pixels to horizontally combine together.
    - `BinningVerticalMode`* (String): Mode to use when BinningHorizontal is used. Values possible: 'Sum', or 'Average'.
    - `Width`* (Int): Width of the image provided by the device (in pixels).
    - `Height`* (Int): Height of the image provided by the device (in pixels).
    - `OffsetX`* (Int): Horizontal offset from the origin to the region of interest (in pixels).
    - `OffsetY`* (Int): Vertical offset from the origin to the region of interest (in pixels).
    - `END` (List): Additional GenICam parameters that are to be set at the end of the 'ImageFormatControl' section. Nested parameter list is evaluated in alphabetical order.
- `AcquisitionControl`
    - `BEGIN` (List): Additional GenICam parameters that are to be set at the beginning of the 'AcquisitionControl' section. Nested parameter list is evaluated in alphabetical order.
    - `AcquisitionMode`* (String): Sets the acquisition mode of the device. Values possible: 'SingleFrame', 'MultiFrame', or 'Continuous'.
    - `AcquisitionFrameCount`* (Int): Number of frames to acquire in MultiFrame Acquisition mode. Only evaluated if 'AcquisitionMode' is 'MultiFrame'.
    - `ExposureMode`* (String): Sets the operation mode of the Exposure. Values possible: 'Off', 'Timed', 'TriggerWidth', or 'TriggerControlled'.
    - `ExposureAuto`* (String): Sets the automatic exposure mode when ExposureMode is Timed. Values possible: 'Off', 'Once', or 'Continuous'.
    - `ExposureTime`* (Double): Sets the Exposure time when 'ExposureMode' is 'Timed' and 'ExposureAuto' is 'Off'.
    - `AcquisitionFrameRateEnable`* (Bool): Controls if the AcquisitionFrameRate feature is writable and used to control the acquisition rate.
    - `AcquisitionFrameRate`* (Double): Controls the acquisition rate (in Hertz) at which the frames are captured.
    - `END` (List): Additional GenICam parameters that are to be set at the end of the 'AcquisitionControl' section. Nested parameter list is evaluated in alphabetical order.
- `AnalogControl`
    - `BEGIN` (List): Additional GenICam parameters that are to be set at the beginning of the 'AcquisitionControl' section. Nested parameter list is evaluated in alphabetical order.
    - `GainAuto`* (String): Sets the automatic gain control mode. Values possible: 'Off', 'Once', or 'Continuous'.
    - `Gain`* (List): Key-Value pairs of type string (key) and double (value) to set specific gain values in case 'GainAuto' is set to 'Off'. In this, the key will be set as value to the 'GainSelector' and the value will be set to the 'Gain' feature. The list is evaluated in alphabetical order.
    - `BlackLevelAuto`* (String): Controls the mode for automatic black level adjustment. Values possible: 'Off', 'Once', or 'Continuous'.
    - `BlackLevel`* (List): Key-Value pairs of type string (key) and double (value) to set specific black level values in case 'BlackLevelAuto' is set to 'Off'. In this, the key will be set as value to the 'BlackLevelSelector' and the value will be set to the 'BlackLevel' feature. The list is evaluated in alphabetical order.
    - `BalanceWhiteAuto`* (String): Controls the mode for automatic black level adjustment. Values possible: 'Off', 'Once', or 'Continuous'.
    - `BalanceRatio`* (List): Key-Value pairs of type string (key) and double (value) to set specific balance ratio values in case 'BalanceWhiteAuto' is set to 'Off'. In this, the key will be set as value to the 'BalanceRatioSelector' and the value will be set to the 'BalanceRatio' feature. The list is evaluated in alphabetical order.
    - `END` (List): Additional GenICam parameters that are to be set at the end of the 'AcquisitionControl' section. Nested parameter list is evaluated in alphabetical order.

In the sections where a certain order of predefined parameters is considered and implemented, the user can specify a list of additional GenICam parameters nested underneath the parameter `BEGIN` and `END`, respectively, which are not explicitly evaluated as part of the list above. 
This allows for the user to specify parameters which are not known to camera_aravis2. 
Similar holds for the parameters within the 'DeviceControl' category.

**IMPORTANT**: It is to be noted, however, that the nested parameters are evaluated in alphabetical order, which might lead to unintended behavior, depending on the camera device.

To force a manual order of some parameters use the prefix `#NN_` in the parameter name. That prefix will be removed but the alphabetical order of the params will be preserved.
Image you have a parameter that needs to be preceded by another selector parameter, with these prefixes now you can use this features. See below for an example.

The example values that are given in case of string parameters are given in accordance with the GenICam SNFC.
The possible values might differ according to the actual implementation by the camera manufacturer.

Parameters marked with * are evaluated and set per stream/channel. 
Meaning, that for a multi-channel or multi-source camera one can either specify a single parameter or a list of parameters (see 'PixelFormat' in the example below).
In case a single parameter is specified, the corresponding value is set for all channels.
If a list of parameters is given, for each channel the value with the corresponding channel index will be set.
If the list is smaller than the available channels, the value of the last entry will be set for the remaining channels.
Please note, that the flexibility of setting different parameters values for the individual channels depends on the actual implementation by the camera manufacturer.

When launching the camera driver, the features are to be configured as nested launch parameters.
While the parameters are evaluated by camera_aravis2 in the specific order which is listed above, they can be specified in an arbitrary order in the launch file.
<details>
<summary>For example ...</summary>

```Python
    ...
    parameters=[{
                    # Driver-specific parameters
                    ...
                    
                    # GenICam-specific parameters
                    "DeviceControl": {
                        "DeviceLinkThroughputLimit": 125000000,
                        "DeviceLinkThroughputLimitMode": "On"
                    },
                    "TransportLayerControl": {
                        "GevSCPSPacketSize": 9000,
                        "PtpEnable": True
                    },
                    "ImageFormatControl": {
                        "BEGIN": {
                            "BinningSelector": "Digital",
                            "#2_ValueParamGoesSecond": 3.1416,
                            "#1_SelectorParamGoesFirst": "pi",
                        },
                        "PixelFormat": "BayerRG8",
                        "Width": 1920,
                        "Height": 1200
                    },
                    "AcquisitionControl": {
                        "ExposureMode": "Timed",
                        "ExposureAuto": "Continuous",
                        "AcquisitionFrameRate": 30.0
                    },
                    "AnalogControl": {
                        "GainAuto": "Continuous",
                        "BalanceWhiteAuto": "Off",
                        "BalanceRatio": {
                            "Red": 1.6,
                            "Blue": 2.0
                        }
                    }
                }]
    ...
```
</details>
<br>

If a feature is omitted but supported by the camera, it will default to the value that is currently set on the device.

To find the features that are available for configuration on the camera, together with their feature names and their value ranges, look into the documentation of the manufacturer or [extract the camera-specific GenICam XML file](#extracting-camera-specific-genicam-xml).

### Example Launch Files

- ```camera_driver_gv```: [camera_aravis2/launch/camera_driver_gv_example.launch.py](./camera_aravis2/launch/camera_driver_gv_example.launch.py)

- ```camera_driver_uv```: [camera_aravis2/launch/camera_driver_uv_example.launch.py](./camera_aravis2/launch/camera_driver_uv_example.launch.py)

## Finding Available Cameras

Camera_aravis2 provides the node `camera_finder` to find available cameras and print out the corresponding GUIDs.
These, in turn, are needed to run the camera drivers or to export the camera specific GenICam XML.

```bash
ros2 run camera_aravis2 camera_finder
```

Alternatively, you can use `aravis-tools`: 

```
sudo apt install aravis-tools
arv-tool-0.8
```

## Extracting Camera-specific GenICam XML

Each camera model has a specific XML data stored inside the device memory which describes the GenICam interface of the camera. 
In this XML data the supported features of the camera are documented and can help to configure the camera.
To extract this XML data and write it into a file, camera_aravis2 provides the node `camera_xml_exporter` which can be invoked as shown below:

```bash
ros2 run camera_aravis2 camera_xml_exporter --ros-args -p guid:=<camera_guid> -p xml_file:=<output_file>
```

If `guid` is omitted, the XML data will be read from any of the cameras which are connected and found by camera_aravis2.
As a `xml_file`, either a relative or absolute path can be provided in which the XML data is to be saved.
If omitted, the data is saved into a file in the current working directory with the 'guid' as filename.

Example launch file: [camera_aravis2/launch/camera_xml_exporter_example.launch.py](./camera_aravis2/launch/camera_xml_exporter_example.launch.py).


Alternatively, you can use `aravis-tools` to see the feature list and the XML file: 

```
sudo apt install aravis-tools
arv-tool-0.8 --name=<camera_guid> features
arv-tool-0.8 --name=<camera_guid> genicam
```

## Building from Source

### Requirements

**camera_aravis2**:

- Aravis 0.8 or later
- ROS 2 (tested with [humble and later](#continuous-integration))
- cv_bridge
- image_transport
- camera_info_manager
- std_msgs
- sensor_msgs
- camera_aravis2_msgs

**camera_aravis2_msgs**:

- ROS 2 (tested with [humble and later](#continuous-integration))
- std_msgs
- sensor_msgs

### Build

Install aravis:

```bash
sudo apt install libaravis-dev
```

Initialize `rosdep` and install dependencies:

```bash
sudo rosdep init
rosdep update
rosdep install --from-paths src -y --ignore-src
```

Run `colcon` to build from source:

```bash
colcon build --symlink-install --packages-up-to camera_aravis2
```

To build in 'Debug' mode add `--cmake-args -DCMAKE_BUILD_TYPE=Debug` to colcon command.
If omitted, camera_aravis2 will be build in 'Release' mode.

### Test

To run tests after build, additionally call:

```bash
colcon test --event-handlers=console_direct+
colcon test-result --all

```

## FAQ

### How to use PTP

*GigEVision Cameras Only*

Some GigEVision cameras support the use of the Precision Time Protocol (PTP) to set the timestamps of the captured images. To activate and use it just set the launch parameter 'TransportLayerControl.PtpEnable' to 'True' as seen blow:

```Python
    ...
    parameters=[{
                    ...
                    "TransportLayerControl": {
                        "PtpEnable": True
                    }
                    ...
                }]
    ...
```

### How to set specific analog control values (e.g. balance ratios)

Typically GenICam cameras support three different modes for automatic control of analog settings (e.g. gain, black level, white balance), namely 'Continuous', 'Once', and 'Off'. These can be set via the launch parameter and corresponding feature names, i.e. 'GainAuto', 'BlackLevelAuto', 'BalanceWhiteAuto'. As the names suggest, the first mode will continuously adjust the white 
balance, while the second mode will measure the white balance once and then freeze the ratio
parameters. In case, of the third mode, the ratios of the different color channels can and need to be set manually.

Use the launch parameters of camera_aravis2 to manually set the ratio parameters and configure the analog control, by

- setting the corresponding auto feature to 'Off',
- and providing a list of key-value pairs of type string (key) and double (value) for the corresponding feature. 

In this, the key will be set as value to the corresponding selector (e.g. 'GainSelector', 'BalanceRatioSelector') and the value will be set to the feature.

For example:

```Python
    ...
    parameters=[{
                    ...
                    "AnalogControl": {
                        "GainAuto": "Continuous",
                        "BalanceWhiteAuto": "Off",
                        "BalanceRatio": {
                            "Red": 1.6,
                            "Blue": 2.0
                        }
                    }
                    ...
                }]
    ...
```

### How to manually trigger calculation of white balance ratios
To trigger an automatic white balance computation and a subsequent setting of ```BalanceWhiteAuto``` to ```Once```, camera_aravis2 provides a service called ```calculate_white_balance_once```. 
Calling this service will trigger a one shot computation of the white balance parameters and return the newly computed balance ratios.
This can be called no matter which mode has been set previously.

### How to dynamically change camera parameters

Camera_aravis allows to customize the camera parameters that are to be made dynamically changeable, for example, by utilizing ```rqt_reconfigure```.
The parameters that are to be changed dynamically can be specified within a yaml file which, in turn, is to be passed to camera_aravis through the [launch parameter](#driver-specific-parameters):
- 'dynamic_parameters_yaml_url'.

This file should hold a list of 
```FeatureName```, representing the camera parameter that is to be changed, together with a corresponding ```Type``` (bool, float, int, or string).
In addition to the feature name and the type an optional ```Description``` can be specified.
Furthermore, for integer and floating point parameters a lower and upper bound can optionally be specified through the fields ```Min``` and ```Max```.

For example, the following snippet will declare the parameter 'AcquisitionFrameRate' as dynamically changeable and set the possible value range to [0.1, 50.0]:
```Yaml
    ...
    - FeatureName: AcquisitionFrameRate
      Type: float
      Min: 0.1
      Max: 50.0
      Description: Controls the acquisition rate (in Hertz) at which the frames are captured.
    ...
```

During the declaration of floating point or integer parameters, camera_aravis will additionally try to extract the lower and upper bounds of the feature from the device and compare it to the possibly declared bounds by the user. 
In this, camera_aravis will set the intersection of both ranges as the range for the corresponding parameter.

An example of this yaml file is given in [dynamic_parameters_example.yaml](camera_aravis2/config/dynamic_parameters_example.yaml).

### How to publish camera diagnostics / status

Camera_aravis allows to periodically monitor custom camera features and publish them in a designated
topic named ```~/diagnostics``` in a message type as specified in 
[CameraDiagnostics.msg](camera_aravis2_msgs/msg/CameraDiagnostics.msg). In order to configure and customize this 
status monitoring, two [launch parameters](#driver-specific-parameters) are provided:
- 'diagnostic_publish_rate', and
- 'diagnostic_yaml_url'.

An example of such a diagnostic yaml file is given in 
[camera_diagnostics_example.yaml](camera_aravis2/config/camera_diagnostics_example.yaml). This file should hold a list of 
```FeatureName``` together with a corresponding ```Type``` (bool, float, int, or string) for each
feature which is to be monitored. If a feature is associated with a feature selector, one can 
additionally specify a list of ```Selectors```. Each entry in this list should again have a 
```FeatureName``` and ```Type```, as well as a ```Value``` to set.

For each feature a key-value pair is constructed and published in the ```data``` field of the 
message stated above. If a feature as a list of selectors, one key-value pair is constructed for each Feature-Selector pair.

### How to access GigEVision camera using the IP address

GigEVision cameras can also be accessed using their IP address instead of their serial number simply by passing the address to the launch parameter [`guid`](#driver-specific-parameters). 
GUIDs and IP addresses of available cameras can be discovered with [`camera_finder`](#finding-available-cameras) node. 

## Known Issues

- **Permanent acquisition and publication of image data prior to Jazzy Jalisco:**
With camera_aravis for ROS1, the acquisition and publication of the image data only started when a subscriber has been registered on the image topic and stopped as soon as no subscriber has been registered anymore.
This helped to minimize unnecessary data transfer. 
With the so-called [`matched-events`](https://docs.ros.org/en/foxy/Releases/Release-Iron-Irwini.html#matched-events), ROS Iron Irwini has introduced a functionality that allows to do the same. 
However, the `image_transport` package for ROS Iron, which is used in `camera_aravis`, does not yet implement this feature.
This feature is only available and implemented sinze ROS Jazzy Jalisco.
