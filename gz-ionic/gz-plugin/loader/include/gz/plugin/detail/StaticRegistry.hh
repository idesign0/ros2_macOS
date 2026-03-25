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


#ifndef GZ_PLUGIN_DETAIL_STATICREGISTRY_HH_
#define GZ_PLUGIN_DETAIL_STATICREGISTRY_HH_

#include <memory>
#include <set>
#include <string>

#include <gz/plugin/Info.hh>
#include <gz/plugin/detail/Registry.hh>
#include <gz/plugin/loader/Export.hh>
#include <gz/utils/SuppressWarning.hh>

namespace gz
{
  namespace plugin
  {
    /// \brief Static registry of plugin classes populated from
    /// gz/plugin/RegisterStatic.hh
    class GZ_PLUGIN_LOADER_VISIBLE StaticRegistry final: public Registry {
      /// \brief Get a reference to the StaticRegistry instance.
      public: static StaticRegistry& GetInstance();

      /// \brief Get a set of the names of all plugins that are in this
      /// registry.
      ///
      /// \return A set of all plugin names in this registry.
      public: virtual std::set<std::string> AllPlugins() const override;

      /// \brief Get info as ConstInfoPtr.
      ///
      /// \param[in] _pluginName
      ///   Name of the plugin as returned by LookupPlugin(~).
      public: virtual ConstInfoPtr GetInfo(
          const std::string &_pluginName) const override;

      /// \brief Register Info for a new plugin.
      ///
      /// This happens automatically when the macros defined in
      /// gz/plugin/RegisterStatic.hh are called in a plugin library. So this
      /// method is assumed to be called only during program start.
      ///
      /// \param[in] _info
      ///   Info for a plugin class.
      ///
      /// \return Always returns true.
      public: virtual bool AddInfo(const Info &_info) override;

      /// \brief This function is a no-op for the static registry.
      ///
      /// Registered plugins cannot be removed.
      ///
      /// \param[in] _pluginName
      ///   Name of the plugin as returned by LookupPlugin(~).
      public: virtual void ForgetInfo(
          const std::string &/*_pluginName*/) override { }

      /// \brief Constructor
      protected: StaticRegistry() = default;

      /// \brief Holds info required to construct a plugin
      GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
      private: InfoMap infos;
      GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
    };
  }
}

#endif
