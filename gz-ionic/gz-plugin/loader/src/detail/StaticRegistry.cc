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


#include <gz/plugin/utility.hh>
#include <gz/plugin/detail/StaticRegistry.hh>

namespace gz
{
  namespace plugin
  {
    /////////////////////////////////////////////////
    StaticRegistry &StaticRegistry::GetInstance()
    {
      static std::unique_ptr<StaticRegistry> instance(new StaticRegistry());
      return *instance;
    }

    /////////////////////////////////////////////////
    std::set<std::string> StaticRegistry::AllPlugins() const
    {
      std::set<std::string> result;

      for (const auto &entry : this->infos)
        result.insert(result.end(), entry.first);

      return result;
    }

    /////////////////////////////////////////////////
    bool StaticRegistry::AddInfo(const Info& _info)
    {
      InfoMap::iterator it;
      bool inserted;

      std::string pluginName = DemangleSymbol(_info.name);

      // We use insert to ensure that we do not accidentally overwrite some
      // existing information for the plugin that has this name.
      std::tie(it, inserted) = this->infos.insert(
          std::make_pair(pluginName, _info));

      if (inserted)
      {
        it->second.name = pluginName;
        for (const auto &interfaceMapEntry : _info.interfaces)
        {
          it->second.demangledInterfaces.insert(
              DemangleSymbol(interfaceMapEntry.first));
        }
      }
      else
      {
        // If the object was not inserted, then an entry already existed for
        // this plugin type. We should still insert each of the interface map
        // entries provided by the input info, just in case any of
        // them are missing from the currently existing entry. This allows the
        // user to specify different interfaces for the same plugin
        // type using different macros in different locations or across
        // multiple translation units.
        for (const auto &interfaceMapEntry : _info.interfaces)
        {
          it->second.interfaces.insert(interfaceMapEntry);
          it->second.demangledInterfaces.insert(
              DemangleSymbol(interfaceMapEntry.first));
        }

        // Add aliases
        for (const std::string &alias : _info.aliases)
          it->second.aliases.insert(alias);
      }

      for (const std::string &alias : _info.aliases)
        this->aliases[alias].insert(pluginName);

      return true;
    }

    /////////////////////////////////////////////////
    ConstInfoPtr StaticRegistry::GetInfo(
        const std::string &_pluginName) const
    {
      const InfoMap::const_iterator it = this->infos.find(_pluginName);
      if (this->infos.end() == it)
        return nullptr;
      return std::make_shared<Info>(it->second);
    }
  }
}
