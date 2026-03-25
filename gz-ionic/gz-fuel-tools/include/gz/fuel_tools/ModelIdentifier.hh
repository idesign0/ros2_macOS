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

#ifndef GZ_FUEL_TOOLS_MODELIDENTIFIER_HH_
#define GZ_FUEL_TOOLS_MODELIDENTIFIER_HH_

#include <cstdint>
#include <ctime>
#include <memory>
#include <string>
#include <vector>

#include "gz/fuel_tools/Helpers.hh"

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace gz::fuel_tools
{
  /// \brief Forward Declaration
  class ModelIdentifierPrivate;
  class ServerConfig;

  /// \brief Defines how to identify a model
  class GZ_FUEL_TOOLS_VISIBLE ModelIdentifier
  {
    /// \brief Constructor.
    public: ModelIdentifier();

    /// \brief Copy Constructor.
    /// \param[in] _orig ModelIdentifier to copy.
    public: ModelIdentifier(const ModelIdentifier &_orig);

    /// \brief Destructor.
    public: ~ModelIdentifier();

    /// \brief Assignment operator
    /// \param[in] _orig ModelIdentifier to copy.
    /// \return Reference to this object.
    public: ModelIdentifier &operator=(const ModelIdentifier &_orig);

    /// \brief Equality operator.
    /// \param[in] _rhs ModelIdentifier to compare.
    /// \return True if the ModelIdentifier names are equal.
    public: bool operator==(const ModelIdentifier &_rhs) const;

    /// \brief Inequality operator.
    /// \param[in] _rhs ModelIdentifier to compare.
    /// \return True if the ModelIdentifier names are not equal.
    public: bool operator!=(const ModelIdentifier &_rhs) const;

    /// \brief Returns just the last part of the model name.
    /// \return Model name.
    public: std::string Name() const;

    /// \brief Returns owner to attribute model to.
    /// \return Owner name.
    public: std::string Owner() const;

    /// \brief Returns server information to retrieve model from.
    /// \return Server information.
    public: ServerConfig &Server() const;

    /// \brief Returns a unique name for the model.
    /// \remarks this is Server/Owner/Name.
    /// \return Unique model name.
    public: std::string UniqueName() const;

    /// \brief Returns a URL for the model.
    /// \remarks this is Server/Owner/Name.
    public: gz::common::URI Url() const;

    /// \brief set the name of the model.
    /// \param[in] _name The name to set. Must be ascii and pass [-_a-z0-9]+.
    /// \return true if successful.
    public: bool SetName(const std::string &_name);

    /// \brief Set the owner of the model
    /// \param[in] _name The name to set. Must be ascii and pass [-_a-z0-9]+
    /// \return true if successful
    public: bool SetOwner(const std::string &_name);

    /// \brief Set the server from which this model comes.
    /// \param[in] _server The server to retrieve the model from.
    /// \return True if successful.
    public: bool SetServer(const ServerConfig &_server);

    /// \brief Returns the description of the model.
    /// \return Model description.
    public: std::string Description() const;

    /// \brief Returns the file size of the model in bytes.
    /// \return Model file size in bytes.
    public: unsigned int FileSize() const;

    /// \brief Returns the upload date of the model.
    /// \return Model upload date.
    public: std::time_t UploadDate() const;

    /// \brief Returns the modify date of the model.
    /// \return Model modify date.
    public: std::time_t ModifyDate() const;

    /// \brief Returns the number of "likes" for the model.
    /// \return Number of "likes".
    public: uint32_t LikeCount() const;

    /// \brief Returns the number of downloads for the model.
    /// \return Number of downloads.
    public: uint32_t DownloadCount() const;

    /// \brief Returns the license name.
    /// \return License name.
    public: std::string LicenseName() const;

    /// \brief Returns the license URL.
    /// \return License URL.
    public: std::string LicenseUrl() const;

    /// \brief Returns the license image URL.
    /// \return License image URL.
    public: std::string LicenseImageUrl() const;

    /// \brief Returns the collection of tags.
    /// \return The collection of tags.
    public: std::vector<std::string> Tags() const;

    /// \brief Returns the privacy setting of the model.
    /// \return True if the model is private, false if the model is
    /// public.
    public: bool Private() const;

    /// \brief Set the privacy setting of the model.
    /// \param[in] _private True indicates the model is private,
    /// false indicates the model is public.
    public: void SetPrivate(bool _private) const;

    /// \brief Set the description of the model.
    /// \param[in] _desc The description
    /// \return True if successful.
    public: bool SetDescription(const std::string &_desc);

    /// \brief Set the file size of the model in bytes.
    /// \param[in] _filesize The model's file size in bytes.
    /// \return True if successful.
    public: bool SetFileSize(const unsigned int _filesize);

    /// \brief Set the upload date.
    /// \param[in] _date Upload date.
    /// \return True if successful.
    public: bool SetUploadDate(const std::time_t &_date);

    /// \brief Set the modify date
    /// \return True if successful
    public: bool SetModifyDate(const std::time_t &_date);

    /// \brief Set the number of "likes" of the model.
    /// \param[in] _likes Number of "likes".
    /// \return True if successful.
    public: bool SetLikeCount(const uint32_t _likes);

    /// \brief Set the number of downloads of the model.
    /// \param[in] _downloads Number of downloads.
    /// \return True if successful.
    public: bool SetDownloadCount(const uint32_t _downloads);

    /// \brief Set the license name of the model.
    /// \param[in] _name The name.
    /// \return True if successful.
    public: bool SetLicenseName(const std::string &_name);

    /// \brief Set the license URL of the model.
    /// \param[in] _url The URL.
    /// \return True if successful.
    public: bool SetLicenseUrl(const std::string &_url);

    /// \brief Set the license image URL of the model.
    /// \param[in] _url The URL.
    /// \return True if successful.
    public: bool SetLicenseImageUrl(const std::string &_url);

    /// \brief Set the list of tags of the model.
    /// \param[in] _tags The tags.
    /// \return True if successful.
    public: bool SetTags(const std::vector<std::string> &_tags);

    /// \brief Returns the model's version as a number. Versions are integers
    /// counting from 1. Version zero means the tip.
    /// \return Model's version.
    /// \sa VersionStr
    /// \sa SetVersion
    public: unsigned int Version() const;

    /// \brief Returns the model's version as a string. Versions are integers
    /// counting from 1. The strings "tip" or "" mean the latest version.
    /// \return Model's version.
    /// \sa Version
    /// \sa SetVersionStr
    public: std::string VersionStr() const;

    /// \brief Set the model's version. Versions are integers counting from
    /// 1.
    /// \param[in] _version The model's version
    /// \return True if successful.
    /// \sa Version
    /// \sa SetVersionStr
    public: bool SetVersion(const unsigned int _version);

    /// \brief Set the model's version. Versions are integers counting from
    /// 1 or "tip" for the latest version.
    /// \param[in] _version The model's version
    /// \return True if successful.
    /// \sa VersionStr
    /// \sa SetVersion
    public: bool SetVersionStr(const std::string &_version);

    // /// \brief returns a SHA 2 256 hash of the model
    // /// \remarks fulfills versioning requirement
    // public: std::array<std::uint8_t, 32> SHA_256() const;

    // /// \brief Sets the SHA 2 256 hash of the model
    // /// \param[in] _hash a 256 bit SHA 2 hash
    // /// \returns true if successful
    // public: bool SHA_256(const std::array<std::uint8_t, 32> &_hash);

    /// \brief Returns all the model information as a string. Convenient for
    /// debugging.
    /// \param[in] _prefix Optional prefix for every line of the string.
    /// \return Model information string
    public: std::string AsString(const std::string &_prefix = "") const;

    /// \brief Returns all the available model information as a string using
    /// colors for better human parsing.
    /// \param[in] _prefix Optional prefix for every line of the string.
    /// \return Model information string
    public: std::string AsPrettyString(const std::string &_prefix = "") const;

    /// \brief PIMPL
    private: std::unique_ptr<ModelIdentifierPrivate> dataPtr;
  };
}  // namespace gz::fuel_tools

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // GZ_FUEL_TOOLS_MODELIDENTIFIER_HH_
