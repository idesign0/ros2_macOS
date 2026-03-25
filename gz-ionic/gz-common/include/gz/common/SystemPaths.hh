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
#ifndef GZ_COMMON_SYSTEMPATHS_HH_
#define GZ_COMMON_SYSTEMPATHS_HH_

#include <functional>
#include <list>
#include <string>
#include <vector>

#include <gz/common/Export.hh>
#include <gz/common/URI.hh>

#include <gz/utils/ImplPtr.hh>

namespace gz
{
  namespace common
  {
    /// \class SystemPaths SystemPaths.hh gz/common/SystemPaths.hh
    /// \brief Functions to handle getting system paths, keeps track of:
    ///        \li SystemPaths#pluginPaths - plugin library paths
    ///            for common::WorldPlugin
    class GZ_COMMON_VISIBLE  SystemPaths
    {
      /// \brief Constructor for SystemPaths
      public: SystemPaths();

      /// \brief Get the log path. If GZ_LOG_PATH environment variable is set,
      /// then this path is used. If not, the path is $HOME/.gz, and in
      /// case even HOME is not set, /tmp/gz is used. If the directory
      /// does not exist, it is created in the constructor of SystemPaths.
      /// \return the path
      public: std::string LogPath() const;

      /// \brief Get the plugin paths
      /// \return a list of paths (with forward slashes as directory separators)
      public: const std::list<std::string> &PluginPaths();

      /// \brief Find a file or path using a URI
      /// \param[in] _uri the uniform resource identifier
      /// \return Returns full path name to file with platform-specific
      /// directory separators, or an empty string if URI couldn't be found.
      /// \sa FindFileURI(const common::URI &_uri)
      public: std::string FindFileURI(const std::string &_uri) const;

      /// \brief Find a file or path using a URI.
      /// If URI is not an absolute path, the URI's path will be matched against
      /// all added paths and environment variables (including URI authority as
      /// needed).
      /// \param[in] _uri the uniform resource identifier
      /// \return Returns full path name to file with platform-specific
      /// directory separators, or an empty string if URI couldn't be found.
      public: std::string FindFileURI(const common::URI &_uri) const;

      /// \brief Set the plugin path environment variable to use
      /// \param [in] _env name of the environment variable
      public: void SetPluginPathEnv(const std::string &_env);

      /// \brief Find a file in the set search paths (not recursive)
      /// \param[in] _filename Name of the file to find.
      /// \param[in] _searchLocalPath True to search in the current working
      /// directory.
      /// \param[in] _verbose False to omit console messages.
      /// \return Returns full path name to file with platform-specific
      /// directory separators, or empty string on error.
      public: std::string FindFile(const std::string &_filename,
                                   const bool _searchLocalPath = true,
                                   const bool _verbose = true) const;

      /// \brief Find a shared library by name in the plugin paths
      ///
      /// This will try different combinations of library names on different
      /// platforms. For example searching for "MyLibName" may try finding
      /// "MyLibName", "libMyLibName.so", "MyLibName.dll", etc...
      /// \param[in] _libName Name of shared libary to look for
      /// \return Path to file with platform-specific directory separators,
      /// or empty string on error.
      public: std::string FindSharedLibrary(const std::string &_libName);

      /// \brief Add colon (semicolon on windows) delimited paths to plugins
      /// \param[in] _path the directory to add
      public: void AddPluginPaths(const std::string &_path);

      /// \brief clear out SystemPaths#pluginPaths
      public: void ClearPluginPaths();

      /// \brief Set the file path environment variable to use, and clears
      /// any previously set file paths. The default
      /// environment variable is GZ_FILE_PATH. The
      /// environment variable should be a set of colon (semicolon on windows)
      /// delimited paths. These paths will be used with the FindFile function.
      /// \param [in] _env name of the environment variable
      /// \sa FilePathEnv
      public: void SetFilePathEnv(const std::string &_env);

