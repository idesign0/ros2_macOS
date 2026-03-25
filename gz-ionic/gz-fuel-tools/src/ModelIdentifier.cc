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

#include <sstream>
#include <string>
#include <vector>

#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/StringUtils.hh>
#include <gz/common/Util.hh>

#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/ModelIdentifier.hh"

namespace gz::fuel_tools
{
class ModelIdentifierPrivate
{
  /// \brief returns true if name follows rules
  /// \param[in] _name Name to validate
  public: bool ValidName(const std::string &_name);

  /// \brief a name given to this model by a user
  public: std::string name;

  /// \brief owner who this model is attributed to
  public: std::string owner;

  /// \brief Server of this model
  public: ServerConfig server;

  /// \brief Description of this model
  public: std::string description;

  /// \brief FileSize of this model
  public: unsigned int fileSize{0u};

  /// \brief Model upload date
  public: std::time_t uploadDate{0};

  /// \brief Model last modified date
  public: std::time_t modifyDate{0};

  /// \brief Number of "likes"
  public: uint32_t likeCount{0u};

  /// \brief Number of downloads
  public: uint32_t downloadCount{0u};

  /// \brief The license name
  public: std::string licenseName;

  /// \brief License URL
  public: common::URI licenseUrl;

  /// \brief License image URL
  public: common::URI licenseImageUrl;

  /// \brief Collection of tags
  public: std::vector<std::string> tags;

  /// \brief Model version. Valid versions start from 1, 0 means the tip.
  public: unsigned int version{0};

