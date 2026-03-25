# Migration Instructions (from Gazebo classic)

This file provides instructions for `Gazebo` library developers to adopt the
`gz-cmake` package into their own library's build system. This document
is primarily targeted at Gazebo libraries that existed before `gz-cmake`
was available, but it might also be useful for getting a new `Gazebo` project
started.

The first section goes over changes that your library **must** make in order to
be compatible with `gz-cmake`. The second section mentions some utilities
provided by `gz-cmake` which might make your project's CMake scripts more
clean and maintainable, but use of those utilities is not required. The third
section details some of the new CMake features that we'll be using through
`gz-cmake` and explains why we want to use those features. The last
section describes some CMake anti-patterns which we should aggressively avoid
as we move forward.

# 1. Required Changes

You can find examples of projects that have been set up to use `gz-cmake` in
the repos of `gz-common` (branch: `CMakeRefactor`) and `gz-math`
(branch: `CMakeRefactor-3`). The following is a checklist to help you make sure
your project is migrated properly.

### Clear out your top-level `CMakeLists.txt` entirely
That's right, just throw it all out.

### Begin your top-level `CMakeLists.txt` with `cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)`

We're migrating to 3.22 because it provides many valuable features that we are
now taking advantage of.

### Then call `find_package(gz-cmake4 REQUIRED)`

This will find `gz-cmake` and load up all its useful features for you.

### Then call `gz_configure_project(<project> <version>)`

This is a wrapper for cmake's native `project(~)` command which additionally
sets a bunch of variables that will be needed by the `gz-cmake` macros and
functions.

### Then search for each dependency using `gz_find_package(~)`

We now have a cmake macro `gz_find_package(~)` which is a wrapper for the
native cmake `find_package(~)` function, which additionally:

1. Collects build errors and build warnings so that they can all be printed out
at the end of the script, instead of quitting immediately upon encountering an
error.

2. Automatically populates the dependencies for your project's pkgconfig file
and cmake config file.

A variety of arguments are available to guide the behavior of
`gz_find_package(~)`. Most of them will not be needed in most situations, but
you should consider reading them over once just in case they might be relevant
for you. The macro's documentation is available in
`gz-cmake/cmake/GzUtils.cmake` just above definition of `gz_find_package(~)`.
Feel free to ask questions about any of its arguments that are unclear.

Any operations that might need to be performed while searching for a package
should be done in a find-module. See the section on anti-patterns for more
information on writing find-modules.

### Then call `gz_configure_build(~)`

This macro accepts the argument `QUIT_IF_BUILD_ERRORS` which you should pass to
it to get the standard behavior for the Gazebo projects. If for some reason
you want to handle build errors in your own way, you can leave that argument
out and then do as you please after the macro finishes.

### Finally, call `gz_create_packages()`

After this, your top-level `CMakeLists.txt` is finished. The remaining changes
listed below must be applied throughout your directory tree.

### Change instances of `PROJECT_<type>_VERSION` variables to `PROJECT_VERSION_<type>`.

In the original Gazebo CMake scripts, we define variables for the components
of the library version: `PROJECT_MAJOR_VERSION`, `PROJECT_MINOR_VERSION`, and
`PROJECT_PATCH_VERSION`. While there is nothing inherently wrong with these
variable names, in CMake 3+ the `project(~)` command automatically defines the
following variables: `PROJECT_VERSION_MAJOR`, `PROJECT_VERSION_MINOR`, and
`PROJECT_VERSION_PATCH`. The pattern that is automatically provided by
`project(~)` is consistent with how CMake names project variables in general,
and adopting their convention will reduce the friction that we experience when
interfacing with a wide variety of native CMake utilities. It's also beneficial
to embrace the "single source of truth" pattern.

### Change instances of `IGN_PROJECT_NAME` to `GZ_DESIGNATION`

We've had a variable called `IGN_PROJECT_NAME` which refers to the `<suffix>`
in the `gz-<suffix>` name of each project. I felt that the name of the
variable was too similar to the `PROJECT_NAME` variable that is automatically
defined by CMake, as well as the `PROJECT_NAME[_NO_VERSION][_UPPER/_LOWER]` that
we define for convenience. Instead of referring to both as `[IGN_]PROJECT_NAME`,
I thought it would be better to use clear and distinct words to distinguish
them. Therefore the `<suffix>` part of the project name is now referred to as
`GZ_DESIGNATION`, and we provide `GZ_DESIGNATION[_LOWER/_UPPER]` for
convenience.

