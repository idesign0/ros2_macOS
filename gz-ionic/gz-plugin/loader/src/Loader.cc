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

#include <dlfcn.h>

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <locale>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <gz/plugin/Info.hh>
#include <gz/plugin/Loader.hh>
#include <gz/plugin/Plugin.hh>
#include <gz/plugin/detail/Registry.hh>
#include <gz/plugin/detail/StaticRegistry.hh>
#include <gz/plugin/utility.hh>

namespace gz
{
  namespace plugin
  {
    /////////////////////////////////////////////////
    /// \brief PIMPL Implementation of the Loader class
    class Loader::Implementation
    {
      /// \brief Attempt to load a library at the given path.
      /// \param[in] _pathToLibrary The full path to the desired library
      /// \return If a library exists at the given path, get a point to its dl
      /// handle. If the library does not exist, get a nullptr.
      public: std::shared_ptr<void> LoadLib(
        const std::string &_pathToLibrary, bool _noDelete);

      /// \brief Using a dl handle produced by LoadLib, extract the
      /// Info from the loaded library.
      /// \param[in] _dlHandle A handle produced by LoadLib
      /// \param[in] _pathToLibrary The path that the library was loaded from
      /// (used for debug purposes)
      /// \return All the Info provided by the loaded library.
      public: std::vector<Info> LoadPlugins(
        const std::shared_ptr<void> &_dlHandle,
        const std::string &_pathToLibrary) const;

      /// \sa Loader::ForgetLibrary()
      public: bool ForgetLibrary(void *_dlHandle);

      public: using PluginToDlHandleMap =
          std::unordered_map< std::string, std::shared_ptr<void> >;
      /// \brief A map from known plugin names to the handle of the library that
      /// provides it.
      ///
      /// CRUCIAL DEV NOTE (MXG): `pluginToDlHandlePtrs` MUST come BEFORE
      /// `plugins` in this class definition to ensure that `plugins` gets
      /// deleted first (member variables get destructed in the reverse order of
      /// their appearance in the class definition). The destructors of the
      /// `deleter` members of the Info class depend on the shared library
      /// still being available, so this map of std::shared_ptrs to the library
      /// handles must be destroyed after the Info.
      ///
      /// If you change this class definition for ANY reason, be sure to
      /// maintain the ordering of these member variables.
      public: PluginToDlHandleMap pluginToDlHandlePtrs;

      /// CRUCIAL DEV NOTE (MXG): `filePlugins` MUST come AFTER
      /// `pluginToDlHandlePtrs` in this class definition. See the comment on
      /// pluginToDlHandlePtrs for an explanation.
      ///
      /// If you change this class definition for ANY reason, be sure to
      /// maintain the ordering of these member variables.
      public: Registry filePlugins;

      using DlHandleMap = std::unordered_map< void*, std::weak_ptr<void> >;
      /// \brief A map which keeps track of which shared libraries have been
      /// loaded by this Loader.
      ///
      /// The key of this map is a pointer to a dl handle, and the values are
      /// weak_ptrs to the shared_ptr that was constructed for that dl handle.
      /// Since these are weak_ptrs, they will automatically get cleared
      /// whenever the library get unloaded. Therefore, if a map entry exists
      /// for a dl handle and the value of that map entry still points to a
      /// valid shared_ptr, then the library for that dl handle is currently
      /// loaded, and we are already managing its reference count.
      ///
      /// This is used to ensure that we keep one single authoritative reference
      /// count of the dl handle per Loader.
      public: DlHandleMap dlHandlePtrMap;

      public: using DlHandleToPluginMap =
          std::unordered_map< void*, std::unordered_set<std::string> >;
      /// \brief A map from the shared library handle to the names of the
      /// plugins that it provides.
      public: DlHandleToPluginMap dlHandleToPluginMap;

      /// \brief Pointer to the singleton StaticRegistry
      public: StaticRegistry* staticPlugins;
    };