  /// \brief True indicates the model is private, false indicates the
  /// model is public.
  public: bool privacy{false};
};

//////////////////////////////////////////////////
bool ModelIdentifierPrivate::ValidName(const std::string &_name)
{
  // ToDo: Enable name validation when model names are prepared for this.
  return true;

  bool valid = true;
  if (_name.empty())
    valid = false;
  else
  {
    for (const char &ch : _name)
    {
      // [-_0-9a-z]+
      if (ch == '-'
          || ch == '_'
          || (ch >= 'a' && ch <= 'z')
          || (ch >= '0' && ch <= '9'))
      {
        continue;
      }
      valid = false;
      break;
    }
  }
  return valid;
}

//////////////////////////////////////////////////
ModelIdentifier::ModelIdentifier()
  : dataPtr(new ModelIdentifierPrivate)
{
}

//////////////////////////////////////////////////
ModelIdentifier::ModelIdentifier(const ModelIdentifier &_orig)
{
  this->dataPtr.reset(new ModelIdentifierPrivate(*(_orig.dataPtr.get())));
}

//////////////////////////////////////////////////
ModelIdentifier &ModelIdentifier::operator=(const ModelIdentifier &_orig)
{
  this->dataPtr.reset(new ModelIdentifierPrivate(*(_orig.dataPtr.get())));
  return *this;
}

//////////////////////////////////////////////////
bool ModelIdentifier::operator==(const ModelIdentifier &_rhs) const
{
  return this->UniqueName() == _rhs.UniqueName();
}

//////////////////////////////////////////////////
bool ModelIdentifier::operator!=(const ModelIdentifier &_rhs) const
{
  return !(*this == _rhs);
}

//////////////////////////////////////////////////
ModelIdentifier::~ModelIdentifier()
{
}

//////////////////////////////////////////////////
std::string ModelIdentifier::UniqueName() const
{
  return common::copyToUnixPath(common::joinPaths(
    uriToPath(this->dataPtr->server.Url()),
    this->dataPtr->owner, "models",
    this->dataPtr->name));
}

//////////////////////////////////////////////////
common::URI ModelIdentifier::Url() const
{
  return common::URI(
    common::joinPaths(this->dataPtr->server.Url().Str(),
                      this->dataPtr->owner,
                      "models",
                      this->dataPtr->name), true);
}

//////////////////////////////////////////////////
std::string ModelIdentifier::Name() const
{
  return this->dataPtr->name;
}

//////////////////////////////////////////////////
bool ModelIdentifier::SetName(const std::string &_name)
{
  bool success = false;
  std::string name = common::lowercase(_name);
  if (this->dataPtr->ValidName(name))
  {
    success = true;
    this->dataPtr->name = name;
  }
  return success;
}

//////////////////////////////////////////////////
bool ModelIdentifier::SetOwner(const std::string &_name)
{
  bool success = false;
  std::string owner = common::lowercase(_name);
  if (this->dataPtr->ValidName(owner))
  {
    success = true;
    this->dataPtr->owner = owner;
  }
  return success;
}

//////////////////////////////////////////////////
bool ModelIdentifier::SetServer(const ServerConfig &_server)
{
  bool success = _server.Url().Valid();
  if (success)
    this->dataPtr->server = _server;

  return success;
}

//////////////////////////////////////////////////
std::string ModelIdentifier::Owner() const
{
  return this->dataPtr->owner;
}

//////////////////////////////////////////////////
ServerConfig &ModelIdentifier::Server() const
{
  return this->dataPtr->server;
}

//////////////////////////////////////////////////
std::string ModelIdentifier::Description() const
{
  return this->dataPtr->description;
}

//////////////////////////////////////////////////
unsigned int ModelIdentifier::FileSize() const
{
  return this->dataPtr->fileSize;
}

//////////////////////////////////////////////////
std::time_t ModelIdentifier::UploadDate() const
{
  return this->dataPtr->uploadDate;
}

//////////////////////////////////////////////////
std::time_t ModelIdentifier::ModifyDate() const
{
  return this->dataPtr->modifyDate;
}

//////////////////////////////////////////////////
uint32_t ModelIdentifier::LikeCount() const
{
  return this->dataPtr->likeCount;
}

//////////////////////////////////////////////////
uint32_t ModelIdentifier::DownloadCount() const
{
  return this->dataPtr->downloadCount;
}

//////////////////////////////////////////////////
std::string ModelIdentifier::LicenseName() const
{
  return this->dataPtr->licenseName;
}

//////////////////////////////////////////////////
std::string ModelIdentifier::LicenseUrl() const
{
  return this->dataPtr->licenseUrl.Str();
}

//////////////////////////////////////////////////
std::string ModelIdentifier::LicenseImageUrl() const
{
  return this->dataPtr->licenseImageUrl.Str();
}

//////////////////////////////////////////////////
std::vector<std::string> ModelIdentifier::Tags() const
{
  return this->dataPtr->tags;
}

//////////////////////////////////////////////////
bool ModelIdentifier::SetDescription(const std::string &_desc)
{
  this->dataPtr->description = _desc;
  return true;
}

//////////////////////////////////////////////////
bool ModelIdentifier::SetFileSize(const unsigned int _fileSize)
{
  this->dataPtr->fileSize = _fileSize;
  return true;
}

//////////////////////////////////////////////////
bool ModelIdentifier::SetModifyDate(const std::time_t &_date)
{
  this->dataPtr->modifyDate = _date;
  return true;
}

//////////////////////////////////////////////////
bool ModelIdentifier::SetUploadDate(const std::time_t &_date)
{
  this->dataPtr->uploadDate = _date;
  return true;
}

//////////////////////////////////////////////////
bool ModelIdentifier::SetLikeCount(const uint32_t _likes)
{
  this->dataPtr->likeCount = _likes;
  return true;
}

//////////////////////////////////////////////////
bool ModelIdentifier::SetDownloadCount(const uint32_t _downloads)
{
  this->dataPtr->downloadCount = _downloads;
  return true;
}

//////////////////////////////////////////////////
bool ModelIdentifier::SetLicenseName(const std::string &_name)
{
  this->dataPtr->licenseName = _name;
  return true;
}

//////////////////////////////////////////////////
bool ModelIdentifier::SetLicenseUrl(const std::string &_url)
{
  return this->dataPtr->licenseUrl.Parse(_url);
}

//////////////////////////////////////////////////
bool ModelIdentifier::SetLicenseImageUrl(const std::string &_url)
{
  return this->dataPtr->licenseImageUrl.Parse(_url);
}

//////////////////////////////////////////////////
bool ModelIdentifier::SetTags(const std::vector<std::string> &_tags)
{
  this->dataPtr->tags = _tags;
  return true;
}

//////////////////////////////////////////////////
unsigned int ModelIdentifier::Version() const
{
  return this->dataPtr->version;
}

//////////////////////////////////////////////////
std::string ModelIdentifier::VersionStr() const
{
  std::string version = this->dataPtr->version == 0 ?
      "tip" : std::to_string(this->dataPtr->version);
  return version;
}

//////////////////////////////////////////////////
bool ModelIdentifier::SetVersion(const unsigned int _version)
{
  this->dataPtr->version = _version;
  return true;
}

//////////////////////////////////////////////////
bool ModelIdentifier::SetVersionStr(const std::string &_version)
{
  if (_version == "tip" || _version.empty())
  {
    this->dataPtr->version = 0;
    return true;
  }

  try
  {
    this->dataPtr->version = std::stoi(_version);
  }
  catch(std::invalid_argument &_e)
  {
    return false;
  }

  return true;
}

//////////////////////////////////////////////////
std::string ModelIdentifier::AsString(const std::string &_prefix) const
{
  std::stringstream out;
  out << _prefix << "Name: " << this->Name() << std::endl
      << _prefix << "Owner: " << this->Owner() << std::endl
      << _prefix << "Version: " << this->VersionStr() << std::endl
      << _prefix << "Unique name: " << this->UniqueName() << std::endl
      << _prefix << "Description: " << this->Description() << std::endl
      << _prefix << "File size: " << this->FileSize() << std::endl
      << _prefix << "Upload date: " << this->UploadDate() << std::endl
      << _prefix << "Likes: " << this->LikeCount() << std::endl
      << _prefix << "Downloads: " << this->DownloadCount() << std::endl
      << _prefix << "License name: " << this->LicenseName() << std::endl
      << _prefix << "License URL: " << this->LicenseUrl() << std::endl
      << _prefix << "License image URL: " << this->LicenseImageUrl()
      << std::endl
      << _prefix << "Tags: " << std::endl;

  for (auto t : this->Tags())
    out << _prefix << "- " << t << std::endl;

  out << _prefix << "Server:" << std::endl
      << this->Server().AsString(_prefix + "  ");
  return out.str();
}

//////////////////////////////////////////////////
std::string ModelIdentifier::AsPrettyString(const std::string &_prefix) const
{
  std::string prop = "\033[96m\033[1m";
  std::string value = "\033[37m";
  std::string reset = "\033[0m";

  std::stringstream out;

  if (!this->Name().empty())
  {
    out << _prefix << prop << "Name: " << reset
        << value << this->Name() << reset << std::endl;
  }

  if (!this->Owner().empty())
  {
    out << _prefix << prop << "Owner: " << reset
        << value << this->Owner() << reset << std::endl;
  }

  if (this->Version() != 0)
  {
    out << _prefix << prop << "Version: " << reset
        << value << this->VersionStr() << reset << std::endl;
  }

  if (!this->Description().empty())
  {
    out << _prefix << prop << "Description: " << reset
        << value << this->Description() << reset << std::endl;
  }

  if (this->FileSize() != 0)
  {
    out << _prefix << prop << "File size: " << reset
        << value << this->FileSize() << reset << std::endl;
  }

  if (this->UploadDate() != 0)
  {
    out << _prefix << prop << "Upload date: " << reset
        << value << this->UploadDate() << reset << std::endl;
  }

  if (this->LikeCount() != 0)
  {
    out << _prefix << prop << "Likes: " << reset
        << value << this->LikeCount() << reset << std::endl;
  }

  if (this->DownloadCount() != 0)
  {
    out << _prefix << prop << "Downloads: " << reset
        << value << this->DownloadCount() << reset << std::endl;
  }

  if (!this->LicenseName().empty())
  {
    out << _prefix << prop << "License name: " << reset
        << value << this->LicenseName() << reset << std::endl;
  }

  if (!this->LicenseUrl().empty())
  {
    out << _prefix << prop << "License URL: " << reset
        << value << this->LicenseUrl() << reset << std::endl;
  }

  if (!this->LicenseImageUrl().empty())
  {
    out << _prefix << prop << "License image URL: " << reset
        << value << this->LicenseImageUrl()
        << reset << std::endl;
  }

  if (!this->Tags().empty())
  {
    out << _prefix << prop << "Tags: " << reset << std::endl;

    for (auto t : this->Tags())
    {
      out << _prefix << prop << "- " << reset
          << value << t << reset << std::endl;
    }
  }

  out << _prefix << prop << "Server:" << reset << std::endl
      << this->Server().AsPrettyString(_prefix + "  ");
  return out.str();
}

//////////////////////////////////////////////////
bool ModelIdentifier::Private() const
{
  return this->dataPtr->privacy;
}

//////////////////////////////////////////////////
void ModelIdentifier::SetPrivate(bool _private) const
{
  this->dataPtr->privacy = _private;
}
}  // namespace gz::fuel_tools
