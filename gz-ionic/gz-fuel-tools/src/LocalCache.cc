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

#ifndef _WIN32
  #include <unistd.h>
#endif

#include <stdio.h>
#include <tinyxml2.h>

#include <algorithm>
#include <fstream>
#include <memory>
#include <regex>
#include <string>
#include <vector>
#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/StringUtils.hh>
#include <gz/common/Util.hh>
#include <gz/math/SemanticVersion.hh>

#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/Helpers.hh"
#include "gz/fuel_tools/Zip.hh"

#include "ModelPrivate.hh"
#include "ModelIterPrivate.hh"
#include "WorldIterPrivate.hh"
#include "LocalCache.hh"

namespace gz::fuel_tools
{
class LocalCachePrivate
{
  /// \brief return all models in a given directory
  /// \param[in] _path A directory for the local server cache
  public: std::vector<Model> ModelsInServer(const std::string &_path) const;

  /// \brief Return all worlds in a given directory
  /// \param[in] _path A directory for the local server cache
  public: std::vector<WorldIdentifier> WorldsInServer(
      const std::string &_path) const;

  /// \brief return all models in a given Owner/models directory
  public: std::vector<Model> ModelsInPath(const std::string &_path);

  /// \brief Associate model:// URI paths with Fuel URLs.
  /// \param[in] _modelVersionedDir Directory containing the model.
  /// \param[in] _id Model's Fuel URL.
  /// \return True if the paths were fixed. False could occur if the
  /// `model.config` file is not present or contains XML errors.
  public: bool FixPaths(const std::string &_modelVersionedDir,
      const ModelIdentifier &_id);

  /// \brief Helper function to fix model:// URI paths in geometry elements.
  /// \param[in] _geomElem Pointer to the geometry element.
  /// \param[in] _id Model identifier
  /// \sa FixPaths
  public: void FixPathsInGeomElement(tinyxml2::XMLElement *_geomElem,
              const ModelIdentifier &_id);

  /// \brief Helper function to fix model:// URI paths in material elements.
  /// \param[in] _matElem Pointer to the material element.
  /// \param[in] _id Model identifier
  /// \sa FixPaths
  public: void FixPathsInMaterialElement(
              tinyxml2::XMLElement *_matElem,
              const ModelIdentifier &_id);

  /// \brief Helper function to fix a single model:// URI that is contained
  /// in an element.
  /// \param[in] _elem Pointer to an element that contains a URI.
  /// \param[in] _id Model identifier
  /// \sa FixPaths
  public: void FixPathsInUri(tinyxml2::XMLElement *_elem,
              const ModelIdentifier &_id);