    /////////////////////////////////////////////////
    std::string Loader::PrettyStr() const
    {
      std::stringstream pretty;
      pretty << "Loaded plugins registry: \n"
             << this->dataPtr->filePlugins.PrettyStr();
      pretty << "Static plugins registry: \n"
             << this->dataPtr->staticPlugins->PrettyStr();
      return pretty.str();
    }

    /////////////////////////////////////////////////
    Loader::Loader()
      : dataPtr(new Implementation())
    {
      this->dataPtr->staticPlugins = &(StaticRegistry::GetInstance());
    }

    /////////////////////////////////////////////////
    Loader::~Loader()
    {
      // Do nothing.
    }

    /////////////////////////////////////////////////
    std::unordered_set<std::string> Loader::LoadLib(
        const std::string &_pathToLibrary)
    {
      return this->LoadLib(_pathToLibrary, false);
    }
    /////////////////////////////////////////////////
    std::unordered_set<std::string> Loader::LoadLib(
        const std::string &_pathToLibrary, bool _noDelete)
    {
      std::unordered_set<std::string> newPlugins;

      // Attempt to load the library at this path
      const std::shared_ptr<void> &dlHandle =
          this->dataPtr->LoadLib(_pathToLibrary, _noDelete);

      // Quit early and return an empty set of plugin names if we did not
      // actually get a valid dlHandle.
      if (nullptr == dlHandle)
      {
        return newPlugins;
      }

      // Found a shared library, does it have the symbols we're looking for?
      std::vector<Info> loadedPlugins = this->dataPtr->LoadPlugins(
            dlHandle, _pathToLibrary);

      for (Info &plugin : loadedPlugins)
      {
        // Demangle the plugin name before creating an entry for it.
        plugin.name = DemangleSymbol(plugin.name);

        // Make a list of the demangled interface names for later convenience.
        for (auto const &interface : plugin.interfaces)
          plugin.demangledInterfaces.insert(DemangleSymbol(interface.first));

        // Add the plugin to the map
        this->dataPtr->filePlugins.AddInfo(plugin);

        // Add the plugin's name to the set of newPlugins
        newPlugins.insert(plugin.name);

        // Save the dl handle for this plugin
        this->dataPtr->pluginToDlHandlePtrs[plugin.name] = dlHandle;
      }

      dataPtr->dlHandleToPluginMap[dlHandle.get()] = newPlugins;

      return newPlugins;
    }

    /////////////////////////////////////////////////
    std::unordered_set<std::string> Loader::InterfacesImplemented() const
    {
      std::unordered_set<std::string> allInterfaces =
          this->dataPtr->filePlugins.InterfacesImplemented();
      std::unordered_set<std::string> staticPluginInterfaces =
          this->dataPtr->staticPlugins->InterfacesImplemented();
      allInterfaces.insert(staticPluginInterfaces.begin(),
          staticPluginInterfaces.end());
      return allInterfaces;
    }

    /////////////////////////////////////////////////
    std::unordered_set<std::string> Loader::PluginsImplementing(
        const std::string &_interface,
        const bool _demangled) const
    {
      std::unordered_set<std::string> allPlugins =
          this->dataPtr->filePlugins.PluginsImplementing(_interface,
          _demangled);
      std::unordered_set<std::string> staticPlugins =
          this->dataPtr->staticPlugins->PluginsImplementing(_interface,
          _demangled);
      allPlugins.insert(staticPlugins.begin(), staticPlugins.end());
      return allPlugins;
    }

    /////////////////////////////////////////////////
    std::set<std::string> Loader::AllPlugins() const
    {
      std::set<std::string> allPlugins =
          this->dataPtr->filePlugins.AllPlugins();
      std::set<std::string> staticPlugins =
          this->dataPtr->staticPlugins->AllPlugins();
      allPlugins.insert(staticPlugins.begin(), staticPlugins.end());
      return allPlugins;
    }

    /////////////////////////////////////////////////
    std::set<std::string> Loader::PluginsWithAlias(
        const std::string &_alias) const
    {
      std::set<std::string> allPlugins =
          this->dataPtr->filePlugins.PluginsWithAlias(_alias);
      std::set<std::string> staticPlugins =
          this->dataPtr->staticPlugins->PluginsWithAlias(_alias);
      allPlugins.insert(staticPlugins.begin(), staticPlugins.end());
      return allPlugins;
    }

