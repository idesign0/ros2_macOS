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


#ifndef GZ_PLUGIN_DETAIL_REGISTER_HH_
#define GZ_PLUGIN_DETAIL_REGISTER_HH_

#include <typeinfo>
#include <utility>

#include <gz/plugin/Info.hh>
#include <gz/plugin/detail/Common.hh>
#include <gz/plugin/utility.hh>


#ifdef DETAIL_GZ_PLUGIN_VISIBLE
  #undef DETAIL_GZ_PLUGIN_VISIBLE
#endif

#if defined _WIN32 || defined __CYGWIN__
  #ifdef __GNUC__
    #define DETAIL_GZ_PLUGIN_VISIBLE __attribute__ ((dllexport))
  #else
    #define DETAIL_GZ_PLUGIN_VISIBLE __declspec(dllexport)
  #endif
#else
  #if __GNUC__ >= 4
    #define DETAIL_GZ_PLUGIN_VISIBLE __attribute__ ((visibility ("default")))
  #else
    #define DETAIL_GZ_PLUGIN_VISIBLE
  #endif
#endif

// extern "C" ensures that the symbol name of GzPluginHook
// does not get mangled by the compiler, so we can easily use dlsym(~) to
// retrieve it.
extern "C"
{
  /// \private GzPluginHook is the hook that's used by the Loader to
  /// retrieve Info from a shared library that provides plugins.
  ///
  /// The symbol is explicitly exported (visibility is turned on) using
  /// DETAIL_GZ_PLUGIN_VISIBLE to ensure that dlsym(~) is able to find it.
  ///
  /// DO NOT CALL THIS FUNCTION DIRECTLY OR CREATE YOUR OWN IMPLEMENTATION OF IT
  /// This function is used by the Registrar and Loader classes. Nothing else
  /// should be using it.
  ///
  /// \param[in] _inputSingleInfo
  ///   This argument is used by Registrar to input a single instance of
  ///   plugin::Info data. Loader will set this to a nullptr when trying to
  ///   receive data from the hook.
  ///
  /// \param[out] _outputAllInfo
  ///   Loader will pass in a pointer to a pointer of an InfoMap pertaining to
  ///   the highest API version that it knows of. If this GzPluginHook was
  ///   built against a version of gz-plugin that provides an equal or greater
  ///   API version, then GzPluginHook will modify *_outputAllInfo to
  ///   point at its internal &InfoMap that corresponds to the requested API
  ///   version, which is identified by _inputAndOutputAPIVersion.
  ///
  ///   If _inputAndOutputAPIVersion is greater than the highest API version
  ///   known by this GzPluginHook, then GzPluginHook will not
  ///   modify _outputAllInfo, and instead it will change the value pointed to
  ///   by _inputAndOutputAPIVersion so that it points to the highest API
  ///   version known by this GzPluginHook. At that point, Loader can call
  ///   this function again, but using the older API version which known by this
  ///   GzPluginHook.
  ///
  /// \param[in,out] _inputAndOutputAPIVersion
  ///   Loader will pass in a pointer to the highest API version that it knows.
  ///   If that API version is higher than what this GzPluginHook is
  ///   compatible with, then this GzPluginHook will change the value
  ///   pointed to by _inputAndOutputAPIVersion to the value of the highest API
  ///   version that it knows.
  ///
  /// \param[in,out] _inputAndOutputInfoSize
  ///   This input/output parameter is used for sanity checking. The Loader
  ///   inputs a pointer to the size that it expects for the Info data
  ///   structure, and GzPluginHook verifies that this expectation matches
  ///   its own Info size. Then, GzPluginHook will overwrite the value
  ///   pointed to so that it matches its own Info size value.
  ///
  /// \param[in,out] _inputAndOutputInfoAlign
  ///   Similar to _inputAndOutputInfoSize, this is used for sanity checking. It
  ///   inspects and returns the alignof(Info) value instead of the sizeof(Info)
  ///   value.
  DETAIL_GZ_PLUGIN_VISIBLE void GzPluginHook(
      const void *_inputSingleInfo,
      const void ** const _outputAllInfo,
      int *_inputAndOutputAPIVersion,
      std::size_t *_inputAndOutputInfoSize,
      std::size_t *_inputAndOutputInfoAlign)
#ifdef GZ_PLUGIN_REGISTER_MORE_TRANS_UNITS
  ; /* NOLINT */
#else
  // ATTENTION: If you get a linking error complaining about
  // multiple definitions of GzPluginHook,
  // then make sure that all but one of your
  // library's translation units (.cpp files) includes the
  // <gz/plugin/RegisterMore.hh> header instead of
  // <gz/plugin/Register.hh>.
  //
  // Only ONE and exactly ONE .cpp file in your library should include
  // Register.hh. All the rest should include RegisterMore.hh. It does not
  // matter which .cpp file you choose, as long as it gets compiled into your
  // plugin library.
  // ^^^^^^^^^^^^^^^^^^^^^ READ ABOVE FOR LINKING ERRORS ^^^^^^^^^^^^^^^^^^^^^
  {
    using InfoMap = gz::plugin::InfoMap;
    // We use a static variable here so that we can accumulate multiple
    // Info objects from multiple plugin registration calls within one
    // shared library, and then provide it all to the Loader through this
    // single hook.
    static InfoMap pluginMap;

    if (_inputSingleInfo)
    {
      // When _inputSingleInfo is not a nullptr, it means that one of the plugin
      // registration macros is providing us with some Info.
      const gz::plugin::Info *input =
          static_cast<const gz::plugin::Info*>(_inputSingleInfo);

      InfoMap::iterator it;
      bool inserted;

      // We use insert(~) to ensure that we do not accidentally overwrite some
      // existing information for the plugin that has this name.
      std::tie(it, inserted) =
          pluginMap.insert(std::make_pair(input->name, *input));

      if (!inserted)
      {
        // If the object was not inserted, then an entry already existed for
        // this plugin type. We should still insert each of the interface map
        // entries and aliases provided by the input info, just in case any of
        // them are missing from the currently existing entry. This allows the
        // user to specify different interfaces and aliases for the same plugin
        // type using different macros in different locations or across multiple
        // translation units.
        gz::plugin::Info &entry = it->second;

        for (const auto &interfaceMapEntry : input->interfaces)
          entry.interfaces.insert(interfaceMapEntry);

        for (const auto &aliasSetEntry : input->aliases)
          entry.aliases.insert(aliasSetEntry);
      }
    }

    if (_outputAllInfo)
    {
      // When _outputAllInfo is not a nullptr, it means that a Loader is
      // trying to retrieve Info from us.

      // The Loader should provide valid pointers to these fields as part
      // of a handshake procedure.
      if (nullptr == _inputAndOutputAPIVersion ||
          nullptr == _inputAndOutputInfoSize ||
          nullptr == _inputAndOutputInfoAlign)
      {
        // This should never happen, or else the function is being misused.
        // LCOV_EXCL_START
        return;
        // LCOV_EXCL_STOP
      }

      bool agreement = true;

      if (gz::plugin::INFO_API_VERSION != *_inputAndOutputAPIVersion)
      {
        // LCOV_EXCL_START
        agreement = false;
        // LCOV_EXCL_STOP
      }

      if (sizeof(gz::plugin::Info) != *_inputAndOutputInfoSize)
      {
        // LCOV_EXCL_START
        agreement = false;
        // LCOV_EXCL_STOP
      }

      if (alignof(gz::plugin::Info) != *_inputAndOutputInfoAlign)
      {
        // LCOV_EXCL_START
        agreement = false;
        // LCOV_EXCL_STOP
      }

      // The handshake parameters that were passed into us are overwritten with
      // the values that we have on our end. That way, if our Info API is
      // lower than that of the Loader, then the Loader will know
      // to call this function using an older version of Info, and then
      // convert it to the newer version on the loader side.
      //
      // This implementation might change when new API versions are introduced,
      // but this current implementation will still be forward compatible with
      // new API versions.
      *_inputAndOutputAPIVersion = gz::plugin::INFO_API_VERSION;
      *_inputAndOutputInfoSize = sizeof(gz::plugin::Info);
      *_inputAndOutputInfoAlign = alignof(gz::plugin::Info);

      // If the size, alignment, or API do not agree, we should return without
      // outputting any of the plugin info; otherwise, we could get a
      // segmentation fault.
      //
      // We will return the current API version to the Loader, and it may
      // then decide to attempt the call to this function again with the correct
      // API version if it supports backwards/forwards compatibility.
      if (!agreement)
      {
        // LCOV_EXCL_START
        return;
        // LCOV_EXCL_STOP
      }

      *_outputAllInfo = &pluginMap;
    }
  }
#endif
}

