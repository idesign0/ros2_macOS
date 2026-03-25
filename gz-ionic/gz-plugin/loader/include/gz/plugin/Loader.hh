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


#ifndef GZ_PLUGIN_LOADER_HH_
#define GZ_PLUGIN_LOADER_HH_

#include <memory>
#include <set>
#include <string>
#include <typeinfo>
#include <unordered_set>

#include <gz/utils/SuppressWarning.hh>

#include <gz/plugin/loader/Export.hh>
#include <gz/plugin/PluginPtr.hh>

namespace gz
{
  namespace plugin
  {
    /// \brief Class for loading plugins
    class GZ_PLUGIN_LOADER_VISIBLE Loader
    {
      /// \brief Constructor
      public: Loader();

      /// \brief Destructor
      public: ~Loader();

      /// \brief Makes a printable string with info about plugins
      ///
      /// \returns A pretty string
      public: std::string PrettyStr() const;

      /// \brief Get demangled names of interfaces that the loader has plugins
      /// for.
      ///
      /// \returns Demangled names of the interfaces that are implemented
      public: std::unordered_set<std::string> InterfacesImplemented() const;

      /// \brief Get plugin names that implement the specified interface
      ///
      /// \return names of plugins that implement the interface.
      public: template <typename Interface>
      std::unordered_set<std::string> PluginsImplementing() const;

      /// \brief Get plugin names that implement the specified interface string.
      /// Note that the templated version of this function is recommended
      /// instead of this version to avoid confusion about whether a mangled or
      /// demangled version of a string is being used. Note that the function
      /// InterfacesImplemented() returns demangled versions of the interface
      /// names.
      ///
      /// If you want to pass in a mangled version of an interface name, e.g.
      /// the result that would be produced by typeid(T).name(), then set
      /// `demangled` to false.
      ///
      /// \param[in] _interface
      ///   Name of an interface
      ///
      /// \param[in] _demangled
      ///   Specify whether the _interface string is demangled (default, true)
      ///   or mangled (false).
      ///
      /// \returns Names of plugins that implement the interface
      public: std::unordered_set<std::string> PluginsImplementing(
          const std::string &_interface,
          const bool _demangled = true) const;

      /// \brief Get a set of the names of all plugins that are currently known
      /// to this Loader.
      /// \return A set of all plugin names known to this Loader.
      public: std::set<std::string> AllPlugins() const;

      /// \brief Get plugin names that correspond to the specified alias string.
      ///
      /// If there is more than one entry in this set, then the alias cannot be
      /// used to instantiate any of those plugins.
      ///
      /// If the name of a plugin matches the alias string, then that plugin
      /// will be instantiated any time the string is used to instantiate a
      /// plugin, no matter how many other plugins use the alias.
      ///
      /// \param[in] _alias
      ///   The std::string of the alias
      ///
      /// \return A set of plugins that correspond to the desired alias
      public: std::set<std::string> PluginsWithAlias(
          const std::string &_alias) const;

      /// \brief Get the aliases of the plugin with the given name
      ///
      /// \param[in] _pluginName
      ///   The name of the desired plugin
      ///
      /// \return A set of aliases corresponding to the desired plugin
      public: std::set<std::string> AliasesOfPlugin(
          const std::string &_pluginName) const;

      /// \brief Resolve the plugin name or alias into the name of the plugin
      /// that it maps to. If this is a name or alias that does not uniquely map
      /// to a known plugin, then the return value will be an empty string.
      ///
      /// \param[in] _nameOrAlias
      ///   The name or alias of the plugin of interest.
      ///
      /// \return The name of the plugin being referred to, or an empty string
      /// if no such plugin is known.
      public: std::string LookupPlugin(const std::string &_nameOrAlias) const;

      /// \brief Load a library at the given path
      ///
      /// \param[in] _pathToLibrary
      ///   The path to a library
      ///
      /// \returns The set of plugins that have been loaded from the library
      public: std::unordered_set<std::string> LoadLib(
                  const std::string &_pathToLibrary);

      /// \brief Load a library at the given path
      ///
      /// \param[in] _pathToLibrary
      ///   The path to a library
      /// \param[in] _noDelete
      ///   If true, RTLD_NODELETE will be used when loading the library.
      ///
      /// \returns The set of plugins that have been loaded from the library
      public: std::unordered_set<std::string> LoadLib(
                  const std::string &_pathToLibrary, bool _noDelete);

      /// \brief Instantiates a plugin for the given plugin name
      ///
      /// \param[in] _pluginNameOrAlias
      ///   Name or alias of the plugin to instantiate.
      ///
      /// \returns Pointer to instantiated plugin
      public: PluginPtr Instantiate(
          const std::string &_pluginNameOrAlias) const;

      /// \brief Instantiates a plugin of PluginType for the given plugin name.
      /// This can be used to create a specialized PluginPtr.
      ///
      /// \tparam PluginPtrType
      ///   The specialized type of PluginPtrPtr that you
      ///   want to construct.
      ///
      /// \param[in] _pluginNameOrAlias
      ///   Name or alias of the plugin that you want to instantiate.
      ///
      /// \returns pointer for the instantiated PluginPtr
      public: template <typename PluginPtrType>
      PluginPtrType Instantiate(const std::string &_pluginNameOrAlias) const;

