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


#ifndef GZ_PLUGIN_DETAIL_LOADER_HH_
#define GZ_PLUGIN_DETAIL_LOADER_HH_

#include <memory>
#include <string>
#include <unordered_set>
#include <gz/plugin/EnablePluginFromThis.hh>
#include <gz/plugin/Loader.hh>

namespace gz
{
  namespace plugin
  {
    template <typename Interface>
    std::unordered_set<std::string> Loader::PluginsImplementing() const
    {
      return this->PluginsImplementing(typeid(Interface).name(), false);
    }

    template <typename PluginPtrType>
    PluginPtrType Loader::Instantiate(
        const std::string &_pluginNameOrAlias) const
    {
      // Higher priority for plugins loaded from file than from the static
      // registry.
      const std::string &resolvedNameForFilePlugin =
          this->PrivateLookupFilePlugin(_pluginNameOrAlias);

      const std::string &resolvedNameForStaticPlugin =
            this->PrivateLookupStaticPlugin(_pluginNameOrAlias);

      PluginPtr ptr;

      if (!resolvedNameForFilePlugin.empty())
      {
        ptr = PluginPtr(
            this->PrivateGetInfoForFilePlugin(resolvedNameForFilePlugin),
            this->PrivateGetPluginDlHandlePtr(resolvedNameForFilePlugin));
      }
      else if (!resolvedNameForStaticPlugin.empty())
      {
        ptr = PluginPtr(
            this->PrivateGetInfoForStaticPlugin(resolvedNameForStaticPlugin));
      }
      else
      {
        return PluginPtr();
      }

      if (auto *enableFromThis = ptr->QueryInterface<EnablePluginFromThis>())
        enableFromThis->PrivateSetPluginFromThis(ptr);

      return ptr;
    }

    template <typename InterfaceType>
    std::shared_ptr<InterfaceType> Loader::Factory(
        const std::string &_pluginNameOrAlias) const
    {
      return this->Instantiate(_pluginNameOrAlias)
          ->template QueryInterfaceSharedPtr<InterfaceType>();
    }
  }
}

#endif
