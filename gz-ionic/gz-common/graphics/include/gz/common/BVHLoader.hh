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
#ifndef GZ_COMMON_BVHLOADER_HH_
#define GZ_COMMON_BVHLOADER_HH_

#include <memory>
#include <string>

#include <gz/math/Pose3.hh>
#include <gz/common/graphics/Export.hh>

namespace gz
{
  namespace common
  {
    class Skeleton;

    /// \class BVHLoader BVHLoader.hh gz/common/BVHLoader.hh
    /// \brief Handles loading BVH animation files
    class GZ_COMMON_GRAPHICS_VISIBLE BVHLoader
    {
      /// \brief Constructor
      public: BVHLoader();

      /// \brief Desutrctor
      public: ~BVHLoader();

      /// \brief Load a BVH file
      /// \param[in] _filename BVH file to load
      /// \param[in] _scale Scaling factor to apply to the skeleton
      /// \return A pointer to a new Skeleton
      public: std::unique_ptr<Skeleton> Load(
                  const std::string &_filename, const double _scale);
    };
  }
}

#endif
