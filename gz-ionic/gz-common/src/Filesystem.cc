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

#include <array>
#include <algorithm>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <regex>

#include <gz/common/config.hh>
#include <gz/common/SystemPaths.hh>
#include <gz/common/Util.hh>
#include <gz/common/Uuid.hh>
#include <gz/common/Console.hh>
#include "gz/common/Filesystem.hh"

namespace fs = std::filesystem;

using namespace gz;

/////////////////////////////////////////////////
// Return true if success, false if error
inline bool fsWarn(const std::string &_fcn,
             const std::error_code &_ec,
             const common::FilesystemWarningOp &_warningOp =
             common::FSWO_LOG_WARNINGS)
{
  if (_ec)
  {
    if (common::FSWO_LOG_WARNINGS == _warningOp)
    {
      gzwarn << "Failed common::" << _fcn
        << " (ec: " << _ec << " " << _ec.message() << ")\n";
    }
    return false;
  }
  return true;
}

/////////////////////////////////////////////////
bool common::exists(const std::string &_path)
{
  return fs::exists(_path);
}

/////////////////////////////////////////////////
bool common::isDirectory(const std::string &_path)
{
  return fs::is_directory(_path);
}

/////////////////////////////////////////////////
bool common::isFile(const std::string &_path)
{
  return fs::is_regular_file(_path);
}

/////////////////////////////////////////////////
bool common::isRelativePath(const std::string &_path)
{
  return fs::path(_path).is_relative();
}

/////////////////////////////////////////////////
bool common::createDirectory(const std::string &_path)
{
  std::error_code ec;
  fs::create_directory(_path, ec);
  return fsWarn("createDirectory", ec);
}

/////////////////////////////////////////////////
bool common::createDirectories(const std::string &_path)
{
  std::error_code ec;
  // Disregard return of create_directories, because it may return false if the
  // directory is not actually created (already exists)
  bool created = fs::create_directories(_path, ec);
  (void) created;
  return fsWarn("createDirectories", ec);
}

/////////////////////////////////////////////////
std::string const common::separator(std::string const &_s)
{
  return _s + std::string{fs::path::preferred_separator};
}

/////////////////////////////////////////////////
void common::changeFromUnixPath(std::string &_path) {
  std::replace(_path.begin(), _path.end(), '/',
      static_cast<char>(fs::path::preferred_separator));
}

/////////////////////////////////////////////////
std::string common::copyFromUnixPath(const std::string &_path)
{
  std::string copy = _path;
  changeFromUnixPath(copy);
  return copy;
}

/////////////////////////////////////////////////
void common::changeToUnixPath(std::string &_path) {
  std::replace(_path.begin(), _path.end(),
      static_cast<char>(fs::path::preferred_separator), '/');
}

/////////////////////////////////////////////////
std::string common::copyToUnixPath(const std::string &_path)
{
  std::string copy = _path;
  changeToUnixPath(copy);
  return copy;;
}

/////////////////////////////////////////////////
std::string common::absPath(const std::string &_path)
{
  std::error_code ec;
  auto path = fs::absolute(_path, ec);
  if (!fsWarn("absPath", ec))
  {
    path = "";
  }

  return path.string();
}

/////////////////////////////////////////////////
std::string common::joinPaths(
    const std::string &_path1, const std::string &_path2)
{
  fs::path p1{_path1};
  fs::path p2{_path2};

  if (p1.empty())
  {
    p1 = common::separator("");
  }

  bool is_url = false;

  if (_path1.find("://") == std::string::npos)
    p1 = p1.lexically_normal();
  else
    is_url = true;

  // TODO(mjcarroll) Address the case that path2 is also a URI.
  // It's likely not a valid scenario, but not currently covered by our test
  // suite and doesn't return an error.
  if (_path2.find("://") == std::string::npos)
    p2 = p2.lexically_normal();
  else
    is_url = true;

  if (p2.string()[0] == fs::path::preferred_separator)
  {
    p2 = fs::path{p2.string().substr(1)};
  }

  auto ret = (p1 / p2);

  if (is_url)
  {
    return copyToUnixPath(ret.string());
  }
  else
  {
    return ret.lexically_normal().string();
  }
}

/////////////////////////////////////////////////
std::string common::cwd()
{
  std::error_code ec;
  auto curdir = fs::current_path(ec);

  if (!fsWarn("cwd", ec))
  {
    curdir = "";
  }

  return curdir.string();
}