namespace gz
{
  namespace plugin
  {
    namespace detail
    {
      //////////////////////////////////////////////////
      /// \brief This specialization of the Register class will be called when
      /// one or more arguments are provided to the GZ_ADD_PLUGIN(~)
      /// macro. This is the only version of the Registrar class that is allowed
      /// to compile.
      template <typename PluginClass, typename... Interfaces>
      struct Registrar
      {
        /// \brief This function registers a plugin along with a set of
        /// interfaces that it provides.
        public: static void Register()
        {
          // Make all info that the user has specified
          Info info = MakeInfo<PluginClass, Interfaces...>();

          // Add the EnablePluginFromThis interface automatically if it is
          // inherited by PluginClass.
          IfEnablePluginFromThis<PluginClass>::AddIt(info.interfaces);

          // Send this information as input to this library's global repository
          // of plugins.
          GzPluginHook(&info, nullptr, nullptr, nullptr, nullptr);
        }


        public: template <typename... Aliases>
        static void RegisterAlias(Aliases&&... aliases)
        {
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

          // Send this information as input to this library's global repository
          // of plugins.
          GzPluginHook(&info, nullptr, nullptr, nullptr, nullptr);
        }
      };
    }
  }
}

//////////////////////////////////////////////////
/// This macro creates a uniquely-named class whose constructor calls the
/// gz::plugin::detail::Registrar::Register function. It then declares a
/// uniquely-named instance of the class with static lifetime. Since the class
/// instance has a static lifetime, it will be constructed when the shared
/// library is loaded. When it is constructed, the Register function will
/// be called.
#define DETAIL_GZ_ADD_PLUGIN_HELPER(UniqueID, ...) \
  namespace gz \
  { \
    namespace plugin \
    { \
      namespace \
      { \
        struct ExecuteWhenLoadingLibrary##UniqueID \
        { \
          ExecuteWhenLoadingLibrary##UniqueID() \
          { \
            ::gz::plugin::detail::Registrar<__VA_ARGS__>::Register(); \
          } \
        }; \
  \
        static ExecuteWhenLoadingLibrary##UniqueID execute##UniqueID; \
      } /* namespace */ \
    } \
  }