    /////////////////////////////////////////////////
    std::set<std::string> Loader::AliasesOfPlugin(
        const std::string &_pluginName) const
    {
      std::set<std::string> allAliases =
          this->dataPtr->filePlugins.AliasesOfPlugin(_pluginName);
      std::set<std::string> staticAliases =
          this->dataPtr->staticPlugins->AliasesOfPlugin(_pluginName);
      allAliases.insert(staticAliases.begin(), staticAliases.end());
      return allAliases;
    }

    /////////////////////////////////////////////////
    std::string Loader::LookupPlugin(const std::string &_nameOrAlias) const
    {
      // Higher priority for plugins loaded from file than from the static
      // registry.
      std::string nameInFilePlugins =
          this->dataPtr->filePlugins.LookupPlugin(_nameOrAlias);
      if (!nameInFilePlugins.empty()) {
        return nameInFilePlugins;
      }

      std::string nameInStaticPlugins =
          this->dataPtr->staticPlugins->LookupPlugin(_nameOrAlias);
      if (!nameInStaticPlugins.empty()) {
        return nameInStaticPlugins;
      }

      std::cerr << "[gz::plugin::Loader::LookupPlugin] Failed to get "
                << "info for [" << _nameOrAlias << "]. Could not find a plugin "
                << "with that name or alias.\n";
      return "";
    }

    /////////////////////////////////////////////////
    PluginPtr Loader::Instantiate(const std::string &_pluginNameOrAlias) const
    {
      // Higher priority for plugins loaded from file than from the static
      // registry.
      const std::string &resolvedNameForFilePlugin =
          this->PrivateLookupFilePlugin(_pluginNameOrAlias);

      const std::string &resolvedNameForStaticPlugin =
            this->PrivateLookupStaticPlugin(_pluginNameOrAlias);

      PluginPtr ptr;

      if (!resolvedNameForFilePlugin.empty())
      {
        ptr = PluginPtr(
            this->PrivateGetInfoForFilePlugin(resolvedNameForFilePlugin),
            this->PrivateGetPluginDlHandlePtr(resolvedNameForFilePlugin));
      }
      else if (!resolvedNameForStaticPlugin.empty())
      {
        ptr = PluginPtr(
            this->PrivateGetInfoForStaticPlugin(resolvedNameForStaticPlugin));
      }
      else
      {
        return PluginPtr();
      }

      if (auto *enableFromThis = ptr->QueryInterface<EnablePluginFromThis>())
        enableFromThis->PrivateSetPluginFromThis(ptr);

      return ptr;
    }

    /////////////////////////////////////////////////
    bool Loader::ForgetLibrary(const std::string &_pathToLibrary)
    {
#ifndef RTLD_NOLOAD
// This macro is not part of the POSIX standard, and is a custom addition to
// glibc-2.2, so we need create a no-op stand-in flag for it if we are not
// using glibc-2.2.
#define RTLD_NOLOAD 0
#endif

      void *dlHandle = dlopen(_pathToLibrary.c_str(),
                              RTLD_NOLOAD | RTLD_LAZY | RTLD_LOCAL);

      if (!dlHandle)
        return false;

      // We should decrement the reference count because we called dlopen. Even
      // with the RTLD_NOLOAD flag, the call to dlopen will still (allegedly)
      // increment the reference count when it returns a valid handle. Note that
      // this knowledge is according to online discussions and is not explicitly
      // stated in the manual pages of dlopen (but it is consistent with the
      // overall behavior of dlopen).
      dlclose(dlHandle);

      return this->dataPtr->ForgetLibrary(dlHandle);
    }

