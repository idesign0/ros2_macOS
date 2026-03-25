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


#ifndef GZ_PLUGIN_DETAIL_COMMON_HH_
#define GZ_PLUGIN_DETAIL_COMMON_HH_

#include <set>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <utility>

#include <gz/plugin/EnablePluginFromThis.hh>
#include <gz/plugin/Info.hh>


namespace gz
{
  namespace plugin
  {
    namespace detail
    {
      //////////////////////////////////////////////////
      /// \brief This default will be called when NoMoreInterfaces is an empty
      /// parameter pack. When one or more Interfaces are provided, the other
      /// template specialization of this class will be called.
      template <typename PluginClass, typename... NoMoreInterfaces>
      struct InterfaceHelper
      {
        public: static void InsertInterfaces(Info::InterfaceCastingMap &)
        {
          // Do nothing. This is the terminal specialization of the variadic
          // template class member function.
        }
      };

      //////////////////////////////////////////////////
      /// \brief This specialization will be called when one or more Interfaces
      /// are specified.
      template <typename PluginClass, typename Interface,
                typename... RemainingInterfaces>
      struct InterfaceHelper<PluginClass, Interface, RemainingInterfaces...>
      {
        public: static void InsertInterfaces(
          Info::InterfaceCastingMap &interfaces)
        {
          // READ ME: If you get a compilation error here, then one of the
          // interfaces that you tried to register for your plugin is not
          // actually a base class of the plugin class. This is not allowed. A
          // plugin class must inherit every interface class that you want it to
          // provide.
          static_assert(std::is_base_of<Interface, PluginClass>::value,
                        "YOU ARE ATTEMPTING TO REGISTER AN INTERFACE FOR A "
                        "PLUGIN, BUT THE INTERFACE IS NOT A BASE CLASS OF THE "
                        "PLUGIN.");

          interfaces.insert(std::make_pair(
                typeid(Interface).name(),
                [=](void* v_ptr)
                {
                    PluginClass *d_ptr = static_cast<PluginClass*>(v_ptr);
                    return static_cast<Interface*>(d_ptr);
                }));

          InterfaceHelper<PluginClass, RemainingInterfaces...>
              ::InsertInterfaces(interfaces);
        }
      };

      //////////////////////////////////////////////////
      /// \brief This overload will be called when no more aliases remain to be
      /// inserted. If one or more aliases still need to be inserted, then the
      /// overload below this one will be called instead.
      inline void InsertAlias(std::set<std::string> &/*aliases*/)
      {
        // Do nothing. This is the terminal overload of the variadic template
        // function.
      }

      template <typename... Aliases>
      void InsertAlias(std::set<std::string> &aliases,
                       const std::string &nextAlias,
                       Aliases&&... remainingAliases)
      {
        aliases.insert(nextAlias);
        InsertAlias(aliases, std::forward<Aliases>(remainingAliases)...);
      }

      //////////////////////////////////////////////////
      template <typename PluginClass, bool DoEnablePluginFromThis>
      struct IfEnablePluginFromThisImpl
      {
        public: static void AddIt(Info::InterfaceCastingMap &_interfaces)
        {
          _interfaces.insert(std::make_pair(
                  typeid(EnablePluginFromThis).name(),
                  [=](void *v_ptr)
                  {
                    PluginClass *d_ptr = static_cast<PluginClass*>(v_ptr);
                    return static_cast<EnablePluginFromThis*>(d_ptr);
                  }));
        }
      };

      //////////////////////////////////////////////////
      template <typename PluginClass>
      struct IfEnablePluginFromThisImpl<PluginClass, false>
      {
        public: static void AddIt(Info::InterfaceCastingMap &)
        {
          // Do nothing, because the plugin does not inherit
          // the EnablePluginFromThis interface.
        }
      };

      //////////////////////////////////////////////////
      template <typename PluginClass>
      struct IfEnablePluginFromThis
          : IfEnablePluginFromThisImpl<PluginClass,
                std::is_base_of<EnablePluginFromThis, PluginClass>::value>
      { }; // NOLINT

      //////////////////////////////////////////////////
      template <typename PluginClass, typename... Interfaces>
      Info MakeInfo()
      {
        Info info;

        // Set the name of the plugin
        info.name = typeid(PluginClass).name();

        // Create a factory for generating new plugin instances
        info.factory = [=]()
        {
          // vvvvvvvvvvvvvvvvvvvvvvvv  READ ME  vvvvvvvvvvvvvvvvvvvvvvvvvvvvv
          // If you get a compilation error here, then you are trying to
          // register an abstract class as a plugin, which is not allowed. To
          // register a plugin class, every one if its virtual functions must
          // have a definition.
          //
          // Read through the error produced by your compiler to see which
          // pure virtual functions you are neglecting to provide overrides
          // for.
          // ^^^^^^^^^^^^^ READ ABOVE FOR COMPILATION ERRORS ^^^^^^^^^^^^^^^^
          return static_cast<void*>(new PluginClass);
        };

GZ_UTILS_WARN_IGNORE__NON_VIRTUAL_DESTRUCTOR
        // Create a deleter to clean up destroyed instances
        info.deleter = [=](void *ptr)
        {
          delete static_cast<PluginClass*>(ptr);
        };
GZ_UTILS_WARN_RESUME__NON_VIRTUAL_DESTRUCTOR

        // Construct a map from the plugin to its interfaces
        InterfaceHelper<PluginClass, Interfaces...>
            ::InsertInterfaces(info.interfaces);

        return info;
      }
    }
  }
}

#endif