//////////////////////////////////////////////////
/// This macro is needed to force the __COUNTER__ macro to expand to a value
/// before being passed to the *_HELPER macro.
#define DETAIL_GZ_ADD_PLUGIN_WITH_COUNTER(UniqueID, ...) \
  DETAIL_GZ_ADD_PLUGIN_HELPER(UniqueID, __VA_ARGS__)


//////////////////////////////////////////////////
/// We use the __COUNTER__ here to give each plugin registration its own unique
/// name, which is required in order to statically initialize each one.
#define DETAIL_GZ_ADD_PLUGIN(...) \
  DETAIL_GZ_ADD_PLUGIN_WITH_COUNTER(__COUNTER__, __VA_ARGS__)


//////////////////////////////////////////////////
/// This macro creates a uniquely-named class whose constructor calls the
/// gz::plugin::detail::Registrar::RegisterAlias function. It then
/// declares a uniquely-named instance of the class with static lifetime. Since
/// the class instance has a static lifetime, it will be constructed when the
/// shared library is loaded. When it is constructed, the Register function will
/// be called.
#define DETAIL_GZ_ADD_PLUGIN_ALIAS_HELPER(UniqueID, PluginClass, ...) \
  namespace gz \
  { \
    namespace plugin \
    { \
      namespace \
      { \
        struct ExecuteWhenLoadingLibrary##UniqueID \
        { \
          ExecuteWhenLoadingLibrary##UniqueID() \
          { \
            ::gz::plugin::detail::Registrar<PluginClass>::RegisterAlias( \
                __VA_ARGS__); \
          } \
        }; \
  \
        static ExecuteWhenLoadingLibrary##UniqueID execute##UniqueID; \
      } /* namespace */ \
    } \
  }


//////////////////////////////////////////////////
/// This macro is needed to force the __COUNTER__ macro to expand to a value
/// before being passed to the *_HELPER macro.
#define DETAIL_GZ_ADD_PLUGIN_ALIAS_WITH_COUNTER( \
  UniqueID, PluginClass, ...) \
  DETAIL_GZ_ADD_PLUGIN_ALIAS_HELPER(UniqueID, PluginClass, __VA_ARGS__)


//////////////////////////////////////////////////
/// We use the __COUNTER__ here to give each plugin registration its own unique
/// name, which is required in order to statically initialize each one.
#define DETAIL_GZ_ADD_PLUGIN_ALIAS(PluginClass, ...) \
  DETAIL_GZ_ADD_PLUGIN_ALIAS_WITH_COUNTER( \
  __COUNTER__, PluginClass, __VA_ARGS__)


//////////////////////////////////////////////////
#define DETAIL_GZ_ADD_FACTORY(ProductType, FactoryType) \
  DETAIL_GZ_ADD_PLUGIN(FactoryType::Producing<ProductType>, FactoryType) \
  DETAIL_GZ_ADD_PLUGIN_ALIAS( \
      FactoryType::Producing<ProductType>, \
      ::gz::plugin::DemangleSymbol(typeid(ProductType).name()))


//////////////////////////////////////////////////
#define DETAIL_GZ_ADD_FACTORY_ALIAS(ProductType, FactoryType, ...) \
  DETAIL_GZ_ADD_FACTORY(ProductType, FactoryType) \
  DETAIL_GZ_ADD_PLUGIN_ALIAS(FactoryType::Producing<ProductType>, \
      __VA_ARGS__)

#endif