    /////////////////////////////////////////////////
    bool Loader::ForgetLibraryOfPlugin(const std::string &_pluginNameOrAlias)
    {
      const std::string &resolvedName = this->LookupPlugin(_pluginNameOrAlias);

      Implementation::PluginToDlHandleMap::iterator it =
          dataPtr->pluginToDlHandlePtrs.find(resolvedName);

      if (dataPtr->pluginToDlHandlePtrs.end() == it)
        return false;

      return dataPtr->ForgetLibrary(it->second.get());
    }

    /////////////////////////////////////////////////
    std::string Loader::PrivateLookupFilePlugin(
        const std::string &_nameOrAlias) const
    {
      return this->dataPtr->filePlugins.LookupPlugin(_nameOrAlias);
    }

    /////////////////////////////////////////////////
    ConstInfoPtr Loader::PrivateGetInfoForFilePlugin(
        const std::string &_resolvedName) const
    {
      ConstInfoPtr info = this->dataPtr->filePlugins.GetInfo(_resolvedName);

      if (info == nullptr)
      {
        // LCOV_EXCL_START
        std::cerr << "[gz::Loader::PrivateGetInfoForFilePlugin] A resolved "
                  << "name [" << _resolvedName << "] could not be found in "
                  << "the registry of loaded plugins. This should not be "
                  << "possible! Please report this bug!\n";
        assert(false);
        return nullptr;
        // LCOV_EXCL_STOP
      }

      return info;
    }

    /////////////////////////////////////////////////
    std::string Loader::PrivateLookupStaticPlugin(
        const std::string &_nameOrAlias) const
    {
      return this->dataPtr->staticPlugins->LookupPlugin(_nameOrAlias);
    }

    /////////////////////////////////////////////////
    ConstInfoPtr Loader::PrivateGetInfoForStaticPlugin(
        const std::string &_resolvedName) const
    {
      ConstInfoPtr info = this->dataPtr->staticPlugins->GetInfo(_resolvedName);

      if (info == nullptr)
      {
        // LCOV_EXCL_START
        std::cerr << "[gz::Loader::PrivateGetInfoForStaticPlugin] A resolved "
                  << "name [" << _resolvedName << "] could not be found in "
                  << "the static plugin registry. This should not be "
                  << "possible! Please report this bug!\n";
        assert(false);
        return nullptr;
        // LCOV_EXCL_STOP
      }

      return info;
    }

    /////////////////////////////////////////////////
    std::shared_ptr<void> Loader::PrivateGetPluginDlHandlePtr(
        const std::string &_resolvedName) const
    {
      Implementation::PluginToDlHandleMap::iterator it =
          dataPtr->pluginToDlHandlePtrs.find(_resolvedName);

      if (this->dataPtr->pluginToDlHandlePtrs.end() == it)
      {
        // LCOV_EXCL_START
        std::cerr << "[gz::Loader::PrivateGetInfo] A resolved name ["
                  << _resolvedName << "] could not be found in the "
                  << "PluginToDlHandleMap. This should not be possible! Please "
                  << "report this bug!\n";
        assert(false);
        return nullptr;
        // LCOV_EXCL_STOP
      }

      return it->second;
    }

