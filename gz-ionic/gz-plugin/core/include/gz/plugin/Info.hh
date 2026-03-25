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


#ifndef GZ_PLUGIN_INFO_HH_
#define GZ_PLUGIN_INFO_HH_

#include <functional>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

#include <gz/utils/SuppressWarning.hh>

#include <gz/plugin/Export.hh>

namespace gz
{
  namespace plugin
  {
    /// \brief sentinel value to check if a plugin was built with the same
    /// version of the Info struct
    //
    /// This must be incremented when the Info struct changes
    const int INFO_API_VERSION = 1;

    // We use an inline namespace to assist in forward-compatibility. Eventually
    // we may want to support a version-2 of the Info API, in which case
    // we will remove the "inline" declaration here, and create a new inline
    // namespace called "v2". This original Info object will continue to
    // be accessible for backwards compatibility, and even its symbol name in
    // the ABI should remain the same.
    inline namespace v1
    {
      /// \brief Holds info required to construct a plugin
      struct GZ_PLUGIN_VISIBLE Info
      {
        /// \brief Clear out all information contained in this Info object
        void Clear();

        /// \brief The name of the plugin
        GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
        std::string name;
        GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING

        /// \brief Alternative names that may be used to instantiate the plugin
        GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
        std::set<std::string> aliases;
        GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING

        /// \brief The keys are the names of the types of interfaces that this
        /// plugin provides. The values are functions that convert a void
        /// pointer (which actually points to the plugin instance) to another
        /// void pointer (which actually points to the location of the interface
        /// within the plugin instance).
        GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
        using InterfaceCastingMap =
            std::unordered_map< std::string, std::function<void*(void*)> >;
        InterfaceCastingMap interfaces;
        GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING

        /// \brief This is a set containing the demangled versions of the names
        /// of the interfaces provided by this plugin. This gets filled in by
        /// the Loader after receiving the Info. It is only used by
        /// the user-facing API. Internally, when looking up Interfaces, the
        /// mangled `interfaces` map will still be used.
        GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
        std::set<std::string> demangledInterfaces;
        GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING

        /// \brief A method that instantiates a new instance of a plugin
        GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
        std::function<void*()> factory;
        GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING

        /// \brief A method that safely deletes an instance of the plugin
        GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
        std::function<void(void*)> deleter;
        GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
      };
    }

    /// This typedef is used simultaneously by detail/Register.hh and Loader.cc,
    /// so we store it in a location that is visible to both of them.
    using InfoMap = std::unordered_map<std::string, Info>;
    using ConstInfoPtr = std::shared_ptr<const Info>;
  }
}

#endif
