\page heightmap Heightmap

This example shows how to add a heightmap to the scene.

It loads 2 different heightmaps (image and Digital Elevation Model (DEM)) with different parameters.

## Compile and run the example

Clone the source code, create a build directory and use `cmake` to compile the code:

```{.sh}
git clone https://github.com/gazebosim/gz-rendering
cd gz-rendering/examples/heightmap
mkdir build
cd build
cmake ..
# Linux
cmake --build .
# Windows
cmake --build . --config Release
```
Example 1 (image heightmap):

```{.sh}
# Linux
./heightmap

# Windows
.\Release\heightmap.exe
```

You'll see:

```{.sh}
[Msg] Loading plugin [gz-rendering9-ogre]
[Msg] Loading heightmap: scene::Heightmap(65528)
[Msg] Heightmap loaded. Process took 217 ms.
===============================
  TAB - Switch render engines
  ESC - Exit
===============================
```
@image html img/heightmaps.png

Example 2 (DEM heightmap):

```{.sh}
./heightmap --dem
```

@image html img/heightmaps_dem.png


## Code

A heightmap is a terrain defined by a 2D grid of height values.
The example 1 demo, loads the heights from a grayscale image, where the color
black represents the lowest point and white represents the heights.
Example 2, loads the heights from the DEM file itself.

The heightmap's information is stored in the `HeightmapDescriptor` class.
In addition to the height data, the heightmap descriptor also exposes
functionality such as:

* Its size in meters in XYZ space.
* The position of its origin in the world.
* The textures to use according to the height.
* How to blend these textures.

Here's the snippet of code from `examples/heightmap/Main.cc` that adds an
image heightmap to the scene:

\snippet examples/heightmap/Main.cc create an image heightmap

And a snippet that adds a DEM heightmap to the scene:

\snippet examples/heightmap/Main.cc create another dem heightmap

Note, since DEMs encode elevation data, the minimum elevation for the `moon.tif`
DEM is `-212.296`. We need to translate the DEM for it to be at `z = 0` by
setting the descriptor's z position to be `212.296`
or `desc2.SetPosition({0, 0, std::abs(data2->MinElevation())});`.

