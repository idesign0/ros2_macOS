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
#ifndef GZ_COMMON_SKELETONANIMATION_HH_
#define GZ_COMMON_SKELETONANIMATION_HH_

#include <map>
#include <utility>
#include <string>

#include <gz/math/Matrix4.hh>
#include <gz/math/Pose3.hh>

#include <gz/utils/ImplPtr.hh>

#include <gz/common/NodeAnimation.hh>
#include <gz/common/graphics/Export.hh>

namespace gz
{
  namespace common
  {
    /// \class SkeletonAnimation SkeletonAnimation.hh
    /// gz/common/SkeletonAnimation.hh
    /// \brief Skeleton animation
    class GZ_COMMON_GRAPHICS_VISIBLE SkeletonAnimation
    {
      /// \brief The Constructor
      /// \param[in] _name the name of the animation
      public: explicit SkeletonAnimation(const std::string &_name);

      /// \brief The destructor. Clears the list without destroying
      /// the animations
      public: ~SkeletonAnimation();

      /// \brief Changes the name
      /// \param[in] _name the new name
      public: void SetName(const std::string& _name);

      /// \brief Returns the name
      /// \return the name
      public: std::string Name() const;

      /// \brief Returns the number of animation nodes
      /// \return the count
      public: unsigned int NodeCount() const;

      /// \brief Returns the node animation for given node name
      /// \param[in] _name Name of node
      /// \return NodeAnimation object
      public: NodeAnimation *NodeAnimationByName(const std::string &_name)
          const;

      /// \brief Check all nodes for x displacement
      /// \return True if x displacement found
      public: bool XDisplacement() const;

      /// \brief Looks for a node with a specific name in the animations
      /// \param[in] _node the name of the node
      /// \return true if the node exits
      public: bool HasNode(const std::string &_node) const;

      /// \brief Adds or replaces a named key frame at a specific time
      /// \param[in] _node the name of the new or existing node
      /// \param[in] _time the time
      /// \param[in] _mat the key frame transformation
      public: void AddKeyFrame(const std::string &_node, const double _time,
                      const math::Matrix4d &_mat);

      /// \brief Adds or replaces a named key frame at a specific time
      /// \param[in] _node the name of the new or existing node
      /// \param[in] _time the time
      /// \param[in] _pose the key frame transformation as a math::Pose
      public: void AddKeyFrame(const std::string &_node, const double _time,
                      const math::Pose3d &_pose);

      /// \brief Returns the key frame transformation for a named animation at
      /// a specific time
      /// if a node does not exist at that time (with tolerance of 1e-6 sec),
      /// the transformation is interpolated.
      /// \param[in] _node the name of the animation node
      /// \param[in] _time the time
      /// \param[in] _loop when true, the time is divided by the duration
      /// (see GetLength)
      /// \return the transformation
      public: math::Matrix4d NodePoseAt(const std::string &_node,
                      const double _time, const bool _loop = true) const;

      /// \brief Returns a dictionary of transformations indexed by name at
      /// a specific time
      /// if a node does not exist at that specific time
      /// (with tolerance of 1e-6 sec), the transformation is interpolated.
      /// \param[in] _time the time
      /// \param[in] _loop when true, the time is divided by the duration
      /// (see GetLength)
      /// \return the transformation for every node
      public: std::map<std::string, math::Matrix4d> PoseAt(
                  const double _time, const bool _loop = true) const;

      /// \brief Returns a dictionary of transformations indexed by name where
      /// a named node transformation's translational value along the X axis is
      /// equal to _x.
      /// \param[in] _x the value along x. You must ensure that _x is within a
      /// valid range.
      /// \param[in] _node the name of the animation node
      /// \param[in] _loop when true, the time is divided by the duration
      /// (see GetLength)
      public: std::map<std::string, math::Matrix4d> PoseAtX(const double _x,
                      const std::string &_node, const bool _loop = true) const;

      /// \brief Scales every animation in the animations list
      /// \param[in] _scale the scaling factor
      public: void Scale(const double _scale);

      /// \brief Returns the duration of the animations
      /// \return the duration in seconds
      public: double Length() const;

      /// \brief Private data pointer.
      GZ_UTILS_IMPL_PTR(dataPtr)
    };
  }
}
#endif
