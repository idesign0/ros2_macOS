/*
 * Copyright 2021 Open Source Robotics Foundation
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

#ifndef GZ_COMMON_TEMPDIRECTORY_HH_
#define GZ_COMMON_TEMPDIRECTORY_HH_

#include <string>

#include <gz/common/Export.hh>
#include <gz/common/Filesystem.hh>

#include <gz/utils/ImplPtr.hh>

namespace gz
{
  namespace common
  {
    /// \brief Return the path to a directory suitable for temporary files.
    ///
    /// Calls std::filesystem::temp_directory_path, refer to the standard
    /// documentation for your platform for behaviors.
    /// \return A directory suitable for temporary files.
    std::string GZ_COMMON_VISIBLE tempDirectoryPath();

    /// \brief Create a directory in the tempDirectoryPath by expanding
    /// a name template
    ///
    /// On execution, will create the directory:
    ///  "_parentPath"/"_baseName" + "XXXXXX", where XXXXXX will be filled
    /// out by an OS-appropriate method (eg mkdtmp/_mktemp_s)
    ///
    /// \param[in] _baseName String to be prepended to the expanded template
    /// \param[in] _parentPath Location to create the directory
    /// \param[in] _warningOp Allow or suppress filesystem warnings
    /// \return Path to newly-created temporary directory
    std::string GZ_COMMON_VISIBLE createTempDirectory(
        const std::string &_baseName,
        const std::string &_parentPath,
        const FilesystemWarningOp _warningOp = FSWO_LOG_WARNINGS);

    /// \class TempDirectory TempDirectory.hh ignitin/common/TempDirectory.hh
    /// \brief Create a temporary directory in the OS temp location.
    /// Upon construction, the current working directory will be set to this
    /// new temporary directory.
    /// Upon destruction, the current working directory will be restored to the
    /// location when the TempDirectory object was constructed.
    class GZ_COMMON_VISIBLE TempDirectory
    {
      /// \brief Create a directory in the tempDirectoryPath by expanding
      /// a name template. This directory can also be automatically cleaned
      /// up when the object goes out of scope.
      ///
      /// The TempDirectory will have the form $TMPDIR/_subdir/_prefixXXXXX/
      ///
      /// \param[in] _prefix String to be expanded for the template
      /// \param[in] _subDir Subdirectory in OS $TMPDIR, if desired
      /// \param[in] _cleanup True to indicate that the filesystem should
      ///   be cleaned as part of the destructor
      public: TempDirectory(const std::string &_prefix = "temp_dir",
                            const std::string &_subDir = "gz",
                            bool _cleanup = true);

      /// \brief Create a directory in the tempDirectoryPath by expanding
      /// a name template. This directory can also be automatically cleaned
      /// up when the object goes out of scope.
      ///
      /// The TempDirectory will have the form _root/_subdir/_prefixXXXXX/
      ///
      /// \param[in[ _root Explicitly set the root directory
      /// \param[in] _prefix String to be expanded for the template
      /// \param[in] _subDir Subdirectory in OS _root, if desired
      /// \param[in] _cleanup True to indicate that the filesystem should
      ///   be cleaned as part of the destructor
      public: explicit TempDirectory(const std::string &_root,
                            const std::string &_prefix = "temp_dir",
                            const std::string &_subDir = "gz",
                            bool _cleanup = true);

      /// \brief Destroy the temporary directory, removing from filesystem
      /// if cleanup is true.
      public: ~TempDirectory();

      /// \brief Indicate if the TempDirectory object is in a valid state
      /// and that the folder exists on the filesystem
      /// \return true if the TempDirectory is valid
      public: bool Valid() const;

      /// \brief Set if the folder on disk should be cleaned.
      ///
      /// This is useful if you wish to clean by default during a test, but
      /// retain the contents of the TempDirectory if the test fails.
      /// \param[in] _doCleanup True to indicate that the filesystem should
      ///  be cleaned as part of the destructor
      public: void DoCleanup(bool _doCleanup);

      /// \brief Retrieve the current cleanup flag state
      /// \return true if filesystem cleanup will occur
      public: bool DoCleanup() const;

      /// \brief Retrieve the fully-expanded temporary directory path
      /// \return the temporary directory path
      public: std::string Path() const;

      /// \brief Pointer to private data.
      GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
    };
  }  // namespace common
}  // namespace gz
#endif  // GZ_COMMON_TEMPDIRECTORY_HH_
