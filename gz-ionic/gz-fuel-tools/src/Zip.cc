/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include <sys/stat.h>
#include <zip.h>

#include <iostream>
#include <fstream>
#include <string>

#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>

#include "gz/fuel_tools/Zip.hh"

using namespace gz;
using namespace fuel_tools;


/////////////////////////////////////////////////
bool CompressFile(zip *_archive, const std::string &_file,
    const std::string &_entry)
{
  if (gz::common::isDirectory(_file))
  {
    if (zip_dir_add(_archive, _entry.c_str(), 0) < 0)
    {
      gzerr << "Error adding directory to zip: " << _file << std::endl;
      return false;
    }

    gz::common::DirIter endIt;
    for (gz::common::DirIter dirIt(_file); dirIt != endIt; ++dirIt)
    {
      std::string file = *dirIt;
      std::string entryName = gz::common::joinPaths(_entry,
          gz::common::basename(file));

      if (!CompressFile(_archive, file, entryName))
      {
        gzerr << "Error compressing file: " << file << std::endl;
      }
    }
  }
  else if (gz::common::isFile(_file))
  {
    std::ifstream in(_file.c_str(),
        std::ifstream::ate | std::ifstream::binary);
    auto end = in.tellg();

    zip_source* source = zip_source_file(_archive, _file.c_str(), 0, end);
    if (!source)
    {
      gzerr << "Error adding file to zip: " << _file << std::endl;
      return false;
    }

    if (zip_file_add(_archive, _entry.c_str(), source, 0)
        < 0)
    {
      gzerr << "Error adding file to zip: " << _file << std::endl;
      zip_source_free(source);
      return false;
    }
  }
  return true;
}

/////////////////////////////////////////////////
bool Zip::Compress(const std::string &_src, const std::string &_dst)
{
  if (!gz::common::exists(_src))
  {
    gzerr << "Directory does not exist: " << _src << std::endl;
    return false;
  }

  int err = 0;
  zip *archive = zip_open(_dst.c_str(), ZIP_CREATE, &err);
  if (!archive)
  {
    gzerr << "Error opening zip archive: '" << _dst << "'" << std::endl;
    return false;
  }

  std::string entry = gz::common::basename(_src);
  if (!CompressFile(archive, _src, entry))
  {
    gzerr << "Error compressing file: " << _src << std::endl;
    zip_close(archive);
    return false;
  }

  zip_close(archive);
  return true;
}

/////////////////////////////////////////////////
bool Zip::Extract(const std::string &_src,
    const std::string &_dst)
{
  if (!gz::common::exists(_src))
  {
    gzerr << "Source archive does not exist: " << _src << std::endl;
    return false;
  }

  int err;
  zip *archive = zip_open(_src.c_str(), 0, &err);
  if (!archive)
  {
    gzerr << "Error opening zip archive: '" << _src << "'" << std::endl;
    return false;
  }

  for (unsigned int i = 0; i < zip_get_num_entries(archive, 0); ++i)
  {
    struct zip_stat sb;
    if (zip_stat_index(archive, i, 0, &sb) != 0)
    {
      gzerr << "Error get stats on archive index: " << i << std::endl;
      continue;
    }

    auto entryname = std::string(sb.name);
    common::changeFromUnixPath(entryname);
    std::string dst = gz::common::joinPaths(_dst, entryname);

    // Check if the entryname contains a / at the end. if so it's a directory
    auto pos = entryname.rfind(gz::common::separator(""));
    if (pos != std::string::npos && pos == (entryname.size() - 1))
    {
      if (!gz::common::createDirectories(dst))
      {
        gzerr << "Error creating directory [" << dst << "]. "
               << "Do you have the right permissions?" << std::endl;
        return false;
      }
      continue;
    }

    // Create and write the files.
    zip_file * zf = zip_fopen_index(archive, i, 0);
    if (!zf)
    {
      gzerr << "Error opening: " << sb.name << std::endl;
      continue;
    }

    std::ofstream file(dst);
    int readSize = sb.size;
    char *buf = new char[readSize];
    int len = zip_fread(zf, buf, readSize);

    if (len <= 0)
      gzerr << "Error reading " << sb.name << std::endl;
    else
    {
      file.write(buf, len);
      if (file.fail())
      {
        gzerr << "Failed to write file [" << dst << "]" << std::endl;
      }
      else
      {
        gzdbg << "Created file [" << dst << "]" << std::endl;
      }
    }

    delete[] buf;
    file.close();
    zip_fclose(zf);
  }

  if (zip_close(archive) < 0)
  {
    gzerr << "Error closing zip archive" << std::endl;
    return false;
  }

  return true;
}