      /// \brief Get the file path environment variable in use.
      /// The environment variable contains a set of colon (semicolon on
      /// windows) delimited paths. These paths are used with the FindFile
      /// functions.
      /// \return Name of the environment variable
      /// \sa SetFilePathEnv
      public: std::string FilePathEnv() const;

      /// \brief Get the file paths
      /// \return a list of paths (with forward slashes as directory separators)
      public: const std::list<std::string> &FilePaths();

      /// \brief Add colon (semicolon on windows) delimited paths to find
      /// files. These paths will be used with the FindFile function.
      /// \param[in] _path A colon (semicolon on windows) delimited
      /// string of paths. The path can have either forward slashes or platform-
      /// specific directory separators, both are okay.
      public: void AddFilePaths(const std::string &_path);

      /// \brief clear out SystemPaths#filePaths
      public: void ClearFilePaths();

      /// \brief add _suffix to the list of path search suffixes
      /// \param[in] _suffix The suffix to add
      public: void AddSearchPathSuffix(const std::string &_suffix);

      /// \brief Add a callback to use when FindFile() can't find a file.
      /// The callback should return a full local path to the requested file, or
      /// and empty string if the file was not found in the callback. The path
      /// should use platform-specific directory separators.
      /// Callbacks will be called in the order they were added until a path is
      /// found (if a callback is set using SetFindFileCallback(), that one is
      /// called first).
      /// \param[in] _cb The callback function, which takes a file path or URI
      /// and returns the full local path.
      public: void AddFindFileCallback(
                  std::function<std::string(const std::string &)> _cb);

      /// \brief Clear the FindFile callbacks added by AddFindFileCallback
      ///
      /// This is useful in tests where we do not want callbacks added from
      /// previous tests to affect the current test.
      public: void ClearFindFileCallbacks();

      /// \brief Add a callback to use when FindFileURI() can't find a file.
      /// The callback should return a full local path to the requested file, or
      /// and empty string if the file was not found in the callback. The path
      /// should use platform-specific directory separators.
      /// Callbacks will be called in the order they were added until a path is
      /// found (if a callback is set using SetFindFileURICallback(), that one
      /// is called first).
      /// \param[in] _cb The callback function, which takes a file path or URI
      /// and returns the full local path.
      public: void AddFindFileURICallback(
          std::function<std::string(const common::URI &)> _cb);

      /// \brief Clear the FindFileURI callbacks added by AddFindFileURICallback
      ///
      /// This is useful in tests where we do not want callbacks added from
      /// previous tests to affect the current test.
      public: void ClearFindFileURICallbacks();

      /// \brief look for a file in a set of search paths (not recursive)
      /// \description This method checks if a file exists in given directories.
      ///              It does so by joining each path with the filename and
      ///              checking if the file exists. If the file exists in
      ///              multiple paths the first one is found.
      /// \param[in] _filename Name of the file to find
      /// \param[in] _paths paths to look for the file
      /// \return Returns a path that will work from the current directory
      ///         or an empty string if the file was not found. The returned
      ///         path will be normalized, i.e. backslashes will be substituted
      ///         with forward slashes.
      public: static std::string LocateLocalFile(const std::string &_filename,
                                  const std::vector<std::string> &_paths);

      /// \brief Format the directory path to use "/" as a separator with "/"
      /// at the end.
      /// \param[in] _path Path to normalize
      /// \return Normalized path
      public: static std::string NormalizeDirectoryPath(
                const std::string &_path);

      /// \brief Return all paths given by an environment variable.
      /// \param[in] _env Environment variable.
      /// \return A list of paths listed by the environment variable.
      public: static std::list<std::string> PathsFromEnv(
                const std::string &_env);

      /// \brief Get the delimiter that the current operating system
      /// uses to separate different paths from each other.
      public: static char Delimiter();

      /// \brief Pointer to private data.
      GZ_UTILS_IMPL_PTR(dataPtr)
    };
  }
}
#endif
