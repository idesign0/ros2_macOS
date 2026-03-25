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

#ifndef GZ_PLUGIN_WEAKPLUGINPTR_HH_
#define GZ_PLUGIN_WEAKPLUGINPTR_HH_

#include <memory>

#include <gz/plugin/PluginPtr.hh>

namespace gz
{
  namespace plugin
  {
    /// \brief WeakPluginPtr is a non-reference-holding smart pointer for a
    /// Plugin. WeakPluginPtr is analogous to std::weak_ptr where PluginPtr is
    /// analogous to std::shared_ptr.
    ///
    /// Use Lock() to receive a reference-holding PluginPtr that points to the
    /// Plugin that this WeakPluginPtr is meant to reference.
    ///
    /// If the Plugin is deleted while this WeakPluginPtr is referring to it,
    /// then Lock() will return an empty PluginPtr, and IsExpired() will return
    /// true.
    class GZ_PLUGIN_VISIBLE WeakPluginPtr
    {
      /// \brief Default constructor
      public: WeakPluginPtr();

      /// \brief Copy constructor
      /// \param[in] _other
      ///   Another WeakPluginPtr to copy
      public: WeakPluginPtr(const WeakPluginPtr &_other);

      /// \brief Move constructor
      /// \param[in] _other
      ///   Another WeakPluginPtr to move from
      public: WeakPluginPtr(WeakPluginPtr &&_other);

      /// \brief Construct from a live PluginPtr
      /// \param[in] _ptr
      ///   A live PluginPtr to refer to. As long as the Plugin instance
      ///   continues to be held by PluginPtr containers, you can use Lock() to
      ///   retrieve a reference to it.
      // cppcheck-suppress noExplicitConstructor
      public: WeakPluginPtr(const PluginPtr &_ptr);

      /// \brief Copy assignment operator
      /// \param[in] _other
      ///   Another WeakPluginPtr to copy
      /// \return reference to this
      public: WeakPluginPtr &operator=(const WeakPluginPtr &_other);

      /// \brief Move assignment operator
      /// \param[in] _other
      ///   Another WeakPluginPtr to move from
      /// \return reference to this
      public: WeakPluginPtr &operator=(WeakPluginPtr &&_other);

      /// \brief Assign from a live PluginPtr
      /// \param[in] _ptr
      ///   A live PluginPtr to refer to. As long as the Plugin instance
      ///   continues to be held by PluginPtr containers, you can use Lock() to
      ///   retrieve a reference to it.
      public: WeakPluginPtr &operator=(const PluginPtr &_ptr);

      /// \brief Retrieve the PluginPtr that this WeakPluginPtr refers to, if it
      /// is still available. Otherwise, retrieve an empty PluginPtr.
      /// \return The PluginPtr that this WeakPluginPtr refers to.
      public: PluginPtr Lock() const;

      /// \brief Check whether the referenced Plugin has already expired.
      /// \return true if this PluginPtr is expired, false otherwise.
      public: bool IsExpired() const;

      /// \brief Destructor
      public: ~WeakPluginPtr();

      private: class Implementation;
      GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \brief PIMPL pointer to the implementation of this class
      private: std::unique_ptr<Implementation> pimpl;
      GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
    };
  }
}

#endif
