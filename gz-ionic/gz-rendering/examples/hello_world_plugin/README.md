# Engine plugin

This example shows how to create a plugin that integrates a rendering engine with
Gazebo Rendering and how to load it with Gazebo.

## Build

From the root of the `gz-rendering` repository, do the following to build the example:

~~~
cd examples/hello_world_plugin
mkdir build
cd build
cmake ..
make
~~~

This will generate the `HelloWorldPlugin` library under `build`.
The exact name of the library file depends on the operating system
such as `libHelloWorldPlugin.so` on Linux, `libHelloWorldPlugin.dylib` on macOS,
and `HelloWorldPlugin.dll` on Windows.

## Run

Be sure to have the `GZ_SIM_RENDER_ENGINE_PATH` environment variable set to the path
where your plugin is located.  From within the `build` directory of this example, you can run

~~~
export GZ_SIM_RENDER_ENGINE_PATH=$PWD
~~~

to set the environment variable accordingly.


Now you can run `gz sim` with the name of the resultant library file (without the `lib` prefix
or the file extension, i.e., libHelloWorldPlugin.so -> HelloWorldPlugin):

~~~
gz sim --render-engine HelloWorldPlugin empty.sdf
~~~

You should see a blank screen within the Gazebo GUI, as this mocked plugin provides no implementation
for the scene. The Gazebo Component Inspector should show that the Render Engine Gui Plugin and
the Render Engine Server Plugin are now set to use the `HelloWorldPlugin`.