/////////////////////////////////////////////////
bool common::chdir(const std::string &_dir)
{
  std::error_code ec;
  fs::current_path(_dir, ec);
  return fsWarn("chdir", ec);
}

/////////////////////////////////////////////////
std::string common::basename(const std::string &_path)
{
  fs::path p(_path);
  p /= "FOO.TXT";
  p = p.lexically_normal();
  p = p.parent_path();
  return p.filename().string().empty() ?
    std::string{fs::path::preferred_separator} : p.filename().string();
}

/////////////////////////////////////////////////
std::string common::parentPath(const std::string &_path)
{
  std::string result;
  size_t last_sep = _path.find_last_of(separator(""));
  // If slash is the last character, find its parent directory
  if (last_sep == _path.length() - 1)
    last_sep = _path.substr(0, last_sep).find_last_of(separator(""));
  result = _path.substr(0, last_sep);
  return result;
}

/////////////////////////////////////////////////
bool common::copyFile(
    const std::string &_existingFilename,
    const std::string &_newFilename,
    const FilesystemWarningOp _warningOp)
{
  const auto copyOptions = fs::copy_options::overwrite_existing;
  std::error_code ec;
  auto ret = fs::copy_file(_existingFilename, _newFilename, copyOptions, ec);
  return ret && fsWarn("copyFile", ec, _warningOp);
}

/////////////////////////////////////////////////
bool common::copyDirectory(
    const std::string &_existingDirname,
    const std::string &_newDirname,
    const FilesystemWarningOp _warningOp)
{
  const auto copyOptions = fs::copy_options::recursive
                         | fs::copy_options::overwrite_existing;

  // std::filesystem won't create intermediate directories
  // before copying, this maintains compatibility with Gazebo behavior.
  if (!common::createDirectories(_newDirname))
  {
    return false;
  }

  std::error_code ec;
  fs::copy(_existingDirname, _newDirname, copyOptions, ec);
  return fsWarn("copyDirectory", ec, _warningOp);
}

/////////////////////////////////////////////////
bool common::moveFile(
    const std::string &_existingFilename,
    const std::string &_newFilename,
    const FilesystemWarningOp _warningOp)
{
  std::error_code ec;
  fs::rename(_existingFilename, _newFilename, ec);
  return fsWarn("moveFile", ec, _warningOp);
}

/////////////////////////////////////////////////
bool common::removeDirectory(
    const std::string &_path,
    const FilesystemWarningOp _warningOp)
{
  if (!isDirectory(_path))
  {
    if (FSWO_LOG_WARNINGS == _warningOp)
    {
      gzwarn << "Cannot remove, not a directory [" << _path << "]\n";
    }

    return false;
  }

  return removeDirectoryOrFile(_path, _warningOp);
}

/////////////////////////////////////////////////
bool common::removeFile(
    const std::string &_existingFilename,
    const FilesystemWarningOp _warningOp)
{
  if (!isFile(_existingFilename))
  {
    if (FSWO_LOG_WARNINGS == _warningOp)
    {
      gzwarn << "Cannot remove, not a file [" << _existingFilename << "]\n";
    }
    return false;
  }

  return removeDirectoryOrFile(_existingFilename, _warningOp);
}

/////////////////////////////////////////////////
bool common::removeDirectoryOrFile(
    const std::string &_path,
    const FilesystemWarningOp _warningOp)
{
  std::error_code ec;
  auto ret = fs::remove(_path, ec);
  fsWarn("removeDirectoryOrFile", ec, _warningOp);
  return ret;
}

/////////////////////////////////////////////////
bool common::removeAll(
    const std::string &_path,
    const FilesystemWarningOp _warningOp)
{
  std::error_code ec;
  fs::remove_all(_path, ec);
  return fsWarn("removeAll", ec, _warningOp);
}

/////////////////////////////////////////////////
std::string common::uniqueFilePath(
    const std::string &_pathAndName,
    const std::string &_extension)
{
  std::string result = _pathAndName + "." + _extension;
  int count = 1;

  // Check if file exists and change name accordingly
  while (exists(result.c_str()))
  {
    result = _pathAndName + "(" + std::to_string(count++) + ").";
    result += _extension;
  }

  return result;
}

/////////////////////////////////////////////////
std::string common::uniqueDirectoryPath(const std::string &_dir)
{
  std::string result = _dir;
  int count = 1;

  // Check if file exists and change name accordingly
  while (exists(result.c_str()))
  {
    result = _dir + "(" + std::to_string(count++) + ")";
  }

  return result;
}
