/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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

#include <cstddef>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "gz/common/graphics/Types.hh"
#include "gz/common/AssimpLoader.hh"
#include "gz/common/Console.hh"
#include "gz/common/Image.hh"
#include "gz/common/Material.hh"
#include "gz/common/Mesh.hh"
#include "gz/common/Skeleton.hh"
#include "gz/common/SkeletonAnimation.hh"
#include "gz/common/SubMesh.hh"
#include "gz/common/SystemPaths.hh"
#include "gz/common/Util.hh"

#ifndef GZ_ASSIMP_PRE_5_2_0
  #include <assimp/GltfMaterial.h>    // GLTF specific material properties
#endif
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/scene.h>           // Output data structure
#include <assimp/material.h>

// Disable warning for converting double to unsigned char
#ifdef _WIN32
  #pragma warning( disable : 4244 )
#endif

namespace gz
{
namespace common
{

using ImagePtr = std::shared_ptr<Image>;

/// \brief Private data for the AssimpLoader class
class AssimpLoader::Implementation
{
  /// \brief the Assimp importer used to parse meshes
  public: Assimp::Importer importer;

  /// \brief Cache for images loaded from memory (e.g. embedded)
  public: mutable std::unordered_map<std::string, ImagePtr> imageCache;

  /// \brief Cache for split metallic roughness maps
  public: mutable std::unordered_map<std::string,
          std::pair<ImagePtr, ImagePtr>> splitMapCache;

  /// \brief Path of the mesh being loaded, used to qualify embedded textures
  public: std::string currentMeshPath;

  /// \brief Convert a color from assimp implementation to Gazebo common
  /// \param[in] _color the assimp color to convert
  /// \return the matching math::Color
  public: math::Color ConvertColor(aiColor4D& _color) const;

  /// \brief Convert a matrix from assimp implementation to gz::Math
  /// \param[in] _matrix the assimp matrix to convert
  /// \return the converted math::Matrix4d
  public: math::Matrix4d ConvertTransform(const aiMatrix4x4& _matrix) const;

  /// \brief Convert from assimp to gz::common::Material
  /// \param[in] _scene the assimp scene
  /// \param[in] _matIdx index of the material in the scene
  /// \param[in] _path path where the mesh is located
  /// \return pointer to the converted common::Material
  public: MaterialPtr CreateMaterial(const aiScene *_scene,
                                     unsigned _matIdx,
                                     const std::string &_path) const;

  /// \brief Load a texture embedded in a mesh (i.e. for GLB format)
  /// into a gz::common::Image
  /// \param[in] _texture the assimp texture object
  /// \return Pointer to a common::Image containing the texture
  public: ImagePtr LoadEmbeddedTexture(const aiTexture* _texture) const;

  /// \brief Utility function to generate a texture name for both embedded
  /// and external textures
  /// \param[in] _texId the unique identifier for the texture (path or index)
  /// \param[in] _type the type of texture (i.e. Diffuse, Metal)
  /// \return the generated texture name
  public: std::string GenerateTextureName(const std::string &_texId,
                                          const std::string &_type) const;

  /// \brief Function to parse texture information and load it if embedded
  /// \param[in] _scene the assimp scene
  /// \param[in] _texturePath the path where the texture is located
  /// \param[in] _textureName the name of the texture
  /// \param[in] _shouldCache Whether the image should be cached
  /// \return a pair containing the name of the texture and a pointer to the
  /// image data, if the texture was loaded in memory
  public: std::pair<std::string, ImagePtr>
          LoadTexture(const aiScene* _scene,
                      const aiString& _texturePath,
                      const std::string& _textureName,
                      bool _shouldCache = true) const;

  /// \brief Function to split a gltf metallicroughness map into
  /// a metalness and roughness map
  /// \param[in] _img the image to split
  /// \return a pair of image pointers with the first being the metalness
  /// map and the second being the roughness map
  public: std::pair<ImagePtr, ImagePtr>
          SplitMetallicRoughnessMap(const Image& _img) const;

  /// \brief Convert an assimp mesh into a gz::common::SubMesh
  /// \param[in] _assimpMesh the assimp mesh to load
  /// \param[in] _transform the node transform for the mesh
  /// \return the converted common::Submesh
  public: SubMesh CreateSubMesh(const aiMesh* _assimpMesh,
                                const math::Matrix4d& _transform) const;

  /// \brief Recursively create submeshes scene starting from the root node
  /// \param[in] _scene the assimp scene
  /// \param[in] _node the node being processed
  /// \param[in] _transform the transform of the node being processed
  /// \param[out] _mesh the common::Mesh to edit
  public: void RecursiveCreate(const aiScene* _scene,
                               const aiNode* _node,
                               const math::Matrix4d& _transform,
                               Mesh* _mesh) const;

