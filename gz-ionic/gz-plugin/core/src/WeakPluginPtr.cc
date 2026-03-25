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

#include <gz/plugin/WeakPluginPtr.hh>

namespace gz
{
  namespace plugin
  {
    /////////////////////////////////////////////////
    class WeakPluginPtr::Implementation
    {
      public: std::weak_ptr<void> instance;
      public: std::weak_ptr<const Info> info;
    };

    /////////////////////////////////////////////////
    WeakPluginPtr::WeakPluginPtr()
      : pimpl(new Implementation())
    {
      // Do nothing
    }

    /////////////////////////////////////////////////
    WeakPluginPtr::WeakPluginPtr(const WeakPluginPtr &_other)
      : pimpl(new Implementation())
    {
      *this->pimpl = *_other.pimpl;
    }

    /////////////////////////////////////////////////
    WeakPluginPtr::WeakPluginPtr(WeakPluginPtr &&_other)
      : pimpl(new Implementation())
    {
      this->pimpl = std::move(_other.pimpl);
    }

    /////////////////////////////////////////////////
    WeakPluginPtr::WeakPluginPtr(const PluginPtr &_ptr)
      : pimpl(new Implementation())
    {
      *this = _ptr;
    }

    /////////////////////////////////////////////////
    WeakPluginPtr &WeakPluginPtr::operator=(const WeakPluginPtr &_other)
    {
      *this->pimpl = *_other.pimpl;
      return *this;
    }

    /////////////////////////////////////////////////
    WeakPluginPtr &WeakPluginPtr::operator=(WeakPluginPtr &&_other)
    {
      this->pimpl = std::move(_other.pimpl);
      return *this;
    }

    /////////////////////////////////////////////////
    WeakPluginPtr &WeakPluginPtr::operator=(const PluginPtr &_ptr)
    {
      this->pimpl->instance = _ptr->PrivateGetInstancePtr();
      this->pimpl->info = _ptr->PrivateGetInfoPtr();
      return *this;
    }

    /////////////////////////////////////////////////
    PluginPtr WeakPluginPtr::Lock() const
    {
      // CRUCIAL DEV NOTE (MXG): We must lock the instance before the info.
      // We must never allow info to get deleted before the plugin instance,
      // because otherwise the library might get unloaded before the info is
      // destructed, but the Info's destructor depends on the library.
      // TODO(MXG): Consider ways to make this less fragile.
      std::shared_ptr<void> instance = this->pimpl->instance.lock();
      ConstInfoPtr info = this->pimpl->info.lock();

      PluginPtr ptr;
      // NOTE(MXG): We do not want to make a PluginPtr constructor overload for
      // this, because its signature would be too easily confused with the
      // constructor that takes a ConstInfoPtr and a std::shared_ptr<void> to a
      // dl handle. Using an explicitly named function avoids any ambiguity.
      ptr->PrivateCopyPluginInstance(info, instance);

      return ptr;
    }

    /////////////////////////////////////////////////
    bool WeakPluginPtr::IsExpired() const
    {
      // CRUCIAL DEV NOTE (MXG): We must lock the instance before the info.
      // We must never allow info to get deleted before the plugin instance,
      // because otherwise the library might get unloaded before the info is
      // destructed, but the Info's destructor depends on the library.
      // TODO(MXG): Consider ways to make this less fragile.
      std::shared_ptr<void> instance = this->pimpl->instance.lock();
      ConstInfoPtr info = this->pimpl->info.lock();

      return !(instance && info);
    }

    /////////////////////////////////////////////////
    WeakPluginPtr::~WeakPluginPtr()
    {
      // Do nothing
    }
  }
}
