/*
 * Copyright (C) 2024 Open Source Robotics Foundation
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

#ifndef GZ_FUEL_TOOLS_RESULTTYPE_HH_
#define GZ_FUEL_TOOLS_RESULTTYPE_HH_

#if defined(_WIN32) && defined(DELETE)
// Windows defines "DELETE" as part of the Win32 "Standard Access Rights"
// Since we aren't using that here, go ahead and undefine so that we can
// use it as part of the ResultType enum class.
// Ref: https://learn.microsoft.com/en-us/windows/win32/secauthz/standard-access-rights
#undef DELETE
#endif

namespace gz::fuel_tools
{
  /// Result type.
  enum class ResultType
  {
    /// \brief Uninitialized type.
    UNKNOWN = 0,

    /// \brief Delete successful.
    DELETE,

    /// \brief Model not found.
    DELETE_NOT_FOUND,

    /// \brief Delete failed. Other errors.
    /// \sa ReadableResult
    DELETE_ERROR,

    /// \brief Fetch successful.
    FETCH,

    /// \brief Model already exists.
    FETCH_ALREADY_EXISTS,

    /// \brief Model not found.
    FETCH_NOT_FOUND,

    /// \brief Fetch failed. Other errors.
    /// \sa ReadableResult
    FETCH_ERROR,

    /// \brief Upload successful.
    UPLOAD,

    /// \brief Model already exists.
    UPLOAD_ALREADY_EXISTS,

    /// \brief Upload failed. Other errors.
    /// \sa ReadableResult
    UPLOAD_ERROR,

    /// \brief Patch failed.
    PATCH_ERROR,

    /// \brief Patch successful.
    PATCH,
  };
}  // namespace gz::fuel_tools

#endif  // GZ_FUEL_TOOLS_RESULTTYPE_HH_
