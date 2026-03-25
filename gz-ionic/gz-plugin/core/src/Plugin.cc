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


#include <cassert>
#include <iostream>

#include "gz/plugin/Plugin.hh"
#include "gz/plugin/Info.hh"

namespace gz
{
  namespace plugin
  {
    /// \brief Struct which wraps a plugin instance together with a
    /// std::shared_ptr to its shared library handle. Instantiating plugin
    /// instances into this struct ensures that the shared library will remain
    /// loaded for as long as the plugin instance continues to exist.
    struct PluginWithDlHandle
    {
      /// \brief Constructor
      public: PluginWithDlHandle(
        void *_loadedInstance,
        const std::function<void(void*)> &_deleter,
        const std::shared_ptr<void> &_dlHandlePtr)
        : dlHandlePtr(_dlHandlePtr),
          loadedInstance(_loadedInstance),
          deleter(_deleter)
      {
        // Do nothing
      }

      /// \brief Destructor. We call the deleter on the loadedInstance while the
      /// deleter and dlHandlePtr are still valid and available.
      public: ~PluginWithDlHandle()
      {
        if (loadedInstance)
        {
          if (!deleter)
          {
            // LCOV_EXCL_START
            std::cerr << "This plugin instance (" << loadedInstance
                      << ") was not given a deleter. This should never happen! "
                      << "Please report this bug!\n";
            assert(false);
            return;
            // LCOV_EXCL_STOP
          }

          deleter(loadedInstance);
        }
        else
        {
          // LCOV_EXCL_START
          std::cerr << "We have a nullptr plugin instance inside of a "
                    << "PluginWithDlHandle. This should not be possible! "
                    << "Please report this bug!\n";
          assert(false);
          return;
          // LCOV_EXCL_STOP
        }
      }

      /// \brief A reference counting handle for the shared library that this
      /// plugin depends on.
      ///
      /// CRUCIAL DEV NOTE (MXG): `dlHandlePtr` MUST come BEFORE `deleter` in
      /// this class definition to ensure that `deleter` gets deleted first
      /// (member variables get destructed in the reverse order of their
      /// appearance in the class definition). The destructor of `deleter`
      /// depends on the shared library still being available, so this reference
      /// counting handle must be destroyed after `deleter` to ensure that the
      /// library is still loaded when `deleter` needs it.
      ///
      /// If you change this class definition for ANY reason, be sure to
      /// maintain the ordering of these member variables.
      public: std::shared_ptr<void> dlHandlePtr;

      /// \brief Pointer to the plugin instance
      public: void *loadedInstance;

      /// \brief Deleter function for the plugin instance
      ///
      /// CRUCIAL DEV NOTE (MXG): `deleter` MUST come AFTER `dlHandlePtr` in
      /// this class definition. See the comment on `dlHandlePtr` for an
      /// explanation.
      ///
      /// If you change this class definition for ANY reason, be sure to
      /// maintain the ordering of these member variables.
      public: std::function<void(void*)> deleter;
    };

    class Plugin::Implementation
    {
      /// \brief Clear this object without invaliding any map entry
      /// iterators.
      public: void Clear()
      {
        this->loadedInstancePtr.reset();
        this->info.reset();

        // Dev note (MXG): We must NOT call clear() on the InterfaceMap or
        // remove ANY of the map entries, because that would potentially
        // invalidate all of the iterators that are pointing to map entries.
        // This would break any specialized plugins that provide instant access
        // to specialized interfaces. Instead, we simply overwrite the map
        // entries with a nullptr.
        for (auto &entry : this->interfaces)
          entry.second = nullptr;
      }

