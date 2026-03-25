/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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
#ifndef GZ_COMMON_NODE_ANIMATION_HH_
#define GZ_COMMON_NODE_ANIMATION_HH_

#include <string>
#include <utility>

#include <gz/math/Matrix4.hh>
#include <gz/math/Pose3.hh>

#include <gz/utils/ImplPtr.hh>

#include <gz/common/graphics/Export.hh>

namespace gz
{
  namespace common
  {
    /// \class NodeAnimation NodeAnimation.hh gz/common/NodeAnimation.hh
    /// \brief Node animation
    class GZ_COMMON_GRAPHICS_VISIBLE NodeAnimation
    {
      /// \brief constructor
      /// \param[in] _name the name of the node
      public: explicit NodeAnimation(const std::string &_name);

      /// \brief Destructor.
      public: ~NodeAnimation();

      /// \brief Changes the name of the animation
      /// \param[in] the new name
      public: void SetName(const std::string &_name);

      /// \brief Returns the name
      /// \return the name
      public: std::string Name() const;

      /// \brief Adds a key frame at a specific time
      /// \param[in] _time the time of the key frame
      /// \param[in] _trans the transformation
      public: void AddKeyFrame(const double _time,
                  const math::Matrix4d &_trans);

      /// \brief Adds a key frame at a specific time
      /// \param[in] _time the tiem of the key frame
      /// \param[in] _pose the pose
      public: void AddKeyFrame(const double _time, const math::Pose3d &_pose);

      /// \brief Returns the number of key frames.
      /// \return the count
      public: unsigned int FrameCount() const;

      /// \brief Finds a key frame using the index. Note the index of a key
      /// frame can change as frames are added.
      /// \param[in] _i the index
      /// \param[out] _time the time of the frame, or -1 if the index id is out
      /// of bounds
      /// \param[out] _trans the transformation for this key frame
      public: void KeyFrame(const unsigned int _i, double &_time,
                      math::Matrix4d &_trans) const;

      /// \brief Returns a key frame using the index. Note the index of a key
      /// frame can change as frames are added.
      /// \param[in] _i the index
      /// \return a pair that contains the time and transformation. Time is -1
      /// if the index is out of bounds
      public: std::pair<double, math::Matrix4d> KeyFrame(
                      const unsigned int _i) const;

      /// \brief Returns the duration of the animations
      /// \return the time of the last animation
      public: double Length() const;

      /// \brief Returns a frame transformation at a specific time
      /// if a node does not exist at that time (with tolerance of 1e-6 sec),
      /// the transformation is interpolated.
      /// \param[in] _time the time
      /// \param[in] _loop when true, the time is divided by the duration
      /// (see GetLength)
      public: gz::math::Matrix4d FrameAt(const double _time,
                  const bool _loop = true) const;

      /// \brief Scales each transformation in the key frames. This only affects
      /// the translational values.
      /// \param[in] _scale the scaling factor
      public: void Scale(const double _scale);

      /// \brief Returns the time where a transformation's translational value
      /// along the X axis is equal to _x.
      /// When no transformation is found (within a tolerance of 1e-6), the time
      /// is interpolated.
      /// \param[in] _x the value along x. You must ensure that _x is within a
      /// valid range.
      public: double TimeAtX(const double _x) const;

      /// \brief Private data pointer.
      GZ_UTILS_IMPL_PTR(dataPtr)
    };
  }
}

#endif
