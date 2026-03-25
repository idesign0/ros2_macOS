# Examples

These examples demonstrate various Gazebo Common features.

## Build

After installing Gazebo Common, from source or from binaries, build with:

```
git clone https://github.com/gazebosim/gz-common/
cd gz-common/examples
mkdir build
cd build
cmake ..
```

### Ubuntu and MacOS

```bash
make
```

### Windows

```bash
cmake --build . --config Release
```

## Run

One executable is created inside the `build` folder for each example.

You can run each executable from the build folder with `./executablename`. For example:

### Ubuntu and MacOS

`./events_example`

### Windows

`.\Release\events_example.exe`
