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


#ifndef GZ_PLUGIN_PLUGINPTR_HH_
#define GZ_PLUGIN_PLUGINPTR_HH_

#include <map>
#include <string>
#include <memory>

#include <gz/plugin/Plugin.hh>

namespace gz
{
  namespace plugin
  {
    // Forward declarations
    namespace detail { template<class, class> class ComposePlugin; }

    /// \brief This class manages the lifecycle of a plugin instance. It can
    /// receive a plugin instance from the gz::plugin::Loader class
    /// or by copy-construction or assignment from another PluginPtr instance.
    ///
    /// This class behaves similarly to a std::shared_ptr where multiple
    /// PluginPtr objects can share a single plugin instance, and the plugin
    /// instance will not be deleted until all PluginPtr objects that refer to
    /// it are either destroyed, cleared, or begin referring to a different
    /// plugin instance.
    ///
    /// A PluginPtr object can be "cast" to a SpecializedPluginPtr object by
    /// simply using the copy/move constructor or assignment operator of a
    /// SpecializedPluginPtr object. Note that this "cast" does have a small
    /// amount of overhead associated with it, but it may result in huge savings
    /// after initialization is finished if you frequently access the interfaces
    /// that the SpecializedPluginPtr is specialized for.
    template <typename PluginType>
    class TemplatePluginPtr final
    {
      /// \brief Destructor. Deletes this PluginPtr's reference to the plugin
      /// instance. Once all PluginPtrs that refer to a plugin instance are
      /// deleted, the plugin will also be deleted.
      public: ~TemplatePluginPtr() = default;

      /// \brief Default constructor. Creates a PluginPtr object that does not
      /// point to any plugin instance. IsEmpty() will return true until a
      /// plugin instance is provided.
      public: TemplatePluginPtr();

      /// \brief Copy constructor. This PluginPtr will now point at the same
      /// plugin instance as _other, and they will share ownership.
      /// \param[in] _other Pointer to plugin being copied.
      public: TemplatePluginPtr(const TemplatePluginPtr &_other);

      /// \brief Move constructor. This PluginPtr will take ownership of the
      /// plugin instance held by _other. If this PluginPtr was holding an
      /// instance to another plugin, that instance will be deleted if no other
      /// PluginPtr is referencing which is being moved.
      /// \param[in] _other Plugin being moved.
      public: TemplatePluginPtr(TemplatePluginPtr &&_other);

      /// \brief Casting constructor. This PluginPtr will now point at the same
      /// plugin instance as _other, and they will share ownership. This
      /// essentially allows casting between PluginPtrs that are holding
      /// different types of plugin wrappers (for example, you can cast a
      /// generic PluginPtr to any SpecializedPluginPtr type, or you can cast
      /// between different types of specializations).
      /// \param[in] _other Pointer to plugin being casted, which is of a
      /// different type.
      public: template <typename OtherPluginType>
              TemplatePluginPtr(
                  const TemplatePluginPtr<OtherPluginType> &_other);

      /// \brief Copy assignment operator. This PluginPtr will now point at the
      /// same plugin instance as _other, and they will share ownership. If this
      /// PluginPtr was holding an instance to another plugin, that instance
      /// will be deleted if no other PluginPtr is referencing it.
      /// \param[in] _other Pointer to plugin being copied.
      public: TemplatePluginPtr &operator =(const TemplatePluginPtr &_other);

      /// \brief Casting operator. This PluginPtr will now point at the same
      /// plugin instance as _other, and they will share ownership. This
      /// essentially allows casting between PluginPtrs that are holding
      /// different types of plugin wrappers.
      /// \param[in] _other Pointer to plugin being casted, which is of a
      /// different type.
      /// \return A reference to this object.
      public: template <typename OtherPluginType>
              TemplatePluginPtr &operator =(
                  const TemplatePluginPtr<OtherPluginType> &_other);

      /// \brief Move assignment operator. This PluginPtr will take ownership
      /// of the plugin instance held by _other. If this PluginPtr was holding
      /// an instance to another plugin, that instance will be deleted if no
      /// other PluginPtr is referencing it.
      /// \param[in] _other Plugin being moved.
      /// \return A reference to this object.
      public: TemplatePluginPtr &operator=(TemplatePluginPtr &&_other);

      /// \brief nullptr assignment operator. Same as calling Clear()
      /// \param[in] _null nullptr object.
      /// \return A reference to this object.
      public: TemplatePluginPtr &operator=(std::nullptr_t _null);