  /// \brief Recursively create the skeleton starting from the root node
  /// \param[in] _node the node being processed
  /// \param[in] _parent the parent skeleton node
  /// \param[in] _transform the transform of the current node
  /// \param[in] _boneNames set of bone names, used to skip nodes without a bone
  public: void RecursiveSkeletonCreate(
          const aiNode* _node,
          SkeletonNode* _parent,
          const math::Matrix4d& _transform,
          const std::unordered_set<std::string> &_boneNames) const;

  /// \brief Recursively store the bone names starting from the root node
  /// to make sure that only nodes that map to a bone are added to the skeleton
  /// \param[in] _scene the assimp scene
  /// \param[in] _node the node being processed
  /// \param[out] _boneNames set of bone names populated while recursing
  public: void RecursiveStoreBoneNames(
          const aiScene *_scene,
          const aiNode* _node,
          std::unordered_set<std::string>& _boneNames) const;

  /// \brief Apply the the inv bind transform to the skeleton pose.
  /// \remarks have to set the model transforms starting from the root in
  /// breadth first order. Because setting the model transform also updates
  /// the transform based on the parent's inv model transform. Setting the
  /// child before the parent results in the child's transform being
  /// calculated from the "old" parent model transform.
  /// \param[in] _skeleton the skeleton to work on
  public: void ApplyInvBindTransform(SkeletonPtr _skeleton) const;

  /// Get the updated root node transform. The function updates the original
  /// transform by setting the rotation to identity if requested.
  /// \param[in] _scene Scene with axes info stored in meta data
  /// \param[in] _useIdentityRotation Whether to set rotation to identity.
  /// Note: This is currently set to false for glTF / glb meshes.
  /// \return Updated transform
  public: aiMatrix4x4 UpdatedRootNodeTransform(const aiScene *_scene,
      bool _useIdentityRotation = true);

