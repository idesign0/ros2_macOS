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


#ifndef GZ_PLUGIN_DETAIL_REGISTERSTATIC_HH_
#define GZ_PLUGIN_DETAIL_REGISTERSTATIC_HH_

#include <utility>

#include <gz/plugin/detail/StaticRegistry.hh>
#include <gz/plugin/detail/Common.hh>

namespace gz
{
  namespace plugin
  {
    namespace detail
    {
      //////////////////////////////////////////////////
      /// \brief This specialization of the Register class will be called when
      /// one or more arguments are provided to the GZ_ADD_STATIC_PLUGIN(~)
      /// macro.
      ///
      /// Calling Register() creates an Info object for the plugin which
      /// includes a factory function to instantiate the plugin and a deleter
      /// function to delete the plugin instance with the proper derived class
      /// destructor. The Info object also contains a map of all the interfaces
      /// that are registered for the plugin and the aliases that the plugin
      /// may be referred by. The Info object is then stored in the global
      /// static plugin registry which may be accessed through the
      /// StaticLoader.
      template <typename PluginClass, typename... Interfaces>
      struct StaticRegistrar
      {
        /// \brief This function registers a plugin along with a set of
        /// interfaces that it provides.
        public: static void Register() {
          // Make all info that the user has specified
          Info info = MakeInfo<PluginClass, Interfaces...>();

          // Add the EnablePluginFromThis interface automatically if it is
          // inherited by PluginClass.
          IfEnablePluginFromThis<PluginClass>::AddIt(info.interfaces);

          // Send this information as input to the global static plugin
          // registry.
          gz::plugin::StaticRegistry::GetInstance().AddInfo(info);
        }

        public: template <typename... Aliases>
        static void RegisterAlias(Aliases &&...aliases) {
          // Dev note (MXG): We expect the RegisterAlias function to be called
          // using the GZ_ADD_PLUGIN_ALIAS(~) macro, which should never
          // contain any interfaces. Therefore, this parameter pack should be
          // empty.
          //
          // In the future, we could allow Interfaces and Aliases to be
          // specified simultaneously, but that would be very tricky to do with
          // macros, so for now we will enforce this assumption to make sure
          // that the implementation is working as expected.
          static_assert(sizeof...(Interfaces) == 0,
                        "THERE IS A BUG IN THE ALIAS REGISTRATION "
                        "IMPLEMENTATION! PLEASE REPORT THIS!");

          Info info = MakeInfo<PluginClass, Interfaces...>();

          // Gather up all the aliases that have been specified for this plugin.
          InsertAlias(info.aliases, std::forward<Aliases>(aliases)...);

          // Send this information as input to the global static plugin
          // registry.
          gz::plugin::StaticRegistry::GetInstance().AddInfo(info);
        }
      };
    }
  }
}

//////////////////////////////////////////////////
/// This macro creates a uniquely-named class whose constructor calls the
/// gz::plugin::detail::StaticRegistrar::RegisterAlias function. It then
/// declares a uniquely-named instance of the class with static lifetime. When
/// it is constructed at program start, the Register function will be called.
#define DETAIL_GZ_ADD_STATIC_PLUGIN_HELPER(UniqueID, ...) \
  namespace gz \
  { \
    namespace plugin \
    { \
      namespace \
      { \
        struct RegisterStaticPlugin##UniqueID \
        { \
          RegisterStaticPlugin##UniqueID() \
          { \
            ::gz::plugin::detail::StaticRegistrar<__VA_ARGS__>::Register(); \
          } \
        }; \
  \
        static RegisterStaticPlugin##UniqueID execute##UniqueID; \
      } /* namespace */ \
    } \
  }


//////////////////////////////////////////////////
/// This macro is needed to force the __COUNTER__ macro to expand to a value
/// before being passed to the *_HELPER macro.
#define DETAIL_GZ_ADD_STATIC_PLUGIN_WITH_COUNTER(UniqueID, ...) \
  DETAIL_GZ_ADD_STATIC_PLUGIN_HELPER(UniqueID, __VA_ARGS__)


//////////////////////////////////////////////////
/// We use the __COUNTER__ here to give each plugin registration its own unique
/// name, which is required in order to statically initialize each one.
#define DETAIL_GZ_ADD_STATIC_PLUGIN(...) \
  DETAIL_GZ_ADD_STATIC_PLUGIN_WITH_COUNTER(__COUNTER__, __VA_ARGS__)


//////////////////////////////////////////////////
/// This macro creates a uniquely-named class whose constructor calls the
/// gz::plugin::detail::StaticRegistrar::RegisterAlias function. It then
/// declares a uniquely-named instance of the class with static lifetime. When
/// it is constructed at program start, the Register function will be called.
#define DETAIL_GZ_ADD_STATIC_PLUGIN_ALIAS_HELPER(UniqueID, PluginClass, ...) \
  namespace gz \
  { \
    namespace plugin \
    { \
      namespace \
      { \
        struct RegisterStaticPlugin##UniqueID \
        { \
          RegisterStaticPlugin##UniqueID() \
          { \
            ::gz::plugin::detail::StaticRegistrar<PluginClass>::RegisterAlias( \
                __VA_ARGS__); \
          } \
        }; \
  \
        static RegisterStaticPlugin##UniqueID register##UniqueID; \
      } /* namespace */ \
    } \
  }


//////////////////////////////////////////////////
/// This macro is needed to force the __COUNTER__ macro to expand to a value
/// before being passed to the *_HELPER macro.
#define DETAIL_GZ_ADD_STATIC_PLUGIN_ALIAS_WITH_COUNTER( \
  UniqueID, PluginClass, ...) \
  DETAIL_GZ_ADD_STATIC_PLUGIN_ALIAS_HELPER(UniqueID, PluginClass, __VA_ARGS__)


//////////////////////////////////////////////////
/// We use the __COUNTER__ here to give each plugin registration its own unique
/// name, which is required in order to statically initialize each one.
#define DETAIL_GZ_ADD_STATIC_PLUGIN_ALIAS(PluginClass, ...) \
  DETAIL_GZ_ADD_STATIC_PLUGIN_ALIAS_WITH_COUNTER( \
  __COUNTER__, PluginClass, __VA_ARGS__)

#endif