    /////////////////////////////////////////////////
    std::shared_ptr<void> Loader::Implementation::LoadLib(
        const std::string &_full_path, bool _noDelete)
    {
      std::shared_ptr<void> dlHandlePtr;

      // Call dlerror() before dlopen(~) to ensure that we get accurate error
      // reporting afterwards. The function dlerror() is stateful, and that
      // state gets cleared each time it is called.
      dlerror();

      // NOTE: We open using RTLD_LOCAL instead of RTLD_GLOBAL to prevent the
      // symbols of different libraries from writing over each other.
#ifdef _WIN32
      // RTLD_NODELETE is not defined in dlfcn-32.
      (void) _noDelete;
      int dlopenMode = RTLD_LAZY | RTLD_LOCAL;
#else
      int dlopenMode = RTLD_LAZY | RTLD_LOCAL | (_noDelete ? RTLD_NODELETE : 0);
#endif
      void *dlHandle = dlopen(_full_path.c_str(), dlopenMode);

      const char *loadError = dlerror();
      if (nullptr == dlHandle || nullptr != loadError)
      {
        std::cerr << "Error while loading the library [" << _full_path << "]: "
                  << loadError << std::endl;

        // Just return a nullptr if the library could not be loaded. The
        // Loader::LoadLib(~) function will handle this gracefully.
        return nullptr;
      }

      // The dl library maintains a reference count of how many times dlopen(~)
      // or dlclose(~) has been called on each loaded library. dlopen(~)
      // increments the reference count while dlclose(~) decrements the count.
      // When the count reaches zero, the operating system is free to unload
      // the library (which it might or might not choose to do; we do not seem
      // to have control over that, probably for the best).

      // The idea in our implementation here is to first check if the library
      // has already been loaded by this Loader. If it has been, then we
      // will see it in the dlHandlePtrMap, and we'll be able to get a shared
      // reference to its handle. The advantage of this is it allows us to keep
      // track of how many places the shared library is being used by plugins
      // that were generated from this Loader. When all of the instances
      // spawned by this Loader are gone, the destructor of the
      // std::shared_ptr will call dlclose on the library handle, bringing down
      // its dl library reference count.

      bool inserted;
      DlHandleMap::iterator it;
      std::tie(it, inserted) = this->dlHandlePtrMap.insert(
            std::make_pair(dlHandle, std::weak_ptr<void>()));

      if (!inserted)
      {
        // This shared library has already been loaded by this Loader in
        // the past, so we should use the reference counter that already
        // exists for it.
        dlHandlePtr = it->second.lock();

        if (dlHandlePtr)
        {
          // The reference counter of this library is still active.

          // The functions dlopen and dlclose keep their own independent
          // counters for how many times each of them has been called. The
          // library is unloaded once dlclose has been called as many times as
          // dlopen.
          //
          // At this line of code, we know that dlopen had been called by this
          // Loader instance prior to this run of LoadLib. Therefore,
          // we should undo the dlopen that we did just a moment ago in this
          // function so that only one dlclose must be performed to finally
          // close the shared library. That final dlclose will be performed in
          // the destructor of the std::shared_ptr<void> which stores the
          // library handle.
          dlclose(dlHandle);
        }
      }

      if (!dlHandlePtr)
      {
        // The library was not already loaded (or if it was loaded in the past,
        // it is no longer active), so we should create a reference counting
        // handle for it.
        dlHandlePtr = std::shared_ptr<void>(
              dlHandle, [](void *ptr) { dlclose(ptr); }); // NOLINT

        it->second = dlHandlePtr;
      }

      return dlHandlePtr;
    }