  /// \brief client configuration
  public: const ClientConfig *config = nullptr;
};

//////////////////////////////////////////////////
std::vector<Model> LocalCachePrivate::ModelsInServer(
    const std::string &_path) const
{
  std::vector<Model> models;
  if (!common::isDirectory(_path))
  {
    gzwarn << "Server directory does not exist [" << _path << "]\n";
    return models;
  }

  common::DirIter end;
  common::DirIter ownIter(_path);
  while (ownIter != end)
  {
    if (!common::isDirectory(*ownIter))
    {
      ++ownIter;
      continue;
    }

    // This is an owner directory, look for models
    common::DirIter modIter(common::joinPaths(*ownIter, "models"));
    while (modIter != end)
    {
      if (!common::isDirectory(*modIter))
      {
        ++modIter;
        continue;
      }

      // Go through all versions
      common::DirIter versionIter(common::absPath(*modIter));
      while (versionIter != end)
      {
        if (!common::isDirectory(*versionIter))
        {
          ++versionIter;
          continue;
        }

        if (common::exists(common::joinPaths(*versionIter, "model.config")))
        {
          std::shared_ptr<ModelPrivate> modPriv(new ModelPrivate);
          modPriv->id.SetName(common::basename(*modIter));
          modPriv->id.SetOwner(common::basename(*ownIter));
          modPriv->id.SetVersionStr(common::basename(*versionIter));
          modPriv->pathOnDisk = common::absPath(*versionIter);
          Model model(modPriv);
          models.push_back(model);
        }
        ++versionIter;
      }
      ++modIter;
    }
    ++ownIter;
  }
  return models;
}

//////////////////////////////////////////////////
std::vector<WorldIdentifier> LocalCachePrivate::WorldsInServer(
    const std::string &_path) const
{
  std::vector<WorldIdentifier> worldIds;
  if (!common::isDirectory(_path))
  {
    gzwarn << "Server directory does not exist [" << _path << "]\n";
    return worldIds;
  }

  common::DirIter end;
  common::DirIter ownIter(_path);
  while (ownIter != end)
  {
    if (!common::isDirectory(*ownIter))
    {
      ++ownIter;
      continue;
    }

    // This is an owner directory, look for worlds
    common::DirIter worldIter(common::joinPaths(*ownIter, "worlds"));
    while (worldIter != end)
    {
      if (!common::isDirectory(*worldIter))
      {
        ++worldIter;
        continue;
      }

      // Go through all versions
      common::DirIter versionIter(common::absPath(*worldIter));
      while (versionIter != end)
      {
        if (!common::isDirectory(*versionIter))
        {
          ++versionIter;
          continue;
        }

        WorldIdentifier id;
        id.SetName(common::basename(*worldIter));
        id.SetOwner(common::basename(*ownIter));
        id.SetVersionStr(common::basename(*versionIter));
        id.SetLocalPath(common::absPath(*versionIter));
        worldIds.push_back(id);

        ++versionIter;
      }
      ++worldIter;
    }
    ++ownIter;
  }
  return worldIds;
}

//////////////////////////////////////////////////
LocalCache::LocalCache(const ClientConfig *_config)
  : dataPtr(new LocalCachePrivate)
{
  this->dataPtr->config = _config;
}

//////////////////////////////////////////////////
LocalCache::~LocalCache()
{
}

//////////////////////////////////////////////////
ModelIter LocalCache::AllModels()
{
  std::vector<Model> models;
  if (this->dataPtr->config)
  {
    for (auto &server : this->dataPtr->config->Servers())
    {
      std::string path = common::joinPaths(
          this->dataPtr->config->CacheLocation(), uriToPath(server.Url()));

      auto srvModels = this->dataPtr->ModelsInServer(path);
      for (auto &mod : srvModels)
      {
        mod.dataPtr->id.SetServer(server);
      }
      models.insert(models.end(), srvModels.begin(), srvModels.end());
    }
  }

  return ModelIterFactory::Create(models);
}

//////////////////////////////////////////////////
WorldIter LocalCache::AllWorlds() const
{
  std::vector<WorldIdentifier> worldIds;
  if (this->dataPtr->config)
  {
    // Iterate over servers
    for (auto &server : this->dataPtr->config->Servers())
    {
      std::string path = common::joinPaths(
          this->dataPtr->config->CacheLocation(), uriToPath(server.Url()));

      // Make sure the server info is correct
      auto srvWorlds = this->dataPtr->WorldsInServer(path);
      for (auto &world : srvWorlds)
      {
        WorldIdentifier id = world;
        id.SetServer(server);

        worldIds.push_back(id);
      }
    }
  }

  return WorldIterFactory::Create(worldIds);
}

//////////////////////////////////////////////////
Model LocalCache::MatchingModel(const ModelIdentifier &_id)
{
  // For the tip, we have to find the highest version
  bool tip = (_id.Version() == 0);
  Model tipModel;
  if (!this->dataPtr->config)
    return tipModel;

  std::string path = common::joinPaths(
      this->dataPtr->config->CacheLocation(), uriToPath(_id.Server().Url()));

  auto srvModels = this->dataPtr->ModelsInServer(path);
  for (auto &model : srvModels)
  {
    model.dataPtr->id.SetServer(_id.Server());
    auto id = model.Identification();
    if (_id == id)
    {
      if (_id.Version() == id.Version())
        return model;
      else if (tip && id.Version() > tipModel.Identification().Version())
        tipModel = model;
    }
  }

  return tipModel;
}

//////////////////////////////////////////////////
bool LocalCache::MatchingWorld(WorldIdentifier &_id) const
{
  // For the tip, we have to find the highest version
  bool tip = (_id.Version() == 0);
  WorldIdentifier tipWorld;

  if (!this->dataPtr->config)
    return false;

  std::string path = common::joinPaths(
      this->dataPtr->config->CacheLocation(), uriToPath(_id.Server().Url()));

  auto srvWorlds = this->dataPtr->WorldsInServer(path);
  for (auto id : srvWorlds)
  {
    id.SetServer(_id.Server());
    if (_id == id)
    {
      if (_id.Version() == id.Version())
      {
        _id = id;
        return true;
      }
      else if (tip && id.Version() > tipWorld.Version())
      {
        tipWorld = id;
      }
    }
  }

  auto foundTip = !(tipWorld == WorldIdentifier());
  if (foundTip)
  {
    _id = tipWorld;
  }

  return foundTip;
}

//////////////////////////////////////////////////
ModelIter LocalCache::MatchingModels(const ModelIdentifier &_id)
{
  if (_id.Name().empty() && !_id.Server().Url().Valid() && _id.Owner().empty())
    return ModelIterFactory::Create();

  std::vector<Model> models;
  for (auto iter = this->AllModels(); iter; ++iter)
  {
    bool matches = true;
    if (!_id.Name().empty() && _id.Name() != iter->Identification().Name())
      matches = false;
    if (!_id.Owner().empty() && _id.Owner() != iter->Identification().Owner())
      matches = false;
    if (_id.Server().Url().Valid() &&
        _id.Server().Url().Str() != iter->Identification().Server().Url().Str())
      matches = false;
    if (matches)
      models.push_back(*iter);
  }
  return ModelIterFactory::Create(models);
}

//////////////////////////////////////////////////
WorldIter LocalCache::MatchingWorlds(const WorldIdentifier &_id) const
{
  if (_id.Name().empty() && !_id.Server().Url().Valid() && _id.Owner().empty())
    return WorldIterFactory::Create();

  std::vector<WorldIdentifier> worldIds;
  for (auto iter = this->AllWorlds(); iter; ++iter)
  {
    bool matches = true;
    if (!_id.Name().empty() && _id.Name() != iter->Name())
      matches = false;
    if (!_id.Owner().empty() && _id.Owner() != iter->Owner())
      matches = false;
    if (_id.Server().Url().Valid() &&
        _id.Server().Url().Str() != iter->Server().Url().Str())
      matches = false;
    if (matches)
      worldIds.push_back(iter);
  }
  return WorldIterFactory::Create(worldIds);
}

//////////////////////////////////////////////////
bool LocalCache::SaveModel(
  const ModelIdentifier &_id, const std::string &_data, const bool _overwrite)
{
  if (_id.Server().Url().Str().empty() || _id.Owner().empty() ||
      _id.Name().empty() || _id.Version() == 0)
  {
    gzerr << "Incomplete model identifier, failed to save model." << std::endl
           << _id.AsString();
    return false;
  }

  std::string cacheLocation = this->dataPtr->config->CacheLocation();

  std::string modelRootDir = common::joinPaths(cacheLocation,
                                               _id.UniqueName());

  std::string modelVersionedDir =
    common::joinPaths(modelRootDir, _id.VersionStr());

  // Is it already in the cache?
  if (common::isDirectory(modelVersionedDir) && !_overwrite)
  {
    gzerr << "Directory [" << modelVersionedDir << "] already exists"
           << std::endl;
    return false;
  }

  // Create the model directory.
  if (!common::createDirectories(modelVersionedDir))
  {
    gzerr << "Unable to create directory [" << modelVersionedDir << "]"
           << std::endl;
  }

  auto zipFile = common::joinPaths(modelVersionedDir, _id.Name() + ".zip");
#ifdef _WIN32
  std::ofstream ofs(zipFile, std::ofstream::out | std::ofstream::binary);
#else
  std::ofstream ofs(zipFile, std::ofstream::out);
#endif
  ofs << _data;
  ofs.close();

  if (!Zip::Extract(zipFile, modelVersionedDir))
  {
    gzerr << "Unable to unzip [" << zipFile << "]" << std::endl;
    return false;
  }

  // Convert model:// URIs to Fuel URLs
  this->dataPtr->FixPaths(modelVersionedDir, _id);

  // Cleanup the zip file.
  if (!common::removeDirectoryOrFile(zipFile))
  {
    gzwarn << "Unable to remove [" << zipFile << "]" << std::endl;
  }

  return true;
}

//////////////////////////////////////////////////
bool LocalCachePrivate::FixPaths(const std::string &_modelVersionedDir,
    const ModelIdentifier &_id)
{
  // Get model.config
  std::string modelConfigPath = common::joinPaths(
      _modelVersionedDir, "model.config");

  // Make sure the model config file exits.
  if (!common::exists(modelConfigPath))
  {
    gzerr << "model.config file does not exist in ["
      << _modelVersionedDir << ".\n";
    return false;
  }

  // Load the model config into tinyxml
  tinyxml2::XMLDocument modelConfigDoc;
  if (modelConfigDoc.LoadFile(modelConfigPath.c_str()) !=
      tinyxml2::XML_SUCCESS)
  {
    gzerr << "Unable to load model.config file[" << modelConfigPath << "]\n";
    return false;
  }

  // Get the first <model> element. There really should only be one, but
  // we are not being strict.
  tinyxml2::XMLElement *modelElement = modelConfigDoc.FirstChildElement(
      "model");

  // Get the <sdf> element with the highest (most recent) version.
  tinyxml2::XMLElement *sdfElementLatest = nullptr;
  math::SemanticVersion maxVersion{"0.0"};
  tinyxml2::XMLElement *sdfElement = modelElement->FirstChildElement("sdf");
  while (sdfElement)
  {
    math::SemanticVersion version;

    auto versionAttribute = sdfElement->Attribute("version");
    if (nullptr == versionAttribute)
    {
      version.Parse("0.0.1");
      gzwarn << "<sdf> element missing version attribute, assuming version ["
              << version << "]" << std::endl;
    }
    else
    {
      version.Parse(versionAttribute);
    }

    if (version > maxVersion)
    {
      maxVersion = version;
      sdfElementLatest = sdfElement;
    }

    sdfElement = sdfElement->NextSiblingElement("sdf");
  }

  // Get name of the model SDF file.
  std::string modelSdfFilePath = common::joinPaths(_modelVersionedDir,
      sdfElementLatest->GetText());

  // Load the model SDF file
  tinyxml2::XMLDocument modelSdfDoc;
  if (modelSdfDoc.LoadFile(modelSdfFilePath.c_str()) !=
      tinyxml2::XML_SUCCESS)
  {
    gzerr << "Unable to load SDF file[" << modelSdfFilePath << "]\n";
    return false;
  }

  // Process each <model>
  tinyxml2::XMLElement *modelElem =
    modelSdfDoc.RootElement()->FirstChildElement("model");
  while (modelElem)
  {
    // Process each <link>
    tinyxml2::XMLElement *linkElem = modelElem->FirstChildElement("link");
    while (linkElem)
    {
      // Process each <collision>
      tinyxml2::XMLElement *collisionElem =
        linkElem->FirstChildElement("collision");
      while (collisionElem)
      {
        this->FixPathsInGeomElement(
            collisionElem->FirstChildElement("geometry"), _id);
        // Next collision element.
        collisionElem = collisionElem->NextSiblingElement("collision");
      }

      // Process each <visual>
      tinyxml2::XMLElement *visualElem =
        linkElem->FirstChildElement("visual");
      while (visualElem)
      {
        this->FixPathsInGeomElement(
            visualElem->FirstChildElement("geometry"), _id);
        this->FixPathsInMaterialElement(
            visualElem->FirstChildElement("material"), _id);
        visualElem = visualElem->NextSiblingElement("visual");
      }
      linkElem = linkElem->NextSiblingElement("link");
    }
    modelElem = modelElem->NextSiblingElement("model");
  }

  // Process each <actor>
  auto actorElem = modelSdfDoc.RootElement()->FirstChildElement("actor");
  while (actorElem)
  {
    // Process each <skin>
    auto skinElem = actorElem->FirstChildElement("skin");
    while (skinElem)
    {
      // Process <filename>
      auto filenameElem = skinElem->FirstChildElement("filename");
      if (filenameElem)
      {
        this->FixPathsInUri(filenameElem, _id);
      }
      skinElem = skinElem->NextSiblingElement("skin");
    }
    // Process each <animation>
    auto animationElem = actorElem->FirstChildElement("animation");
    while (animationElem)
    {
      // Process <filename>
      auto filenameElem = animationElem->FirstChildElement("filename");
      if (filenameElem)
      {
        this->FixPathsInUri(filenameElem, _id);
      }
      animationElem = animationElem->NextSiblingElement("animation");
    }
    actorElem = actorElem->NextSiblingElement("actor");
  }
  modelSdfDoc.SaveFile(modelSdfFilePath.c_str());

  return true;
}

//////////////////////////////////////////////////
void LocalCachePrivate::FixPathsInUri(tinyxml2::XMLElement *_elem,
    const ModelIdentifier &_id)
{
  if (!_elem)
    return;

  std::string oldUri = _elem->GetText();
  std::string prefix =  "model://";

  // Make sure the URI is of the form model://
  if (oldUri.find(prefix) == std::string::npos)
    return;

  int firstSlash = oldUri.find('/', prefix.size()+1);

  auto resourceName = oldUri.substr(prefix.size(), firstSlash - prefix.size());

  if (resourceName != _id.Name())
  {
    // On Blueprint and Citadel, just warn the user
    // From Dome, use the name on the URI (resourceName) and assume the same
    // owner
    gzdbg << "Model [" << _id.Name()
           << "] loading resource from another model, named [" << resourceName
           << "]. On Blueprint (gz-fuel-tools 3) and Citadel "
           << "(gz-fuel-tools 4), [" << resourceName << "] is ignored. "
           << "From Dome (gz-fuel-tools 5), [" << _id.Name()
           << "] will be used. If [" << resourceName
           << "] is not a model belonging to owner [" << _id.Owner()
           << "], fix your SDF file!" << std::endl;
  }

  auto filePath = oldUri.substr(firstSlash);

  // Construct a model file URL used to download from the server
  auto fuelUrl =
      _id.Server().Url().Str() + '/' +
      _id.Server().Version() + '/' +
      _id.Owner() +
      "/models/" +
       _id.Name() + '/' +
       _id.VersionStr() +
      "/files" +
       filePath;

  _elem->SetText(fuelUrl.c_str());
}

//////////////////////////////////////////////////
void LocalCachePrivate::FixPathsInMaterialElement(
    tinyxml2::XMLElement *_matElem,
    const ModelIdentifier &_id)
{
  if (!_matElem)
    return;

  // Get the <script> element, if present.
  tinyxml2::XMLElement *scriptElem = _matElem->FirstChildElement("script");

  if (scriptElem)
  {
    tinyxml2::XMLElement *uriElem = scriptElem->FirstChildElement("uri");
    // Convert the "model://" URI pattern to file://
    while (uriElem)
    {
      this->FixPathsInUri(uriElem, _id);
      uriElem = uriElem->NextSiblingElement("uri");
    }
  }

  // Get the <pbr> element, if present.
  tinyxml2::XMLElement *pbrElem = _matElem->FirstChildElement("pbr");
  if (pbrElem)
  {
    std::vector<std::string> workflows{"metal", "specular"};
    for (auto workflow : workflows)
    {
      tinyxml2::XMLElement *workflowElem =
          pbrElem->FirstChildElement(workflow.c_str());
      if (workflowElem)
      {
        tinyxml2::XMLElement *albedoElem =
            workflowElem->FirstChildElement("albedo_map");
        if (albedoElem)
          this->FixPathsInUri(albedoElem, _id);
        tinyxml2::XMLElement *normalElem =
            workflowElem->FirstChildElement("normal_map");
        if (normalElem)
          this->FixPathsInUri(normalElem, _id);
        tinyxml2::XMLElement *envElem =
            workflowElem->FirstChildElement("environment_map");
        if (envElem)
          this->FixPathsInUri(envElem, _id);
        tinyxml2::XMLElement *emissiveElem =
            workflowElem->FirstChildElement("emissive_map");
        if (emissiveElem)
          this->FixPathsInUri(emissiveElem, _id);
        tinyxml2::XMLElement *lightElem =
            workflowElem->FirstChildElement("light_map");
        if (lightElem)
          this->FixPathsInUri(lightElem, _id);
        // metal workflow specific elements
        if (workflow == "metal")
        {
          tinyxml2::XMLElement *metalnessElem =
              workflowElem->FirstChildElement("metalness_map");
          if (metalnessElem)
            this->FixPathsInUri(metalnessElem, _id);
          tinyxml2::XMLElement *roughnessElem =
              workflowElem->FirstChildElement("roughness_map");
          if (roughnessElem)
            this->FixPathsInUri(roughnessElem, _id);
        }
        // specular workflow specific elements
        else if (workflow == "specular")
        {
          tinyxml2::XMLElement *specularElem =
              workflowElem->FirstChildElement("specular_map");
          if (specularElem)
            this->FixPathsInUri(specularElem, _id);
          tinyxml2::XMLElement *glossinessElem =
              workflowElem->FirstChildElement("glossiness_map");
          if (glossinessElem)
            this->FixPathsInUri(glossinessElem, _id);
        }
      }
    }
  }
}


//////////////////////////////////////////////////
void LocalCachePrivate::FixPathsInGeomElement(tinyxml2::XMLElement *_geomElem,
    const ModelIdentifier &_id)
{
  if (!_geomElem)
    return;

  // Get the mesh element, if present.
  tinyxml2::XMLElement *meshElem = _geomElem->FirstChildElement("mesh");

  if (meshElem)
  {
    tinyxml2::XMLElement *uriElem = meshElem->FirstChildElement("uri");
    // Convert the "model://" URI pattern to file://
    this->FixPathsInUri(uriElem, _id);
  }
}

//////////////////////////////////////////////////
bool LocalCache::SaveWorld(
  WorldIdentifier &_id, const std::string &_data, const bool _overwrite)
{
  if (!_id.Server().Url().Valid() || _id.Owner().empty() ||
      _id.Name().empty() || _id.Version() == 0)
  {
    gzerr << "Incomplete world identifier, failed to save world." << std::endl
           << _id.AsString();
    return false;
  }

  auto cacheLocation = this->dataPtr->config->CacheLocation();
  auto worldRootDir = common::joinPaths(cacheLocation, _id.UniqueName());
  auto worldVersionedDir = common::joinPaths(worldRootDir, _id.VersionStr());

  // Is it already in the cache?
  if (common::isDirectory(worldVersionedDir) && !_overwrite)
  {
    gzerr << "Directory [" << worldVersionedDir << "] already exists"
           << std::endl;
    return false;
  }

  // Create the world directory.
  if (!common::createDirectories(worldVersionedDir))
  {
    gzerr << "Unable to create directory [" << worldVersionedDir << "]"
           << std::endl;
  }

  auto zipFile = common::joinPaths(worldVersionedDir, _id.Name() + ".zip");
  #ifdef _WIN32
    std::ofstream ofs(zipFile, std::ofstream::out | std::ofstream::binary);
  #else
    std::ofstream ofs(zipFile, std::ofstream::out);
  #endif
  ofs << _data;
  ofs.close();

  if (!Zip::Extract(zipFile, worldVersionedDir))
  {
    gzerr << "Unable to unzip [" << zipFile << "]" << std::endl;
    return false;
  }

  if (!common::removeDirectoryOrFile(zipFile))
  {
    gzwarn << "Unable to remove [" << zipFile << "]" << std::endl;
  }

  _id.SetLocalPath(worldVersionedDir);
  gzmsg << "Saved world at:" << std::endl
         << "  " << worldVersionedDir << std::endl;

  return true;
}
}  // namespace gz::fuel_tools
