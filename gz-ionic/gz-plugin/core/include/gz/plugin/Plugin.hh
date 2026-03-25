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


#ifndef GZ_PLUGIN_PLUGIN_HH_
#define GZ_PLUGIN_PLUGIN_HH_

#include <memory>
#include <map>
#include <string>

#include <gz/utils/SuppressWarning.hh>

#include <gz/plugin/Export.hh>
#include <gz/plugin/Info.hh>

namespace gz
{
  namespace plugin
  {
    // Forward declaration
    namespace detail {
      template <class, class> class ComposePlugin;
      template <class> class SelectSpecializers;
    }
    class EnablePluginFromThis;
    class WeakPluginPtr;

    class GZ_PLUGIN_VISIBLE Plugin
    {
      // -------------------- Public API ---------------------

      /// \brief Get an interface of the specified type, if it is provided by
      /// this plugin.
      ///
      /// Note that the interface pointer you receive is owned by the Plugin
      /// object. You MUST NOT ever try to deallocate it yourself. Moreover, the
      /// pointer will be invalidated once all Plugin objects that refer to the
      /// same Plugin instance are destructed. Use the QueryInterfaceSharedPtr
      /// function in order to get a reference-counting pointer to an interface
      /// of this Plugin object. The pointer will remain valid as long as the
      /// std::shared_ptr provided by QueryInterfaceSharedPtr is alive.
      ///
      /// \return A raw pointer to the specified interface. If the requested
      /// _interfaceName is not provided by this Plugin, this returns a nullptr.
      /// This pointer is invalidated when the reference count of the plugin
      /// instance drops to zero.
      public: template <class Interface>
              Interface *QueryInterface();

      /// \brief const-qualified version of QueryInterface<Interface>()
      public: template <class Interface>
              const Interface *QueryInterface() const;

      /// \brief Get the requested interface as a std::shared_ptr. The template
      /// argument Interface must exactly match the underlying type associated
      /// with _interfaceName, or else the behavior of this function is
      /// undefined.
      ///
      /// This std::shared_ptr and the interface+plugin that it refers to will
      /// remain valid, even if all Plugin objects which refer to the plugin
      /// instance are destructed.
      ///
      /// You MUST NOT attempt to pass a QueryInterface pointer into a
      /// std::shared_ptr yourself; that will result in double-delete memory
      /// errors. You must always call QueryInterfaceSharedPtr for a reference-
      /// counting pointer to an interface.
      ///
      /// \return A reference-counting pointer to the specified interface. This
      /// will keep the interface valid and the plugin instance alive, even if
      /// all Plugin objects that refer to this plugin instance are destructed.
      public: template <class Interface>
              std::shared_ptr<Interface> QueryInterfaceSharedPtr();

      /// \brief Same as QueryInterfaceSharedPtr<Interface>(), but it returns a
      /// std::shared_ptr to a const-qualified Interface.
      public: template <class Interface>
              std::shared_ptr<const Interface> QueryInterfaceSharedPtr() const;

      /// \brief Checks if this Plugin has the specified type of interface.
      /// \return Returns true if this Plugin has the specified type of
      /// interface, and false otherwise.
      public: template <class Interface>
              bool HasInterface() const;

      /// \brief Checks if this Plugin has the specified type of interface.
      ///
      /// By default, we expect you to pass in a demangled version of the
      /// interface name. If you want to use a mangled version of the name,
      /// set the `demangled` argument to false.
      ///
      /// \param[in] _interfaceName The name of the desired interface, as a
      /// std::string. Note that this expects the name to be mangled.
      /// \param[in] _demangled If _interfaceName is demangled, set this to
      /// true. If you are instead using the raw mangled name that gets provided
      /// by typeid(T).name(), then set _demangled to false.
      /// \return Returns true if this Plugin has the specified type of
      /// interface, and false otherwise.
      public: bool HasInterface(const std::string &_interfaceName,
                                const bool _demangled = true) const;

      /// \brief Gets the name of this Plugin.
      /// \return A pointer to the name of this Plugin, or nullptr if this
      /// Plugin is not instantiated.
      public: const std::string *Name() const;

      // -------------------- Private API -----------------------

      template <class> friend class TemplatePluginPtr;
      template <class...> friend class SpecializedPlugin;
      template <class, class> friend class detail::ComposePlugin;
      template <class> friend class detail::SelectSpecializers;
      friend class EnablePluginFromThis;
      friend class WeakPluginPtr;

      /// \brief Default constructor. This is kept protected to discourage users
      /// from instantiating them directly. They should instead only be
      /// retrieved as a PluginPtr from the plugin Loader.
      protected: Plugin();

      /// \brief Type-agnostic retriever for interfaces
      private: void *PrivateQueryInterface(
                  const std::string &_interfaceName) const;

      /// \brief Copy the plugin instance from another Plugin object
      private: void PrivateCopyPluginInstance(const Plugin &_other) const;

      /// \brief Copy an existing plugin instance into this plugin
      /// \param[in] _info
      ///   Pointer to the Info for this plugin
      /// \param[in] _instancePtr
      ///   Pointer to an already-existing abstract plugin instance pointer
      private: void PrivateCopyPluginInstance(
                  const ConstInfoPtr &_info,
                  const std::shared_ptr<void> &_instancePtr) const;

      /// \brief Create a new plugin instance based on the info provided
      /// \param[in] _info
      ///   Pointer to the Info for this plugin
      /// \param[in] _dlHandlePtr
      ///   Reference counter for the dl handle of this Plugin
      private: void PrivateCreatePluginInstance(
                  const ConstInfoPtr &_info,
                  const std::shared_ptr<void> &_dlHandlePtr) const;

      /// \brief Create a new plugin instance based on the info provided for a
      /// plugin from the static plugin loader registry.
      /// \param[in] _info
      ///   Pointer to the Info for this plugin
      private: void PrivateCreateStaticPluginInstance(
                  const ConstInfoPtr &_info) const;


      /// \brief Get a reference to the abstract instance being managed by this
      /// wrapper
      private: const std::shared_ptr<void> &PrivateGetInstancePtr() const;

      /// \brief Get a reference to the Info being used by this wrapper
      private: const ConstInfoPtr &PrivateGetInfoPtr() const;

      /// \brief The InterfaceMap type needs to get used in several places, like
      /// Plugin::Implementation and SpecializedPlugin<T>. We make the typedef
      /// public so that those other classes can use it without needing to be
      /// friends of Plugin. End-users should not have any need for this
      /// typedef.
      public: using InterfaceMap = std::map<std::string, void*>;

      /// \brief Get or create an iterator to the std::map that holds pointers
      /// to the various interfaces provided by this plugin instance.
      private: InterfaceMap::iterator PrivateGetOrCreateIterator(
          const std::string &_interfaceName);

      class Implementation;
      GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \brief PIMPL pointer to the implementation of this class.
      private: const std::unique_ptr<Implementation> dataPtr;
      GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING

      /// \brief Virtual destructor
      public: virtual ~Plugin();
    };
  }
}

#include "gz/plugin/detail/Plugin.hh"

#endif
