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
#ifndef GZ_RENDERING_NATIVEWINDOW_HH_
#define GZ_RENDERING_NATIVEWINDOW_HH_

#include <cstdint>
#include <map>
#include <string>
#include "gz/rendering/config.hh"
#include "gz/rendering/Export.hh"
#include "gz/rendering/RenderTypes.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \class NativeWindow NativeWindow.hh gz/rendering/NativeWindow.hh
    /// \brief An abstract interface to a basic native window render-engine.
    ///
    /// Note: This is useful when you need a basic single window with both
    /// great performance and compatibility. But e.g. creating
    /// more than one window may have additional complexities depending
    /// on OS and RenderSystem
    class GZ_RENDERING_VISIBLE NativeWindow
    {
      /// \brief Constructor
      public: NativeWindow();

      /// \brief Destructor
      public: virtual ~NativeWindow();

      /// \brief Tells the native window whether it's under focus
      /// \param[in] _focused True if we acquired focus. False if we lost it
      public: virtual void NotifyFocused(bool _focused) = 0;

      /// \brief Tells the native window whether it's visible
      /// \param[in] _visible True if we are at least partially visible.
      /// False otherwise
      public: virtual void NotifyVisible(bool _visible) = 0;

      /// \brief Call this when window resolution has changed
      public: virtual void NotifyWindowMovedOrResized() = 0;

      /// \brief Requests a particular resolution.
      /// It may not be possible to satisfy it though and the new resolution
      /// may be different from current and requested (e.g.
      /// if you request 1920x1080 when that's the max monitor resolution
      /// after evaluating window borders, the title, and the start menu
      /// the OS may shrink the window so the whole thing is visible)
      /// \param[in] _width New width of the window being requested
      /// \param[in] _height New height of the window being requested
      public: virtual void RequestResolution(
            uint32_t _width, uint32_t _height) = 0;

      /// \brief A native window is not associated with anything.
      /// Calling this function will draw whatever is currently
      /// the given camera.
      /// \remarks Assumes _camera->Render() has already been called
      /// If the camera resolution doesn't match, the contents will
      /// be stretched.
      /// \param[in] _camera Camera to draw
      public: virtual void Draw(CameraPtr _camera) = 0;
    };
    }
  }
}
#endif
