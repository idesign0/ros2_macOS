\page simple_demo Simple demo

This example shows how move the camera automatically.

## Compile and run the example

Clone the source code, create a build directory and use `cmake` and `make` to compile the code:

```{.sh}
git clone https://github.com/gazebosim/gz-rendering
cd gz-rendering/examples/simple_demo
mkdir build
cd build
cmake ..
# Linux
cmake --build .
# Windows
cmake --build . --config Release
```
Execute the example:

```{.sh}
# Linux
./simple_demo

# Windows
.\Release\simple_demo
```

You'll see:

```{.sh}
[Msg] Loading plugin [gz-rendering9-ogre]
===============================
  TAB - Switch render engines
  ESC - Exit
===============================
```

@image html img/simple_demo.gif

## Code

The function `updateCameras()` is called each time the `DisplayCB` function runs. Using the method `SetLocalPosition` from the `Camera` class we can move the camera in the world:

\snippet examples/simple_demo/GlutWindow.cc update camera