      /// \brief Instantiates a plugin for the given plugin name, and then
      /// returns a reference-counting interface corresponding to InterfaceType.
      ///
      /// If you use this function to retrieve a Factory, you can call
      /// Construct(...) on the returned interface, as long as the returned
      /// interface is not a nullptr.
      ///
      /// \remark This function is identical to:
      ///
      /// \code
      /// loader->Instantiate(_pluginNameOrAlias)
      ///   ->QueryInterfaceSharedPtr<InterfaceType>();
      /// \endcode
      ///
      /// \tparam InterfaceType
      ///   The type of interface to look for. This function is meant for
      ///   producing Factories, but any type of Interface can be requested.
      ///
      /// \param[in] _pluginNameOrAlias
      ///   Name or alias of the plugin that you want to use for production.
      ///
      /// \return reference to an InterfaceType if it can be provided by the
      /// requested plugin.
      public: template <typename InterfaceType>
      std::shared_ptr<InterfaceType> Factory(
          const std::string &_pluginNameOrAlias) const;

      /// \brief This loader will forget about the library at the given path
      /// location. If you want to instantiate a plugin from this library using
      /// this loader, you will first need to call LoadLib again.
      ///
      /// After this function has been called, once all plugin instances that
      /// are tied to the library have been deleted, the library will
      /// automatically be unloaded from the executable. Note that when this
      /// Loader leaves scope (or gets deleted), it will have the same
      /// effect as calling ForgetLibrary on all of the libraries that it
      /// loaded, so there is generally no need to call this function. However,
      /// it may be useful if you want to reduce clutter in the Loader
      /// instance or let go of library resources that are no longer being used.
      ///
      /// Note that even if you have released all references to a library, it is
      /// still up to the discretion of your operating system whether (or when)
      /// that library will be unloaded. In some cases, the operating system
      /// might not choose to unload it until the program exits completely.
      ///
      /// \param[in] _pathToLibrary
      ///   Path to the library that you want to forget
      ///
      /// \return True if the library was actively loaded and is now
      /// successfully forgotten. If the library was not actively loaded, this
      /// returns false.
      public: bool ForgetLibrary(const std::string &_pathToLibrary);

      /// \brief Forget the library that provides the plugin with the given
      /// name. Note that this will also forget all other plugin types which
      /// are provided by that library.
      ///
      /// \param[in] _pluginNameOrAlias
      ///   Name or alias of the plugin whose library you want to forget.
      ///
      /// \sa bool ForgetLibrary(const std::string &_pathToLibrary)
      public: bool ForgetLibraryOfPlugin(const std::string &_pluginNameOrAlias);

      /// \brief Specifically look up a plugin loaded from file.
      ///
      /// \param[in] _nameOrAlias
      ///   The name or alias of the plugin of interest.
      ///
      /// \return The name of the plugin being referred to, or an empty string
      /// if no such plugin is known.
      private: std::string PrivateLookupFilePlugin(
          const std::string &_nameOrAlias) const;

      /// \brief Get a pointer to the Info corresponding to _pluginName for a
      /// plugin loaded from file.
      ///
      /// \param[in] _resolvedName
      ///   The resolved name, i.e. the demangled class symbol name as returned
      ///   by PrivateLookupFilePlugin(~), of the plugin that you want to
      ///   instantiate.
      ///
      /// \return Pointer to the corresponding Info, or nullptr if there
      /// is no info for the requested _pluginName.
      private: ConstInfoPtr PrivateGetInfoForFilePlugin(
          const std::string &_resolvedName) const;

      /// \brief Specifically look up a plugin in the static registry.
      ///
      /// \param[in] _nameOrAlias
      ///   The name or alias of the plugin of interest.
      ///
      /// \return The name of the plugin being referred to, or an empty string
      /// if no such plugin is known.
      private: std::string PrivateLookupStaticPlugin(
          const std::string &_nameOrAlias) const;

      /// \brief Get a pointer to the Info corresponding to _pluginName for a
      /// plugin loaded from the static registry.
      ///
      /// \param[in] _resolvedName
      ///   The resolved name, i.e. the demangled class symbol name as returned
      ///   by PrivateLookupStaticPlugin(~), of the plugin that you want to
      ///   instantiate.
      ///
      /// \return Pointer to the corresponding Info, or nullptr if there
      /// is no info for the requested _pluginName.
      private: ConstInfoPtr PrivateGetInfoForStaticPlugin(
          const std::string &_resolvedName) const;

      /// \brief Get a std::shared_ptr that manages the lifecycle of the shared
      /// library handle which provides the specified plugin.
      ///
      /// \param[in] _resolvedName
      ///   The resolved name, i.e. the demangled class symbol name as returned
      ///   by LookupPlugin(~), of the plugin that you want to instantiate.
      ///
      /// \return Reference-counting pointer to a library handle, or else a
      /// nullptr if the plugin is not available.
      private: std::shared_ptr<void> PrivateGetPluginDlHandlePtr(
          const std::string &_resolvedName) const;

      class Implementation;
      GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \brief PIMPL pointer to class implementation
      private: std::unique_ptr<Implementation> dataPtr;
      GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
    };
  }
}

#include <gz/plugin/detail/Loader.hh>

#endif