  /// \brief Helper to generate a globally unique key for a texture
  /// \param[in] _texturePath path string from assimp
  /// \return the unique key
  public: std::string FullTextureKey(const std::string &_texturePath) const;
};

//////////////////////////////////////////////////
// Utility function to convert to std::string from aiString
static std::string ToString(const aiString& str)
{
  return std::string(str.C_Str());
}

//////////////////////////////////////////////////
std::string AssimpLoader::Implementation::FullTextureKey(
    const std::string &_texturePath) const
{
  if (_texturePath.empty())
    return "";

  // Assimp uses '*' as a prefix for embedded texture paths
  // See https://github.com/assimp/assimp/blob/3e672ff856b0bad35f478cc11acdd903674066ee/include/assimp/texture.h#L66-L81
  if (_texturePath[0] == '*')
  {
    return this->currentMeshPath + "#" + _texturePath;
  }

  if (common::isRelativePath(_texturePath))
  {
    return common::joinPaths(common::parentPath(this->currentMeshPath),
                             _texturePath);
  }

  return _texturePath;
}

//////////////////////////////////////////////////
math::Color AssimpLoader::Implementation::ConvertColor(aiColor4D& _color) const
{
  math::Color col(_color.r, _color.g, _color.b, _color.a);
  return col;
}

//////////////////////////////////////////////////
math::Matrix4d AssimpLoader::Implementation::ConvertTransform(
    const aiMatrix4x4& _sm) const
{
  return math::Matrix4d(
      _sm.a1, _sm.a2, _sm.a3, _sm.a4,
      _sm.b1, _sm.b2, _sm.b3, _sm.b4,
      _sm.c1, _sm.c2, _sm.c3, _sm.c4,
      _sm.d1, _sm.d2, _sm.d3, _sm.d4);
}

//////////////////////////////////////////////////
void AssimpLoader::Implementation::RecursiveCreate(const aiScene* _scene,
    const aiNode* _node, const math::Matrix4d& _transform, Mesh* _mesh) const
{
  if (!_node)
    return;
  // Visit this node, add the submesh
  for (unsigned meshIdx = 0; meshIdx < _node->mNumMeshes; ++meshIdx)
  {
    auto assimpMeshIdx = _node->mMeshes[meshIdx];
    auto& assimpMesh = _scene->mMeshes[assimpMeshIdx];
    auto nodeName = ToString(_node->mName);
    auto subMesh = this->CreateSubMesh(assimpMesh, _transform);
    subMesh.SetName(nodeName);
    // Now add the bones to the skeleton
    if (assimpMesh->HasBones() && _scene->HasAnimations())
    {
      // TODO(luca) merging skeletons here
      auto skeleton = _mesh->MeshSkeleton();
      // TODO(luca) Append to existing skeleton if multiple submeshes?
      skeleton->SetNumVertAttached(subMesh.VertexCount());
      // Now add the bone weights
      for (unsigned boneIdx = 0; boneIdx < assimpMesh->mNumBones; ++boneIdx)
      {
        auto& bone = assimpMesh->mBones[boneIdx];
        auto boneNodeName = ToString(bone->mName);
        // Apply inverse bind transform to the matching node
        SkeletonNode *skelNode =
            skeleton->NodeByName(boneNodeName);
        if (skelNode == nullptr)
          continue;
        skelNode->SetInverseBindTransform(
            this->ConvertTransform(bone->mOffsetMatrix));
        for (unsigned weightIdx = 0; weightIdx < bone->mNumWeights; ++weightIdx)
        {
          auto vertexWeight = bone->mWeights[weightIdx];
          skeleton->AddVertNodeWeight(
              vertexWeight.mVertexId, boneNodeName, vertexWeight.mWeight);
        }
      }
      // Add node assignment to mesh
      for (unsigned vertexIdx = 0; vertexIdx < subMesh.VertexCount();
          ++vertexIdx)
      {
        for (unsigned i = 0; i < skeleton->VertNodeWeightCount(vertexIdx); ++i)
        {
          std::pair<std::string, double> nodeWeight =
            skeleton->VertNodeWeight(vertexIdx, i);
          SkeletonNode *node =
              skeleton->NodeByName(nodeWeight.first);
          subMesh.AddNodeAssignment(vertexIdx,
                          node->Handle(), nodeWeight.second);
        }
      }
    }
    _mesh->AddSubMesh(std::move(subMesh));
  }

  // Iterate over children
  for (unsigned childIdx = 0; childIdx < _node->mNumChildren; ++childIdx)
  {
    // Calculate the transform
    auto& child_node = _node->mChildren[childIdx];
    auto nodeTrans = this->ConvertTransform(child_node->mTransformation);
    nodeTrans = _transform * nodeTrans;

    // Finally recursive call to explore subnode
    this->RecursiveCreate(_scene, child_node, nodeTrans, _mesh);
  }
}

void AssimpLoader::Implementation::RecursiveStoreBoneNames(
    const aiScene *_scene, const aiNode *_node,
    std::unordered_set<std::string>& _boneNames) const
{
  if (!_node)
    return;

  for (unsigned meshIdx = 0; meshIdx < _node->mNumMeshes; ++meshIdx)
  {
    auto assimpMeshIdx = _node->mMeshes[meshIdx];
    auto assimpMesh = _scene->mMeshes[assimpMeshIdx];
    for (unsigned boneIdx = 0; boneIdx < assimpMesh->mNumBones; ++boneIdx)
    {
      auto bone = assimpMesh->mBones[boneIdx];
      _boneNames.insert(ToString(bone->mName));
    }
  }

  // Iterate over children
  for (unsigned childIdx = 0; childIdx < _node->mNumChildren; ++childIdx)
  {
    auto child_node = _node->mChildren[childIdx];
    // Finally recursive call to explore subnode
    this->RecursiveStoreBoneNames(_scene, child_node, _boneNames);
  }
}

//////////////////////////////////////////////////
void AssimpLoader::Implementation::RecursiveSkeletonCreate(const aiNode* _node,
    SkeletonNode* _parent, const math::Matrix4d& _transform,
    const std::unordered_set<std::string> &_boneNames) const
{
  if (_node == nullptr || _parent == nullptr)
    return;
  // First explore this node
  auto nodeName = ToString(_node->mName);
  auto boneExist = _boneNames.find(nodeName) != _boneNames.end();
  auto nodeTrans = this->ConvertTransform(_node->mTransformation);
  auto skelNode = _parent;

  if (boneExist)
  {
    skelNode = new SkeletonNode(
        _parent, nodeName, nodeName, SkeletonNode::JOINT);
    skelNode->SetTransform(nodeTrans);
  }

  nodeTrans = _transform * nodeTrans;

  for (unsigned childIdx = 0; childIdx < _node->mNumChildren; ++childIdx)
  {
    this->RecursiveSkeletonCreate(
        _node->mChildren[childIdx], skelNode, nodeTrans, _boneNames);
  }
}

//////////////////////////////////////////////////
MaterialPtr AssimpLoader::Implementation::CreateMaterial(
    const aiScene *_scene, unsigned _matIdx, const std::string &_path) const
{
  MaterialPtr mat = std::make_shared<Material>();
  aiColor4D color;
  bool specularDefine = false;
  // gcc is complaining about this variable not being used.
  (void) specularDefine;
  auto& assimpMat = _scene->mMaterials[_matIdx];
  auto ret = assimpMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
  if (ret == AI_SUCCESS)
  {
    mat->SetDiffuse(this->ConvertColor(color));
  }
  ret = assimpMat->Get(AI_MATKEY_COLOR_AMBIENT, color);
  if (ret == AI_SUCCESS)
  {
    specularDefine = true;
    mat->SetAmbient(this->ConvertColor(color));
  }
  ret = assimpMat->Get(AI_MATKEY_COLOR_SPECULAR, color);
  if (ret == AI_SUCCESS)
  {
    mat->SetSpecular(this->ConvertColor(color));
  }
  ret = assimpMat->Get(AI_MATKEY_COLOR_EMISSIVE, color);
  if (ret == AI_SUCCESS)
  {
    mat->SetEmissive(this->ConvertColor(color));
  }
  float shininess;
  ret = assimpMat->Get(AI_MATKEY_SHININESS, shininess);
  if (ret == AI_SUCCESS)
  {
    mat->SetShininess(shininess);
  }
  float opacity = 1.0;
  ret = assimpMat->Get(AI_MATKEY_OPACITY, opacity);
  if (ret == AI_SUCCESS)
  {
    mat->SetTransparency(1.0 - opacity);
    mat->SetBlendFactors(opacity, 1.0 - opacity);
  }

#ifndef GZ_ASSIMP_PRE_5_1_0
  // basic support for transmission - currently just overrides opacity
  // \todo(iche033) The transmission factor can be used with volume
  // material extension to simulate effects like refraction
  // so consider also extending support for other properties like
  // AI_MATKEY_VOLUME_THICKNESS_FACTOR
  float transmission = 0.0;
  ret = assimpMat->Get(AI_MATKEY_TRANSMISSION_FACTOR, transmission);
  if (ret == AI_SUCCESS)
  {
    mat->SetTransparency(transmission);
  }
#endif

  // TODO(luca) more than one texture, Gazebo assumes UV index 0
  Pbr pbr;
  aiString texturePath(_path.c_str());
  ret = assimpMat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
  // TODO(luca) check other arguments,
  // type of mappings to be UV, uv index, blend mode
  if (ret == AI_SUCCESS)
  {
    std::string textureKey = this->FullTextureKey(texturePath.C_Str());

    // Check if the texture is embedded or not
    auto [texName, texData] = this->LoadTexture(
        _scene, texturePath, this->GenerateTextureName(textureKey, "Diffuse"));
    mat->SetTextureImage(texName, texData);
#ifndef GZ_ASSIMP_PRE_5_2_0
    // Now set the alpha from texture, if enabled, only supported in GLTF
    aiString alphaMode;
    auto paramRet = assimpMat->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
    if (paramRet == AI_SUCCESS)
    {
      // Only enable if it's set to MASK, BLEND not supported yet
      if (strcmp(alphaMode.C_Str(), "MASK") == 0)
      {
        double alphaCutoff = mat->AlphaThreshold();
        bool twoSided = mat->TwoSidedEnabled();
        // Ignore return value, parameter unchanged if value is not set
        assimpMat->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutoff);
        assimpMat->Get(AI_MATKEY_TWOSIDED, twoSided);
        mat->SetAlphaFromTexture(true, alphaCutoff, twoSided);
      }
    }
#endif
  }
#ifndef GZ_ASSIMP_PRE_5_2_0
  // Edge case for GLTF, Metal and Rough texture are embedded in a
  // MetallicRoughness texture with metalness in B and roughness in G
  // Open, preprocess and split into metal and roughness map
  ret = assimpMat->GetTexture(
      AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE,
      &texturePath);
  if (ret == AI_SUCCESS)
  {
    std::string textureKey = this->FullTextureKey(texturePath.C_Str());
    auto it = this->splitMapCache.find(textureKey);
    if (it != this->splitMapCache.end())
    {
      gzdbg << "Split PBR maps for [" << textureKey << "] found in cache"
            << std::endl;
      pbr.SetMetalnessMap(this->GenerateTextureName(textureKey, "Metalness"),
                          it->second.first);
      pbr.SetRoughnessMap(this->GenerateTextureName(textureKey, "Roughness"),
                          it->second.second);
    }
    else
    {
      // There is no need to cache this image (last argument to LoadTexture) and
      // use up more memory because we will be caching the split images. We will
      // not be trying to load the image again because the if condition above
      // would have found a cache hit based on the texture key.
      auto [texName, texData] = this->LoadTexture(
          _scene, texturePath,
          this->GenerateTextureName(textureKey, "MetallicRoughness"), false);

      if (texData == nullptr)
      {
        gzerr << "Unable to load MetallicRoughness texture [" << textureKey
              << "]" << std::endl;
      }
      else
      {
        gzdbg << "Splitting MetallicRoughness map for [" << textureKey << "]"
              << std::endl;
        auto splitMaps = this->SplitMetallicRoughnessMap(*texData);
        if (splitMaps.first)
        {
          pbr.SetMetalnessMap(
              this->GenerateTextureName(textureKey, "Metalness"),
              splitMaps.first);
        }
        if (splitMaps.second)
        {
          pbr.SetRoughnessMap(
              this->GenerateTextureName(textureKey, "Roughness"),
              splitMaps.second);
        }
        this->splitMapCache[textureKey] = splitMaps;
      }
    }
  }
  else
  {
    // Load the textures separately
    ret = assimpMat->GetTexture(aiTextureType_METALNESS, 0, &texturePath);
    if (ret == AI_SUCCESS)
    {
      std::string textureKey = this->FullTextureKey(texturePath.C_Str());
      auto [texName, texData] =
          this->LoadTexture(_scene, texturePath,
                            this->GenerateTextureName(textureKey, "Metalness"));
      pbr.SetMetalnessMap(texName, texData);
    }
    ret = assimpMat->GetTexture(
        aiTextureType_DIFFUSE_ROUGHNESS, 0, &texturePath);
    if (ret == AI_SUCCESS)
    {
      std::string textureKey = this->FullTextureKey(texturePath.C_Str());
      auto [texName, texData] =
          this->LoadTexture(_scene, texturePath,
                            this->GenerateTextureName(textureKey, "Roughness"));
      pbr.SetRoughnessMap(texName, texData);
    }
  }