    /////////////////////////////////////////////////
    std::vector<Info> Loader::Implementation::LoadPlugins(
        const std::shared_ptr<void> &_dlHandle,
        const std::string& _pathToLibrary) const
    {
      std::vector<Info> loadedPlugins;

      // This function should never be called with a nullptr _dlHandle
      assert(_dlHandle &&
             "Bug in code: Loader::Implementation::LoadPlugins was called with "
             "a nullptr value for _dlHandle.");

      const std::string infoSymbol = "GzPluginHook";
      void *infoFuncPtr = dlsym(_dlHandle.get(), infoSymbol.c_str());

      // Does the library have the right symbol?
      if (nullptr == infoFuncPtr)
      {
        std::cerr << "Library [" << _pathToLibrary << "] does not export any "
                  << "plugins. The symbol [" << infoSymbol << "] is missing, "
                  << "or it is not externally visible.\n";

        return loadedPlugins;
      }

      using PluginLoadFunctionSignature =
          void(*)(void * const, const void ** const,
                  int *, std::size_t *, std::size_t *);

      // Note: InfoHook (below) is a function with a signature that matches
      // PluginLoadFunctionSignature.
      auto InfoHook =
          reinterpret_cast<PluginLoadFunctionSignature>(infoFuncPtr);

      int version = INFO_API_VERSION;
      std::size_t size = sizeof(Info);
      std::size_t alignment = alignof(Info);
      const InfoMap *allInfo = nullptr;

      // Note: static_cast cannot be used to convert from a T** to a void**
      // because of the possibility of breaking the type system by assigning a
      // Non-T pointer to the T* memory location. However, we need to retrieve
      // a reference to an STL-type using a C-compatible function signature, so
      // we resort to a reinterpret_cast to achieve this.
      //
      // Despite its many dangers, reinterpret_cast is well-defined for casting
      // between pointer types as of C++11, as explained in bullet point 1 of
      // the "Explanation" section in this reference:
      // http://en.cppreference.com/w/cpp/language/reinterpret_cast
      //
      // We have a tight grip over the implementation of how the `allInfo`
      // pointer gets used, so we do not need to worry about its memory address
      // being filled with a non-compatible type. The only risk would be if a
      // user decides to implement their own version of
      // GzPluginHook, but they surely would have no
      // incentive in doing that.
      //
      // Also note that the main reason we jump through these hoops is in order
      // to safely support plugin libraries on Windows that choose to compile
      // against the static runtime. Using this pointer-to-a-pointer approach is
      // the cleanest way to ensure that all dynamically allocated objects are
      // deleted in the same heap that they were allocated from.
      InfoHook(nullptr, reinterpret_cast<const void**>(&allInfo),
           &version, &size, &alignment);

      if (gz::plugin::INFO_API_VERSION != version)
      {
        // TODO(anyone): When we need to support multiple API versions,
        // put the logic for it into here.
        // We can call GzPluginHook(~) again with the
        // API version that it expects.

        std::cerr << "The library [" << _pathToLibrary << "] is using an "
                  << "incompatible version [" << version << "] of the "
                  << "gz::plugin Info API. The version in this library "
                  << "is [" << INFO_API_VERSION << "].\n";
        return loadedPlugins;
      }

      if (sizeof(Info) != size || alignof(Info) != alignment)
      {
        std::cerr << "The plugin::Info size or alignment are not consistent "
               << "with the expected values for the library [" << _pathToLibrary
               << "]:\n -- size: expected " << sizeof(Info)
               << " | received " << size << "\n -- alignment: expected "
               << alignof(Info) << " | received " << alignment << "\n"
               << " -- We will not be able to safely load plugins from that "
               << "library.\n";

        return loadedPlugins;
      }

      if (!allInfo)
      {
        std::cerr << "The library [" << _pathToLibrary << "] failed to provide "
                  << "gz::plugin Info for unknown reasons. Please report "
                  << "this error as a bug!\n";

        return loadedPlugins;
      }

      for (const InfoMap::value_type &info : *allInfo)
      {
        loadedPlugins.push_back(info.second);
      }

      return loadedPlugins;
    }

    /////////////////////////////////////////////////
    bool Loader::Implementation::ForgetLibrary(void *_dlHandle)
    {
      DlHandleToPluginMap::iterator it = dlHandleToPluginMap.find(_dlHandle);
      if (dlHandleToPluginMap.end() == it)
        return false;

      const std::unordered_set<std::string> &forgottenPlugins = it->second;

      for (const std::string &forget : forgottenPlugins)
      {
        // CRUCIAL DEV NOTE (MXG): Be sure to erase the Info from
        // `plugins` BEFORE erasing the plugin entry in `pluginToDlHandlePtrs`,
        // because the Info structs require the library to remain loaded
        // for the destructors of their `deleter` member variables.

        // This erase should come FIRST.
        this->filePlugins.ForgetInfo(forget);

        // This erase should come LAST.
        this->pluginToDlHandlePtrs.erase(forget);
      }

      // Dev note (MXG): We do not need to delete anything from `dlHandlePtrMap`
      // because it uses std::weak_ptrs. It will clear itself automatically.

      // Dev note (MXG): This erase call should come at the very end of this
      // function to ensure that the `forgottenPlugins` reference remains valid
      // while it is being used.
      this->dlHandleToPluginMap.erase(it);

      // Dev note (MXG): We do not need to call dlclose because that will be
      // taken care of automatically by the std::shared_ptr that manages the
      // shared library handle.

      return true;
    }
  }
}
