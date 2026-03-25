/*
 * Copyright 2017 Open Source Robotics Foundation
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

#ifndef GZ_COMMON_FILESYSTEM_HH_
#define GZ_COMMON_FILESYSTEM_HH_

#include <string>

#include <gz/common/Export.hh>

#include <gz/utils/ImplPtr.hh>

namespace gz
{
  namespace common
  {
    /// \brief Options for how to handle errors that occur in functions that
    /// manipulate the filesystem.
    enum FilesystemWarningOp
    {
      /// \brief Errors that occur during filesystem manipulation should be
      /// logged as warnings using gzwarn. (Recommended)
      FSWO_LOG_WARNINGS = 0,

      /// \brief Errors that occur during filesystem manipulation should
      /// not be logged. The user will be responsible for checking the
      /// system's error flags.
      FSWO_SUPPRESS_WARNINGS
    };

    /// \brief Determine whether the given path exists on the filesystem.
    /// \param[in] _path  The path to check for existence
    /// \return True if the path exists on the filesystem, false otherwise.
    bool GZ_COMMON_VISIBLE exists(const std::string &_path);

    /// \brief Determine whether the given path is a directory.
    /// \param[in] _path  The path to check
    /// \return True if given path exists and is a directory, false otherwise.
    bool GZ_COMMON_VISIBLE isDirectory(const std::string &_path);

    /// \brief Check if the given path is a file.
    /// \param[in] _path Path to a file.
    /// \return True if _path is a file.
    bool GZ_COMMON_VISIBLE isFile(const std::string &_path);

    /// \brief Check if the given path is relative.
    /// \param[in] _path Path.
    /// \return True if _path is relative.
    bool GZ_COMMON_VISIBLE isRelativePath(const std::string &_path);

    /// \brief Create a new directory on the filesystem.  Intermediate
    ///        directories must already exist.
    /// \param[in] _path  The new directory path to create
    /// \return True if directory creation was successful, false otherwise.
    bool GZ_COMMON_VISIBLE createDirectory(const std::string &_path);

    /// \brief Create directories for the given path
    /// \param[in] _path Path to create directories from
    /// \return true on success
    bool GZ_COMMON_VISIBLE createDirectories(const std::string &_path);

    /// \brief Append the preferred path separator character for this platform
    ///        onto the passed-in string.
    /// \param[in] _s  The path to start with.
    /// \return The original path with the platform path separator appended.
    std::string GZ_COMMON_VISIBLE const separator(
        std::string const &_s);

    /// \brief Replace forward-slashes '/' with the preferred directory
    /// separator of the current operating system. On Windows, this will turn
    /// forward-slashes into backslashes. If forward-slash is the preferred
    /// separator of the current operating system, this will do nothing.
    ///
    /// Note that this will NOT convert backslashes (or any other separator)
    /// into forward slashes, even on operating systems that use
    /// forward-slashes as separators.
    ///
    /// \param[out] _path This string will be directly modified by
    /// replacing its
    /// forward-slashes with the preferred directory separator of the current
    /// operating system.
    void GZ_COMMON_VISIBLE changeFromUnixPath(std::string &_path);

    /// \brief Returns a copy of _path which has been passed through
    /// changeFromUnixPath.
    ///
    /// \param[in] _path The path to start with
    /// \return A modified path that uses the preferred directory separator of
    /// the current operating system.
    std::string GZ_COMMON_VISIBLE copyFromUnixPath(
        const std::string &_path);

    /// \brief Replace the preferred directory separator of the current
    /// operating system with a forward-slash '/'. On Windows, this will turn
    /// backslashes into forward-slashes.
    ///
    /// \param[out] _path This string will be directly modified to use forward
    /// slashes to separate its directory names.
    void GZ_COMMON_VISIBLE changeToUnixPath(std::string &_path);

    /// \brief Returns a copy of _path which has been passed through
    /// changeToUnixPath.
    ///
    /// \param[in] _path The path to start with
    /// \return A modified path that uses forward slashes to separate
    /// directory names.
    std::string GZ_COMMON_VISIBLE copyToUnixPath(
        const std::string &_path);

    /// \brief Get the absolute path of a provided path. Relative paths are
    /// resolved relative to the current working directory.
    /// \param[in] _path Relative or absolute path.
    /// \return Absolute path (with platform-dependent directory separators).
    std::string GZ_COMMON_VISIBLE absPath(const std::string &_path);

    /// \brief Join two strings together to form a path
    /// \param[in] _path1 the left portion of the path
    /// \param[in] _path2 the right portion of the path
    /// \return Joined path. The function can do simplifications such as
    /// elimination of ../ and removal of duplicate // (but is not guaranteed to
    /// do so).
    std::string GZ_COMMON_VISIBLE joinPaths(const std::string &_path1,
                                            const std::string &_path2);

    /// \brief base case for joinPaths(...) below
    inline std::string joinPaths(const std::string &_path)
    {
      return _path;
    }

    // The below is C++ variadic template magic to allow a joinPaths
    // method that takes 1-n number of arguments to append together.

    /// \brief Append one or more additional path elements to the first
    ///        passed in argument.
    /// \param[in] args  The paths to append together
    /// \return A new string with the paths appended together.
    template<typename... Args>
    inline std::string joinPaths(const std::string &_path1,
                                 const std::string &_path2,
                                 Args const &..._args)
    {
      return joinPaths(joinPaths(_path1, _path2),
                       joinPaths(_args...));
    }

    /// \brief Get the current working directory
    /// \return Name of the current directory
    std::string GZ_COMMON_VISIBLE cwd();

    /// \brief Change current working directory to _dir.
    /// \param[in] _dir The directory to change to.
    /// \return Whether the operation succeeded.
    bool GZ_COMMON_VISIBLE chdir(const std::string &_dir);

    /// \brief Given a path, get just the basename portion.
    /// \param[in] _path  The full path.
    /// \return A new string with just the basename portion of the path.
    std::string GZ_COMMON_VISIBLE basename(
        const std::string &_path);

    /// \brief Given a path, get just its parent path portion, without
    /// separator at the end.
    /// \param[in] _path  Path of which to find parent path
    /// \return A new string with just the parent path of the path.
    std::string GZ_COMMON_VISIBLE parentPath(
        const std::string &_path);

    /// \brief Copy a file.
    /// \param[in] _existingFilename Path to an existing file.
    /// \param[in] _newFilename Path of the new file.
    /// \param[in] _warningOp Log or suppress warnings that may occur.
    /// \return True on success.
    bool GZ_COMMON_VISIBLE copyFile(
        const std::string &_existingFilename,
        const std::string &_newFilename,
        const FilesystemWarningOp _warningOp = FSWO_LOG_WARNINGS);

    /// \brief Copy a directory, overwrite the destination directory if exists.
    /// \param[in] _source Path to an existing directory to copy from.
    /// \param[in] _destination Path to the destination directory.
    /// \return True on success.
    bool GZ_COMMON_VISIBLE copyDirectory(
        const std::string &_existingDirname,
        const std::string &_newDirname,
        const FilesystemWarningOp _warningOp = FSWO_LOG_WARNINGS);

    /// \brief Move a file.
    /// \param[in] _existingFilename Full path to an existing file.
    /// \param[in] _newFilename Full path of the new file.
    /// \param[in] _warningOp Log or suppress warnings that may occur.
    /// \return True on success.
    bool GZ_COMMON_VISIBLE moveFile(
        const std::string &_existingFilename,
        const std::string &_newFilename,
        const FilesystemWarningOp _warningOp = FSWO_LOG_WARNINGS);

    /// \brief Remove an empty directory
    /// \remarks the directory must be empty to be removed
    /// \param[in] _path Path to a directory.
    /// \param[in] _warningOp Log or suppress warnings that may occur.
    /// \return True if _path is a directory and was removed.
    bool GZ_COMMON_VISIBLE removeDirectory(
        const std::string &_path,
        const FilesystemWarningOp _warningOp = FSWO_LOG_WARNINGS);

    /// \brief Remove a file.
    /// \param[in] _existingFilename Full path to an existing file.
    /// \param[in] _warningOp Log or suppress warnings that may occur.
    /// \return True on success.
    bool GZ_COMMON_VISIBLE removeFile(
        const std::string &_existingFilename,
        const FilesystemWarningOp _warningOp = FSWO_LOG_WARNINGS);

    /// \brief Remove an empty directory or file.
    /// \param[in] _path Path to a directory or file.
    /// \param[in] _warningOp Log or suppress warnings that may occur.
    /// \return True if _path was removed.
    bool GZ_COMMON_VISIBLE removeDirectoryOrFile(
        const std::string &_path,
        const FilesystemWarningOp _warningOp = FSWO_LOG_WARNINGS);

    /// \brief Remove a file or a directory and all its contents.
    /// \param[in] _path Path to a directory or file.
    /// \param[in] _warningOp Log or suppress warnings that may occur.
    /// \return True if _path was removed.
    bool GZ_COMMON_VISIBLE removeAll(
        const std::string &_path,
        const FilesystemWarningOp _warningOp = FSWO_LOG_WARNINGS);

    /// \brief Generates a path for a file which doesn't collide with existing
    /// files, by appending numbers to it (i.e. (0), (1), ...)
    /// \param[in] _pathAndName Full absolute path and file name up to the
    /// file extension.
    /// \param[in] _extension File extension, such as "sdf".
    /// \return Full path with name and extension, which doesn't collide with
    /// existing files
    std::string GZ_COMMON_VISIBLE uniqueFilePath(
        const std::string &_pathAndName, const std::string &_extension);

    /// \brief Unique directory path to not overwrite existing directory
    /// \param[in] _pathAndName Full absolute path
    /// \return Full path which doesn't collide with existing files
    std::string GZ_COMMON_VISIBLE uniqueDirectoryPath(
        const std::string &_dir);

    /// \class DirIter Filesystem.hh
    /// \brief A class for iterating over all items in a directory.
    class GZ_COMMON_VISIBLE DirIter
    {
      /// \brief Constructor for end element.
      public: DirIter();

      /// \brief Constructor.
      /// \param[in] _in  Directory to iterate over.
      public: explicit DirIter(const std::string &_in);

      /// \brief Dereference operator; returns current directory record.
      /// \return A string representing the entire path of the directory
      /// record.
      public: std::string operator*() const;

      /// \brief Pre-increment operator; moves to next directory record.
      /// \return This iterator.
      public: const DirIter &operator++();

      /// \brief Comparison operator to see if this iterator is at the
      ///        same point as another iterator.
      /// \param[in] _other  The other iterator to compare against.
      /// \return true if the iterators are equal, false otherwise.
      public: bool operator!=(const DirIter &_other) const;

      /// \brief Pointer to private data.
      GZ_UTILS_IMPL_PTR(dataPtr)
    };
  }
}

#endif
