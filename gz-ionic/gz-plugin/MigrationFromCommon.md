# Migration Instructions (from common::Plugin)

This file provides migration instructions for `gz` library developers to
replace the `gz-common` plugin framework with the `gz-plugin`
framework. Some of the instructions here may also be useful to new adopters of
`gz-plugin`.

# Linking to gz-plugin

`gz-plugin` has three components: `core`, `loader`, and `register`. Code that
just wants to use `PluginPtr` objects can link to `core`, e.g.:

```
target_link_libraries(my_target PUBLIC gz-plugin3::core)
```

However, if your code wants to be able to load plugins, it should link to the
`loader` component. In most cases, it should probably link privately, unless you
need the `gz::plugin::Loader` class to be part of your library's API:

```
target_link_libraries(my_target PRIVATE gz-plugin3::loader)
```

If `gz::plugin::PluginPtr` objects are part of your library's API, then
you may want `loader` to be private while `core` is public:

```
target_link_libraries(my_target
  PUBLIC
    gz-plugin3::core
  PRIVATE
    gz-plugin3::loader
)
```

If you are building a plugin library that needs to register itself as a plugin,
then you should link against the `register` component. This should almost always
be a private link, since plugin registration is purely internal for a library:

```
target_link_libraries(my_plugin PRIVATE gz-plugin3::register)
```

# Registering a plugin

The name of the header for registering plugins has changed:

* `<gz/common/PluginMacros.hh>` should be replaced by `<gz/plugin/Register.hh>`

The old `gz-common` plugin registration method had numerous macros for registering
plugins. Those have all been replaced with `GZ_ADD_PLUGIN`. Specifically:

* `IGN_COMMON_REGISTER_SINGLE_MACRO` can be directly replaced with `GZ_ADD_PLUGIN`.
* `IGN_COMMON_ADD_PLUGIN` can be directly replaced with `GZ_ADD_PLUGIN`.
* All uses of `IGN_COMMON_BEGIN_ADDING_PLUGINS` can simply be removed.
* All uses of `IGN_COMMON_FINISH_ADDING_PLUGINS` can simply be removed.
* All uses of `IGN_COMMON_SPECIALIZE_INTERFACE` can simply be removed. Interfaces no longer need to be "specialized" in order to have specialized plugins.

You can also register multiple interfaces with a single call to `GZ_ADD_PLUGIN`, e.g.:

```
GZ_ADD_PLUGIN(MyPluginClass, MyInterface1, MyInterface2, MyInterface3)
```

You may also place the `GZ_ADD_PLUGIN` macro into **any namespace**. You
simply need to make sure that the compiler can resolve the names of the classes
that you pass to it (and there will be a compilation error if it cannot).

It is now possible to register plugins across **multiple translation units**
within a single library. To do this, use `#include <gz/plugin/Register.hh>`
in **exactly one** of your library's translation units, and then use
`#include <gz/plugin/RegisterMore.hh>` in all other translation units. It
does not matter which translation unit you choose to be the "first", as long as
you choose exactly one.


# Loading a library

The `gz::common::SystemPaths` class was not ported into `gz-plugin`
because it is more related to filesystem utilities than to plugins. If you are
currently using `gz::common::SystemPaths` to help with loading plugins,
then you should continue to use it. It does not have a replacement in `gz-plugin`.

Here is a list of things that you *should* replace:

* `#include <ignition/common/PluginLoader.hh>` should be replaced with `#include <gz/plugin/Loader.hh>`
* `#include <gz/common/PluginLoader.hh>` should be replaced with `#include <gz/plugin/Loader.hh>`
* `gz::common::PluginLoader` should be replaced with `gz::plugin::Loader`
* `ignition::common::PluginLoader` should be replaced with `gz::plugin::Loader`
* When calling `Loader::Instantiate("....")` do **NOT** prefix the class name with `::`. E.g. `"::some_namespace::MyClass"` should now be `"some_namespace::MyClass"`.


# Querying an interface

Functions like `Plugin::QueryInterface()` and `Plugin::QueryInterfaceSharedPtr()`
used to require a `std::string` argument with the name of the interface. This is
no longer necessary, ever. Functions that accept `std::string` arguments will
remain in the initial release 0 of `gz-plugin`, but they are marked as
deprecated and will be removed by release 1. All interfaces can now be queried
by simply passing the class as a template argument to a query function. This is
a much safer method than relying on users to spell a string correctly.


# Template support

The new registration scheme also supports templated classes as plugins and as
interfaces. Note that the class template must be fully instantiated (i.e. all of
its template arguments must be provided).

For example, the following can work:

```
template <typename T>
class GetT_Interface
{
public:
  virtual T get() const = 0;
};


template <typename T>
class MyGetT_Plugin : public GetT_Interface<T>
{
public:
  T get() const override
  {
    return T();
  }
};

GZ_ADD_PLUGIN(MyGetT_Plugin<double>, GetT_Interface<double>)
```

When using the loader, you would call

```
loader.Instantiate("MyGetT_Plugin<double>");
```

to load this plugin.

Note that different compilers may have different conventions for how templated
objects get formatted. For example, nested templates like

```
std::vector<std::vector<int>>
```

might get formatted as `std::vector<std::vector<int> >`. That space between the
trailing brackets at the end can cause the Loader to not recognize the plugin
class that you're asking for if you neglect to include it in the string argument
that you pass to `Loader::Instantiate(std::string)`.

Additionally, STL implementations may vary in how they name their classes. For
example, if you use `std::string` as a template argument, then GCC will format
it as:

```
std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >
```

whereas MSVC might format it as something different.

In general, plugin names that get passed to a `Loader` should not be hard-coded.
They should be selected by either inspecting what interfaces they provide, or by
having a user specify the plugin name. This rule of thumb applies to both
template-based classes and to regular classes.
