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


#ifndef GZ_PLUGIN_DETAIL_PLUGIN_HH_
#define GZ_PLUGIN_DETAIL_PLUGIN_HH_

#include <memory>
#include <string>
#include <gz/plugin/Plugin.hh>

namespace gz
{
  namespace plugin
  {
    //////////////////////////////////////////////////
    template <class Interface>
    Interface *Plugin::QueryInterface()
    {
      return static_cast<Interface*>(
            this->PrivateQueryInterface(typeid(Interface).name()));
    }

    //////////////////////////////////////////////////
    template <class Interface>
    const Interface *Plugin::QueryInterface() const
    {
      return static_cast<const Interface*>(
            this->PrivateQueryInterface(typeid(Interface).name()));
    }

    //////////////////////////////////////////////////
    template <class Interface>
    std::shared_ptr<Interface> Plugin::QueryInterfaceSharedPtr()
    {
      Interface *ptr = this->QueryInterface<Interface>();
      if (ptr)
        return std::shared_ptr<Interface>(this->PrivateGetInstancePtr(), ptr);

      return nullptr;
    }

    //////////////////////////////////////////////////
    template <class Interface>
    std::shared_ptr<const Interface> Plugin::QueryInterfaceSharedPtr() const
    {
      const Interface *ptr = this->QueryInterface<Interface>();
      if (ptr)
      {
        return std::shared_ptr<const Interface>(
              this->PrivateGetInstancePtr(), ptr);
      }

      return nullptr;
    }

    //////////////////////////////////////////////////
    template <class Interface>
    bool Plugin::HasInterface() const
    {
      return this->HasInterface(typeid(Interface).name(), false);
    }
  }
}

#endif
