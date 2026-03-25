# ads_vendor

This `ads_vendor` package for ROS 2 vendors the [Beckhoff ADS library](https://github.com/Beckhoff/ADS). It downloads, builds, and installs the ADS library as part of the ROS 2 build process, making it easily available for other ROS 2 packages to depend on.


## Features

* **Automated Vending:** Automatically downloads and builds the specified version of the Beckhoff ADS library during the ROS 2 workspace compilation.
* **ROS 2 Integration:** Integrates the ADS library into the ROS 2 build system, allowing other packages to simply `find_package(ads_vendor)` and link against `ads`.

## Requirements

* ROS 2 (Humble Hawksbill or newer recommended)
* CMake 3.13 or higher
* Git

## Installation

To use this package, you'll need to clone it into your ROS 2 workspace and then build your workspace.

2.  **Clone this repository:**
    ```bash
    git clone git@github.com:b-robotized/ads_vendor.git
    ```

3.  **Build your ROS 2 workspace:**
    ```bash
    colcon build --packages-up-to ads_vendor
    ```

## Usage

Once `ads_vendor` is built and installed, any other ROS 2 package in your workspace can declare a dependency on it and use the ADS library.

### In your `package.xml`:

Add `ads_vendor` as a build and export dependency:

```xml
<depend>ads_vendor</depend>
```
And use the ADS headers like so:
```c++
#include <ads/AdsLib.h>
#include <ads/AdsDef.h>
...
```
## License

This ads_vendor package was created by [B-Robotized GmbH](https://www.b-robotized.com/) and is provided under the [Apache 2.0 License](https://www.apache.org/licenses/LICENSE-2.0).

The vendored Beckhoff ADS library is subject to its own license, which can be found in [its repository](https://github.com/Beckhoff/ADS).

