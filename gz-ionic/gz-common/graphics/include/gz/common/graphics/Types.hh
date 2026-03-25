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

#ifndef GZ_COMMON_GRAPHICS_TYPES_HH_
#define GZ_COMMON_GRAPHICS_TYPES_HH_

#include <memory>

// This header contains forward declarations for some graphics types
namespace gz
{
  namespace common
  {
    class Material;
    class Mesh;
    class Skeleton;
    class SubMesh;

    /// \def MaterialPtr
    /// \brief Shared pointer to a Material object
    using MaterialPtr = std::shared_ptr<Material>;

    /// \def MeshPtr
    /// \brief Standrd shared pointer to a Mesh object
    using MeshPtr = std::shared_ptr<Mesh>;

    /// \def SubMeshPtr
    /// \brief Shared pointer to a SubMesh object
    using SubMeshPtr = std::shared_ptr<SubMesh>;

    /// \def SkeletonPtr
    /// \brief Shared pointer to a Skeleton object
    using SkeletonPtr = std::shared_ptr<Skeleton>;
  }
}

#endif
