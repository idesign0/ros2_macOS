/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#ifndef GZ_PLUGIN_ENABLEPLUGINFROMTHIS_HH_
#define GZ_PLUGIN_ENABLEPLUGINFROMTHIS_HH_

#include <memory>

#include <gz/plugin/PluginPtr.hh>

namespace gz
{
  namespace plugin
  {
    // Forward declaration
    class Loader;

    /// \brief EnablePluginFromThis is an optional base class which can be
    /// inherited by Plugin classes. When a Plugin class inherits it and that
    /// Plugin class is instantiated using the Loader class, its instance will
    /// be able to access the PluginPtr that manages its lifecycle. This
    /// interface will also be available by calling
    /// `instance->QueryInterface<EnablePluginFromThis>()`
    ///
    /// \remark This class is analogous to std::enable_shared_from_this
    class GZ_PLUGIN_VISIBLE EnablePluginFromThis
    {
      /// \brief Default constructor
      public: EnablePluginFromThis();

      /// \brief Get a copy of the PluginPtr that manages this object.
      /// \return a copy of the PluginPtr that manages this object.
      public: PluginPtr PluginFromThis();

      /// \brief Get a const-qualified copy of the PluginPtr that manages this
      /// object.
      /// \return a const-qualified copy of the PluginPtr that manages this
      /// object.
      public: ConstPluginPtr PluginFromThis() const;

      /// \brief Destructor
      public: virtual ~EnablePluginFromThis();

      /// \brief This is an abstract shared_ptr to the Plugin instance. This
      /// shared_ptr will maintain a reference count on the shared library that
      /// provides the plugin so that the shared library does not get unloaded
      /// while this shared_ptr is still alive.
      ///
      /// This function is meant for advanced use-cases, such as creating
      /// Factory plugins, so we keep it protected. Only use this if you know
      /// what you are doing.
      ///
      /// \return shared_ptr to the Plugin instance.
      protected: std::shared_ptr<void> PluginInstancePtrFromThis() const;

      // Declare friendship so that the internal WeakPluginPtr can be set by
      // the Loader class.
      friend class Loader;

      /// \brief This function is called by the Loader class whenever a plugin
      /// containing this interface gets instantiated.
      private: void PrivateSetPluginFromThis(const PluginPtr &_ptr);

      private: class Implementation;
      GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
      private: std::unique_ptr<Implementation> pimpl;
      GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
    };
  }
}

#endif
