/*
 * Copyright (C) 2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef GZ_COMMON_SVGLOADER_HH_
#define GZ_COMMON_SVGLOADER_HH_

#include <memory>
#include <string>
#include <vector>

#include <gz/math/Vector2.hh>
#include <gz/math/Matrix3.hh>

#include <gz/utils/ImplPtr.hh>

#include <gz/common/graphics/Export.hh>
#include <gz/common/Console.hh>

namespace gz
{
  namespace common
  {
    /// \brief SVG command data structure
    class GZ_COMMON_GRAPHICS_VISIBLE SVGCommand
    {
      /// \brief Constructor
      public: SVGCommand() : cmd(' ') {}

      /// \brief Destructor
      public: virtual ~SVGCommand() = default;

      /// \brief A letter that describe the segment
      public: char cmd;

#ifdef _WIN32
// Disable warning C4251
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
      /// \brief Coordinates for the command
      public: std::vector<double> numbers;
#ifdef _WIN32
#pragma warning(pop)
#endif
    };

    /// \brief An SVG path element data structure
    struct GZ_COMMON_GRAPHICS_VISIBLE SVGPath
    {
#ifdef _WIN32
// Disable warning C4251
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

      /// \brief An id or name
      std::string id;

      /// \brief The style (i.e. stroke style, color, thickness etc)
      std::string style;

      /// \brief A 2D transform (or a list of transforms)
      gz::math::Matrix3d transform;

      /// \brief A list of subpaths (as lists of commands)
      std::vector< std::vector<SVGCommand> > subpaths;

      /// \brief The polylines described by the commands
      std::vector< std::vector<gz::math::Vector2d> > polylines;
#ifdef _WIN32
#pragma warning(pop)
#endif
    };

    /// \brief A loader for SVG files
    class GZ_COMMON_GRAPHICS_VISIBLE SVGLoader
    {
      /// \brief Constructor
      /// \param[in] _samples The number of points for cubic spline segments
      /// Samples must be greater than zero. A value of one will be used if
      /// _samples is set to zero.
      public: explicit SVGLoader(const unsigned int _samples);

      /// \brief destructor
      public: ~SVGLoader();

      /// \brief Reads an SVG file and loads all the paths
      /// \param[in] _filename The SVG file
      /// \param[out] _paths Vector that receives path datai
      /// \return false when the file cannot be processed
      public: bool Parse(const std::string &_filename,
                         std::vector<SVGPath> &_paths);

      /// \brief Reads in paths and outputs closed polylines and open polylines
      /// \param[in] _paths The input paths
      /// \param[in] _tol Tolerence when comparing distance between 2 points.
      /// \param[out] _closedPolys A vector to collect new closed loops
      /// \param[out] _openPolys A vector to collect non closed paths
      public: static void PathsToClosedPolylines(
          const std::vector<common::SVGPath> &_paths,
          const double _tol,
          std::vector< std::vector<gz::math::Vector2d> > &_closedPolys,
          std::vector< std::vector<gz::math::Vector2d> > &_openPolys);

      /// \brief Outputs the content of the paths to file (or console)
      /// \param[in] _paths The paths
      /// \param[in] _out The output stream (can be a file or std::cout)
      public: void DumpPaths(const std::vector<SVGPath> &_paths,
                             std::ostream &_out) const;

      /// \internal
      /// \brief Private data pointer.
      GZ_UTILS_IMPL_PTR(dataPtr)
    };
  }
}

#endif
