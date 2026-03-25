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
#ifndef GZ_RENDERING_OGRE2_OGRE2CONVERSIONS_HH_
#define GZ_RENDERING_OGRE2_OGRE2CONVERSIONS_HH_

#include <gz/math/Color.hh>
#include <gz/math/Matrix4.hh>
#include <gz/math/Quaternion.hh>
#include <gz/math/Vector3.hh>

#include "gz/rendering/config.hh"
#include "gz/rendering/PixelFormat.hh"
#include "gz/rendering/ogre2/Export.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif

#include <OgreColourValue.h>
#include <OgreVector3.h>
#include <OgrePixelFormatGpu.h>

#ifdef _MSC_VER
  #pragma warning(pop)
#endif

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \addtogroup gz_rendering
    /// \{

    /// \brief Conversions Conversions.hh rendering/Conversions.hh
    /// \brief A set of utility function to convert between Gazebo and Ogre
    /// data types
    class GZ_RENDERING_OGRE2_VISIBLE Ogre2Conversions
    {
      /// \brief Return the equivalent ogre color
      /// \param[in] _color gz-math color to convert
      /// \return Ogre color value
      public: static Ogre::ColourValue Convert(
          const math::Color &_color);

      /// \brief Return the equivalent gz-math color
      /// \param[in] _color Ogre color to convert
      /// \return gz-math color value
      public: static math::Color Convert(
          const Ogre::ColourValue &_color);

      /// \brief return Ogre Vector from gz-math Vector3
      /// \param[in] _vector gz-math vector
      /// \return Ogre vector
      public: static Ogre::Vector3 Convert(const math::Vector3d &_vector);

      /// \brief return gz-math Vector from ogre Vector3
      /// \param[in] _vector Ogre vector
      /// \return gz-math vector
      public: static math::Vector3d Convert(const Ogre::Vector3 &_vector);

      /// \brief gz-math quaternion to Ogre quaternion
      /// \param[in] _quat gz-math quaternion
      /// \return Ogre quaternion
      public: static Ogre::Quaternion Convert(const math::Quaterniond &_quat);

      /// \brief Ogre quaternion to gz-math quaternion
      /// \param[in] _quat Ogre quaternion
      /// return gz-math quaternion
      public: static math::Quaterniond Convert(const Ogre::Quaternion &_quat);

      /// \brief gz-math angle to Ogre angle
      /// \param[in] _angle gz-math angle
      /// \return Ogre angle
      public: static Ogre::Radian Convert(const math::Angle &_angle);

      /// \brief Ogre angle to gz-math angle
      /// \param[in] _angle Ogre angle
      /// return gz-math angle
      public: static math::Angle Convert(const Ogre::Radian &_angle);

      /// \brief Ogre Matrix4 to Gazebo Math Matrix4d
      /// \param[in] _m Ogre Matrix4
      /// \return Gazebo Math Matrix4d
      public: static math::Matrix4d Convert(const Ogre::Matrix4 &_m);

      /// \brief Gazebo Math Matrix4d to Ogre Matrix4
      /// \param[in] _m Gazebo Math Matrix4d
      /// \return Ogre Matrix4
      public: static Ogre::Matrix4 Convert(const math::Matrix4d &_m);

      /// \brief gz-rendering PixelFormat to Ogre PixelFormat
      /// \param[in] _format gz-rendering PixelFormat
      /// return Ogre PixelFormat
      public: static Ogre::PixelFormatGpu Convert(PixelFormat _format);

      /// \brief A list of ogre pixel formats
      private: static const Ogre::PixelFormatGpu ogrePixelFormats[PF_COUNT];
    };
    /// \}
    }
  }
}
#endif
