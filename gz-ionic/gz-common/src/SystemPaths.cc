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
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <functional>
#include <list>
#include <locale>
#include <string>
#include <vector>

#include "gz/common/Console.hh"
#include "gz/common/StringUtils.hh"
#include "gz/common/SystemPaths.hh"
#include "gz/common/URI.hh"
#include "gz/common/Util.hh"

using namespace gz;
using namespace common;

// Private data class
class gz::common::SystemPaths::Implementation
{
  /// \brief Name of the environment variable to check for plugin paths
  public: std::string pluginPathEnv = "GZ_PLUGIN_PATH";

  /// \brief Name of the environment variable to check for file paths
  public: std::string filePathEnv = "GZ_FILE_PATH";

  /// \brief Paths to plugins
  public: std::list<std::string> pluginPaths;

  /// \brief Paths to files
  public: std::list<std::string> filePaths;

  /// \brief Suffix paths
  public: std::list<std::string> suffixPaths;

  /// \brief Log path
  public: std::string logPath;

  /// \brief Function type for finding files
  public: using FindFileFunction =
          std::function<std::string(const std::string &)>;

  /// \brief Function type for finding URIs
  public: using FindURIFunction =
          std::function<std::string(const URI &)>;

  /// \brief Callbacks to be called in order in case a file can't be found.
  public: std::vector<FindFileFunction> findFileCbs;

  /// \brief Callbacks to be called in order in case a file can't be found.
  public: std::vector<FindURIFunction> findFileURICbs;

  /// \brief generates paths to try searching for the named library
  public: std::vector<std::string> GenerateLibraryPaths(
              const std::string &_libName) const;
};

//////////////////////////////////////////////////
/// \brief adds a path to the list if not already present
/// \param[in] _path the path
/// \param[in, out] _list the list
void insertUnique(const std::string &_path, std::list<std::string> &_list)
{
  if (std::find(_list.begin(), _list.end(), _path) == _list.end())
    _list.push_back(_path);
}

//////////////////////////////////////////////////
SystemPaths::SystemPaths()
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
  std::string home, path, fullPath;
  if (!env(GZ_HOMEDIR, home))
    home = "/tmp/gz";

  if (path.empty())
  {
    if (home != "/tmp/gz")
      fullPath = joinPaths(home, ".gz");
    else
      fullPath = home;
  }
  else
    fullPath = path;

  if (!exists(fullPath))
  {
    createDirectories(fullPath);
  }

  this->dataPtr->logPath = fullPath;

  // Populate this->dataPtr->filePaths with values from the default
  // environment variable.
  this->SetFilePathEnv(this->dataPtr->filePathEnv);
}

/////////////////////////////////////////////////
std::string SystemPaths::LogPath() const
{
  return this->dataPtr->logPath;
}

/////////////////////////////////////////////////
void SystemPaths::SetPluginPathEnv(const std::string &_env)
{
  this->dataPtr->pluginPathEnv = _env;
}

/////////////////////////////////////////////////
const std::list<std::string> &SystemPaths::PluginPaths()
{
  if (this->dataPtr->pluginPathEnv.size())
  {
    std::string result;
    if (env(this->dataPtr->pluginPathEnv, result))
    {
      this->AddPluginPaths(result);
    }
  }
  return this->dataPtr->pluginPaths;
}

/////////////////////////////////////////////////
std::string SystemPaths::FindSharedLibrary(const std::string &_libName)
{
  URIPath libname(_libName);
  // Short circuit if the given library name is an absolute path to a file.
  if (libname.IsAbsolute() && exists(_libName))
    return _libName;

  // Trigger loading paths from env
  this->PluginPaths();

  std::string pathToLibrary;
  std::vector<std::string> searchNames =
    this->dataPtr->GenerateLibraryPaths(_libName);

  // TODO(anyone) return list of paths that match if more than one matches?
  for (auto const &possibleName : searchNames)
  {
    if (exists(possibleName))
    {
      pathToLibrary = possibleName;
      break;
    }
  }

  changeFromUnixPath(pathToLibrary);
  return pathToLibrary;
}

/////////////////////////////////////////////////
void SystemPaths::SetFilePathEnv(const std::string &_env)
{
  this->dataPtr->filePathEnv = _env;
  std::string result;

  if (!this->dataPtr->filePathEnv.empty())
  {
    this->ClearFilePaths();
    if (env(this->dataPtr->filePathEnv, result))
    {
      this->AddFilePaths(result);
    }
  }
}

/////////////////////////////////////////////////
std::string SystemPaths::FilePathEnv() const
{
  return this->dataPtr->filePathEnv;
}

/////////////////////////////////////////////////
const std::list<std::string> &SystemPaths::FilePaths()
{
  return this->dataPtr->filePaths;
}

/////////////////////////////////////////////////
void SystemPaths::ClearFilePaths()
{
  this->dataPtr->filePaths.clear();
}

