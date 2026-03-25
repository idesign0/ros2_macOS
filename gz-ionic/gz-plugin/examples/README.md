# Examples

These examples demonstrate various Gazebo Plugin features.

## Build

After installing Gazebo Plugin, from source or from binaries, build with:

```
git clone https://github.com/gazebosim/gz-plugin/
cd gz-plugin/examples
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

Two executables are created inside the `build` folder for each example.

You can run each executable from the build folder with `./executablename`. For example:

### Ubuntu and MacOS

`./robot`

### Windows

`.\Release\robot.exe`
