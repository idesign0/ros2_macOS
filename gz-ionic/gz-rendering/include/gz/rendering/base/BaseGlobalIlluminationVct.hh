/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_BASE_BASEGLOBALILLUMINATIONVCT_HH_
#define GZ_RENDERING_BASE_BASEGLOBALILLUMINATIONVCT_HH_

#include "gz/rendering/GlobalIlluminationVct.hh"

#include "gz/common/Util.hh"
#include "gz/math/Helpers.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    template <class T>
    class BaseGlobalIlluminationVct :
        public virtual GlobalIlluminationVct,
        public virtual T
    {
      protected: BaseGlobalIlluminationVct();

      public: virtual ~BaseGlobalIlluminationVct();

      // Documentation inherited.
      public: virtual void SetHighQuality(bool _highQuality) override;

      // Documentation inherited.
      public: virtual bool HighQuality() const override;

      // Documentation inherited.
      public: virtual void SetAnisotropic(bool _anisotropic) override;

      // Documentation inherited.
      public: virtual bool Anisotropic() const override;

      // Documentation inherited.
      public: virtual void SetConserveMemory(bool _conserveMemory) override;

      // Documentation inherited.
      public: virtual bool ConserveMemory() const override;

      // Documentation inherited.
      public: virtual void SetResolution(const uint32_t _resolution[3])
          override;

      // Documentation inherited.
      public: virtual const uint32_t* Resolution() const override;

      // Documentation inherited.
      public: virtual void SetOctantCount(const uint32_t _octants[3]) override;

      // Documentation inherited.
      public: virtual const uint32_t* OctantCount() const override;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseGlobalIlluminationVct<T>::BaseGlobalIlluminationVct()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseGlobalIlluminationVct<T>::~BaseGlobalIlluminationVct()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGlobalIlluminationVct<T>::SetHighQuality(
          bool /*_highQuality*/) // NOLINT
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseGlobalIlluminationVct<T>::HighQuality() const
    {
      return false;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGlobalIlluminationVct<T>::SetAnisotropic(
          bool /*_anisotropic*/) // NOLINT
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseGlobalIlluminationVct<T>::Anisotropic() const
    {
      return true;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGlobalIlluminationVct<T>::SetConserveMemory(
      bool /*_conserveMemory*/)
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseGlobalIlluminationVct<T>::ConserveMemory() const
    {
      return false;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGlobalIlluminationVct<T>::SetResolution(
      const uint32_t _resolution[3])
    {
      (void)(_resolution);
    GZ_ASSERT(math::isPowerOfTwo(_resolution[0]),
          "Resolution must be power of 2");
    GZ_ASSERT(math::isPowerOfTwo(_resolution[1]),
          "Resolution must be power of 2");
    GZ_ASSERT(math::isPowerOfTwo(_resolution[2]),
          "Resolution must be power of 2");
    }

    //////////////////////////////////////////////////
    template <class T>
    const uint32_t* BaseGlobalIlluminationVct<T>::Resolution() const
    {
      static const uint32_t tmp[3] = { 1u, 1u, 1u };
      return tmp;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseGlobalIlluminationVct<T>::SetOctantCount(
      const uint32_t _octants[3])
    {
      (void)(_octants);
    GZ_ASSERT(_octants[0] > 0u, "Subdivision must be greater than 0");
    GZ_ASSERT(_octants[1] > 0u, "Subdivision must be greater than 0");
    GZ_ASSERT(_octants[2] > 0u, "Subdivision must be greater than 0");
    }

    //////////////////////////////////////////////////
    template <class T>
    const uint32_t* BaseGlobalIlluminationVct<T>::OctantCount() const
    {
      static const uint32_t tmp[3] = { 1u, 1u, 1u };
      return tmp;
    }
    }
  }
}
#endif
