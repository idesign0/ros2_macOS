# Using gz-msgs as part of your project 

The example in this folder demonstrates how to use messages as part of your build.

To use messages as part of your project, simply find the gz-msgs package

```
find_package(gz-msgs11 REQUIRED)
set(GZ_MSGS_VER ${gz-msgs11_VERSION_MAJOR}
```

And link the messages library to your library or executable

```
add_executable(${PROJECT_NAME} main.cc)
target_link_libraries(${PROJECT_NAME} PRIVATE gz-msgs${GZ_MSGS_VER})
```

For a more advanced example generating custom message types, consult the generating custom mgs tutorial

