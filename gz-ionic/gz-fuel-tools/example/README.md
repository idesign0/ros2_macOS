# Examples

These examples demonstrate various Gazebo Fuel tools features.

### Dependencies

Remember to install the `gflags` dependency:

#### Ubuntu

```bash
sudo apt-get install libgflags-dev
```

#### Windows
```bash
conda install gflags  --channel conda-forge
```

## Build

After installing Gazebo Fuel tools, from source or from binaries, build with:

```
git clone https://github.com/gazebosim/gz-fuel-tools/
cd gz-fuel-tools/example
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
cmake ..
cmake --build . --config Release
```

## Run

Several executables are created inside the `build` folder for each example.

You can run each executable from the build folder with `./executablename`. For example:

### Ubuntu and MacOS

`./list -t world`

### Windows

`.\Release\list.exe -t world`
