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


#ifndef GZ_PLUGIN_SPECIALIZEDPLUGIN_HH_
#define GZ_PLUGIN_SPECIALIZEDPLUGIN_HH_

#include <memory>
#include "gz/plugin/Plugin.hh"

namespace gz
{
  namespace plugin
  {
    // Forward declarations
    namespace detail {
      template <class, class> class ComposePlugin;
      template <class> class SelectSpecializers;
    }

    // Forward declaration of the variadic template SpecializedPlugin class.
    template <class... OtherSpecInterfaces>
    class SpecializedPlugin;

    /// \brief This class allows Plugin instances to have high-speed access to
    /// interfaces that can be anticipated at compile time. The plugin does
    /// not have to actually offer the specialized interface in order to get
    /// this performance improvement. This template is variadic, so it can
    /// support arbitrarily many interfaces.
    ///
    /// Usage example:
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
    /// will have extremely high-speed associated with it. It will provide
    /// direct access to the the `FooInterface*` of `plugin`. If `plugin` does
    /// not actually offer `FooInterface`, then it will return a nullptr, still
    /// at extremely high speed.
    template <class SpecInterface>
    class SpecializedPlugin<SpecInterface> : public virtual Plugin
    {
      // -------------------- Public API ---------------------

      // Inherit function overloads
      public: using Plugin::QueryInterface;
      public: using Plugin::QueryInterfaceSharedPtr;
      public: using Plugin::HasInterface;

      // Documentation inherited
      public: template <class Interface>
              Interface *QueryInterface();

      // Documentation inherited
      public: template <class Interface>
              const Interface *QueryInterface() const;

      // Documentation inherited
      public: template <class Interface>
              std::shared_ptr<Interface> QueryInterfaceSharedPtr();

      // Documentation inherited
      public: template <class Interface>
              std::shared_ptr<const Interface> QueryInterfaceSharedPtr() const;

      // Documentation inherited
      public: template <class Interface>
              bool HasInterface() const;


      // -------------------- Private API ---------------------

      // Declare friendship
      template <class...> friend class SpecializedPlugin;
      template <class, class> friend class detail::ComposePlugin;
      template <class> friend class detail::SelectSpecializers;
      template <class> friend class TemplatePluginPtr;

      /// \brief type is an empty placeholder class which is used by the private
      /// member functions to provide two overloads: a high-performance one for
      /// the specialized interface, and a normal-performance one for all other
      /// Interface types.
      private: template <class T> struct type { };

      /// \brief Delegate the function to the standard Plugin method
      /// \param[in] _type Empty object meant to guide the compiler to pick the
      /// desired implementation.
      /// \return Pointer to the intergace
      private: template <class Interface>
               Interface *PrivateQueryInterface(type<Interface>);

      /// \brief Use a high-speed accessor to provide this specialized interface
      /// \param[in] _type Empty object meant to guide the compiler to pick the
      /// desired implementation.
      /// \return Pointer to the specialized intergace
      private: SpecInterface *PrivateQueryInterface(type<SpecInterface>);

      /// \brief Delegate the function to the standard Plugin method
      /// \param[in] _type Empty object meant to guide the compiler to pick the
      /// desired implementation.
      /// \return Pointer to the specialized intergace
      private: template <class Interface>
               const Interface *PrivateQueryInterface(type<Interface>) const;

      /// \brief Use a high-speed accessor to provide this specialized interface
      /// \param[in] _type Empty object meant to guide the compiler to pick the
      /// desired implementation.
      /// \return Pointer to the specialized intergace
      private: const SpecInterface *PrivateQueryInterface(
                   type<SpecInterface>) const;

      /// \brief Delegate the function to the standard PluginPtr method
      /// \param[in] _type Empty object meant to guide the compiler to pick the
      /// desired implementation.
      /// \return True if the interface is present.
      private: template <class Interface>
               bool PrivateHasInterface(type<Interface>) const;

      /// \brief Use a high-speed accessor to check this specialized interface
      /// \param[in] _type Empty object meant to guide the compiler to pick the
      /// desired implementation.
      /// \return True if the interface is present.
      private: bool PrivateHasInterface(type<SpecInterface>) const;

      // Dev note (MXG): The privateSpecializedInterfaceIterator object must be
      // available to the user during their compile time, so it cannot be hidden
      // using PIMPL. The iterator is const because it must always point to the
      // same entry throughout its entire lifecycle.
      /// \brief Iterator that points to the entry of the specialized interface
      private:
      const Plugin::InterfaceMap::iterator privateSpecializedInterfaceIterator;

      /// \brief Default constructor
      protected: SpecializedPlugin();
    };
  }
}

#include "gz/plugin/detail/SpecializedPlugin.hh"

#endif
