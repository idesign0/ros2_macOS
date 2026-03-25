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

#ifndef GZ_FUEL_TOOLS_WORLDITER_HH_
#define GZ_FUEL_TOOLS_WORLDITER_HH_

#include <memory>

#include "gz/fuel_tools/Helpers.hh"

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace gz::fuel_tools
{
  /// \brief Forward declarations
  class WorldIdentifier;
  class WorldIterPrivate;
  class WorldIterFactory;

  /// \brief class for iterating through worlds
  class GZ_FUEL_TOOLS_VISIBLE WorldIter
  {
    friend WorldIterFactory;

    /// \brief Construct an iterator with the data it needs to function
    /// \param[in] _dptr Pointer to private data to copy
    protected: explicit WorldIter(std::unique_ptr<WorldIterPrivate> _dptr);

    /// \brief Move constructor
    /// \param[in] _old Iter to move
    public: WorldIter(WorldIter && _old);

    /// \brief Default destructor.
    public: ~WorldIter();

    /// \brief Conversion operator
    /// \return False once the iterator is one past the end of the worlds
    public: operator bool() const;

    /// \brief Conversion operator
    /// \return Internal world identifier.
    public: operator WorldIdentifier() const;

    /// \brief Prefix increment
    /// \return Next iteration
    public: WorldIter &operator++();

    /// \brief -> operator
    /// \return Internal world identifier
    public: WorldIdentifier *operator->();

    /// \brief Dereference operator
    /// \return Internal world identifier
    public: WorldIdentifier &operator*();

    /// \brief Private data pointer.
    private: std::unique_ptr<WorldIterPrivate> dataPtr;
  };
}  // namespace gz::fuel_tools

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // GZ_FUEL_TOOLS_WORLDITER_HH_
