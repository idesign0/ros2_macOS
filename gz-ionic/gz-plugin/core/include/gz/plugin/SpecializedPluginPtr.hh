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


#ifndef GZ_PLUGIN_SPECIALIZEDPLUGINPTR_HH_
#define GZ_PLUGIN_SPECIALIZEDPLUGINPTR_HH_

#include "gz/plugin/PluginPtr.hh"
#include "gz/plugin/SpecializedPlugin.hh"

namespace gz
{
  namespace plugin
  {
    /// \brief This alias allows PluginPtr instances to have high-speed access
    /// to interfaces that can be anticipated at compile time. The plugin does
    /// not have to actually offer the specialized interface in order to get
    /// this performance improvement. This template is variadic, so it can
    /// support arbitrarily many interfaces.
    ///
    /// Usage example:
    ///
    /// Suppose you want to instantiate a plugin that might (or might not) have
    /// some combination of four interfaces which are known at compile time:
    /// `MyInterface1`, `FooInterface`, `MyInterface2`, and `BarInterface`. You
    /// can use SpecializedPluginPtr as shown here:
    ///
    /// \code
    ///     using MySpecialPluginPtr = SpecializedPluginPtr<
    ///         MyInterface1, FooInterface, MyInterface2, BarInterface>;
    ///
    ///     MySpecialPluginPtr plugin = loader->Instantiate(pluginName);
    /// \endcode
    ///
    /// Then, calling the function
    ///
    /// \code
    ///     plugin->QueryInterface<FooInterface>();
    /// \endcode
    ///
    /// will have extremely high speed associated with it. It will provide
    /// direct access to the the `FooInterface*` of `plugin`. If `plugin` does
    /// not actually offer `FooInterface`, then it will return a nullptr, still
    /// at extremely high speed.
    ///
    /// This same rule also applies to `MyInterface1`, `MyInterface2`, and
    /// `BarInterface`, because those interfaces were also provided to
    /// SpecializedPluginPtr<...> at compile time.
    template <typename... SpecInterfaces>
    using SpecializedPluginPtr =
              TemplatePluginPtr< SpecializedPlugin<SpecInterfaces...> >;

    /// \brief This alias creates a specialized PluginPtr whose interfaces are
    /// all const-qualified.
    template <typename... SpecInterfaces>
    using ConstSpecializedPluginPtr =
              TemplatePluginPtr< const SpecializedPlugin<SpecInterfaces...> >;
  }
}

#endif