  // The lightmap / ambient occlusion texture may be the same texture as the
  // metallicRoughness texture but it can also be a separate texture using a
  // different uv index. In the former case, we expect the occlusion
  // data to be packed in the R channel of the metallicRoughness texture,
  // so load the occlusion data in the same ways as we do in the
  // SplitMetallicRoughnessMap function.
  // In the latter case (separate texture), no extra processing is
  // required.
  unsigned int uvIdx = 0;
  ret = assimpMat->GetTexture(
      aiTextureType_LIGHTMAP, 0, &texturePath, NULL, &uvIdx);
  if (ret == AI_SUCCESS)
  {
    std::string textureKey = this->FullTextureKey(texturePath.C_Str());
    auto [texName, texData] = this->LoadTexture(
        _scene, texturePath, this->GenerateTextureName(textureKey, "Lightmap"));
    // Separate uv set so treat it as a separate texture
    if (uvIdx > 0)
    {
      pbr.SetLightMap(texName, uvIdx, texData);
    }
    // else split the occlusion data from the metallicRoughness texture
    else
    {
      if (!texData)
      {
        gzerr << "Texture [" << textureKey << "] not loaded, cannot extract "
              << "lightmap channel" << std::endl;
      }
      else
      {
        // R channel contains the occlusion data
        // Note we are using L_INT8 format to save memory, and it will be
        // converted to RGBA by common::Image if requested by the renderer.
        auto texRData = texData->ChannelData(Image::Channel::RED);
        if (texRData.empty())
        {
          gzerr << "Unable to extract channel data for lightmap\n";
        }
        else
        {
          auto tex = std::make_shared<Image>();
          tex->SetFromData(texRData.data(), texData->Width(), texData->Height(),
              Image::L_INT8);
          pbr.SetLightMap(texName, uvIdx, tex);
        }
      }
    }
  }
#endif
  ret = assimpMat->GetTexture(aiTextureType_NORMALS, 0, &texturePath);
  if (ret == AI_SUCCESS)
  {
    std::string textureKey = this->FullTextureKey(texturePath.C_Str());
    auto [texName, texData] = this->LoadTexture(
        _scene, texturePath, this->GenerateTextureName(textureKey, "Normal"));
    // TODO(luca) different normal map spaces
    pbr.SetNormalMap(texName, NormalMapSpace::TANGENT, texData);
  }
  ret = assimpMat->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath);
  if (ret == AI_SUCCESS)
  {
    std::string textureKey = this->FullTextureKey(texturePath.C_Str());
    auto [texName, texData] = this->LoadTexture(
        _scene, texturePath, this->GenerateTextureName(textureKey, "Emissive"));
    pbr.SetEmissiveMap(texName, texData);
  }
#ifndef GZ_ASSIMP_PRE_5_2_0
  float value;
  ret = assimpMat->Get(AI_MATKEY_METALLIC_FACTOR, value);
  if (ret == AI_SUCCESS)
  {
    if (!specularDefine)
    {
      mat->SetSpecular(math::Color(value, value, value));
    }
    pbr.SetMetalness(value);
  }
  ret = assimpMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, value);
  if (ret == AI_SUCCESS)
  {
    pbr.SetRoughness(value);
  }