### Do not use `append_to_cached_string` or `append_to_cached_list` anymore.

These macros have been removed because they were facilitating bad practices
which we should aggressively avoid as we move forward. To put it briefly, we
should not be using the CMake cache except to allow human users to set build
options. For more explanation about why and how we should avoid using the cache,
see the below section on CMake anti-patterns.

### Replace `gz_add_library(${PROJECT_LIBRARY_TARGET_NAME} ${sources})` with `gz_create_core_library(SOURCES ${sources})`

The `gz_add_library(~)` macro has been removed and replaced with the macro
`gz_create_core_library(~)`. With this new macro, you no longer need to specify
the library name, because it will be inferred from your project information.
Instead, you should pass the `SOURCES` argument, followed by the source files
which will be used to generate your library.

You may also use the `CXX_STANDARD` argument to specify which C++ standard your
library requires (current options are 11 or 14). Note that if your library
requires a certain standard, it MUST be specified directly to this function in
order to ensure that the requirement gets correctly propagated into the
project's package information so that dependent libraries will also be aware of
the requirement. See the documentation of `gz_create_core_library(~)` in
`gz-cmake/cmake/GzUtils.cmake` for more details on how to specify your
library's C++ standard requirement.

### Specify `TYPE` and `SOURCES` arguments in `gz_build_tests(~)`

Previously, Gazebo libraries would set a `TEST_TYPE` variable before calling
`gz_build_tests(~)`, and that variable would be used by the macro to determine
the type of tests it should create. This resulted in some anti-patterns where
the `TEST_TYPE` variable would be set somewhere far away from the call to
`gz_build_tests(~)`, making it unclear to a human reader what type of tests the
call would produce. Instead, we now explicitly specify the test type using the
`TYPE` tag when calling the macro, and to avoid confusion with backwards
compatibility, the `SOURCES` tag must be used before specifying sources. We are
also introducing some new arguments:

`LIB_DEPS`: Libraries or (preferably) targets which follow the `LIB_DEPS` tag
will be linked (using `target_link_libraries`) to *each* test that gets
generated by the macro. Note that `gtest`, `gtest_main`, and your project's
library target will automatically be linked to each test by the macro (for Unix
systems, `pthread` is also added, since gtest requires it on those platforms).
Since the tests link to your library's target, all of your library's "interface"
dependencies will also be automatically linked to each test. In most cases, this
will make `LIB_DEPS` unnecessary, but it is still provided for edge cases.
Note that when individual tests depend on additional libraries, those individual
tests should be linked to their dependencies using
`target_link_libraries(<test_name> <dependency>)` after the call to
`gz_build_tests(~)`. `LIB_DEPS` should only be used for dependencies that are
needed by (nearly) all of the tests. For component libraries, you can use
`LIB_DEPS` to have the tests link to your component library.

`INCLUDE_DIRS`: Include directories that need to be visible to all (or most) of
the tests can be specified using the `INCLUDE_DIRS` tag. Note that the macro
will automatically include the "interface include directories" of your project's
library target, as well as the `PROJECT_SOURCE_DIR` and `PROJECT_BINARY_DIR`.
Also note that all of the "interface include directories" of any targets that
you pass to `LIB_DEPS` will automatically be visible to all the tests, so this
tag should be even less commonly needed than `LIB_DEPS`.

### Move your project's `cmake/config.hh.in` file to `include/gz/<project>/config.hh.in`

The `config.hh.in` file has traditionally lived in the `cmake` subdirectory of
each project, but that subdirectory should be deleted at the end of the
migration process. Since `config.hh.in` may need to be different between
projects, each project should still maintain its own copy, and that copy should
be standardized to the source include directory.

### Use imported targets instead of package variables

Calling `find_package(~)` will generally produce a set of variables which look
like `DEPENDENCY_FOUND`, `DEPENDENCY_LIBRARIES`, `DEPENDENCY_INCLUDE_DIRS`, and
`DEPENDENCY_CXX_FLAGS`. These variables are often passed to cmake functions like
`target_add_library(my_target ${DEPENDENCY_LIBRARIES})`. These variables often
contain explicit full system paths. This results in package information which
is not relocatable, and that may cause significant problems when distributing
pre-built packages, or when relocating a package within a system.

