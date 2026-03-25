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


#ifndef GZ_PLUGIN_DETAIL_PLUGINPTR_HH_
#define GZ_PLUGIN_DETAIL_PLUGINPTR_HH_

#include <memory>
#include <utility>
#include <gz/plugin/PluginPtr.hh>
#include <gz/plugin/utility.hh>

namespace gz
{
  namespace plugin
  {
    //////////////////////////////////////////////////
    template <typename PluginType>
    TemplatePluginPtr<PluginType>::TemplatePluginPtr()
      : dataPtr(new PluginType)
    {
      // Do nothing
    }

    //////////////////////////////////////////////////
    template <typename PluginType>
    TemplatePluginPtr<PluginType>::TemplatePluginPtr(
        const TemplatePluginPtr &_other)
      : dataPtr(new PluginType)
    {
      this->dataPtr->PrivateCopyPluginInstance(*_other.dataPtr);
    }

    //////////////////////////////////////////////////
    template <typename PluginType>
    // the following is a false positive with cppcheck 1.82 fixed in 1.83
    // cppcheck-suppress syntaxError
    template <typename OtherPluginType>
    TemplatePluginPtr<PluginType>::TemplatePluginPtr(
        const TemplatePluginPtr<OtherPluginType> &_other)
      : dataPtr(new PluginType)
    {
      static_assert(ConstCompatible<PluginType, OtherPluginType>::value,
                "The requested PluginPtr cast would discard const qualifiers");
      this->dataPtr->PrivateCopyPluginInstance(*_other.dataPtr);
    }

    //////////////////////////////////////////////////
    template <typename PluginType>
    TemplatePluginPtr<PluginType>& TemplatePluginPtr<PluginType>::operator =(
        const TemplatePluginPtr &_other)
    {
      this->dataPtr->PrivateCopyPluginInstance(*_other.dataPtr);
      return *this;
    }

    //////////////////////////////////////////////////
    template <typename PluginType>
    template <typename OtherPluginType>
    TemplatePluginPtr<PluginType>& TemplatePluginPtr<PluginType>::operator =(
        const TemplatePluginPtr<OtherPluginType> &_other)
    {
      static_assert(ConstCompatible<PluginType, OtherPluginType>::value,
                "The requested PluginPtr cast would discard const qualifiers");
      this->dataPtr->PrivateCopyPluginInstance(*_other.dataPtr);
      return *this;
    }

    //////////////////////////////////////////////////
    template <typename PluginType>
    TemplatePluginPtr<PluginType>::TemplatePluginPtr(
        TemplatePluginPtr &&_other)
      : dataPtr(std::move(_other.dataPtr))
    {
      // Do nothing
    }

    //////////////////////////////////////////////////
    template <typename PluginType>
    TemplatePluginPtr<PluginType>& TemplatePluginPtr<PluginType>::operator =(
        TemplatePluginPtr &&_other)
    {
      this->dataPtr = std::move(_other.dataPtr);
      return *this;
    }

    //////////////////////////////////////////////////
    template <typename PluginType>
    TemplatePluginPtr<PluginType>& TemplatePluginPtr<PluginType>::operator =(
        std::nullptr_t)
    {
      this->Clear();
      return *this;
    }

    //////////////////////////////////////////////////
    template <typename PluginType>
    PluginType* TemplatePluginPtr<PluginType>::operator ->() const
    {
      return dataPtr.get();
    }

    //////////////////////////////////////////////////
    template <typename PluginType>
    PluginType& TemplatePluginPtr<PluginType>::operator *() const
    {
      return (*dataPtr);
    }

    //////////////////////////////////////////////////
    #define DETAIL_GZ_PLUGIN_PLUGINPTR_IMPLEMENT_OPERATOR(op)\
      template <typename PluginType>\
      bool TemplatePluginPtr<PluginType>::operator op (\
            const TemplatePluginPtr &_other) const\
      {\
        return (this->dataPtr->PrivateGetInstancePtr() op \
                _other.dataPtr->PrivateGetInstancePtr() );\
      }

    DETAIL_GZ_PLUGIN_PLUGINPTR_IMPLEMENT_OPERATOR( == )  // NOLINT
    DETAIL_GZ_PLUGIN_PLUGINPTR_IMPLEMENT_OPERATOR( < )   // NOLINT
    DETAIL_GZ_PLUGIN_PLUGINPTR_IMPLEMENT_OPERATOR( > )   // NOLINT
    DETAIL_GZ_PLUGIN_PLUGINPTR_IMPLEMENT_OPERATOR( != )  // NOLINT
    DETAIL_GZ_PLUGIN_PLUGINPTR_IMPLEMENT_OPERATOR( <= )  // NOLINT
    DETAIL_GZ_PLUGIN_PLUGINPTR_IMPLEMENT_OPERATOR( >= )  // NOLINT

    //////////////////////////////////////////////////
    template <typename PluginType>
    std::size_t TemplatePluginPtr<PluginType>::Hash() const
    {
      return std::hash< std::shared_ptr<void> >()(
                   this->dataPtr->PrivateGetInstancePtr());
    }

    //////////////////////////////////////////////////
    template <typename PluginType>
    bool TemplatePluginPtr<PluginType>::IsEmpty() const
    {
      return (nullptr == this->dataPtr->PrivateGetInstancePtr());
    }

    //////////////////////////////////////////////////
    template <typename PluginType>
    TemplatePluginPtr<PluginType>::operator bool() const
    {
      return !this->IsEmpty();
    }

    //////////////////////////////////////////////////
    template <typename PluginType>
    void TemplatePluginPtr<PluginType>::Clear()
    {
      this->dataPtr->PrivateCreatePluginInstance(nullptr, nullptr);
    }

    //////////////////////////////////////////////////
    template <typename PluginType>
    TemplatePluginPtr<PluginType>::TemplatePluginPtr(
        const ConstInfoPtr &_info,
        const std::shared_ptr<void> &_dlHandlePtr)
      : dataPtr(new PluginType)
    {
      dataPtr->PrivateCreatePluginInstance(_info, _dlHandlePtr);
    }

    //////////////////////////////////////////////////
    template <typename PluginType>
    TemplatePluginPtr<PluginType>::TemplatePluginPtr(
        const ConstInfoPtr &_info)
      : dataPtr(new PluginType)
    {
      dataPtr->PrivateCreateStaticPluginInstance(_info);
    }
  }
}

// Note that opening up namespace std is legal here because we are specializing
// a templated structure from the STL, which is permitted (and even encouraged).
namespace std
{
  /// \brief Template specialization that provides a hash function for PluginPtr
  /// so that it can easily be used in STL objects like std::unordered_set and
  /// std::unordered_map
  template <typename PluginType>
  struct hash<gz::plugin::TemplatePluginPtr<PluginType>>
  {
    size_t operator()(
        const gz::plugin::TemplatePluginPtr<PluginType> &ptr) const
    {
      return ptr.Hash();
    }
  };
}

#endif