#endif
  mat->SetPbrMaterial(pbr);
  return mat;
}

//////////////////////////////////////////////////
std::pair<std::string, ImagePtr> AssimpLoader::Implementation::LoadTexture(
    const aiScene* _scene,
    const aiString& _texturePath,
    const std::string& _textureName,
    bool _shouldCache) const
{
  std::pair<std::string, ImagePtr> ret;
  std::string textureKey = this->FullTextureKey(_texturePath.C_Str());

  // Check if the texture is already in the cache
  auto it = this->imageCache.find(textureKey);
  if (it != this->imageCache.end())
  {
    gzdbg << "Texture [" << textureKey << "] found in cache" << std::endl;
    ret.first = _textureName;
    ret.second = it->second;
    return ret;
  }

  // Check if the texture is embedded or not
  auto embeddedTexture = _scene->GetEmbeddedTexture(_texturePath.C_Str());
  if (embeddedTexture)
  {
    // Load embedded texture
    ret.first = _textureName;
    gzdbg << "Loading embedded texture [" << textureKey << "] as ["
          << _textureName << "]" << std::endl;
    ret.second = this->LoadEmbeddedTexture(embeddedTexture);
    if (_shouldCache)
    {
      this->imageCache[textureKey] = ret.second;
    }
  }
  else
  {
    // Load external texture from disk
    if (common::exists(textureKey))
    {
      gzdbg << "Loading external texture [" << textureKey << "]" << std::endl;
      ret.second = std::make_shared<Image>(textureKey);
      if (_shouldCache)
      {
        this->imageCache[textureKey] = ret.second;
      }
    }
    else
    {
      gzerr << "External texture [" << textureKey << "] not found" << std::endl;
    }
    ret.first = _textureName;
  }
  return ret;
}