Instead, we should prefer passing **only targets** into `target_link_libraries(~)`.
When using `target_link_libraries(~)` to link targets instead of libraries, all
of the "interface" properties of the dependency target will be linked to the
dependent target, like a transitive property. An "interface" property is a
property which a target specifies as being required by any packages that want to
interface with it. This includes the interface include directories, interface
compiler flags, interface libraries, among other properties. These properties
will be propagated in a way which is relocatable. Also, be sure to specify
`PUBLIC` or `PRIVATE` depending on whether libraries which depend on your
project will need to link to symbols in the library that your project is linking
to.

Note that you must also specify which targets' interface include directories
will be needed by libraries which depend on your project's library. This should
be done using `gz_target_interface_include_directories(<project_target> <dependency_targets>)`.
That function will add the interface include directories of the dependency
targets that you pass in to the interface include directory list of
`<project_target>` in a way which is relocatable by using generator expressions.
This is in contrast to just using
`target_include_directories(<project_target> ${DEPENDENCY_INCLUDE_DIRS})` which
will not be relocatable, because `${DEPENDENCY_INCLUDE_DIRS}` just contains an
explicit list of full paths to the directories.

**BEWARE**: Very often, a target name might be identical to the name of the
library that it is meant to represent. This can cause confusion for cmake when
calling `target_link_libraries(~)`, because subtle typos might cause it to
select a library even though you meant to specify a target. To avoid this,
target names can contain a scoping operator `::` which is not allowed in the
names of libraries. When an item containing `::` is passed to
`target_link_libraries(~)`, CMake will know that a target is being specified,
and it will throw an error and quit if that target is not found, instead of
failing quietly or subtly. Therefore, we should always exercise the practice of
using `::` in the names of any imported targets that we intend to use.
`gz-cmake` will automatically export all Gazebo library targets to have
the name `gz-<project><major_version>::gz-<project><major_version>`
(for example, `gz-common0::gz-common0`). When creating a cmake
find-module, the macro `gz_import_target(~)` should be used generate an
imported target which follows this convention. More about creating find-modules
can be found in the section on anti-patterns.

### Remove gz_install_library()

Calling `gz_create_core_library()` will also take care of installing the
library. Simply remove this function from your cmake script.

### Replace calls to `#include "gz/<project>/System.hh"` with `#include "gz/<project>/Export.hh"`, and delete the file `System.hh`.

