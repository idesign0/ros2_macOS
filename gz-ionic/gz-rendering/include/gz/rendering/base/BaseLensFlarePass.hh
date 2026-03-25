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
#ifndef GZ_RENDERING_BASE_BASELENSFLAREPASS_HH_
#define GZ_RENDERING_BASE_BASELENSFLAREPASS_HH_

#include <string>
#include <gz/math/Rand.hh>

#include "gz/rendering/LensFlarePass.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {

    /// \brief Base Gaussian noise render pass.
    template <class T>
    class BaseLensFlarePass :
      public virtual LensFlarePass,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseLensFlarePass();

      /// \brief Destructor
      public: virtual ~BaseLensFlarePass() override;

      // Documentation inherited
      public: void SetLight(LightPtr _light) override;

      /// Light that generates the lens flare
      protected: LightPtr light;
    };

    //////////////////////////////////////////////////
    // BaseLensFlarePass
    //////////////////////////////////////////////////
    template <class T>
    BaseLensFlarePass<T>::BaseLensFlarePass()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseLensFlarePass<T>::~BaseLensFlarePass()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseLensFlarePass<T>::SetLight(LightPtr _light)
    {
      this->light = _light;
    }
    }
  }
}
#endif
