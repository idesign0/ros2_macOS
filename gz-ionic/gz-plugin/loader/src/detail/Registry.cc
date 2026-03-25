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


#include <iostream>
#include <sstream>

#include <gz/plugin/detail/Registry.hh>

namespace gz
{
  namespace plugin
  {
    /////////////////////////////////////////////////
    std::string Registry::PrettyStr() const
    {
      auto interfaces = this->InterfacesImplemented();
      std::stringstream pretty;
      pretty << "Registry state" << std::endl;
      pretty << "\tKnown Interfaces: " << interfaces.size() << std::endl;
      for (auto const &interface : interfaces)
        pretty << "\t\t" << interface << std::endl;

      std::set<std::string> pluginNames = this->AllPlugins();

      pretty << "\tKnown Plugins: " << pluginNames.size() << std::endl;
      for (const auto &name : pluginNames)
      {
        const ConstInfoPtr &plugin = this->GetInfo(name);
        const std::size_t aSize = plugin->aliases.size();

        pretty << "\t\t[" << plugin->name << "]\n";
        if (0 < aSize)
        {
          pretty << "\t\t\thas "
                 << aSize << (aSize == 1? " alias" : " aliases") << ":\n";
          for (const auto &alias : plugin->aliases)
            pretty << "\t\t\t\t[" << alias << "]\n";
        }
        else
        {
          pretty << "\t\t\thas no aliases\n";
        }

        const std::size_t iSize = plugin->interfaces.size();
        pretty << "\t\t\timplements " << iSize
               << (iSize == 1? " interface" : " interfaces") << ":\n";
        for (const auto &interface : plugin->demangledInterfaces)
          pretty << "\t\t\t\t" << interface << "\n";
      }

      AliasMap badAliases;
      for (const auto &entry : this->aliases)
      {
        if (entry.second.size() > 1)
        {
          badAliases.insert(entry);
        }
      }

      if (!badAliases.empty())
      {
        const std::size_t aSize = badAliases.size();
        pretty << "\tThere " << (aSize == 1? "is " : "are ")  << aSize
               << (aSize == 1? " alias" : " aliases") << " with a "
               << "name collision:\n";
        for (const auto &alias : badAliases)
        {
          pretty << "\t\t[" << alias.first << "] collides between:\n";
          for (const auto &name : alias.second)
            pretty << "\t\t\t[" << name << "]\n";
        }
      }

      pretty << std::endl;

      return pretty.str();
    }

    /////////////////////////////////////////////////
    std::unordered_set<std::string> Registry::InterfacesImplemented() const
    {
      std::set<std::string> pluginNames = this->AllPlugins();
      std::unordered_set<std::string> interfaces;
      for (auto const &name : pluginNames)
      {
        const ConstInfoPtr &plugin = this->GetInfo(name);
        for (auto const &interface : plugin->demangledInterfaces)
          interfaces.insert(interface);
      }
      return interfaces;
    }

    /////////////////////////////////////////////////
    std::unordered_set<std::string> Registry::PluginsImplementing(
        const std::string &_interface,
        const bool demangled) const
    {
      std::set<std::string> allPlugins = this->AllPlugins();
      std::unordered_set<std::string> availablePlugins;

      for (auto const &name : allPlugins)
      {
        const ConstInfoPtr &plugin = this->GetInfo(name);
        if (demangled)
        {
          if (plugin->demangledInterfaces.find(_interface) !=
              plugin->demangledInterfaces.end())
            availablePlugins.insert(plugin->name);
        }
        else
        {
          if (plugin->interfaces.find(_interface) !=
              plugin->interfaces.end())
            availablePlugins.insert(plugin->name);
        }
      }

      return availablePlugins;
    }

    /////////////////////////////////////////////////
    std::set<std::string> Registry::PluginsWithAlias(
        const std::string &_alias) const
    {
      std::set<std::string> result;

      const AliasMap::const_iterator names = this->aliases.find(_alias);

      if (names != this->aliases.end())
        result = names->second;

      ConstInfoPtr plugin = this->GetInfo(_alias);

      if (plugin != nullptr)
        result.insert(_alias);

      return result;
    }

    /////////////////////////////////////////////////
    std::set<std::string> Registry::AliasesOfPlugin(
        const std::string &_pluginName) const
    {
      ConstInfoPtr plugin = this->GetInfo(_pluginName);

      if (plugin != nullptr)
        return plugin->aliases;

      return {};
    }

    /////////////////////////////////////////////////
    std::string Registry::LookupPlugin(const std::string &_nameOrAlias) const
    {
      ConstInfoPtr pluginPtr = this->GetInfo(_nameOrAlias);

      if (pluginPtr != nullptr)
        return _nameOrAlias;

      const AliasMap::const_iterator alias = this->aliases.find(_nameOrAlias);
      if (this->aliases.end() != alias && !alias->second.empty())
      {
        if (alias->second.size() == 1)
          return *alias->second.begin();

        // We use a stringstream because we're going to output to std::cerr, and
        // we want it all to print at once, but std::cerr does not support
        // buffering.
        std::stringstream ss;

        ss << "[gz::plugin::Registry::LookupPlugin] Failed to resolve the "
           << "alias [" << _nameOrAlias << "] because it refers to multiple "
           << "plugins:\n";
        for (const std::string &plugin : alias->second)
          ss << " -- [" << plugin << "]\n";

        std::cerr << ss.str();

        return "";
      }

      return "";
    }

    /////////////////////////////////////////////////
    std::set<std::string> Registry::AllPlugins() const
    {
      std::set<std::string> result;

      for (const auto &entry : this->plugins)
        result.insert(result.end(), entry.first);

      return result;
    }

    /////////////////////////////////////////////////
    ConstInfoPtr Registry::GetInfo(const std::string &_pluginName) const {
      const PluginMap::const_iterator it = this->plugins.find(_pluginName);
      if (this->plugins.end() == it)
        return nullptr;
      return it->second;
    }

    /////////////////////////////////////////////////
    bool Registry::AddInfo(const Info &_info) {
      for (const std::string &alias : _info.aliases)
        this->aliases[alias].insert(_info.name);

      auto result = this->plugins.insert(
          std::make_pair(_info.name, std::make_shared<Info>(_info)));

      return result.second;
    }

    /////////////////////////////////////////////////
    void Registry::ForgetInfo(const std::string &_pluginName) {
      ConstInfoPtr info = this->GetInfo(_pluginName);
      if (info == nullptr)
        return;

      for (const std::string &alias : info->aliases)
          this->aliases.at(alias).erase(info->name);
      this->plugins.erase(_pluginName);
      return;
    }
  }
}