      /// \brief Access the wrapper for the plugin instance and call one of its
      /// member functions.
      /// \return The ability to call a member function on the underlying Plugin
      /// object.
      public: PluginType *operator->() const;

      /// \brief Get a reference to the wrapper for the plugin instance that is
      /// being managed by this PluginPtr.
      /// \return A reference to the underlying Plugin object.
      public: PluginType &operator*() const;

      /// \brief Comparison operator.
      /// \param[in] _other Plugin to compare to.
      /// \returns True if this Plugin is holding the same plugin
      /// instance as _other, otherwise returns false.
      public: bool operator ==(const TemplatePluginPtr &_other) const;

      /// \brief Comparison operator.
      /// \param[in] _other Plugin to compare to.
      /// \returns True if the pointer value of the plugin instance held
      /// by this PluginPtr is less than the pointer value of the instance held
      /// by _other.
      public: bool operator <(const TemplatePluginPtr &_other) const;

      /// \brief Comparison operator.
      /// \param[in] _other Plugin to compare to.
      /// \returns True if the pointer value of the plugin instance held
      /// by this PluginPtr is greater than the pointer value of the instance
      /// held by _other.
      public: bool operator >(const TemplatePluginPtr &_other) const;

      /// \brief Comparison operator.
      /// \param[in] _other Plugin to compare to.
      /// \returns True if the pointer instance held by this PluginPtr is
      /// different from the pointer instance held by _other.
      public: bool operator !=(const TemplatePluginPtr &_other) const;

      /// \brief Comparison operator.
      /// \param[in] _other Plugin to compare to.
      /// \returns True if the value of the pointer instance held by this
      /// PluginPtr is less than or equal to the value of the pointer instance
      /// held by _other.
      public: bool operator <=(const TemplatePluginPtr &_other) const;

      /// \brief Comparison operator.
      /// \param[in] _other Plugin to compare to.
      /// \returns True if the value of the pointer instance held by this
      /// PluginPtr is greater than or equal to the value of the pointer
      /// instance held by _other.
      public: bool operator >=(const TemplatePluginPtr &_other) const;

      /// \brief Produces a hash for the plugin instance that this PluginPtr is
      /// holding. This function allows PluginPtr instances to be used as values
      /// in a std::unordered_set<PluginPtr> or keys in a
      /// std::unordered_map<PluginPtr, T>. Using this function directly should
      /// not normally be necessary.
      /// \return A hash of the underlying pointer object.
      public: std::size_t Hash() const;

      /// \brief Check whether this is pointing at a valid plugin.
      /// \returns False if this PluginPtr is pointing at a valid plugin
      /// instance. If it is instead pointing at a nullptr, this returns true.
      public: bool IsEmpty() const;

      /// \brief Implicitly convert this PluginPtr to a boolean.
      /// \return The opposite value of IsEmpty().
      public: operator bool() const;

      /// \brief Clears the Plugin instance from this PluginPtr. IsEmpty() will
      /// return true after this is used, and none of the interfaces will be
      /// available any longer.
      public: void Clear();

      /// \brief Pointer to the plugin wrapper that this PluginPtr is managing.
      private: std::unique_ptr<PluginType> dataPtr;

      // Declare friendship
      friend class Loader;
      template <class> friend class TemplatePluginPtr;

      /// \brief Private constructor. Creates a plugin instance based on the
      /// Info provided. This should only be called by Loader to ensure that
      /// the Info is well-formed, so we keep it private.
      /// \param[in] _info An Info instance that was generated by
      /// Loader. Alternatively, this can take a nullptr to create an
      /// empty PluginPtr.
      /// \param[in] _dlHandlePtr A reference count for the DL handle.
      private: explicit TemplatePluginPtr(
          const ConstInfoPtr &_info,
          const std::shared_ptr<void> &_dlHandlePtr);

      /// \brief Private constructor. Used by the Loader to instantiate a
      /// plugin class from the static registry.
      /// \param[in] _info An Info instance that was generated by Loader.
      private: explicit TemplatePluginPtr(const ConstInfoPtr &_info);
    };

    /// \brief Typical usage for TemplatePluginPtr is to just hold a generic
    /// Plugin type.
    using PluginPtr = TemplatePluginPtr<Plugin>;

    /// \brief This produces a PluginPtr whose Plugin wrapper only grants access
    /// to const-qualified interfaces of the plugin instance.
    using ConstPluginPtr = TemplatePluginPtr<const Plugin>;
  }
}

#include "gz/plugin/detail/PluginPtr.hh"

#endif
