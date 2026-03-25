\page renderingplugin Understanding the Rendering Plugin

This is an introduction to different rendering engines and how they are integrated into the Gazebo Rendering library.

## Gazebo Rendering

The \ref gz::rendering "Gazebo Rendering" library integrates external rendering engines into the Gazebo Simulation eco-system.
It allows users to select from multiple supported rendering engines based on their simulation needs.
Its plugin interface loads rendering engines at runtime.
It is also possible to integrate your own selected rendering engine by writing a compatible plugin interface.

#### How to Write Your Own Rendering Engine Plugin

A mocked example of a custom rendering engine plugin can be found [here](https://github.com/gazebosim/gz-rendering/tree/gz-rendering9/examples/hello_world_plugin).  In order
to make your own custom rendering engine, this example is a good starting point.  There are a few key things which will need to be done in order for a custom rendering engine to function:

 * A singleton Render Engine class which implements the pure virtual functions in [`gz::rendering::BaseRenderEngine`](https://github.com/gazebosim/gz-rendering/blob/gz-rendering9/include/gz/rendering/base/BaseRenderEngine.hh).
 * A plugin class which implements the pure virtual functions in [`gz::rendering::RenderEnginePlugin`](https://github.com/gazebosim/gz-rendering/blob/gz-rendering9/include/gz/rendering/RenderEnginePlugin.hh)
 * Registering the plugin, this line can be seen at the bottom of the [`HelloWorldPlugin`](https://github.com/gazebosim/gz-rendering/tree/gz-rendering9/examples/hello_world_plugin/HelloWorldPlugin.cc) example

Finally, for your custom rendering engine to actually have any functionality and at minimum, display something in a window, you will need to implement your own `Scene` and `Camera` classes, which inherit from and implement the pure virtual functions of [`gz::rendering::Scene`](https://github.com/gazebosim/gz-rendering/blob/gz-rendering9/include/gz/rendering/Scene.hh) and  [`gz::rendering::Camera`](https://github.com/gazebosim/gz-rendering/blob/gz-rendering9/include/gz/rendering/Camera.hh), respectively.  The mocked example simply returns `nullptr` for its `CreateSceneImpl(...)` function, so it may be useful to look at the current `Scene` implementations for the other rendering engines within `gz::rendering` such as [`OGRE`](https://github.com/gazebosim/gz-rendering/blob/gz-rendering9/ogre/src/OgreScene.cc) or [`OGRE2`](https://github.com/gazebosim/gz-rendering/blob/gz-rendering9/ogre2/src/Ogre2Scene.cc).  Likewise, it may be helpful to look at the `Camera` implementations from [`OGRE`](https://github.com/gazebosim/gz-rendering/blob/gz-rendering9/ogre/src/OgreCamera.cc) and [`OGRE2`](https://github.com/gazebosim/gz-rendering/blob/gz-rendering9/ogre2/src/Ogre2Camera.cc)

#### Building and Running Your Rendering Engine Plugin with Gazebo

Once you have your own rendering plugin written, you can build it similarly to how the example is built.  It may be helpful to look at the [`CMakeLists.txt`](https://github.com/gazebosim/gz-rendering/tree/gz-rendering9/examples/hello_world_plugin) from the example as it contains the boilerplate code needed to get a custom rendering engine plugin built.

After you have your plugin built, you will need to set the `GZ_SIM_RENDER_ENGINE_PATH` environment variable to the path in which your built shared library resides.  Note that you will need to do this for every command line instance unless you add the line to your startup shell script (`.bashrc`, `.zshrc`, etc.).

From here, you should be able to reference your rendering plugin within Gazebo by the name of the generated shared library file (without the `lib` prefix or the file extension, i.e., libHelloWorldPlugin.so -> HelloWorldPlugin).

\note You can use the `--render-engine`, `--render-engine-server`, and `--render-engine-gui` flags to specify any supported or custom rendering engine at runtime, this flag will override any other pre-existing rendering engine specifications (such as in an `.sdf` file).  The command line call would look something like:

~~~
gz sim --render-engine HelloWorldPlugin shapes.sdf
~~~