/////////////////////////////////////////////////
void SystemPaths::AddFilePaths(const std::string &_path)
{
  if (_path.size())
  {
    std::vector<std::string> paths = Split(_path, Delimiter());
    for (auto const &path : paths)
    {
      std::string normalPath = NormalizeDirectoryPath(path);
      insertUnique(normalPath, this->dataPtr->filePaths);
    }
  }
}
/////////////////////////////////////////////////
std::string SystemPaths::NormalizeDirectoryPath(const std::string &_path)
{
  std::string path = _path;
  // Use '/' because it works on Linux, OSX, and Windows
  std::replace(path.begin(), path.end(), '\\', '/');
  // Make last character '/'
  if (!EndsWith(path, "/"))
  {
    path += '/';
  }
  return path;
}

/////////////////////////////////////////////////
std::vector<std::string> SystemPaths::Implementation::GenerateLibraryPaths(
    const std::string &_libName) const
{
  std::string lowercaseLibName = lowercase(_libName);
  // test for possible prefixes or extensions on the library name
  bool hasLib = StartsWith(_libName, "lib");
  bool hasDotSo = EndsWith(lowercaseLibName, ".so");
  bool hasDotDll = EndsWith(lowercaseLibName, ".dll");
  bool hasDotDylib = EndsWith(lowercaseLibName, ".dylib");

  // Try removing non cross platform parts of names
  std::vector<std::string> initNames;
  initNames.push_back(_libName);
  if (hasLib && hasDotSo)
    initNames.push_back(_libName.substr(3, _libName.size() - 6));
  if (hasDotDll)
    initNames.push_back(_libName.substr(0, _libName.size() - 4));
  if (hasLib && hasDotDylib)
    initNames.push_back(_libName.substr(3, _libName.size() - 9));

  // Create possible basenames on different platforms
  std::vector<std::string> basenames;
  for (auto const &name : initNames)
  {
    basenames.push_back(name);
    basenames.push_back("lib" + name + ".so");
    basenames.push_back(name + ".so");
    basenames.push_back(name + ".dll");
    basenames.push_back("Release/" + name + ".dll");
    basenames.push_back("Debug/" + name + ".dll");
    basenames.push_back(name + ".dll");
    basenames.push_back("lib" + name + ".dylib");
    basenames.push_back(name + ".dylib");
    basenames.push_back("lib" + name + ".SO");
    basenames.push_back(name + ".SO");
    basenames.push_back(name + ".DLL");
    basenames.push_back("Release/" + name + ".DLL");
    basenames.push_back("Debug/" + name + ".DLL");
    basenames.push_back("lib" + name + ".DYLIB");
    basenames.push_back(name + ".DYLIB");
  }

  std::vector<std::string> searchNames;
  // Concatenate these possible basenames with the search paths
  for (auto const &path : this->pluginPaths)
  {
    for (auto const &name : basenames)
    {
      searchNames.push_back(path + name);
    }
  }
  return searchNames;
}

//////////////////////////////////////////////////
std::string SystemPaths::FindFileURI(const std::string &_uri) const
{
  if (!URI::Valid(_uri))
  {
    gzerr << "The passed value [" << _uri << "] is not a valid URI, "
              "trying as a file" << std::endl;
    return this->FindFile(_uri);
  }

  const auto uri = URI(_uri);
  return this->FindFileURI(uri);
}

//////////////////////////////////////////////////
std::string SystemPaths::FindFileURI(const URI &_uri) const
{
  std::string prefix = _uri.Scheme();
  std::string suffix;
  if (_uri.Authority())
  {
    // Strip //
    suffix = _uri.Authority()->Str().substr(2) + _uri.Path().Str();
  }
  else
  {
    // Strip /
    if (_uri.Path().IsAbsolute() && prefix != "file")
      suffix += _uri.Path().Str().substr(1);
    else
      suffix += _uri.Path().Str();
  }
  suffix += _uri.Query().Str();

  std::string filename;

  // First try to find the file on the current system
  filename = this->FindFile(gz::common::copyFromUnixPath(suffix),
      true, false);

  // Look in custom paths.
  // Tries the suffix against all paths, regardless of the scheme
  if (filename.empty())
  {
    for (const std::string &filePath : this->dataPtr->filePaths)
    {
      auto withSuffix = NormalizeDirectoryPath(filePath) + suffix;
      if (exists(withSuffix))
      {
        filename = copyFromUnixPath(withSuffix);
        break;
      }
    }
  }

  // If still not found, try custom callbacks
  if (filename.empty())
  {
    for (const auto &cb : this->dataPtr->findFileURICbs)
    {
      filename = cb(_uri);
      if (!filename.empty())
        break;
    }
  }

  if (filename.empty())
  {
    gzerr << "Unable to find file with URI [" << _uri.Str() << "]" <<
           std::endl;
    return std::string();
  }

  if (!exists(filename))
  {
    gzerr << "URI [" << _uri.Str() << "] resolved to path [" << filename <<
           "] but the path does not exist" << std::endl;
    return std::string();
  }

  return filename;
}

