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

#include <memory>

#include "gz/common/Console.hh"
#include "gz/common/Filesystem.hh"
#include "gz/common/Material.hh"
#include "gz/common/Mesh.hh"
#include "gz/common/SubMesh.hh"
#include "gz/common/OBJLoader.hh"

#define GZ_COMMON_TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace gz
{
  namespace common
  {
    /// \internal
    /// \brief OBJLoader private data
    class OBJLoader::Implementation
    {
    };
  }
}

using namespace gz;
using namespace common;

//////////////////////////////////////////////////
OBJLoader::OBJLoader()
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
}

//////////////////////////////////////////////////
OBJLoader::~OBJLoader()
{
}

//////////////////////////////////////////////////
Mesh *OBJLoader::Load(const std::string &_filename)
{
  std::map<std::string, Material *> materialIds;
  std::string path = common::parentPath(_filename);

  // check if obj is exported by blender
  // blender shoves BR fields in standard textures
  bool exportedByBlender = false;
  std::ifstream infile(_filename);
  if (infile.good())
  {
    std::string line;
    std::getline(infile, line);
    std::transform(line.begin(), line.end(), line.begin(),
        [](unsigned char c){ return std::tolower(c); });
    if (line.find("blender") != std::string::npos)
      exportedByBlender = true;
  }
  infile.close();

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  // convert polygons to triangles
  bool triangulate = true;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
      _filename.c_str(), path.c_str(), triangulate);

  if (!warn.empty())
  {
    gzwarn << warn << std::endl;
  }

  if (!err.empty())
  {
    gzerr << err << std::endl;
  }

  if (!ret)
  {
    gzerr << "Failed to load/parse " << _filename << std::endl;
    return nullptr;
  }

  Mesh *mesh = new Mesh();
  mesh->SetPath(path);

  for (auto const &s : shapes)
  {
    // obj mesh assigns a material id to each 'face' but Gazebo assigns a
    // single material to each 'submesh'. The strategy here is to identify
    // the number of unique material ids in each obj shape and create a new
    // submesh per unique material id
    std::map<int, SubMesh *> subMeshMatId;
    for (auto const id :  s.mesh.material_ids)
    {
      if (subMeshMatId.find(id) == subMeshMatId.end())
      {
        std::unique_ptr<SubMesh> subMesh(new SubMesh());
        subMesh->SetName(s.name);
        subMesh->SetPrimitiveType(SubMesh::TRIANGLES);
        subMeshMatId[id] = subMesh.get();

        Material *mat = nullptr;
        if (id >= 0 && static_cast<size_t>(id) < materials.size())
        {
          auto &m = materials[id];
          if (materialIds.find(m.name) != materialIds.end())
          {
            mat = materialIds[m.name];
          }
          else
          {
            // Create new material and pass it to mesh who will take ownership
            // of the object
            mat = new Material();
            mat->SetAmbient(
                math::Color(m.ambient[0], m.ambient[1], m.ambient[2]));
            mat->SetDiffuse(
                math::Color(m.diffuse[0], m.diffuse[1], m.diffuse[2]));
            mat->SetSpecular(
                math::Color(m.specular[0], m.specular[1], m.specular[2]));
            mat->SetEmissive(
                math::Color(m.emission[0], m.emission[1], m.emission[2]));
            mat->SetShininess(m.shininess);
            mat->SetTransparency(1.0 - m.dissolve);
            if (!m.diffuse_texname.empty())
              mat->SetTextureImage(m.diffuse_texname, path.c_str());

            // load PBR textures
            // Some obj exporters put PBR maps in the standard textures
            // while others have proper support for obj PBR extension
            Pbr pbrMat;
            // PBR shoved into standard textures
            if (!m.specular_texname.empty())
              pbrMat.SetRoughnessMap(m.specular_texname);

            // check if obj is exported by blender
            // blender obj exporter puts roughness map in specular highlight
            // field and metalness map in reflection map field!
            // see summary in https://developer.blender.org/D8868
            // detailing the existing exporter issues
            // todo(anyone) add a check for blender version to avoid this hack
            // when blender fixes their exporter issue
            if (!m.specular_highlight_texname.empty() && exportedByBlender)
            {
              pbrMat.SetRoughnessMap(m.specular_highlight_texname);
              if (!m.reflection_texname.empty())
                pbrMat.SetMetalnessMap(m.reflection_texname);
            }
            else if (!m.reflection_texname.empty())
            {
              pbrMat.SetEnvironmentMap(m.reflection_texname);
            }
            if (!m.bump_texname.empty())
              pbrMat.SetNormalMap(m.bump_texname);

            // PBR extension - overrides standard materials
            if (!m.roughness_texname.empty())
              pbrMat.SetRoughnessMap(m.roughness_texname);
            if (!m.metallic_texname.empty())
              pbrMat.SetMetalnessMap(m.metallic_texname);
            if (!m.normal_texname.empty())
              pbrMat.SetNormalMap(m.normal_texname);
            if (!m.emissive_texname.empty())
              pbrMat.SetEmissiveMap(m.emissive_texname);

            pbrMat.SetRoughness(m.roughness);
            pbrMat.SetMetalness(m.metallic);

            mat->SetPbrMaterial(pbrMat);

            materialIds[m.name] = mat;
          }
          int matIndex = mesh->IndexOfMaterial(mat);
          if (matIndex < 0)
            matIndex = mesh->AddMaterial(MaterialPtr(mat));
          subMesh->SetMaterialIndex(matIndex);
        }
        else
        {
          gzwarn << "Missing material for shape[" << s.name << "] "
              << "in OBJ file[" << _filename << "]" << std::endl;
        }
        mesh->AddSubMesh(std::move(subMesh));
      }
    }

    unsigned int indexOffset = 0;
    // For each face
    for (unsigned int f = 0; f < s.mesh.num_face_vertices.size(); ++f)
    {
      // find the submesh that corresponds to the current face material
      unsigned int matId = s.mesh.material_ids[f];
      SubMesh *subMesh = subMeshMatId[matId];

      unsigned int fnum = s.mesh.num_face_vertices[f];
      // For each vertex in the face
      for (unsigned int v = 0; v < fnum; ++v)
      {
        auto i = s.mesh.indices[indexOffset + v];

        // vertices
        int vIdx = i.vertex_index;
        gz::math::Vector3d vertex(attrib.vertices[3 * vIdx],
                                        attrib.vertices[3 * vIdx + 1],
                                        attrib.vertices[3 * vIdx + 2]);
        subMesh->AddVertex(vertex);

        // normals
        if (attrib.normals.size() > 0)
        {
          int nIdx = i.normal_index;
          gz::math::Vector3d normal(attrib.normals[3 * nIdx],
                                          attrib.normals[3 * nIdx + 1],
                                          attrib.normals[3 * nIdx + 2]);
          normal.Normalize();
          subMesh->AddNormal(normal);
        }
        // texcoords
        if (attrib.texcoords.size() > 0)
        {
          int tIdx = i.texcoord_index;
          gz::math::Vector2d uv(attrib.texcoords[2 * tIdx],
                                      attrib.texcoords[2 * tIdx + 1]);
          subMesh->AddTexCoord(uv.X(), 1.0-uv.Y());
        }
        subMesh->AddIndex(subMesh->IndexCount());
      }
      indexOffset += fnum;
    }
  }

  return mesh;
}