      /// \brief Initialize this object by creating a new plugin instance from
      /// the info provided.
      /// \param[in] _info Information describing the plugin to initialize
      /// \param[in] _dlHandlePtr A reference to the dl handle that manages the
      ///            lifecycle of the plugin library.
      /// \param[in] _allowNullDlHandlePtr Allow _dlHandlePtr to be null. Only
      ///            set true for plugin instances created from the static
      ///            registry.
      public: void Create(
          const ConstInfoPtr &_info,
          const std::shared_ptr<void> &_dlHandlePtr,
          bool _allowNullDlHandlePtr = false)
      {
        this->Clear();

        if (!_info)
          return;

        this->info = _info;

        if (!_dlHandlePtr && !_allowNullDlHandlePtr)
        {
          // LCOV_EXCL_START
          std::cerr << "Received Info for [" << _info->name << "], "
                    << "but we were not provided a shared library handle. "
                    << "This should never happen! Please report this bug!\n";
          assert(false);
          return;
          // LCOV_EXCL_STOP
        } else if (_dlHandlePtr != nullptr && _allowNullDlHandlePtr) {
          std::cerr << "Trying to create a plugin from a static plugin, but "
                    << "a shared library handle was provided. This should "
                    << "never happen! Please report this bug!\n";
          assert(false);
        }

        // Create a std::shared_ptr to a struct which ensures that the
        // _dlHandlePtr will remain alive for as long as this plugin instance
        // exists.
        std::shared_ptr<PluginWithDlHandle> pluginWithDlHandle =
            std::make_shared<PluginWithDlHandle>(
              _info->factory(), _info->deleter, _dlHandlePtr);

        // Use the aliasing constructor of std::shared_ptr to disguise
        // pluginWithDlHandle as just a simple std::shared_ptr<void> which
        // points at the plugin instance, so we have the benefit of
        // automatically managing the lifecycle of the dlHandlePtr without
        // needing to actually keep track of it.
        this->loadedInstancePtr =
            std::shared_ptr<void>(
              pluginWithDlHandle,
              pluginWithDlHandle->loadedInstance);

        for (const auto &entry : _info->interfaces)
        {
          // entry.first:  name of the interface
          // entry.second: function which casts the loadedInstance pointer to
          //               the correct location of the interface within the
          //               plugin
          this->interfaces[entry.first] =
              entry.second(this->loadedInstancePtr.get());
        }
      }

      /// \brief Initialize this object using another instance
      /// \param[in] _other Another instance of a Plugin::Implementation object
      public: void Copy(const Implementation *_other)
      {
        this->Clear();

        if (!_other)
        {
          // LCOV_EXCL_START
          std::cerr << "Received a nullptr _other in the constructor "
                    << "which uses `const Plugin::Implementation*`. This "
                    << "should not be possible! Please report this bug."
                    << std::endl;
          assert(false);
          return;
          // LCOV_EXCL_STOP
        }

        this->loadedInstancePtr = _other->loadedInstancePtr;
        this->info = _other->info;

        if (this->loadedInstancePtr)
        {
          for (const auto &entry : _other->interfaces)
          {
            // entry.first:  name of the interface
            // entry.second: pointer to the location of that interface within
            //               the plugin instance
            this->interfaces[entry.first] = entry.second;
          }
        }
      }

      /// \brief Initialize this object using another instance
      /// \param[in] _info
      ///   A reference to the plugin's Info
      /// \param[in] _instance
      ///   A reference to the plugin's abstract instance
      public: void Copy(const ConstInfoPtr &_info,
                        const std::shared_ptr<void> &_instance)
      {
        this->loadedInstancePtr = _instance;
        this->info = _info;

        if (this->loadedInstancePtr)
        {
          if (!this->info)
          {
            // LCOV_EXCL_START
            std::cerr << "[Plugin::Implementation::Copy(_info, _instance)] A "
                      << "Plugin has been copied from its info and instance, "
                      << "but the info was null even though the instance was "
                      << "valid. This should never happen! Please report this "
                      << "bug!" << std::endl;
            assert(false);
            return;
            // LCOV_EXCL_STOP
          }

          // We need to construct the interface map for this Plugin
          for (const auto &entry : this->info->interfaces)
          {
            // entry.first:  name of the interface
            // entry.second: function which casts the loadedInstance pointer to
            //               the correct location of the interface within the
            //               plugin
            this->interfaces[entry.first] =
                entry.second(this->loadedInstancePtr.get());
          }
        }
      }

      /// \brief Map from interface names to their locations within the plugin
      /// instance
      //
      // Dev Note (MXG): We use std::map here instead of std::unordered_map
      // because iterators to a std::map are not invalidated by the insertion
      // operation (whereas all iterators to a std::unordered_map are
      // potentially invalidated each time an insertion is performed on the
      // std::unordered_map). Holding onto valid iterators allows us to do
      // optimizations with template magic to provide direct access to
      // interfaces whose availability we can anticipate at run time.
      //
      // It is also worth noting that ordered vs unordered lookup time is very
      // similar on sets where the strings have relatively small lengths (5-20
      // characters) and a relatively small number of entries in the set (5-20
      // entries). Those conditions match our expected use case here. In fact,
      // ordered lookup can sometimes outperform unordered in these conditions.
      public: Plugin::InterfaceMap interfaces;

