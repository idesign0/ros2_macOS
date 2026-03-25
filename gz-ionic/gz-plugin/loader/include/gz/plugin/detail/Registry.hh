/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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


#ifndef GZ_PLUGIN_DETAIL_REGISTRY_HH_
#define GZ_PLUGIN_DETAIL_REGISTRY_HH_

#include <map>
#include <set>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>

#include <gz/plugin/Info.hh>
#include <gz/plugin/loader/Export.hh>
#include <gz/utils/SuppressWarning.hh>

namespace gz
{
  namespace plugin
  {
    /// \brief Manages a set of plugin Infos and allows querying by name or
    /// by alias.
    class GZ_PLUGIN_LOADER_VISIBLE Registry {
      /// \brief Constructor
      public: Registry() = default;

      /// \brief Destructor
      public: virtual ~Registry() = default;

      /// \brief Makes a printable string with info about plugins
      ///
      /// \returns A pretty string
      public: std::string PrettyStr() const;

      /// \brief Get demangled names of interfaces that the registry has plugins
      /// for.
      ///
      /// \returns Demangled names of the interfaces that are implemented
      public: std::unordered_set<std::string> InterfacesImplemented() const;

      /// \brief Get plugin names that implement the specified interface
      ///
      /// \return names of plugins that implement the interface.
      public: template <typename Interface>
      std::unordered_set<std::string> PluginsImplementing() const
      {
        return this->PluginsImplementing(typeid(Interface).name(), false);
      }

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

      /// \brief Get a set of the names of all plugins that are in this
      /// registry.
      ///
      /// Method is virtual to allow other Info storage and memory management
      /// models.
      ///
      /// \return A set of all plugin names in this registry.
      public: virtual std::set<std::string> AllPlugins() const;

      /// \brief Get info as ConstInfoPtr.
      ///
      /// Method is virtual to allow other Info storage and memory management
      /// models.
      ///
      /// \param[in] _pluginName
      ///   Name of the plugin as returned by LookupPlugin(~).
      public: virtual ConstInfoPtr GetInfo(
          const std::string &_pluginName) const;

      /// \brief Add a new plugin info.
      ///
      /// Tracked aliases in this registry are also updated.
      ///
      /// \param[in] _info
      ///   Plugin info to add.
      ///
      /// \return True if the info was added, false if a plugin with this name
      ///   already exists in the registry.
      public: virtual bool AddInfo(const Info &_info);

      /// \brief Forget a plugin info.
      ///
      /// Tracked aliases in this registry are also updated.
      ///
      /// \param[in] _pluginName
      ///   Name of the plugin as returned by LookupPlugin(~).
      public: virtual void ForgetInfo(const std::string &_pluginName);

      /// \brief Deleted copy constructor
      public: Registry(const Registry&) = delete;

      /// \brief Deleted copy assignment operator
      public: Registry& operator=(Registry&) = delete;

      GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
      protected: using AliasMap = std::map<std::string, std::set<std::string>>;
      /// \brief A map from known alias names to the plugin names that they
      /// correspond to. Since an alias might refer to more than one plugin,
      /// the key of this map is a set.
      protected: AliasMap aliases;

      protected: using PluginMap =
          std::unordered_map<std::string, ConstInfoPtr>;
      /// \brief A map from known plugin names to their Info.
      protected: PluginMap plugins;
      GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
    };
  }
}

#endif