std::pair<ImagePtr, ImagePtr>
    AssimpLoader::Implementation::SplitMetallicRoughnessMap(
    const Image &_img) const
{
  std::pair<ImagePtr, ImagePtr> ret;
  // Metalness in B roughness in G
  const auto width = _img.Width();
  const auto height = _img.Height();

  std::vector<unsigned char> metalnessData8bit =
      _img.ChannelData(Image::Channel::BLUE);
  std::vector<unsigned char> roughnessData8bit =
      _img.ChannelData(Image::Channel::GREEN);

  if (metalnessData8bit.empty() || roughnessData8bit.empty())
  {
    gzerr << "Unable to extract channel data when splitting metallic "
          << "roughness map: " << std::endl;
    return ret;
  }

  // First is metal, second is rough
  ret.first = std::make_shared<Image>();
  ret.first->SetFromData(&metalnessData8bit[0], width, height, Image::L_INT8);
  ret.second = std::make_shared<Image>();
  ret.second->SetFromData(&roughnessData8bit[0], width, height, Image::L_INT8);
  return ret;
}

//////////////////////////////////////////////////
ImagePtr AssimpLoader::Implementation::LoadEmbeddedTexture(
    const aiTexture* _texture) const
{
  if (_texture->mHeight == 0)
  {
    Image::PixelFormatType format =
        Image::PixelFormatType::UNKNOWN_PIXEL_FORMAT;
    if (_texture->CheckFormat("png"))
    {
      format = Image::PixelFormatType::COMPRESSED_PNG;
    }
    else if (_texture->CheckFormat("jpg"))
    {
      format = Image::PixelFormatType::COMPRESSED_JPEG;
    }
    if (format != Image::PixelFormatType::UNKNOWN_PIXEL_FORMAT)
    {
      auto img = std::make_shared<Image>();
      img->SetFromCompressedData(
          reinterpret_cast<unsigned char *>(_texture->pcData),
          _texture->mWidth, format);
      return img;
    }
    else
    {
      gzerr << "Unable to load embedded texture. "
            << "Unsupported compressed image format"
            << std::endl;
    }
  }
  return ImagePtr();
}

//////////////////////////////////////////////////
std::string AssimpLoader::Implementation::GenerateTextureName(
    const std::string &_texId, const std::string &_type) const
{
  return _texId + "_" + _type;
}