Up until now, we've been maintaining a "System" header in each Gazebo library.
This is being replaced by a set of auto-generated headers, because some of the
individual projects' implementations had errors or issues in them. The new
auto-generated headers will enforce consistency and compatibility across all of
the Gazebo projects. The header is also being renamed because the role of the
header is to provide macros that facilitate exporting the library, therefore it
seems more appropriate to name it "Export" instead of "System". Nothing in the
header is interacting with the operating system, so the current name feels like
somewhat of a misnomer (presumably the name "System" came from the fact that the
macros are system-dependent, but I think naming the header after the role that
it's performing would be more appropriate).

### Replace `GZ_<VISIBLE/HIDDEN>` with `GZ_<PROJECT>_<VISIBLE/HIDDEN>` in all headers

The export (a.k.a. visibility) macros used by each Gazebo library must be
unique. Different Gazebo libraries might depend on each other, and the
compiler/linker would be misinformed about which symbols to export if two
different libraries share the same export macro.

Note that component libraries will generate their own visibility macros so that
they can correctly be compiled alongside their core library. Those macros will
look like `GZ_<PROJECT>_<COMPONENT>_<VISIBLE/HIDDEN>`.

### Move all find-modules in your project's `cmake/` directory to your `gz-cmake` repo, and submit a pull request for them

We are centralizing all find-modules into `gz-cmake` so that everyone benefits
from them, and we get a single place to maintain them.

### Remove the entire `cmake/` subdirectory from your project

Once the above steps are complete, your project's `cmake/` subdirectory should
no longer be needed. If your `cmake/` subdirectory contained some features
that are not already present in `gz-cmake`, then you should add those
features to `gz-cmake` and submit a pull request. I will try to be very prompt
about reviewing and approving those PRs.

### To add a component library, use `gz_add_component(<component> SOURCES ${sources})`

This new function allows you to create a "component" library which will be
compiled separately from your core library. It will be packaged as a cmake
component of your core library, and it will also be packaged as its own
independent cmake package. For pkg-config, it will packaged as its own
independent package because pkg-config does not seem to have a concept analogous
to components.

By default, your component will be publicly linked to your core library. To
change this behavior, you may pass one of the following arguments:
`INDEPENDENT_FROM_PROJECT_LIB`, `PRIVATELY_DEPENDS_ON_PROJECT_LIB`, or
`INTERFACE_DEPENDS_ON_PROJECT_LIB`.

By default, the auto-generate public headers for this component will go into the
directory `gz/<project>/<component>/`. You can change the subdirectory
using the optional argument `INCLUDE_SUBDIR <subdir>` which will instead put
the auto-generate public headers into `gz/<project>/<subdir>/`.

You may also pass the argument `GET_TARGET_NAME <output_var>` to retrieve the
auto-generated name of the target. You can then use the target with
`${<output_var>}`.


# 2. Recommended Changes

The following changes are not necessary, but may improve the readability and
maintainability of your CMake code. Use of these utilities is optional.

### GLOB up library source files and unit test source files using `gz_get_libsources_and_unittests(sources tests)`

Placing this in `src/CMakeLists.txt` will collect all the source files in the
directory and sort them into a `source` variable (containing the library sources)
and a `tests` variable (containing the unit test sources).

If there are files that you want to exclude from either of these lists, you can
use `list(REMOVE_ITEM <list> <filenames>)` after calling the function. That
approach can be used to conditionally remove files from a list (see
`gz-common/src/CMakeLists.txt` for an example). Alternatively, if you always
want a file to be excluded, you can change its extension (e.g. `*.cc.backup` or
`.cc.old`) until a later time when you want it to be used again.

### Use `gz_install_all_headers(~)` in `include/gz/<project>/CMakeLists.txt`

Using this macro will install all files ending in `*.h` and `*.hh` in the
current source directory recursively (so all the files in all subdirectories as
well as their subdirectories will also be installed). It will also configure
your project's `<project>.hh` and `config.hh.in` files and install them.

You can use the argument `EXCLUDE_FILES` to specify files that should not be
installed. The argument `EXCLUDE_DIRS` lets you specify subdirectories to not
install. Note that the files or directories must be specified relative to the
current directory.

### Use `gz_get_sources(~)` in `test/<type>/CMakeLists.txt` to collect source files

Similar to `gz_get_libsources_and_unittests(~)` except it only produces one
list of source files, which is sufficient to be passed to `gz_build_tests(~)`.




# 3. Anti-patterns to avoid

### Do not use `include(${cmake_dir}/ModuleName.cmake)`

Files that end in `*.cmake` are known as "modules" and are not meant to be
invoked using the fully qualified filename. Instead, the path that leads up to
the module should be added to `${CMAKE_MODULE_PATH}` if it is not in there
already (the module path of `gz-cmake` will automatically be added when
you call `find_package(gz-cmake# REQUIRED)`, so you do not have to worry
about this for `gz-cmake` modules). After that, the module should be invoked
using `include(ModuleName)` with no path or extension. CMake will automatically
find the appropriate file.

### Do not use `include(FindSomePackage)`

When a `*.cmake` file begins with the word `Find`, it is a special type of
cmake module known as a find-module. Its purpose is to search for a package
after being invoked by the command `find_package(SomePackage)`. Notice that the
`SomePackage` argument must match the string of characters in between `Find` and
`.cmake` in the filename `FindSomePackage.cmake`. Case matters. This is not just
a convention; it is a cmake requirement.

Note that while using `gz-cmake`, you should be using `gz_find_package(~)`
instead of the native `find_package(~)` command. It does the same thing, except
that it adds some additional functionality which is important for ensuring
correctness in the package configuration files that we generate for our projects.

### Do not name a module `Find<Module>.cmake` unless it is genuinely a find-module

We had files named `FindOS.cmake` which checked the operating system type, and
`FindSSE.cmake` which checked the SSE compatibility of the build machine.
Neither of these were searching for packages, so neither of them should begin
with the word `Find`. As explained above, that pattern of filename is reserved
for find-modules that are supposed to search for packages after being invoked by
the `find_package(~)` command.

### All package search behavior must go in its own find-module

Up until now, we have generally used the `SearchForStuff.cmake` file to find
packages that our libraries depend on. Any logic or procedures that are needed
to find a package will often end up buried in that file, making it difficult to
transfer the procedure between different projects (often leading to redundant
copies of the same procedure, ultimately resulting in different projects using
divergent methods of varying quality for solving the same problem). Instead, any
procedures or operations that are needed to find a package dependency should be
put into a file called `Find<PACKAGE>.cmake` where `<PACKAGE>` should be
replaced with the name of the package (often this is done in all uppercase
letters). This `Find<PACKAGE>.cmake` should be added to `gz-cmake/cmake`.
Pull requests for adding find-modules will be reviewed and approved as quickly
as possible. This way, all projects can benefit from any one person's effort in
writing a good quality find-module.

In many cases, a package that we depend on will be distributed with a pkgconfig
(`*.pc`) file. In such a case, `gz-cmake` provides a macro that can easily
find the package and create an imported target for it. Simply use `include(GzPkgConfig)`
and then `gz_pkg_check_modules(~)` in your find-module, and you are done. An
example of a simple case of this can be found in `gz-cmake/cmake/FindGTS.cmake`.

If certain version-based behavior is needed, that must be handled within the
find-module. A simple example using pkgconfig can be found in
`gz-cmake/cmake/FindAVDEVICE.cmake`.

Sometimes a package may be needed but there is no guarantee that a pkgconfig
file will be available for it. For an example of how to handle that, see
`gz-cmake/cmake/FindFreeImage.cmake`.

Some libraries are never distributed with a pkgconfig file. For an example of
how to create a find-module when a pkgconfig file is guaranteed to not exist,
see `gz-cmake/cmake/FindDL.cmake`. Note that you must manually specify the
variables `<PACKAGE>_PKGCONFIG_ENTRY` and `<PACKAGE>_PKGCONFIG_TYPE` in such
cases. The entry will have to be the name of library (or libraries), preceded by
`-l`, while the type must be `PROJECT_PKGCONFIG_LIBS`.

### Do not use `CACHE INTERNAL`

There is almost never a situation where `CACHE INTERNAL` is appropriate for us
to use. If you think you need to use it, you probably don't. If you're certain
you need to use it, you should discuss it with someone first. Using
`CACHE INTERNAL` can have very negative side effects that may easily go
unnoticed because the internally cached data isn't readily visible to a
developer.

To understand why `CACHE INTERNAL` should be unnecessary, it is important to
understand variable scope in cmake. When you call `add_subdirectory(~)`, you
will enter a child scope. Each child scope can see all the variables that were
set in its ancestors (parent directory, grandparent directory, etc.). This
allows variables to easily trickle down the directory tree. A child directory
can override a variable that was set in its parent directory, and that change
will trickle down into all the children of that child, but the parent and the
parent's other children will not see the change. This is an intentional feature
to make sure that the special needs of one child do not impact its siblings (or
cousins, etc).

If for some reason a child *should* change a variable for its parent and
siblings, then the `set(~)` function accepts the `PARENT_SCOPE` option. If a
child needs to change a variable in a way that is supposed to impact its parent,
grandparent, siblings, cousins, etc, then there is almost certainly something
wrong with the way the cmake script is designed. An operation (or variable) that
needs to be visible to such a broad scope should simply be performed (or set) in
a higher scope rather than added to the cache.

Note that a cmake function will behave as though it has a child scope, while a
macro will behave as though it has the same scope as the parent that calls it.
If the role of the function/macro is to effectively copy/paste a bunch of
text into the file that calls it, it should be written as a macro. If the role
is to perform some complex operations and then return just a small number of
variables, then it should be written as a function, and `set( ... PARENT_SCOPE)`
should be used to provide the variable to the parent scope.

This is not to say that the cache itself should never be used. The cache is
useful for exposing build options to the user. However, in those cases, a type
(such as `FILEPATH`, `PATH`, `STRING`, or `BOOL`) must be specified instead of
`INTERNAL`. When providing a bool option, you should prefer to use the command
`option(<variable> "Description" <default>)`. When providing a string option
where a set of valid choices is known ahead of time, use
`set(<variable> "Default Variable Value" CACHE STRING "Description")` followed
by `set_property(CACHE <variable> PROPERTY STRINGS <list_of_choices>)`. This
will explicitly inform the user of their choices for the option.

### Do not use `link_directories(~)`

The convention when finding packages in cmake is to provide full library paths,
so specifying a link directory should not generally be needed, except in edge
cases where a find-module does not comply with the established convention.
