/*
 * Copyright (C) 2017 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/


#ifndef GZ_PLUGIN_REGISTER_HH_
#define GZ_PLUGIN_REGISTER_HH_

#include <gz/plugin/detail/Register.hh>


// ------------- Add a set of plugins or a set of interfaces ------------------

/// \brief Add a plugin and interface from this shared library.
///
/// This macro can be put in any namespace and may be called any number of
/// times. It can be called multiple times on the same plugin class in order to
/// register multiple interfaces, e.g.:
///
/// \code
/// GZ_ADD_PLUGIN(PluginClass, Interface1)
/// GZ_ADD_PLUGIN(PluginClass, Interface2)
///
/// /* Some other code */
///
/// GZ_ADD_PLUGIN(PluginClass, Interface3)
/// \endcode
///
/// Or you can list multiple interfaces in a single call to the macro, e.g.:
///
/// \code
/// GZ_ADD_PLUGIN(PluginClass, Interface1, Interface2, Interface3)
/// \endcode
///
/// If your library has multiple translation units (.cpp files) and you want to
/// register plugins in multiple translation units, use this
/// gz/plugin/Register.hh header in ONE of the translation units, and then
/// the gz/plugin/RegisterMore.hh header in all of the rest of the
/// translation units.
#define GZ_ADD_PLUGIN(PluginClass, ...) \
  DETAIL_GZ_ADD_PLUGIN(PluginClass, __VA_ARGS__)

/// \brief Add an alias for one of your plugins.
///
/// This macro can be put in any namespace and may be called any number of
/// times. It can be called multiple times on the same plugin class in order to
/// register multiple aliases, e.g.:
///
/// \code
/// GZ_ADD_PLUGIN_ALIAS(PluginClass, "PluginClass")
/// GZ_ADD_PLUGIN_ALIAS(PluginClass, "SomeOtherName", "Yet another name")
/// GZ_ADD_PLUGIN_ALIAS(AnotherPluginClass, "Foo", "Bar", "Baz")
/// \endcode
///
/// You can give the same alias to multiple plugins, but then that alias can no
/// longer be used to instantiate any plugin.
///
/// If you give a plugin an alias string that matches the demangled symbol name
/// of another plugin, then the Loader will always prefer to instantiate the
/// plugin whose symbol name matches that string.
#define GZ_ADD_PLUGIN_ALIAS(PluginClass, ...) \
  DETAIL_GZ_ADD_PLUGIN_ALIAS(PluginClass, __VA_ARGS__)


/// \brief Add a plugin factory.
///
/// A plugin factory is a plugin that is able to generate objects that implement
/// some interface. These objects can be passed off to a consumer, and as long
/// as the object is alive, it will ensure that the shared library of the plugin
/// remains loaded. The objects are handed off with a std::unique_ptr, so the
/// raw pointer can be released from its std::unique_ptr and passed into any
/// memory management system the consumer prefers.
///
/// The inputs and output of a factory are defined using the
/// gz::plugin::Factory class in the gz/plugin/Factory.hh header.
///
/// The first argument of this macro should be the class that implements the
/// factory's output interface. The second argument should be the factory
/// definition.
///
/// NOTE: If your factory has any input arguments, then you must define it
/// outside of this macro, or else you will get a compilation error. This
/// happens because macros will parse the commas between your template arguments
/// as separators for the macro arguments. For example:
///
/// \code
/// class MyBase
/// {
///   public: virtual double SomeFunction() = 0;
/// };
///
/// class MyType : public MyBase
/// {
///   public: MyType(double value);
///   public: double SomeFunction() override;
/// };
///
/// /* BAD! Will not compile:
/// GZ_ADD_FACTORY(MyType, gz::plugin::Factory<MyBase, double>);
/// */
///
/// // Instead do this:
/// using MyFactory = gz::plugin::Factory<MyBase, double>;
/// GZ_ADD_FACTORY(MyType, MyFactory);
/// \endcode
#define GZ_ADD_FACTORY(ProductType, FactoryType) \
  DETAIL_GZ_ADD_FACTORY(ProductType, FactoryType)

/// \brief Add an alias for a factory.
///
/// This will do the same as GZ_ADD_FACTORY(), but you may also add in
/// any number of strings which can then be used as aliases for this factory.
/// For example:
///
/// \code
/// GZ_ADD_FACTORY_ALIAS(MyType, MyFactory, "Foo", "My favorite factory")
/// \endcode
///
/// This macro can be called any number of times for the same factory or for
/// different factories. If you call this macro, you do not need to call
/// GZ_ADD_FACTORY(), but there is nothing wrong with calling both (except
/// it might imperceptibly increase your compile time).
#define GZ_ADD_FACTORY_ALIAS(ProductType, FactoryType, ...) \
  DETAIL_GZ_ADD_FACTORY_ALIAS(ProductType, FactoryType, __VA_ARGS__)


#endif
