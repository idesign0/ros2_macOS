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
#ifndef GZ_COMMON_NODE_TRANSFORM_HH_
#define GZ_COMMON_NODE_TRANSFORM_HH_
#include <memory>
#include <string>

#include <gz/math/Matrix4.hh>
#include <gz/math/Vector3.hh>

#include <gz/utils/ImplPtr.hh>

#include <gz/common/graphics/Export.hh>
#include <gz/common/Util.hh>

namespace gz
{
  namespace common
  {
    /// \class NodeTransform NodeTransform.hh gz/common/NodeTransform.hh
    /// \brief A transformation node
    class GZ_COMMON_GRAPHICS_VISIBLE NodeTransform
    {
      /// \brief Constructor
      /// \param[in] _type the type of transform
      public: explicit NodeTransform(const NodeTransformType _type = MATRIX);

      /// \brief Constructor
      /// \param[in] _mat the matrix
      /// \param[in] _sid identifier
      /// \param[in] _type the type of transform
      public: NodeTransform(const math::Matrix4d &_mat,
                  const std::string &_sid = "_default_",
                  const NodeTransformType _type = MATRIX);

      /// \brief Destructor.
      public: ~NodeTransform();

      /// \brief Assign a transformation
      /// \param[in] _mat the transform
      public: void Set(const math::Matrix4d &_mat);

      /// \brief Returns the transformation matrix
      /// \return the matrix
      public: math::Matrix4d Get() const;

      /// \brief Set transform type
      /// \param[in] _type the type
      public: void SetType(const NodeTransformType _type);

      /// \brief Returns the transformation type
      /// \return the type
      public: NodeTransformType Type() const;

      /// \brief Set the SID
      /// \param[in] _sid the sid
      public: void SetSID(const std::string &_sid);

      /// \brief Returns the SID
      /// \return the SID
      public: std::string SID() const;

      /// \brief Set a transformation matrix component value
      /// \param[in] _idx the component index
      /// \param[in] _value the value
      public: void SetComponent(const unsigned int _idx, const double _value);

      /// \brief Set source data values
      /// \param[in] _mat the values
      public: void SetSourceValues(const math::Matrix4d &_mat);

      /// \brief Set source data values
      /// \param[in] _vec the values
      public: void SetSourceValues(const math::Vector3d &_vec);

      /// \brief Sets source matrix values from roation
      /// \param[in] _axis of rotation
      /// \param[in] _angle of rotation
      public: void SetSourceValues(const math::Vector3d &_axis,
                  const double _angle);

      /// \brief Sets the transform matrix from the source according to the type
      public: void RecalculateMatrix();

      /// \brief Prints the transform matrix to standard out
      public: void PrintSource() const;

      /// \brief Matrix cast operator
      /// \return the transform
      public: math::Matrix4d operator()() const;

      /// \brief Node transform multiplication operator
      /// \param[in] _t a transform
      /// \return transform matrix multiplied by _t's transform
      public: math::Matrix4d operator*(const NodeTransform &_t) const;

      /// \brief Matrix multiplication operator
      /// \param[in] _m a matrix
      /// \return transform matrix multiplied by _m
      public: math::Matrix4d operator*(const math::Matrix4d &_m) const;

      /// \brief Private data pointer.
      GZ_UTILS_IMPL_PTR(dataPtr)
    };
  }
}
#endif