      /// \brief shared_ptr which manages the lifecycle of the plugin instance.
      ///
      /// CRUCIAL DEV NOTE (MXG): `loadedInstancePtr` must come BEFORE `info` in
      /// this class definition to ensure that `info` gets deleted first (member
      /// variables get destructed in the reverse order of their appearance in
      /// the class definition). The destructor of `info` depends on the shared
      /// library still being available, so this reference counting handle must
      /// be destroyed after `info` to ensure that the library is still loaded
      /// when `info` needs it.
      ///
      /// If you change this class definition for ANY reason, be sure to
      /// maintain the ordering of these member variables.
      public: std::shared_ptr<void> loadedInstancePtr;

      /// \brief A copy of the Info that was used to create the Plugin
      ///
      /// CRUCIAL DEV NOTE (MXG): `info` must come AFTER `loadedInstancePtr` in
      /// this class definition. See the comment on `loadedInstancePtr` for an
      /// explanation.
      ///
      /// If you change this class definition for ANY reason, be sure to
      /// maintain the ordering of these member variables.
      public: ConstInfoPtr info;
    };

    //////////////////////////////////////////////////
    bool Plugin::HasInterface(
        const std::string &_interfaceName,
        const bool _demangled) const
    {
      const ConstInfoPtr &info = this->dataPtr->info;
      if (!info)
        return false;

      if (_demangled)
      {
        return (info->demangledInterfaces.count(_interfaceName) != 0);
      }

      return (this->dataPtr->interfaces.count(_interfaceName) != 0);
    }

    //////////////////////////////////////////////////
    const std::string *Plugin::Name() const
    {
      if (!this->dataPtr->info)
        return nullptr;

      return &this->dataPtr->info->name;
    }

    //////////////////////////////////////////////////
    Plugin::Plugin()
      : dataPtr(new Implementation)
    {
      // Do nothing
    }

    //////////////////////////////////////////////////
    void *Plugin::PrivateQueryInterface(
        const std::string &_interfaceName) const
    {
      const auto &it = this->dataPtr->interfaces.find(_interfaceName);
      if (this->dataPtr->interfaces.end() == it)
        return nullptr;

      return it->second;
    }

    //////////////////////////////////////////////////
    void Plugin::PrivateCopyPluginInstance(const Plugin &_other) const
    {
      this->dataPtr->Copy(_other.dataPtr.get());
    }

    //////////////////////////////////////////////////
    void Plugin::PrivateCopyPluginInstance(
        const ConstInfoPtr &_info,
        const std::shared_ptr<void> &_instancePtr) const
    {
      this->dataPtr->Copy(_info, _instancePtr);
    }

    //////////////////////////////////////////////////
    void Plugin::PrivateCreatePluginInstance(
        const ConstInfoPtr &_info,
        const std::shared_ptr<void> &_dlHandlePtr) const
    {
      this->dataPtr->Create(_info, _dlHandlePtr);
    }

    //////////////////////////////////////////////////
    void Plugin::PrivateCreateStaticPluginInstance(
        const ConstInfoPtr &_info) const
    {
      this->dataPtr->Create(_info, /*_dlHandlePtr=*/nullptr,
          /*_allowNullDlHandlePtr=*/true);
    }

    //////////////////////////////////////////////////
    const std::shared_ptr<void> &Plugin::PrivateGetInstancePtr() const
    {
      return this->dataPtr->loadedInstancePtr;
    }

    //////////////////////////////////////////////////
    const ConstInfoPtr &Plugin::PrivateGetInfoPtr() const
    {
      return this->dataPtr->info;
    }

    //////////////////////////////////////////////////
    Plugin::InterfaceMap::iterator Plugin::PrivateGetOrCreateIterator(
        const std::string &_interfaceName)
    {
      // We want to use the insert function here to avoid accidentally
      // overwriting a value which might exist at the desired map key.
      return this->dataPtr->interfaces.insert(
            std::make_pair(_interfaceName, nullptr)).first;
    }

    //////////////////////////////////////////////////
    Plugin::~Plugin()
    {
      // Do nothing. We need this definition to be in a source file so that
      // the destructor of Implementation is visible to std::unique_ptr.
    }
  }
}