//////////////////////////////////////////////////
SubMesh AssimpLoader::Implementation::CreateSubMesh(
    const aiMesh* _assimpMesh, const math::Matrix4d& _transform) const
{
  SubMesh subMesh;
  math::Matrix4d rot = _transform;
  rot.SetTranslation(math::Vector3d::Zero);
  // Now create the submesh
  for (unsigned vertexIdx = 0; vertexIdx < _assimpMesh->mNumVertices;
      ++vertexIdx)
  {
    // Add the vertex
    math::Vector3d vertex;
    vertex.X(_assimpMesh->mVertices[vertexIdx].x);
    vertex.Y(_assimpMesh->mVertices[vertexIdx].y);
    vertex.Z(_assimpMesh->mVertices[vertexIdx].z);
    vertex = _transform * vertex;
    subMesh.AddVertex(vertex);
    if (_assimpMesh->HasNormals()) {
      math::Vector3d normal;
      normal.X(_assimpMesh->mNormals[vertexIdx].x);
      normal.Y(_assimpMesh->mNormals[vertexIdx].y);
      normal.Z(_assimpMesh->mNormals[vertexIdx].z);
      normal = rot * normal;
      normal.Normalize();
      subMesh.AddNormal(normal);
    }
    // Iterate over sets of texture coordinates
    for (unsigned int i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++i)
    {
      if (!_assimpMesh->HasTextureCoords(i))
        continue;
      math::Vector3d texcoords;
      texcoords.X(_assimpMesh->mTextureCoords[i][vertexIdx].x);
      texcoords.Y(_assimpMesh->mTextureCoords[i][vertexIdx].y);
      subMesh.AddTexCoordBySet(texcoords.X(), texcoords.Y(), i);
    }
  }
  for (unsigned faceIdx = 0; faceIdx < _assimpMesh->mNumFaces; ++faceIdx)
  {
    auto& face = _assimpMesh->mFaces[faceIdx];
    subMesh.AddIndex(face.mIndices[0]);
    subMesh.AddIndex(face.mIndices[1]);
    subMesh.AddIndex(face.mIndices[2]);
  }
  subMesh.SetMaterialIndex(_assimpMesh->mMaterialIndex);
  if (subMesh.NormalCount() == 0u){
    subMesh.RecalculateNormals();
  }
  return subMesh;
}

//////////////////////////////////////////////////
AssimpLoader::AssimpLoader()
: MeshLoader(), dataPtr(utils::MakeUniqueImpl<Implementation>())
{
  this->dataPtr->importer.SetPropertyBool(AI_CONFIG_PP_FD_REMOVE, true);
  this->dataPtr->importer.SetPropertyBool(
      AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, false);
}

//////////////////////////////////////////////////
AssimpLoader::~AssimpLoader()
{
}

