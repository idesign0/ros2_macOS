\page rotation_example Rotation example

This example explains how to use quaternions and euler angles, and how to convert between them.

## Compiling and running the code

Go to `gz-math/examples` and use `cmake` to compile the code:

```{.sh}
git clone https://github.com/gazebosim/gz-math/ -b gz-math8
cd gz-math/examples
mkdir build
cd build
cmake ..
# Linux and macOS
make
# Windows
cd ..
cmake --build build --config Release
```

When the code is compiled, you can run two different examples, one which converts from quaternion to euler angles:

```{.sh}
Usage:
  ./quaternion_to_euler <float_w> <float_x> <float_y> <float_z>

Example
  ./quaternion_to_euler 0.5 0.5 0.5 0.5
```

And the other which converts from euler to quaternion:

```{.sh}
Usage (angles specified in radians):
  quaternion_from_euler <float_roll> <float_pitch> <float_yaw>

Example
  quaternion_from_euler 0 0 1.57
```

The output of each program, respectively:

```{.sh}
# Linux and macOS
./quaternion_to_euler 0.5 0.5 0.5 0.5
# Windows
.\build\Release\quaternion_to_euler.exe 0.5 0.5 0.5 0.5
Normalizing Quaternion components:
  W 0.5
  X 0.5
  Y 0.5
  Z 0.5
to
  W 0.5
  X 0.5
  Y 0.5
  Z 0.5

Converting to Euler angles
 roll   1.570796 radians
 pitch -0.000000 radians
 yaw    1.570796 radians

 roll   90.000000 degrees
 pitch -0.000000 degrees
 yaw    90.000000 degrees

to Rotation matrix
    0.000000   0.000000   1.000000
    1.000000   0.000000   0.000000
    0.000000   1.000000   0.000000
```

```{.sh}
# Linux and macOS
./quaternion_from_euler 0 0 1.57
# Windows
.\build\Release\quaternion_from_euler.exe 0 0 1.57
Converting Euler angles:
 roll   0.000000 radians
 pitch  0.000000 radians
 yaw    1.570000 radians

 roll      0.000000 degrees
 pitch     0.000000 degrees
 yaw      89.954374 degrees

to Quaternion
 W  0.707388
 X  0.000000
 Y  0.000000
 Z  0.706825

to Rotation matrix
    0.000796  -1.000000   0.000000
    1.000000   0.000796   0.000000
    0.000000   0.000000   1.000000
```

## Code

You can create some objects to express rotations:

\snippet examples/quaternion_from_euler.cc constructor
\snippet examples/quaternion_to_euler.cc constructor

To access the quaternions attributes:

\snippet examples/quaternion_from_euler.cc access quaternion

Or to access the rotation matrix elements:

\snippet examples/quaternion_from_euler.cc rotation matrix
