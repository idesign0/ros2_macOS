ament_nodl
==========

Ament extension for exporting NoDL .xml files

Usage
---

With a file named `package.nodl.xml` in the same folder as your project's `CMakeLists.txt`, add the following line to your `CMakeLists.txt` before calling `ament_package()`

```CMake
nodl_export_node_description_file(package.nodl.xml)
```