//////////////////////////////////////////////////
Mesh *AssimpLoader::Load(const std::string &_filename)
{
  this->dataPtr->currentMeshPath = _filename;
  Mesh *mesh = new Mesh();
  std::string path = common::parentPath(_filename);
  const aiScene* scene = this->dataPtr->importer.ReadFile(_filename,
      aiProcess_JoinIdenticalVertices |
      aiProcess_RemoveRedundantMaterials |
      aiProcess_SortByPType |
      aiProcess_FlipUVs |
#ifndef GZ_ASSIMP_PRE_5_2_0
      aiProcess_PopulateArmatureData |
#endif
      aiProcess_Triangulate |
      aiProcess_GenNormals |
      0);
  if (scene == nullptr)
  {
    gzerr << "Unable to import mesh [" << _filename << "]" << std::endl;
    return mesh;
  }
  auto& rootNode = scene->mRootNode;
  auto rootName = ToString(rootNode->mName);
  std::string extension;
  std::size_t extIdx = _filename.rfind(".");
  if (extIdx != std::string::npos)
  {
    extension = _filename.substr(extIdx + 1, _filename.size());
  }
  std::transform(extension.begin(), extension.end(),
      extension.begin(), ::tolower);

  // compute assimp root node transform
  bool useIdentityRotation = (extension != "glb" && extension != "glTF");
  auto transform = this->dataPtr->UpdatedRootNodeTransform(scene,
    useIdentityRotation);
  auto rootTransform = this->dataPtr->ConvertTransform(transform);

  // Add the materials first
  for (unsigned _matIdx = 0; _matIdx < scene->mNumMaterials; ++_matIdx)
  {
    auto mat = this->dataPtr->CreateMaterial(scene, _matIdx, path);
    mesh->AddMaterial(mat);
  }
  // Create the skeleton
  {
    std::unordered_set<std::string> boneNames;
    this->dataPtr->RecursiveStoreBoneNames(scene, rootNode, boneNames);
    auto rootSkelNode = new SkeletonNode(
        nullptr, rootName, rootName, SkeletonNode::NODE);
    rootSkelNode->SetTransform(rootTransform);
    rootSkelNode->SetModelTransform(rootTransform);
    for (unsigned childIdx = 0; childIdx < rootNode->mNumChildren; ++childIdx)
    {
      // First populate the skeleton with the node transforms
      this->dataPtr->RecursiveSkeletonCreate(
          rootNode->mChildren[childIdx], rootSkelNode,
          rootTransform, boneNames);
    }
    rootSkelNode->SetParent(nullptr);

    SkeletonPtr rootSkeleton = std::make_shared<Skeleton>(rootSkelNode);
    mesh->SetSkeleton(rootSkeleton);
  }
  // Now create the meshes
  // Recursive call to keep track of transforms,
  // mesh is passed by reference and edited throughout
  this->dataPtr->RecursiveCreate(scene, rootNode, rootTransform, mesh);
  auto rootSkeleton = mesh->MeshSkeleton();
  // Add the animations
  for (unsigned animIdx = 0; animIdx < scene->mNumAnimations; ++animIdx)
  {
    auto& anim = scene->mAnimations[animIdx];
    auto animName = ToString(anim->mName);
    if (animName.empty())
    {
      animName = "animation" +
                 std::to_string(rootSkeleton->AnimationCount() + 1);
    }
    SkeletonAnimation* skelAnim = new SkeletonAnimation(animName);
    for (unsigned chanIdx = 0; chanIdx < anim->mNumChannels; ++chanIdx)
    {
      auto& animChan = anim->mChannels[chanIdx];
      auto chanName = ToString(animChan->mNodeName);
      auto numKeys = std::max(
          animChan->mNumPositionKeys, animChan->mNumRotationKeys);
      // Position and rotation arrays might be different lengths,
      // iterate over the maximum of the two, safely access by checking
      // number of keys
      for (unsigned keyIdx = 0; keyIdx < numKeys; ++keyIdx)
      {
        // Note, Scaling keys are not supported right now
        // Compute the position into a math pose
        auto& posKey = animChan->mPositionKeys[
          std::min(keyIdx, animChan->mNumPositionKeys - 1)];
        auto& quatKey = animChan->mRotationKeys[
          std::min(keyIdx, animChan->mNumRotationKeys - 1)];
        math::Vector3d pos(posKey.mValue.x, posKey.mValue.y, posKey.mValue.z);
        math::Quaterniond quat(quatKey.mValue.w, quatKey.mValue.x,
            quatKey.mValue.y, quatKey.mValue.z);
        math::Pose3d pose(pos, quat);
        // Time is in ms
        skelAnim->AddKeyFrame(chanName, posKey.mTime / 1000.0, pose);
      }
    }
    mesh->MeshSkeleton()->AddAnimation(skelAnim);
  }

  this->dataPtr->ApplyInvBindTransform(mesh->MeshSkeleton());

  return mesh;
}

/////////////////////////////////////////////////
void AssimpLoader::Implementation::ApplyInvBindTransform(
    SkeletonPtr _skeleton) const

{
  std::queue<SkeletonNode *> queue;
  queue.push(_skeleton->RootNode());

  while (!queue.empty())
  {
    SkeletonNode *node = queue.front();
    queue.pop();
    if (nullptr == node)
      continue;

    if (node->HasInvBindTransform())
    {
      node->SetModelTransform(node->InverseBindTransform().Inverse(), false);
    }
    for (unsigned int i = 0; i < node->ChildCount(); i++)
      queue.push(node->Child(i));
  }
}

/////////////////////////////////////////////////
aiMatrix4x4 AssimpLoader::Implementation::UpdatedRootNodeTransform(
    const aiScene *_scene, bool _useIdentityRotation)
{
  // Some assets apear to be rotated by 90 degrees as documented here
  // https://github.com/assimp/assimp/issues/849.
  auto transform = _scene->mRootNode->mTransformation;
  if (_useIdentityRotation)
  {
    // drop rotation, but keep scaling and position
    aiVector3D rootScaling, rootAxis, rootPos;
    float angle;
    transform.Decompose(rootScaling, rootAxis, angle, rootPos);
    transform = aiMatrix4x4(rootScaling, aiQuaternion(), rootPos);
  }
  // for glTF / glb meshes, it was found that the transform is needed to
  // produce a result that is consistent with other engines / glTF viewers.
  else
  {
    transform = _scene->mRootNode->mTransformation;
  }
  return transform;
}

}
}
