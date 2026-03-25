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
#ifndef GZ_RENDERING_OGRE2_OGRE2NATIVEWINDOW_HH_
#define GZ_RENDERING_OGRE2_OGRE2NATIVEWINDOW_HH_

#include "gz/rendering/base/BaseNativeWindow.hh"
#include "gz/rendering/ogre2/Export.hh"

#include <memory>

namespace Ogre
{
  class Window;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {

    class Ogre2NativeWindowPrivate;

    /// \brief Ogre2 implementation of NativeWindow
    class GZ_RENDERING_OGRE2_VISIBLE Ogre2NativeWindow :
      public virtual BaseNativeWindow
    {
      /// \brief Constructor
      private: Ogre2NativeWindow(Ogre::Window *_window);

      /// \brief Destructor
      public: virtual ~Ogre2NativeWindow();

      // Documentation Inherited.
      public: virtual void NotifyFocused(bool _focused) override;

      // Documentation Inherited.
      public: virtual void NotifyVisible(bool _visible) override;

      // Documentation Inherited.
      public: virtual void NotifyWindowMovedOrResized() override;

      // Documentation Inherited.
      public: virtual void RequestResolution(
            uint32_t _width, uint32_t _height) override;

      // Documentation Inherited.
      public: virtual void Draw(CameraPtr _camera) override;

      /// \brief Pointer to private data
      private: std::unique_ptr<Ogre2NativeWindowPrivate> dataPtr;

      /// \brief Make the render engine our friend
      private: friend class Ogre2RenderEngine;
    };
    }
  }
}
#endif
