# avt_vimba_camera (ROS2)

This repo contains a ROS2 driver for cameras manufactured by [Allied Vision Technologies](https://www.alliedvision.com).
The driver relies on libraries provided by AVT as part of their [Vimba SDK](https://www.alliedvision.com/en/products/software.html).

*See the ROS1 version of this README [here](https://github.com/astuff/avt_vimba_camera/blob/ros1_master/README.md).*

## Installation

### Dependencies
First, you will need to install the Vimba SDK.
Download it from AVT's website [here](https://www.alliedvision.com/en/products/vimba-sdk/#c1497).

Also see the [linux vimba installation instructions](https://cdn.alliedvision.com/fileadmin/content/documents/products/software/software/Vimba/appnote/Vimba_installation_under_Linux.pdf).

It is highly recommended to open the "Vimba Viewer" tool that came along with the SDK and make sure you can connect to your camera.

It may be useful to create a desktop shortcut to Vimba Viewer:
```sh
ln -sf "Vimba_5_0/Tools/Viewer/Bin/x86_64bit/VimbaViewer" "$HOME/Desktop"
```

### ROS Driver

Once you've successfully connected to your camera using Vimba Viewer, you can continue with the ROS driver install:

```
sudo apt install ros-$ROS_DISTRO-avt-vimba-camera
```

## Operational Advice

### MTU Size (GigE Cameras)
If you are using a GigE camera (ethernet-based camera), it is recommended to adjust some settings in your network interface to be able to handle the potentially high bandwidth usage of the camera stream.

On Linux, you will need to increase the MTU (Maximum Transmission Unit) on the network interface attached to the camera.

You can check what your current mtu setting is by running the following command:
```
ip a | grep mtu
``` 

According to AVT documentation, increase the mtu to `9014`.
If you use Network Manager, this can be done by opening the network interface settings and editing the "MTU" box under the "Identity" tab. 

See the "Optimize system performance" section of your camera's technical manual for full details.
For example, the Mako camera technical manual is available [here](https://cdn.alliedvision.com/fileadmin/content/documents/products/cameras/Mako/techman/Mako_TechMan_en.pdf).

### Receive Buffer Size (GigE Cameras)

It is also recommended to increase your network receive buffer size.
By default, Ubuntu uses `212992`.

You can check what your current buffer size is:
```
sudo sysctl 'net.core.rmem_max'
```
Update the buffer size with the following command:
```
sudo sysctl -w 'net.core.rmem_max=26214400'
```

`26214400` has been tested successfully, but anything above `2000000` is likely fine.

Once you find a value that works for you, you can make the change permanent (persist across reboots) by updating the `/etc/sysctl.conf` file with the following line:

```
net.core.rmem_max=26214400
```

### Camera Settings in General

When the ROS2 driver starts up, it queries the camera for all features and creates ROS parameters for every feature on the camera.
All camera-related ROS parameters are prefixed with "feature/" in their name to indicate they are camera features.
Note that some features are read-only, and each camera model will have slightly different feature sets.
If the user does not specify the ROS parameter for a given feature using yaml config files or launch files, then the value for that feature is untouched and not overridden by the driver.

This allows you to take two different approaches towards params/features with the ROS driver:
1. Don't configure anything via ROS params and instead use the camera's "Saved User Sets" functionality to load a custom configuration every time it boots. 
The ROS driver won't change anything (as long as no "feature/" ROS params are set) and just use the camera as it's configured.

2. Use ROS params defined in a yaml file to configure the camera when the driver starts.
Existing params can be saved using `ros2 param dump [node_name]` .

Reagrdless of your approach, if you are having difficulty getting the camera configured, it is suggested to first use Vimba Viewer to play around with settings that work. 
The Vimba Viewer GUI will help you determine what settings are available to your camera model and help you tune them easier.
Once you have settings that you are happy with, save them into your own rosparam file or onto the camera using "Saved User Sets". 


## ROS Nodes

### mono_camera_node

The mono_camera_node is the main driver that connects to the camera, configures it according to ROS parameters/dynamic reconfigure, and starts publishing image frames.
The driver uses [image_transport](http://wiki.ros.org/image_transport) to publish image frames, so all expected image topics should be available.
See the config file (`cfg/AvtVimbaCamera.cfg`) for documentation regarding the various parameters that can be used to configure the camera itself.
See the launch file (launch/mono_camera.launch) for documentation regarding the operational parameters of the driver.

### trigger_node

The trigger_node is a standalone node for sending out ethernet-based action commands to AVT cameras. 
Action commands are useful for triggering frame captures over ethernet.
See AVT's [application note](https://cdn.alliedvision.com/fileadmin/content/documents/products/cameras/various/appnote/GigE/Action-Commands_Appnote.pdf) for more details.
Note that cameras must be configured to receive the action commands in addition to running the trigger_node.

## Clock Synchronization

If you wish to use the exact time the image was measured in the header of the ROS messages, it is suggested to use PTP synchronization.
PTP will ensure the clock on the camera is synchronized with the computer, so that measurement times are all based off of the same clock.
Setting the `use_measurement_time` parameter will set the ROS header timestamp to the frame timestamp, but it is up to you to make sure the camera clock is synced with the computer.
[linuxptp](http://linuxptp.sourceforge.net) is a great tool for PTP synchronization and is suggested for ensuring the camera is in sync with the computer.
See the links below for more details on PTP sync.

## Useful Technical References and Application Notes

- [GigE Features Reference](https://cdn.alliedvision.com/fileadmin/content/documents/products/cameras/various/features/GigE_Features_Reference.pdf) (To better understand what features your camera supports and how to tune them)
- [Trigger over Ethernet - Action Commands](https://cdn.alliedvision.com/fileadmin/content/documents/products/cameras/various/appnote/GigE/Action-Commands_Appnote.pdf) 
- [PTP Clock Sync](https://cdn.alliedvision.com/fileadmin/content/documents/products/cameras/various/appnote/GigE/PTP_IEEE1588_with_Prosilica_GT_GC_Manta.pdf) (Highly recommended if you care about exact image acquisition time)
- [Image Timestamp on Allied Vision GigE Cameras](https://cdn.alliedvision.com/fileadmin/content/documents/products/cameras/various/appnote/GigE/Image_Timestamp.pdf) 
- [Decimation](https://cdn.alliedvision.com/fileadmin/content/documents/products/cameras/various/appnote/various/Decimation.pdf) (Binning is similar)