//////////////////////////////////////////////////
std::string SystemPaths::FindFile(const std::string &_filename,
                                  const bool _searchLocalPath,
                                  const bool _verbose) const
{
  std::string path;
  std::string filename = _filename;

  if (filename.empty())
    return path;

#ifdef _WIN32
  // First of all, try if filename as a Windows absolute path exists
  // The Windows absolute path is tried first as a Windows drive such as
  // C:/ is also a valid URI scheme
  if (filename.length() >= 2 && filename[1] == ':' && exists(filename))
  {
    path = filename;
  }
#endif  // _WIN32
  // If the filename is not an existing absolute Windows path, try others
  if (path.empty()) {
    // Handle as URI
    if (gz::common::URI::Valid(filename))
    {
      path = this->FindFileURI(gz::common::URI(filename));
    }
    // Handle as local absolute path
    else if (filename[0] == '/')
    {
      path = filename;
    }
    // Try appending to local paths
    else
    {
      auto cwdPath = joinPaths(cwd(), filename);
      if (_searchLocalPath && exists(cwdPath))
      {
        path = cwdPath;
      }
      else if ((filename[0] == '.' || _searchLocalPath) && exists(filename))
      {
        path = filename;
      }
      else
      {
        for (const auto &cb : this->dataPtr->findFileCbs)
        {
          path = cb(filename);
          if (!path.empty())
            break;
        }
      }
    }
  }

  // Look in custom paths.
  if (path.empty())
  {
    for (const std::string &filePath : this->dataPtr->filePaths)
    {
      auto withSuffix = NormalizeDirectoryPath(filePath) + filename;
      if (exists(withSuffix))
      {
        path = copyFromUnixPath(withSuffix);
        break;
      }
    }
  }

  // If still not found, try custom callbacks
  if (path.empty())
  {
    for (const auto &cb : this->dataPtr->findFileCbs)
    {
      path = cb(filename);
      if (!path.empty())
        break;
    }
  }

  if (path.empty())
  {
    if (_verbose)
    {
      gzerr << "Could not resolve file [" << _filename << "]" << std::endl;
    }
    return std::string();
  }

  if (!exists(path))
  {
    if (_verbose)
    {
      gzerr << "File [" << _filename << "] resolved to path [" << path <<
                "] but the path does not exist" << std::endl;
    }
    return std::string();
  }

  return path;
}

//////////////////////////////////////////////////
std::string SystemPaths::LocateLocalFile(const std::string &_filename,
    const std::vector<std::string> &_paths)
{
  std::string foundPath = "";
  for (auto const &path : _paths)
  {
    std::string checkPath = NormalizeDirectoryPath(path) + _filename;
    if (exists(checkPath))
    {
      foundPath = checkPath;
      break;
    }
  }
  return foundPath;
}

/////////////////////////////////////////////////
void SystemPaths::ClearPluginPaths()
{
  this->dataPtr->pluginPaths.clear();
}

/////////////////////////////////////////////////
void SystemPaths::AddPluginPaths(const std::string &_path)
{
  if (_path.size())
  {
    std::vector<std::string> paths = Split(_path, Delimiter());
    for (auto const &path : paths)
    {
      std::string normalPath = NormalizeDirectoryPath(path);
      insertUnique(normalPath, this->dataPtr->pluginPaths);
    }
  }
}

/////////////////////////////////////////////////
void SystemPaths::AddSearchPathSuffix(const std::string &_suffix)
{
  std::string s;

  if (_suffix[0] != '/')
    s = std::string("/") + _suffix;
  else
    s = _suffix;

  if (_suffix[_suffix.size()-1] != '/')
    s += "/";

  this->dataPtr->suffixPaths.push_back(s);
}

/////////////////////////////////////////////////
void SystemPaths::AddFindFileCallback(
    SystemPaths::Implementation::FindFileFunction _cb)
{
  this->dataPtr->findFileCbs.push_back(_cb);
}

/////////////////////////////////////////////////
void SystemPaths::ClearFindFileCallbacks()
{
  this->dataPtr->findFileCbs.clear();
}

/////////////////////////////////////////////////
void SystemPaths::AddFindFileURICallback(
    SystemPaths::Implementation::FindURIFunction _cb)
{
  this->dataPtr->findFileURICbs.push_back(_cb);
}

/////////////////////////////////////////////////
void SystemPaths::ClearFindFileURICallbacks()
{
  this->dataPtr->findFileURICbs.clear();
}

/////////////////////////////////////////////////
std::list<std::string> SystemPaths::PathsFromEnv(const std::string &_env)
{
  std::list<std::string> paths;

  std::string envPathsStr;
  if (!env(_env, envPathsStr))
    return paths;

  if (envPathsStr.empty())
    return paths;

  auto ps = Split(envPathsStr, Delimiter());
  for (auto const &path : ps)
  {
    std::string normalPath = NormalizeDirectoryPath(path);
    insertUnique(normalPath, paths);
  }

  return paths;
}

/////////////////////////////////////////////////
char SystemPaths::Delimiter()
{
#ifdef _WIN32
  return ';';
#else
  return ':';
#endif
}
