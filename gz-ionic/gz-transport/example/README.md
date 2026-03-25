## Examples

Example programs using Gazebo Transport.

## Build

Create directory:

```
cd examples
mkdir build
cd build
```

Configure:

```
cmake ..
```

Build on Unix:


```
make
```

To build on Windows, make sure the configuration matches `gz-transport`'s 
configuration:

```
cmake --build . --config Release
```

## Run

Several executables were created in the build folder. 

For example, run the subscriber on one terminal:

Unix:

```
cd example/build
./subscriber
```

Windows:

```
cd example\build\Release
subscriber.exe
```

And on another terminal, run a publisher:

Unix:

```
cd example/build
./publisher
```

Windows:

```
cd example\build\Release
publisher.exe
```

You'll see on the publisher terminal:

```
Publishing hello on topic [/foo]
Publishing hello on topic [/foo]
Publishing hello on topic [/foo]   
```

And on the subscriber terminal:

```
Msg: HELLO

Msg: HELLO

Msg: HELLO
```

