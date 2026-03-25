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
#include <sstream>
#include <unordered_map>
#include <map>
#include <vector>
#include <set>

#include <gz/math/Helpers.hh>
#include <gz/math/Matrix4.hh>
#include <gz/math/Vector2.hh>
#include <gz/math/Vector3.hh>

#include "tinyxml2.h"

#include "gz/common/graphics/Types.hh"
#include "gz/common/Console.hh"
#include "gz/common/Material.hh"
#include "gz/common/SubMesh.hh"
#include "gz/common/Mesh.hh"
#include "gz/common/Skeleton.hh"
#include "gz/common/SkeletonAnimation.hh"
#include "gz/common/SystemPaths.hh"
#include "gz/common/Util.hh"
#include "gz/common/ColladaLoader.hh"

using namespace gz;
using namespace common;
using RawNodeAnim = std::map<double, std::vector<NodeTransform> >;
using RawSkeletonAnim = std::map<std::string, RawNodeAnim>;

namespace gz
{
  namespace common
  {
    /// \brief Private data for the ColladaLoader class
    class  ColladaLoader::Implementation
    {
      /// \brief scaling factor
      public: double meter;

      /// \brief COLLADA file name
      public: std::string filename;

      /// \brief material dictionary indexed by name
      public: std::map<std::string, std::string> materialMap;

      /// \brief root xml element of COLLADA data
      public: tinyxml2::XMLElement *colladaXml;

      /// \brief directory of COLLADA file name
      public: std::string path;

      /// \brief Name of the current node.
      public: std::string currentNodeName;

      /// \brief Map of collada POSITION ids to list of vectors.
      public: std::map<std::string,
              std::vector<gz::math::Vector3d> > positionIds;

      /// \brief Map of collada NORMAL ids to list of normals.
      public: std::map<std::string,
              std::vector<gz::math::Vector3d> > normalIds;

      /// \brief Map of collada TEXCOORD ids to list of texture coordinates.
      public: std::map<std::string,
              std::vector<gz::math::Vector2d> >texcoordIds;

      /// \brief Map of collada Material ids to Gazebo materials.
      public: std::map<std::string, MaterialPtr> materialIds;

      /// \brief Map of collada POSITION ids to a map of
      /// duplicate positions.
      public: std::map<std::string, std::map<unsigned int, unsigned int> >
          positionDuplicateMap;

      /// \brief Map of collada NORMAL ids to a map of
      /// duplicate normals.
      public: std::map<std::string, std::map<unsigned int, unsigned int> >
          normalDuplicateMap;

      /// \brief Map of collada TEXCOORD ids to a map of
      /// duplicate texture coordinates.
      public: std::map<std::string, std::map<unsigned int, unsigned int> >
          texcoordDuplicateMap;

      /// \brief Current scene being parsed
      public: tinyxml2::XMLElement *currentScene = nullptr;

      /// \brief Load a controller instance
      /// \param[in] _contrXml Pointer to the control XML instance
      /// \param[in] _skelXml Pointer the skeleton xml instance
      /// \param[in] _transform A tranform to apply
      /// \param[in,out] _mesh The mesh being loaded
      public: void LoadController(tinyxml2::XMLElement *_contrXml,
                                   std::vector<tinyxml2::XMLElement*> _skelXmls,
                                   const gz::math::Matrix4d &_transform,
                                   Mesh *_mesh);

      /// \brief Load animations for a skeleton
      /// \param[in] _xml Animation XML instance
      /// \param[in,out] _skel Pointer to the skeleton
      public: void LoadAnimations(tinyxml2::XMLElement *_xml,
                                   SkeletonPtr _skel);

      /// \brief Load a set of animations for a skeleton
      /// \param[in] _xml Pointer to the animation set XML instance
      /// \param[in,out] _skel Pointer to the skeleton
      public: void LoadAnimationSet(tinyxml2::XMLElement *_xml,
                                     SkeletonPtr _skel);

      /// \brief Load a single skeleton node
      /// \param[in] _xml Pointer to the XML instance
      /// \param[in,out] _parent Pointer to the Skeleton node parent
      public: SkeletonNode *LoadSingleSkeletonNode(tinyxml2::XMLElement *_xml,
                                              SkeletonNode *_parent);

      /// \brief Load skeleton nodes
      /// \param[in] _xml Pointer to the XML instance
      /// \param[in,out] _parent Pointer to the Skeleton node parent
      public: SkeletonNode *LoadSkeletonNodes(tinyxml2::XMLElement *_xml,
                                               SkeletonNode *_parent);

      /// \brief Set the tranform for a skeleton node
      /// \param[in] _elem Pointer to the XML instance
      /// \param[in,out] _node The skeleton node
      public: void SetSkeletonNodeTransform(tinyxml2::XMLElement *_elem,
                                             SkeletonNode *_node);

      /// \brief Load geometry elements
      /// \param[in] _xml Pointer to the XML instance
      /// \param[in] _tranform Transform to apply to the loaded geometry
      /// \param[in,out] _mesh Pointer to the mesh currently being loaded
      public: void LoadGeometry(tinyxml2::XMLElement *_xml,
                                 const gz::math::Matrix4d &_transform,
                                 Mesh *_mesh);

      /// \brief Get an XML element by ID
      /// \param[in] _parent The parent element
      /// \param[in] _name String name of the element
      /// \param[in] _id String ID of the element
      /// \return XML element with the specified ID
      public: tinyxml2::XMLElement *ElementId(tinyxml2::XMLElement *_parent,
                                               const std::string &_name,
                                               const std::string &_id);

      /// \brief Get an XML element by ID
      /// \param[in] _name String name of the element
      /// \param[in] _id String ID of the element
      /// \return XML element with the specified ID
      public: tinyxml2::XMLElement *ElementId(const std::string &_name,
                                               const std::string &_id);

      /// \brief Load a node
      /// \param[in] _elem Pointer to the node XML instance
      /// \param[in,out] _mesh Pointer to the current mesh
      /// \param[in] _transform Transform to apply to the node
      public: void LoadNode(tinyxml2::XMLElement *_elem,
                             Mesh *_mesh,
                             const gz::math::Matrix4d &_transform);

      /// \brief Load a transform
      /// \param[in] _elem Pointer to the transform XML instance
      /// \return A Matrix4 transform
      public: gz::math::Matrix4d LoadNodeTransform(
          tinyxml2::XMLElement *_elem);

      /// \brief Load vertices
      /// \param[in] _id String id of the vertices XML node
      /// \param[in] _transform Transform to apply to all vertices
      /// \param[out] _verts Holds the resulting vertices
      /// \param[out] _norms Holds the resulting normals
      public: void LoadVertices(const std::string &_id,
          const gz::math::Matrix4d &_transform,
          std::vector<gz::math::Vector3d> &_verts,
          std::vector<gz::math::Vector3d> &_norms);

      /// \brief Load vertices
      /// \param[in] _id String id of the vertices XML node
      /// \param[in] _transform Transform to apply to all vertices
      /// \param[out] _verts Holds the resulting vertices
      /// \param[out] _norms Holds the resulting normals
      /// \param[out] _vertDup Holds a map of duplicate position indices
      /// \param[out] _normDup Holds a map of duplicate normal indices
      public: void LoadVertices(const std::string &_id,
                                 const gz::math::Matrix4d &_transform,
                                 std::vector<gz::math::Vector3d> &_verts,
                                 std::vector<gz::math::Vector3d> &_norms,
                                 std::map<unsigned int, unsigned int> &_vertDup,
                                std::map<unsigned int, unsigned int> &_normDup);

      /// \brief Load positions
      /// \param[in] _id String id of the XML node
      /// \param[in] _transform Transform to apply to all positions
      /// \param[out] _values Holds the resulting position values
      /// \param[out] _duplicates Holds a map of duplicate position indices
      public: void LoadPositions(const std::string &_id,
                                  const gz::math::Matrix4d &_transform,
                                 std::vector<gz::math::Vector3d> &_values,
                             std::map<unsigned int, unsigned int> &_duplicates);

      /// \brief Load normals
      /// \param[in] _id String id of the XML node
      /// \param[in] _transform Transform to apply to all normals
      /// \param[out] _values Holds the resulting normal values
      /// \param[out] _duplicates Holds a map of duplicate normal indices
      public: void LoadNormals(const std::string &_id,
                                const gz::math::Matrix4d &_transform,
                                std::vector<gz::math::Vector3d> &_values,
                             std::map<unsigned int, unsigned int> &_duplicates);

      /// \brief Load texture coordinates
      /// \param[in] _id String id of the XML node
      /// \param[out] _values Holds the resulting uv values
      /// \param[out] _duplicates Holds a map of duplicate uv indices
      public: void LoadTexCoords(const std::string &_id,
                                 std::vector<gz::math::Vector2d> &_values,
                             std::map<unsigned int, unsigned int> &_duplicates);

      /// \brief Load a material
      /// \param _name Name of the material XML element
      /// \return A pointer to the new material
      public: MaterialPtr LoadMaterial(const std::string &_name);

      /// \brief Load a color or texture
      /// \param[in] _elem Pointer to the XML element
      /// \param[in] _type One of {diffuse, ambient, emission}
      /// \param[out] _mat Material to load the texture or color into
      public: void LoadColorOrTexture(tinyxml2::XMLElement *_elem,
                                       const std::string &_type,
                                       MaterialPtr _mat);

      /// \brief Load triangles
      /// \param[in] _trianglesXml Pointer the triangles XML instance
      /// \param[in] _transform Transform to apply to all triangles
      /// \param[out] _mesh Mesh that is currently being loaded
      public: void LoadTriangles(tinyxml2::XMLElement *_trianglesXml,
                                  const gz::math::Matrix4d &_transform,
                                  Mesh *_mesh);

      /// \brief Load a polygon list
      /// \param[in] _polylistXml Pointer to the XML element
      /// \param[in] _transform Transform to apply to each polygon
      /// \param[out] _mesh Mesh that is currently being loaded
      public: void LoadPolylist(tinyxml2::XMLElement *_polylistXml,
                                   const gz::math::Matrix4d &_transform,
                                   Mesh *_mesh);

      /// \brief Load lines
      /// \param[in] _xml Pointer to the XML element
      /// \param[in] _transform Transform to apply
      /// \param[out] _mesh Mesh that is currently being loaded
      public: void LoadLines(tinyxml2::XMLElement *_xml,
                              const gz::math::Matrix4d &_transform,
                              Mesh *_mesh);

      /// \brief Load an entire scene
      /// \param[out] _mesh Mesh that is currently being loaded
      public: void LoadScene(Mesh *_mesh);

      /// \brief Load a float value
      /// \param[out] _elem Pointer to the XML element
      /// \return The float value
      public: float LoadFloat(tinyxml2::XMLElement *_elem);

      /// \brief Load a transparent material. NOT FULLY IMPLEMENTED
      /// \param[in] _elem Pointer to the XML element
      /// \param[out] _mat Material to hold the transparent properties
      public: void LoadTransparent(tinyxml2::XMLElement *_elem,
                                    MaterialPtr _mat);

      /// \brief Merges a new root node to the skeleton
      /// \details This will do 1 of the things:
      ///     1: If `_mergeNode` is already part of the skeleton, do nothing
      ///     2: If the skeleton's root is a descendent of `_mergeNode`, sets
      ///         the new root node as `_mergeNode`
      ///     3: If the skeleton and `_mergeNode` is unrelated, creates a new
      ///         dummy root and adds both of them as childrens.
      //      4: If a dummy root already exists but the merge node contains
      //          all its children, set the merge node as the new root.
      /// \param[in] _skeleton skeleton to merge
      /// \param[in] _mergeNode new root node to merge
      public: void MergeSkeleton(SkeletonPtr _skeleton,
          SkeletonNode *_mergeNode);

      /// \brief Apply the the inv bind transform to the skeleton pose.
      /// \remarks have to set the model transforms starting from the root in
      /// breadth first order. Because setting the model transform also updates
      /// the transform based on the parent's inv model transform. Setting the
      /// child before the parent results in the child's transform being
      /// calculated from the "old" parent model transform.
      /// \param[in] _skeleton the skeleton to work on
      public: void ApplyInvBindTransform(SkeletonPtr _skeleton);
    };

    /// \brief Helper data structure for loading collada geometries.
    class GeometryIndices
    {
      /// \brief Index of a vertex in the collada <p> element
      public: unsigned int vertexIndex;

      /// \brief Index of a normal in the collada <p> element
      public: unsigned int normalIndex;

      /// \brief A map of texture coordinate set index to index of a texture
      /// coordinate in the collada <p> element
      public: std::map<unsigned int, unsigned int> texcoordIndex;

      /// \brief Index of a vertex in the Gazebo mesh
      public: unsigned int mappedIndex;
    };
  }
}

/////////////////////////////////////////////////
void hash_combine(std::size_t &_seed, const double &_v)
{
  std::hash<double> hasher;
  _seed ^= hasher(_v) + 0x9e3779b9 + (_seed << 6) + (_seed >> 2);
}

/////////////////////////////////////////////////
struct Vector3Hash
{
  std::size_t operator()(const gz::math::Vector3d &_v) const
  {
    std::size_t seed = 0;
    hash_combine(seed, _v.X());
    hash_combine(seed, _v.Y());
    hash_combine(seed, _v.Z());
    return seed;
  }
};

/////////////////////////////////////////////////
struct Vector2dHash
{
  std::size_t operator()(const gz::math::Vector2d &_v) const
  {
    std::size_t seed = 0;
    hash_combine(seed, _v.X());
    hash_combine(seed, _v.Y());
    return seed;
  }
};

//////////////////////////////////////////////////
ColladaLoader::ColladaLoader()
: MeshLoader(), dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->meter = 1.0;
}

//////////////////////////////////////////////////
ColladaLoader::~ColladaLoader() = default;

//////////////////////////////////////////////////
Mesh *ColladaLoader::Load(const std::string &_filename)
{
  this->dataPtr->positionIds.clear();
  this->dataPtr->normalIds.clear();
  this->dataPtr->texcoordIds.clear();
  this->dataPtr->materialIds.clear();
  this->dataPtr->positionDuplicateMap.clear();
  this->dataPtr->normalDuplicateMap.clear();
  this->dataPtr->texcoordDuplicateMap.clear();

  // reset scale
  this->dataPtr->meter = 1.0;

  tinyxml2::XMLDocument xmlDoc;

  this->dataPtr->path.clear();
  std::string separator("/");
#ifdef WIN32
  separator = std::string("\\");
#endif
  if (_filename.rfind(separator) != std::string::npos)
  {
    this->dataPtr->path = _filename.substr(0, _filename.rfind(separator));
  }

  this->dataPtr->filename = _filename;
  if (xmlDoc.LoadFile(_filename.c_str()) != tinyxml2::XML_SUCCESS)
    gzerr << "Unable to load collada file[" << _filename << "]\n";

  this->dataPtr->colladaXml = xmlDoc.FirstChildElement("COLLADA");
  if (!this->dataPtr->colladaXml)
    gzerr << "Missing COLLADA tag\n";

  if (std::string(this->dataPtr->colladaXml->Attribute("version")) != "1.4.0" &&
      std::string(this->dataPtr->colladaXml->Attribute("version")) != "1.4.1")
    gzerr << "Invalid collada file. Must be version 1.4.0 or 1.4.1\n";

  tinyxml2::XMLElement *assetXml =
      this->dataPtr->colladaXml->FirstChildElement("asset");
  if (assetXml)
  {
    tinyxml2::XMLElement *unitXml = assetXml->FirstChildElement("unit");
    if (unitXml && unitXml->Attribute("meter"))
      this->dataPtr->meter = gz::math::parseFloat(
          unitXml->Attribute("meter"));
  }

  Mesh *mesh = new Mesh();
  mesh->SetPath(this->dataPtr->path);

  this->dataPtr->LoadScene(mesh);

  if (mesh->HasSkeleton())
    this->dataPtr->ApplyInvBindTransform(mesh->MeshSkeleton());

  // This will make the model the correct size.
  mesh->Scale(gz::math::Vector3d(
      this->dataPtr->meter, this->dataPtr->meter, this->dataPtr->meter));
  if (mesh->HasSkeleton())
    mesh->MeshSkeleton()->Scale(this->dataPtr->meter);

  return mesh;
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::LoadScene(Mesh *_mesh)
{
  auto *sceneXml = this->colladaXml->FirstChildElement("scene");
  std::string sceneURL =
      sceneXml->FirstChildElement("instance_visual_scene")->Attribute("url");

  tinyxml2::XMLElement *visSceneXml = this->ElementId("visual_scene", sceneURL);
  this->currentScene = visSceneXml;

  if (!visSceneXml)
  {
    gzerr << "Unable to find visual_scene id ='" << sceneURL << "'\n";
    return;
  }

  tinyxml2::XMLElement *nodeXml = visSceneXml->FirstChildElement("node");
  while (nodeXml)
  {
    this->LoadNode(nodeXml, _mesh, gz::math::Matrix4d::Identity);
    nodeXml = nodeXml->NextSiblingElement("node");
  }
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::LoadNode(
    tinyxml2::XMLElement *_elem, Mesh *_mesh,
    const gz::math::Matrix4d &_transform)
{
  tinyxml2::XMLElement *nodeXml;
  tinyxml2::XMLElement *instGeomXml;

  gz::math::Matrix4d transform = this->LoadNodeTransform(_elem);
  transform = _transform * transform;

  nodeXml = _elem->FirstChildElement("node");
  while (nodeXml)
  {
    this->LoadNode(nodeXml, _mesh, transform);
    nodeXml = nodeXml->NextSiblingElement("node");
  }

  if (_elem->Attribute("name"))
  {
    this->currentNodeName = _elem->Attribute("name");
  }
  else
  {
    // if node does not have a name, then append the mesh in this node
    // to the first ancestor node that has a name, i.e. this mesh becomes
    // part of the parent / ancestor mesh
    tinyxml2::XMLElement *parent =
        dynamic_cast<tinyxml2::XMLElement *>(_elem->Parent());
    std::string nodeName;
    while (parent && std::string(parent->Value()) == "node")
    {
      const char *name = parent->Attribute("name");
      if (name)
      {
        nodeName = name;
        break;
      }
    }
    if (nodeName.empty())
    {
      // if none of the ancestor node has a name, then create a custom name
      static int nodeCounter = 0;
      nodeName = "unnamed_submesh_" + std::to_string(nodeCounter++);
    }
    this->currentNodeName = nodeName;
  }

  if (_elem->FirstChildElement("instance_node"))
  {
    std::string nodeURLStr =
      _elem->FirstChildElement("instance_node")->Attribute("url");

    nodeXml = this->ElementId("node", nodeURLStr);
    if (!nodeXml)
    {
      gzerr << "Unable to find node[" << nodeURLStr << "]\n";
      return;
    }
    this->LoadNode(nodeXml, _mesh, transform);
    return;
  }
  else
    nodeXml = _elem;

  instGeomXml = nodeXml->FirstChildElement("instance_geometry");
  while (instGeomXml)
  {
    std::string geomURL = instGeomXml->Attribute("url");
    tinyxml2::XMLElement *geomXml = this->ElementId("geometry", geomURL);

    this->materialMap.clear();
    tinyxml2::XMLElement *bindMatXml, *techniqueXml, *matXml;
    bindMatXml = instGeomXml->FirstChildElement("bind_material");
    while (bindMatXml)
    {
      if ((techniqueXml = bindMatXml->FirstChildElement("technique_common")))
      {
        matXml = techniqueXml->FirstChildElement("instance_material");
        while (matXml)
        {
          std::string symbol = matXml->Attribute("symbol");
          std::string target = matXml->Attribute("target");
          this->materialMap[symbol] = target;
          matXml = matXml->NextSiblingElement("instance_material");
        }
      }
      bindMatXml = bindMatXml->NextSiblingElement("bind_material");
    }

    if (_mesh->HasSkeleton())
      _mesh->MeshSkeleton()->SetNumVertAttached(0);
    this->LoadGeometry(geomXml, transform, _mesh);
    instGeomXml = instGeomXml->NextSiblingElement("instance_geometry");
  }

  tinyxml2::XMLElement *instContrXml =
    nodeXml->FirstChildElement("instance_controller");
  while (instContrXml)
  {
    std::string contrURL = instContrXml->Attribute("url");
    tinyxml2::XMLElement *contrXml = this->ElementId("controller", contrURL);

    this->materialMap.clear();
    tinyxml2::XMLElement *bindMatXml, *techniqueXml, *matXml;
    bindMatXml = instContrXml->FirstChildElement("bind_material");
    while (bindMatXml)
    {
      if ((techniqueXml = bindMatXml->FirstChildElement("technique_common")))
      {
        matXml = techniqueXml->FirstChildElement("instance_material");
        while (matXml)
        {
          std::string symbol = matXml->Attribute("symbol");
          std::string target = matXml->Attribute("target");
          this->materialMap[symbol] = target;
          matXml = matXml->NextSiblingElement("instance_material");
        }
      }
      bindMatXml = bindMatXml->NextSiblingElement("bind_material");
    }

    std::vector<tinyxml2::XMLElement*> rootNodeXmls;
    for (tinyxml2::XMLElement *skelXml =
        instContrXml->FirstChildElement("skeleton"); skelXml;
        skelXml = skelXml->NextSiblingElement("skeleton"))
    {
      std::string rootURL = skelXml->GetText();
      rootNodeXmls.emplace_back(this->ElementId(currentScene,
          "node", rootURL));
    }
    // no skeleton tag present, assume whole scene is a skeleton
    if (rootNodeXmls.empty())
    {
      for (tinyxml2::XMLElement *child =
          this->currentScene->FirstChildElement();
          child; child = child->NextSiblingElement())
      {
        rootNodeXmls.emplace_back(child);
      }
    }

    this->LoadController(contrXml, rootNodeXmls, transform, _mesh);
    instContrXml = instContrXml->NextSiblingElement("instance_controller");
  }
}

/////////////////////////////////////////////////
gz::math::Matrix4d ColladaLoader::Implementation::LoadNodeTransform(
    tinyxml2::XMLElement *_elem)
{
  gz::math::Matrix4d transform(gz::math::Matrix4d::Identity);

  if (_elem->FirstChildElement("matrix"))
  {
    std::string matrixStr = _elem->FirstChildElement("matrix")->GetText();
    std::istringstream iss(matrixStr);
    std::vector<double> values(16);
    for (unsigned int i = 0; i < 16; ++i)
      iss >> values[i];
    transform.Set(values[0], values[1], values[2], values[3],
        values[4], values[5], values[6], values[7],
        values[8], values[9], values[10], values[11],
        values[12], values[13], values[14], values[15]);
  }
  else
  {
    if (_elem->FirstChildElement("translate"))
    {
      std::string transStr = _elem->FirstChildElement("translate")->GetText();
      gz::math::Vector3d translate;
      std::istringstream stream(transStr);
      stream >> translate;
      // translate *= this->meter;
      transform.SetTranslation(translate);
    }

    tinyxml2::XMLElement *rotateXml = _elem->FirstChildElement("rotate");
    while (rotateXml)
    {
      gz::math::Matrix3d mat;
      gz::math::Vector3d axis;
      double angle;

      std::string rotateStr = rotateXml->GetText();
      std::istringstream iss(rotateStr);

      iss >> axis.X() >> axis.Y() >> axis.Z();
      iss >> angle;
      mat.SetFromAxisAngle(axis, GZ_DTOR(angle));
      gz::math::Matrix4d mat4(gz::math::Matrix4d::Identity);
      mat4 = mat;

      transform = transform * mat4;

      rotateXml = rotateXml->NextSiblingElement("rotate");
    }

    if (_elem->FirstChildElement("scale"))
    {
      std::string scaleStr = _elem->FirstChildElement("scale")->GetText();
      gz::math::Vector3d scale;
      std::istringstream stream(scaleStr);
      stream >> scale;
      gz::math::Matrix4d scaleMat;
      scaleMat.Scale(scale);
      transform = transform * scaleMat;
    }
  }

  return transform;
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::LoadController(
    tinyxml2::XMLElement *_contrXml,
    std::vector<tinyxml2::XMLElement *> _skelXmls,
    const gz::math::Matrix4d &_transform,
    Mesh *_mesh)
{
  if (nullptr == _contrXml)
  {
    gzerr << "Can't load null controller element." << std::endl;
    return;
  }

  tinyxml2::XMLElement *skinXml = _contrXml->FirstChildElement("skin");
  if (nullptr == skinXml)
  {
    gzerr << "Failed to find skin element" << std::endl;
    return;
  }

  std::string geomURL = skinXml->Attribute("source");

  auto shapeMat = skinXml->FirstChildElement("bind_shape_matrix");
  if (nullptr == shapeMat || nullptr == shapeMat->GetText())
  {
    gzerr << "Missing <bind_shape_matrix>" << std::endl;
    return;
  }
  std::string matrixStr = shapeMat->GetText();
  std::istringstream iss(matrixStr);
  std::vector<double> values(16);
  for (unsigned int i = 0; i < 16; ++i)
    iss >> values[i];

  gz::math::Matrix4d bindTrans;
  bindTrans.Set(values[0], values[1], values[2], values[3],
                values[4], values[5], values[6], values[7],
                values[8], values[9], values[10], values[11],
                values[12], values[13], values[14], values[15]);

  tinyxml2::XMLElement *jointsXml = skinXml->FirstChildElement("joints");
  if (nullptr == jointsXml)
  {
    gzerr << "Failed to find <joints> element" << std::endl;
    return;
  }

  std::string jointsURL, invBindMatURL;
  tinyxml2::XMLElement *inputXml = jointsXml->FirstChildElement("input");
  while (inputXml)
  {
    std::string semantic = inputXml->Attribute("semantic");
    std::string source = inputXml->Attribute("source");
    if (semantic == "JOINT")
      jointsURL = source;
    else if (semantic == "INV_BIND_MATRIX")
      invBindMatURL = source;

    inputXml = inputXml->NextSiblingElement("input");
  }

  if (jointsURL.empty())
  {
    gzwarn << "Missing semantic='JOINT' input source" << std::endl;
  }

  if (invBindMatURL.empty())
  {
    gzwarn << "Missing semantic='INV__BIND_MATRIX' input source" << std::endl;
  }

  jointsXml = this->ElementId("source", jointsURL);

  if (!jointsXml)
  {
    gzerr << "Could not find node [" << jointsURL << "]. "
        << "Failed to parse skinning information in Collada file." << std::endl;
    return;
  }

  auto nameArray = jointsXml->FirstChildElement("Name_array");
  if (nullptr == nameArray)
  {
    gzerr << "Missing <Name_array>" << std::endl;
    return;
  }

  std::string jointsStr = nameArray->GetText();

  std::vector<std::string> joints = split(jointsStr, " \t\r\n");

  // Load the skeleton
  SkeletonPtr skeleton = nullptr;
  if (_mesh->HasSkeleton())
    skeleton = _mesh->MeshSkeleton();

  for (tinyxml2::XMLElement *rootNodeXml : _skelXmls)
  {
    SkeletonNode *rootSkelNode =
        this->LoadSkeletonNodes(rootNodeXml, nullptr);

    if (nullptr == skeleton)
    {
      skeleton = std::make_shared<Skeleton>(rootSkelNode);
      _mesh->SetSkeleton(skeleton);
    }
    else if (nullptr != rootSkelNode)
    {
      this->MergeSkeleton(skeleton, rootSkelNode);
    }
  }
  if (nullptr == skeleton)
  {
    gzerr << "Failed to create skeleton." << std::endl;
    return;
  }
  skeleton->SetBindShapeTransform(bindTrans);

  tinyxml2::XMLElement *rootXml = _contrXml->GetDocument()->RootElement();
  if (rootXml && rootXml->FirstChildElement("library_animations"))
  {
    this->LoadAnimations(rootXml->FirstChildElement("library_animations"),
        skeleton);
  }

  tinyxml2::XMLElement *invBMXml = this->ElementId("source", invBindMatURL);

  if (nullptr == invBMXml)
  {
    gzerr << "Could not find node[" << invBindMatURL << "]. "
        << "Faild to parse skinning information in Collada file." << std::endl;
    return;
  }

  std::string posesStr = invBMXml->FirstChildElement("float_array")->GetText();

  std::vector<std::string> strs = split(posesStr, " \t\r\n");

  for (unsigned int i = 0; i < joints.size(); ++i)
  {
    auto node = skeleton->NodeByName(joints[i]);
    if (nullptr == node)
    {
      gzerr << "Node [" << joints[i] << "] is null." << std::endl;
      continue;
    }

    unsigned int id = i * 16;
    gz::math::Matrix4d mat;
    mat.Set(gz::math::parseFloat(strs[id +  0]),
            gz::math::parseFloat(strs[id +  1]),
            gz::math::parseFloat(strs[id +  2]),
            gz::math::parseFloat(strs[id +  3]),
            gz::math::parseFloat(strs[id +  4]),
            gz::math::parseFloat(strs[id +  5]),
            gz::math::parseFloat(strs[id +  6]),
            gz::math::parseFloat(strs[id +  7]),
            gz::math::parseFloat(strs[id +  8]),
            gz::math::parseFloat(strs[id +  9]),
            gz::math::parseFloat(strs[id + 10]),
            gz::math::parseFloat(strs[id + 11]),
            gz::math::parseFloat(strs[id + 12]),
            gz::math::parseFloat(strs[id + 13]),
            gz::math::parseFloat(strs[id + 14]),
            gz::math::parseFloat(strs[id + 15]));

    node->SetInverseBindTransform(mat);
  }

  tinyxml2::XMLElement *vertWeightsXml =
      skinXml->FirstChildElement("vertex_weights");
  if (nullptr == vertWeightsXml)
  {
    gzerr << "Failed to find vertex_weights" << std::endl;
    return;
  }

  inputXml = vertWeightsXml->FirstChildElement("input");
  unsigned int jOffset = 0;
  unsigned int wOffset = 0;
  std::string weightsURL;
  while (inputXml)
  {
    std::string semantic = inputXml->Attribute("semantic");
    std::string source = inputXml->Attribute("source");
    int offset = std::stoi(inputXml->Attribute("offset"));

    if (semantic == "JOINT")
    {
      jOffset = offset;
    }
    else
    {
      if (semantic == "WEIGHT")
      {
        weightsURL = source;
        wOffset = offset;
      }
    }
    inputXml = inputXml->NextSiblingElement("input");
  }

  tinyxml2::XMLElement *weightsXml = this->ElementId("source", weightsURL);

  std::string wString = weightsXml->FirstChildElement("float_array")->GetText();
  std::vector<std::string> wStrs = split(wString, " \t\r\n");

  std::vector<float> weights;
  for (unsigned int i = 0; i < wStrs.size(); ++i)
    weights.push_back(math::parseFloat(wStrs[i]));

  std::string cString = vertWeightsXml->FirstChildElement("vcount")->GetText();
  std::string vString = vertWeightsXml->FirstChildElement("v")->GetText();
  std::vector<std::string> vCountStrs = split(cString, " \t\r\n");
  std::vector<std::string> vStrs = split(vString, " \t\r\n");

  std::vector<unsigned int> vCount;
  std::vector<unsigned int> v;

  for (unsigned int i = 0; i < vCountStrs.size(); ++i)
    vCount.push_back(math::parseInt(vCountStrs[i]));

  for (unsigned int i = 0; i < vStrs.size(); ++i)
    v.push_back(math::parseInt(vStrs[i]));

  skeleton->SetNumVertAttached(vCount.size());

  unsigned int vIndex = 0;
  for (unsigned int i = 0; i < vCount.size(); ++i)
  {
    for (unsigned int j = 0; j < vCount[i]; ++j)
    {
      skeleton->AddVertNodeWeight(i, joints[v[vIndex + jOffset]],
                                    weights[v[vIndex + wOffset]]);
      vIndex += (jOffset + wOffset + 1);
    }
  }

  tinyxml2::XMLElement *geomXml = this->ElementId("geometry", geomURL);
  this->LoadGeometry(geomXml, _transform, _mesh);
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::LoadAnimations(tinyxml2::XMLElement *_xml,
    SkeletonPtr _skel)
{
  tinyxml2::XMLElement *childXml = _xml->FirstChildElement("animation");
  if (childXml->FirstChildElement("animation"))
  {
    while (childXml)
    {
      this->LoadAnimationSet(childXml, _skel);
      childXml = childXml->NextSiblingElement("animation");
    }
  }
  else
    this->LoadAnimationSet(_xml, _skel);
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::LoadAnimationSet(tinyxml2::XMLElement *_xml,
    SkeletonPtr _skel)
{
  std::stringstream animName;
  if (_xml->Attribute("name"))
  {
    animName << _xml->Attribute("name");
  }
  else
  {
    if (_xml->Attribute("id"))
      animName << _xml->Attribute("id");
    else
      animName << "animation" << (_skel->AnimationCount() + 1);
  }

  RawSkeletonAnim animation;

  tinyxml2::XMLElement *animXml = _xml->FirstChildElement("animation");
  while (animXml)
  {
    tinyxml2::XMLElement *chanXml = animXml->FirstChildElement("channel");

    while (chanXml)
    {
      std::string sourceURL = chanXml->Attribute("source");
      std::string targetStr = chanXml->Attribute("target");

      std::string targetBone = targetStr.substr(0, targetStr.find('/'));
      char sep = '0';
      if (targetStr.find('(') != std::string::npos)
        sep = '(';
      else
        if (targetStr.find('.') != std::string::npos)
          sep = '.';

      std::string targetTrans;
      if (sep == '0')
        targetTrans = targetStr.substr(targetStr.find('/') + 1);
      else
        targetTrans = targetStr.substr(targetStr.find('/') + 1,
                          targetStr.find(sep) - targetStr.find('/') - 1);

      std::string idxStr = targetStr.substr(targetStr.find(sep) + 1);
      int idx1 = -1;
      int idx2 = -1;

      if (sep == '.')
      {
        idx1 = (idxStr == "X") ? 0 : ((idxStr == "Y") ? 1 : ((idxStr == "Z")
            ? 2 : ((idxStr == "ANGLE") ? 3 : -1)));
      }
      else
      {
        if (sep == '(')
        {
          std::string idx1Str = idxStr.substr(0, 1);
          idx1 = gz::math::parseInt(idx1Str);
          if (idxStr.length() > 4)
          {
            std::string idx2Str = idxStr.substr(3, 1);
            idx2 = gz::math::parseInt(idx2Str);
          }
        }
      }

      tinyxml2::XMLElement *frameTimesXml = NULL;
      tinyxml2::XMLElement *frameTransXml = NULL;

      tinyxml2::XMLElement *sampXml = this->ElementId("sampler", sourceURL);
      tinyxml2::XMLElement *inputXml = sampXml->FirstChildElement("input");
      while (inputXml)
      {
        std::string semantic = inputXml->Attribute("semantic");
        if (semantic == "INPUT")
          frameTimesXml = this->ElementId("source",
                              inputXml->Attribute("source"));
        else
          if (semantic == "OUTPUT")
            frameTransXml = this->ElementId("source",
                              inputXml->Attribute("source"));
        /// FIXME interpolation semantic?

        inputXml = inputXml->NextSiblingElement("input");
      }
      tinyxml2::XMLElement *timeArray =
          frameTimesXml->FirstChildElement("float_array");
      std::string timeStr = timeArray->GetText();
      std::vector<std::string> timeStrs = split(timeStr, " \t\r\n");

      std::vector<double> times;
      for (unsigned int i = 0; i < timeStrs.size(); ++i)
        times.push_back(math::parseFloat(timeStrs[i]));

      tinyxml2::XMLElement *output =
          frameTransXml->FirstChildElement("float_array");
      std::string outputStr = output->GetText();
      std::vector<std::string> outputStrs = split(outputStr, " \t\r\n");

      std::vector<double> values;
      for (unsigned int i = 0; i < outputStrs.size(); ++i)
        values.push_back(math::parseFloat(outputStrs[i]));

      tinyxml2::XMLElement *accessor =
        frameTransXml->FirstChildElement("technique_common");
      accessor = accessor->FirstChildElement("accessor");

      // stride is optional, default to 1
      unsigned int stride = 1;
      auto *strideAttribute = accessor->Attribute("stride");
      if (strideAttribute)
      {
        stride = static_cast<unsigned int>(
            gz::math::parseInt(strideAttribute));
      }

      SkeletonNode *targetNode = _skel->NodeById(targetBone);
      if (targetNode == nullptr)
      {
        tinyxml2::XMLElement *targetNodeXml =
            this->ElementId("node", targetBone);
        if (targetNodeXml == nullptr)
        {
          gzerr << "Failed to load animation, [" << targetBone << "] not found"
              << std::endl;
          continue;
        }
        targetNode = this->LoadSkeletonNodes(targetNodeXml, nullptr);
        this->MergeSkeleton(_skel, targetNode);
      }
      if (targetNode == nullptr)
      {
        gzerr << "Failed to load bone [" << targetBone << "]." << std::endl;
        continue;
      }

      // In COLLOADA, `target` is specified to be the `id` of a node, however
      // the nodes are identified by `name` in this loader. Here, we resolve
      // `targetBone` to the node's `name` to prevent missing animations.
      std::string targetBoneName = targetNode->Name();
      for (unsigned int i = 0; i < times.size(); ++i)
      {
        if (animation[targetBoneName].find(times[i]) ==
            animation[targetBoneName].end())
        {
          auto bone = _skel->NodeById(targetBone);
          if (nullptr != bone)
          {
            animation[targetBoneName][times[i]] = bone->Transforms();
          }
          else
          {
            gzerr << "Failed to find node with ID [" << targetBone << "]"
                   << std::endl;
          }
        }

        std::vector<NodeTransform> *frame =
            &animation[targetBoneName][times[i]];

        for (unsigned int j = 0; j < (*frame).size(); ++j)
        {
          NodeTransform *nt = &((*frame)[j]);
          if (nt->SID() == targetTrans)
          {
            if (idx1 != -1)
            {
              int index = (idx2 == -1) ? idx1 : (idx1 * 4) + idx2;
              nt->SetComponent(index, values[i]);
            }
            else
            {
              for (unsigned int k = 0; k < stride; k++)
                nt->SetComponent(k, values[(i*stride) + k]);
            }
          }
        }
      }

      chanXml = chanXml->NextSiblingElement("channel");
    }

    animXml = animXml->NextSiblingElement("animation");
  }

  SkeletonAnimation *anim = new SkeletonAnimation(animName.str());

  for (RawSkeletonAnim::iterator iter = animation.begin();
        iter != animation.end(); ++iter)
    for (RawNodeAnim::iterator niter = iter->second.begin();
          niter != iter->second.end(); ++niter)
    {
      gz::math::Matrix4d transform(gz::math::Matrix4d::Identity);
      for (unsigned int i = 0; i < niter->second.size(); ++i)
      {
        niter->second[i].RecalculateMatrix();
        transform = transform * niter->second[i]();
      }
      anim->AddKeyFrame(iter->first, niter->first, transform);
    }

  _skel->AddAnimation(anim);
}

/////////////////////////////////////////////////
SkeletonNode *ColladaLoader::Implementation::LoadSingleSkeletonNode(
    tinyxml2::XMLElement *_xml, SkeletonNode *_parent)
{
  if (nullptr == _xml)
  {
    gzerr << "Can't load single skeleton node from null XML." << std::endl;
    return nullptr;
  }

  std::string name;
  if (_xml->Attribute("sid"))
    name = _xml->Attribute("sid");
  else if (_xml->Attribute("name"))
    name = _xml->Attribute("name");
  else if (_xml->Attribute("id"))
    name = _xml->Attribute("id");
  else
  {
    gzerr << "Failed to create skeleton node without a name." << std::endl;
    return nullptr;
  }

  SkeletonNode* node = new SkeletonNode(_parent, name, _xml->Attribute("id"));

  if (!_xml->Attribute("type")
      || std::string(_xml->Attribute("type")) == "NODE")
  {
    node->SetType(SkeletonNode::NODE);
  }

  return node;
}

/////////////////////////////////////////////////
SkeletonNode *ColladaLoader::Implementation::LoadSkeletonNodes(
    tinyxml2::XMLElement *_xml, SkeletonNode *_parent)
{
  if (nullptr == _xml)
  {
    gzerr << "Can't load skeleton nodes from null XML element." << std::endl;
    return nullptr;
  }

  // Skip extras
  if (std::string(_xml->Value()) == "extra")
  {
    gzwarn << "Skipping [extra] element." << std::endl;
    return nullptr;
  }

  if (std::string(_xml->Value()) != "node")
  {
    gzwarn << "Failed to load element [" << _xml->Value()
            << "] as skeleton node." << std::endl;
    return nullptr;
  }

  auto *node = this->LoadSingleSkeletonNode(_xml, _parent);
  this->SetSkeletonNodeTransform(_xml, node);

  auto childXml = _xml->FirstChildElement("node");
  while (childXml)
  {
    this->LoadSkeletonNodes(childXml, node);
    childXml = childXml->NextSiblingElement("node");
  }
  return node;
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::SetSkeletonNodeTransform(
    tinyxml2::XMLElement *_elem, SkeletonNode *_node)
{
  if (nullptr == _elem)
  {
    gzerr << "Can't set transform from null XML." << std::endl;
    return;
  }

  if (nullptr == _node)
  {
    gzerr << "Can't set transform to null skeleton node." << std::endl;
    return;
  }

  gz::math::Matrix4d transform(gz::math::Matrix4d::Identity);

  if (_elem->FirstChildElement("matrix"))
  {
    std::string matrixStr = _elem->FirstChildElement("matrix")->GetText();
    std::istringstream iss(matrixStr);
    std::vector<double> values(16);
    for (unsigned int i = 0; i < 16; ++i)
      iss >> values[i];
    transform.Set(values[0], values[1], values[2], values[3],
        values[4], values[5], values[6], values[7],
        values[8], values[9], values[10], values[11],
        values[12], values[13], values[14], values[15]);

    NodeTransform nt(transform);
    nt.SetSourceValues(transform);
    if (_elem->FirstChildElement("matrix")->Attribute("sid"))
      nt.SetSID(_elem->FirstChildElement("matrix")->Attribute("sid"));
    _node->AddRawTransform(nt);
  }
  else
  {
    if (_elem->FirstChildElement("translate"))
    {
      std::string transStr = _elem->FirstChildElement("translate")->GetText();
      gz::math::Vector3d translate;
      std::istringstream stream(transStr);
      stream >> translate;
      // translate *= this->meter;
      transform.SetTranslation(translate);

      NodeTransform nt(transform);
      tinyxml2::XMLElement *matrix = _elem->FirstChildElement("matrix");
      if (matrix && matrix->Attribute("sid"))
        nt.SetSID(_elem->FirstChildElement("translate")->Attribute("sid"));
      nt.SetType(NodeTransformType::TRANSLATE);
      nt.SetSourceValues(translate);
      _node->AddRawTransform(nt);
    }

    tinyxml2::XMLElement *rotateXml = _elem->FirstChildElement("rotate");
    while (rotateXml)
    {
      gz::math::Matrix3d mat;
      gz::math::Vector3d axis;
      double angle;

      std::string rotateStr = rotateXml->GetText();
      std::istringstream iss(rotateStr);

      iss >> axis.X() >> axis.Y() >> axis.Z();
      iss >> angle;
      mat.SetFromAxisAngle(axis, GZ_DTOR(angle));

      gz::math::Matrix4d mat4(gz::math::Matrix4d::Identity);
      mat4 = mat;
      NodeTransform nt(mat4);
      if (rotateXml->Attribute("sid"))
        nt.SetSID(rotateXml->Attribute("sid"));
      nt.SetType(NodeTransformType::ROTATE);
      nt.SetSourceValues(axis, angle);
      _node->AddRawTransform(nt);

      transform = transform * mat4;

      rotateXml = rotateXml->NextSiblingElement("rotate");
    }

    if (_elem->FirstChildElement("scale"))
    {
      std::string scaleStr = _elem->FirstChildElement("scale")->GetText();
      gz::math::Vector3d scale;
      std::istringstream stream(scaleStr);
      stream >> scale;
      gz::math::Matrix4d scaleMat;
      scaleMat.Scale(scale);

      NodeTransform nt(scaleMat);
      tinyxml2::XMLElement *matrix = _elem->FirstChildElement("matrix");
      if (matrix && matrix->Attribute("sid"))
        nt.SetSID(_elem->FirstChildElement("matrix")->Attribute("sid"));
      nt.SetType(NodeTransformType::SCALE);
      nt.SetSourceValues(scale);
      _node->AddRawTransform(nt);

      transform = transform * scaleMat;
    }
  }

  _node->SetTransform(transform);
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::LoadGeometry(tinyxml2::XMLElement *_xml,
    const gz::math::Matrix4d &_transform, Mesh *_mesh)
{
  tinyxml2::XMLElement *meshXml = _xml->FirstChildElement("mesh");
  tinyxml2::XMLElement *childXml;

  if (!meshXml)
    return;

  childXml = meshXml->FirstChildElement("triangles");
  while (childXml)
  {
    this->LoadTriangles(childXml, _transform, _mesh);
    childXml = childXml->NextSiblingElement("triangles");
  }

  childXml = meshXml->FirstChildElement("polylist");
  while (childXml)
  {
    this->LoadPolylist(childXml, _transform, _mesh);
    childXml = childXml->NextSiblingElement("polylist");
  }

  childXml = meshXml->FirstChildElement("lines");
  while (childXml)
  {
    this->LoadLines(childXml, _transform, _mesh);
    childXml = childXml->NextSiblingElement("lines");
  }
}

/////////////////////////////////////////////////
tinyxml2::XMLElement *ColladaLoader::Implementation::ElementId(
    const std::string &_name, const std::string &_id)
{
  return this->ElementId(this->colladaXml, _name, _id);
}

/////////////////////////////////////////////////
tinyxml2::XMLElement *ColladaLoader::Implementation::ElementId(
    tinyxml2::XMLElement *_parent,
    const std::string &_name, const std::string &_id)
{
  std::string id = _id;
  if (id.length() > 0 && id[0] == '#')
    id.erase(0, 1);

  if ((id.empty() && _parent->Value() == _name) ||
      (_parent->Attribute("id") && _parent->Attribute("id") == id) ||
      (_parent->Attribute("sid") && _parent->Attribute("sid") == id))
  {
    return _parent;
  }

  tinyxml2::XMLElement *elem = _parent->FirstChildElement();
  while (elem)
  {
    tinyxml2::XMLElement *result = this->ElementId(elem, _name, _id);
    if (result)
    {
      return result;
    }

    elem = elem->NextSiblingElement();
  }

  return NULL;
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::LoadVertices(const std::string &_id,
    const gz::math::Matrix4d &_transform,
    std::vector<gz::math::Vector3d> &_verts,
    std::vector<gz::math::Vector3d> &_norms)
{
  std::map<unsigned int, unsigned int> vertDup;
  std::map<unsigned int, unsigned int> normDup;
  this->LoadVertices(_id, _transform, _verts, _norms, vertDup, normDup);
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::LoadVertices(const std::string &_id,
    const gz::math::Matrix4d &_transform,
    std::vector<gz::math::Vector3d> &_verts,
    std::vector<gz::math::Vector3d> &_norms,
    std::map<unsigned int, unsigned int> &_vertDups,
    std::map<unsigned int, unsigned int> &_normDups)
{
  tinyxml2::XMLElement *verticesXml = this->ElementId(this->colladaXml,
      "vertices", _id);

  if (!verticesXml)
  {
    gzerr << "Unable to find vertices[" << _id << "] in collada file\n";
    return;
  }

  tinyxml2::XMLElement *inputXml = verticesXml->FirstChildElement("input");
  while (inputXml)
  {
    std::string semantic = inputXml->Attribute("semantic");
    std::string sourceStr = inputXml->Attribute("source");
    if (semantic == "NORMAL")
    {
      this->LoadNormals(sourceStr, _transform, _norms, _normDups);
    }
    else if (semantic == "POSITION")
    {
      this->LoadPositions(sourceStr, _transform, _verts, _vertDups);
    }

    inputXml = inputXml->NextSiblingElement("input");
  }
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::LoadPositions(const std::string &_id,
    const gz::math::Matrix4d &_transform,
    std::vector<gz::math::Vector3d> &_values,
    std::map<unsigned int, unsigned int> &_duplicates)
{
  if (this->positionIds.find(_id) != this->positionIds.end())
  {
    _values = this->positionIds[_id];
    _duplicates = this->positionDuplicateMap[_id];
    return;
  }

  tinyxml2::XMLElement *sourceXml = this->ElementId("source", _id);
  if (!sourceXml)
  {
    gzerr << "Unable to find source\n";
    return;
  }

  tinyxml2::XMLElement *floatArrayXml =
      sourceXml->FirstChildElement("float_array");
  if (!floatArrayXml || !floatArrayXml->GetText())
  {
    int count = 1;
    if (floatArrayXml && floatArrayXml->Attribute("count"))
    {
      try
      {
        count = std::stoi(floatArrayXml->Attribute("count"));
      }
      catch(...)
      {
        // Do nothing. Messages are printed out below.
      }
    }

    if (count)
    {
      gzerr << "Vertex source missing float_array element, "
        << "or count is invalid.\n";
    }
    else
    {
      gzlog << "Vertex source has a float_array with a count of zero. "
        << "This is likely not desired\n";
    }

    return;
  }
  std::string valueStr = floatArrayXml->GetText();

  std::unordered_map<gz::math::Vector3d,
      unsigned int, Vector3Hash> unique;

  std::vector<std::string>::iterator iter, end;
  std::vector<std::string> strs = split(valueStr, " \t\r\n");
  end = strs.end();
  for (iter = strs.begin(); iter != end; iter += 3)
  {
    gz::math::Vector3d vec(math::parseFloat(*iter),
        gz::math::parseFloat(*(iter+1)),
        gz::math::parseFloat(*(iter+2)));

    vec = _transform * vec;
    _values.push_back(vec);

    // create a map of duplicate indices
    if (unique.find(vec) != unique.end())
      _duplicates[_values.size()-1] = unique[vec];
    else
      unique[vec] = _values.size()-1;
  }

  this->positionDuplicateMap[_id] = _duplicates;
  this->positionIds[_id] = _values;
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::LoadNormals(const std::string &_id,
    const gz::math::Matrix4d &_transform,
    std::vector<gz::math::Vector3d> &_values,
    std::map<unsigned int, unsigned int> &_duplicates)
{
  if (this->normalIds.find(_id) != this->normalIds.end())
  {
    _values = this->normalIds[_id];
    _duplicates = this->normalDuplicateMap[_id];
    return;
  }

  gz::math::Matrix4d rotMat = _transform;
  rotMat.SetTranslation(gz::math::Vector3d::Zero);

  tinyxml2::XMLElement *normalsXml = this->ElementId("source", _id);
  if (!normalsXml)
  {
    gzerr << "Unable to find normals[" << _id << "] in collada file\n";
    return;
  }

  tinyxml2::XMLElement *floatArrayXml =
      normalsXml->FirstChildElement("float_array");
  if (!floatArrayXml || !floatArrayXml->GetText())
  {
    int count = 1;
    if (floatArrayXml && floatArrayXml->Attribute("count"))
    {
      try
      {
        count = std::stoi(floatArrayXml->Attribute("count"));
      }
      catch(...)
      {
        // Do nothing. Messages are printed out below.
      }
    }

    if (count)
    {
      gzwarn << "Normal source missing float_array element, or count is "
        << "invalid.\n";
    }
    else
    {
      gzlog << "Normal source has a float_array with a count of zero. "
        << "This is likely not desired\n";
    }

    return;
  }

  std::unordered_map<gz::math::Vector3d,
      unsigned int, Vector3Hash> unique;

  std::string valueStr = floatArrayXml->GetText();
  std::istringstream iss(valueStr);
  do
  {
    gz::math::Vector3d vec;
    iss >> vec.X() >> vec.Y() >> vec.Z();
    if (iss)
    {
      vec = rotMat * vec;
      vec.Normalize();
      _values.push_back(vec);

      // create a map of duplicate indices
      if (unique.find(vec) != unique.end())
        _duplicates[_values.size()-1] = unique[vec];
      else
        unique[vec] = _values.size()-1;
    }
  } while (iss);

  this->normalDuplicateMap[_id] = _duplicates;
  this->normalIds[_id] = _values;
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::LoadTexCoords(const std::string &_id,
    std::vector<gz::math::Vector2d> &_values,
    std::map<unsigned int, unsigned int> &_duplicates)
{
  if (this->texcoordIds.find(_id) != this->texcoordIds.end())
  {
    _values = this->texcoordIds[_id];
    _duplicates = this->texcoordDuplicateMap[_id];
    return;
  }

  int stride = 0;
  int texCount = 0;
  int totCount = 0;

  // Get the source element for the texture coordinates.
  tinyxml2::XMLElement *xml = this->ElementId("source", _id);
  if (!xml)
  {
    gzerr << "Unable to find tex coords[" << _id << "] in collada file\n";
    return;
  }

  // Get the array of float values. These are the raw values for the texture
  // coordinates.
  tinyxml2::XMLElement *floatArrayXml = xml->FirstChildElement("float_array");
  if (!floatArrayXml || !floatArrayXml->GetText())
  {
    int count = 1;
    if (floatArrayXml && floatArrayXml->Attribute("count"))
    {
      try
      {
        count = std::stoi(floatArrayXml->Attribute("count"));
      }
      catch(...)
      {
        // Do nothing. Messages are printed out below.
      }
    }

    if (count)
    {
      gzerr << "Normal source missing float_array element, or count is "
        << "invalid.\n";
    }
    else
    {
      gzlog << "Normal source has a float_array with a count of zero. "
        << "This is likely not desired\n";
    }

    return;
  }
  // Read in the total number of texture coordinate values
  else if (floatArrayXml->Attribute("count"))
    totCount = std::stoi(floatArrayXml->Attribute("count"));
  else
  {
    gzerr << "<float_array> has no count attribute in texture coordinate "
          << "element with id[" << _id << "]\n";
    return;
  }

  // The technique_common holds an <accessor> element that indicates how to
  // parse the float array.
  xml = xml->FirstChildElement("technique_common");
  if (!xml)
  {
    gzerr << "Unable to find technique_common element for texture "
          << "coordinates with id[" << _id << "]\n";
    return;
  }

  // Get the accessor XML element.
  xml = xml->FirstChildElement("accessor");
  if (!xml)
  {
    gzerr << "Unable to find <accessor> as a child of <technique_common> "
          << "for texture coordinates with id[" << _id << "]\n";
    return;
  }

  // Read in the stride for the texture coordinate values. The stride
  // indicates the number of values in the float array the comprise
  // a complete texture coordinate.
  if (xml->Attribute("stride"))
  {
    stride = std::stoi(xml->Attribute("stride"));
  }
  else
  {
    gzerr << "<accessor> has no stride attribute in texture coordinate "
          << "element with id[" << _id << "]\n";
    return;
  }

  // Read in the count of texture coordinates.
  if (xml->Attribute("count"))
    texCount = std::stoi(xml->Attribute("count"));
  else
  {
    gzerr << "<accessor> has no count attribute in texture coordinate element "
          << "with id[" << _id << "]\n";
    return;
  }

  // \TODO This is a good a GZ_ASSERT
  // The total number of texture values should equal the stride multiplied
  // by the number of texture coordinates.
  if (texCount * stride != totCount)
  {
    gzerr << "Error reading texture coordinates. Coordinate counts in element "
             "with id[" << _id << "] do not add up correctly\n";
    return;
  }

  // Nothing to read. Don't print a warning because the collada file is
  // correct.
  if (totCount == 0)
    return;

  std::unordered_map<gz::math::Vector2d,
      unsigned int, Vector2dHash> unique;

  // Read the raw texture values, and split them on spaces.
  std::string valueStr = floatArrayXml->GetText();
  std::vector<std::string> values = split(valueStr, " \t\r\n");

  // Read in all the texture coordinates.
  for (int i = 0; i < totCount; i += stride)
  {
    // We only handle 2D texture coordinates right now.
    gz::math::Vector2d vec(std::stod(values[i]),
          1.0 - std::stod(values[i+1]));
    _values.push_back(vec);

    // create a map of duplicate indices
    if (unique.find(vec) != unique.end())
    {
      _duplicates[_values.size()-1] = unique[vec];
    }
    else
      unique[vec] = _values.size()-1;
  }

  this->texcoordDuplicateMap[_id] = _duplicates;
  this->texcoordIds[_id] = _values;
}

/////////////////////////////////////////////////
MaterialPtr ColladaLoader::Implementation::LoadMaterial(
    const std::string &_name)
{
  if (this->materialIds.find(_name)
      != this->materialIds.end())
  {
    return this->materialIds[_name];
  }

  tinyxml2::XMLElement *matXml = this->ElementId("material", _name);
  if (!matXml || !matXml->FirstChildElement("instance_effect"))
    return NULL;

  MaterialPtr mat(new Material());
  std::string effectName =
    matXml->FirstChildElement("instance_effect")->Attribute("url");
  tinyxml2::XMLElement *effectXml = this->ElementId("effect", effectName);

  tinyxml2::XMLElement *commonXml =
     effectXml->FirstChildElement("profile_COMMON");
  if (commonXml)
  {
    tinyxml2::XMLElement *techniqueXml =
       commonXml->FirstChildElement("technique");
    tinyxml2::XMLElement *lambertXml =
       techniqueXml->FirstChildElement("lambert");

    tinyxml2::XMLElement *phongXml = techniqueXml->FirstChildElement("phong");
    tinyxml2::XMLElement *blinnXml = techniqueXml->FirstChildElement("blinn");
    if (lambertXml)
    {
      this->LoadColorOrTexture(lambertXml, "ambient", mat);
      this->LoadColorOrTexture(lambertXml, "emission", mat);
      this->LoadColorOrTexture(lambertXml, "diffuse", mat);
      // order matters: transparency needs to be loaded before transparent
      if (lambertXml->FirstChildElement("transparency"))
      {
        mat->SetTransparency(
            this->LoadFloat(lambertXml->FirstChildElement("transparency")));
      }

      if (lambertXml->FirstChildElement("transparent"))
      {
        tinyxml2::XMLElement *transXml =
            lambertXml->FirstChildElement("transparent");
        this->LoadTransparent(transXml, mat);
      }
      else
      {
        // no <transparent> tag, revert to zero transparency
        mat->SetTransparency(0.0);
      }
    }
    else if (phongXml)
    {
      this->LoadColorOrTexture(phongXml, "ambient", mat);
      this->LoadColorOrTexture(phongXml, "emission", mat);
      this->LoadColorOrTexture(phongXml, "specular", mat);
      this->LoadColorOrTexture(phongXml, "diffuse", mat);
      if (phongXml->FirstChildElement("shininess"))
        mat->SetShininess(
            this->LoadFloat(phongXml->FirstChildElement("shininess")));

      // order matters: transparency needs to be loaded before transparent
      if (phongXml->FirstChildElement("transparency"))
        mat->SetTransparency(
            this->LoadFloat(phongXml->FirstChildElement("transparency")));
      if (phongXml->FirstChildElement("transparent"))
      {
        tinyxml2::XMLElement *transXml =
            phongXml->FirstChildElement("transparent");
        this->LoadTransparent(transXml, mat);
      }
      else
      {
        // no <transparent> tag, revert to zero transparency
        mat->SetTransparency(0.0);
      }
    }
    else if (blinnXml)
    {
      this->LoadColorOrTexture(blinnXml, "ambient", mat);
      this->LoadColorOrTexture(blinnXml, "emission", mat);
      this->LoadColorOrTexture(blinnXml, "specular", mat);
      this->LoadColorOrTexture(blinnXml, "diffuse", mat);
      if (blinnXml->FirstChildElement("shininess"))
        mat->SetShininess(
            this->LoadFloat(blinnXml->FirstChildElement("shininess")));

      if (blinnXml->FirstChildElement("transparency"))
        mat->SetTransparency(
            this->LoadFloat(blinnXml->FirstChildElement("transparency")));

      // order matters: transparency needs to be loaded before transparent
      if (blinnXml->FirstChildElement("transparent"))
      {
        tinyxml2::XMLElement *transXml =
            blinnXml->FirstChildElement("transparent");
        this->LoadTransparent(transXml, mat);
      }
      else
      {
        // no <transparent> tag, revert to zero transparency
        mat->SetTransparency(0.0);
      }
    }
  }

  tinyxml2::XMLElement *glslXml = effectXml->FirstChildElement("profile_GLSL");
  if (glslXml)
    gzerr << "profile_GLSL unsupported\n";

  tinyxml2::XMLElement *cgXml = effectXml->FirstChildElement("profile_CG");
  if (cgXml)
    gzerr << "profile_CG unsupported\n";

  this->materialIds[_name] = mat;

  return mat;
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::LoadColorOrTexture(
    tinyxml2::XMLElement *_elem, const std::string &_type, MaterialPtr _mat)
{
  if (!_elem || !_elem->FirstChildElement(_type.c_str()))
    return;

  tinyxml2::XMLElement *typeElem = _elem->FirstChildElement(_type.c_str());

  if (typeElem->FirstChildElement("color"))
  {
    std::istringstream stream(typeElem->FirstChildElement("color")->GetText());
    math::Color color;
    stream >> color;
    if (_type == "diffuse")
      _mat->SetDiffuse(color);
    else if (_type == "ambient")
      _mat->SetAmbient(color);
    else if (_type == "emission")
      _mat->SetEmissive(color);
    else if (_type == "specular")
      _mat->SetSpecular(color);
  }
  else if (typeElem->FirstChildElement("texture"))
  {
    if (_type == "ambient")
    {
      gzwarn << "ambient texture not supported" << std::endl;
      return;
    }
    if (_type == "emission")
    {
      gzwarn << "emission texture not supported" << std::endl;
      return;
    }
    if (_type == "specular")
    {
      gzwarn << "specular texture not supported" << std::endl;
      return;
    }

    // rendering pipeline doesn't respect the blend mode, here we set
    // the diffuse to full white as a workaround.
    if (_type == "diffuse"
        && _mat->Blend() == Material::BlendMode::REPLACE)
    {
      _mat->SetDiffuse(math::Color(1, 1, 1, 1));
    }

    _mat->SetLighting(true);
    tinyxml2::XMLElement *imageXml = NULL;
    std::string textureName =
      typeElem->FirstChildElement("texture")->Attribute("texture");
    tinyxml2::XMLElement *textureXml = this->ElementId("newparam", textureName);
    if (textureXml)
    {
      if (std::string(textureXml->Value()) == "image")
      {
        imageXml = textureXml;
      }
      else
      {
        tinyxml2::XMLElement *sampler =
            textureXml->FirstChildElement("sampler2D");
        if (sampler)
        {
          std::string sourceName =
              sampler->FirstChildElement("source")->GetText();
          tinyxml2::XMLElement *sourceXml =
              this->ElementId("newparam", sourceName);
          if (sourceXml)
          {
            tinyxml2::XMLElement *surfaceXml =
                sourceXml->FirstChildElement("surface");
            if (surfaceXml && surfaceXml->FirstChildElement("init_from"))
            {
              imageXml = this->ElementId("image",
                  surfaceXml->FirstChildElement("init_from")->GetText());
            }
          }
        }
      }
    }
    else
    {
      imageXml = this->ElementId("image", textureName);
    }

    if (imageXml && imageXml->FirstChildElement("init_from"))
    {
      std::string imgFile =
        imageXml->FirstChildElement("init_from")->GetText();
      _mat->SetTextureImage(imgFile, this->path);
    }
  }
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::LoadPolylist(
    tinyxml2::XMLElement *_polylistXml,
    const gz::math::Matrix4d &_transform,
    Mesh *_mesh)
{
  // This function parses polylist types in collada into
  // a set of triangle meshes.  The assumption is that
  // each polylist polygon is convex, and we do decomposion
  // by anchoring each triangle about vertex 0 or each polygon
  std::unique_ptr<SubMesh> subMesh(new SubMesh);
  subMesh->SetName(this->currentNodeName);
  bool combinedVertNorms = false;

  subMesh->SetPrimitiveType(SubMesh::TRIANGLES);

  if (_polylistXml->Attribute("material"))
  {
    std::map<std::string, std::string>::iterator iter;
    std::string matStr = _polylistXml->Attribute("material");

    int matIndex = -1;
    iter = this->materialMap.find(matStr);
    if (iter != this->materialMap.end())
      matStr = iter->second;

    MaterialPtr mat = this->LoadMaterial(matStr);

    matIndex = _mesh->IndexOfMaterial(mat.get());
    if (matIndex < 0)
      matIndex = _mesh->AddMaterial(mat);

    if (matIndex < 0)
      gzwarn << "Unable to add material[" << matStr << "]\n";
    else
      subMesh->SetMaterialIndex(matIndex);
  }

  tinyxml2::XMLElement *polylistInputXml =
      _polylistXml->FirstChildElement("input");

  std::vector<gz::math::Vector3d> verts;
  std::vector<gz::math::Vector3d> norms;
  std::map<unsigned int, std::vector<gz::math::Vector2d>> texcoords;
  std::vector<std::pair<unsigned int, unsigned int>> texcoordsOffsetToSet;

  const unsigned int VERTEX = 0;
  const unsigned int NORMAL = 1;
  const unsigned int TEXCOORD = 2;
  unsigned int otherSemantics = TEXCOORD + 1;

  // look up table of position/normal/texcoord duplicate indices
  std::unordered_map<unsigned int, std::map<unsigned int, unsigned int>>
      texDupMap;
  std::map<unsigned int, unsigned int> normalDupMap;
  std::map<unsigned int, unsigned int> positionDupMap;

  gz::math::Matrix4d bindShapeMat(gz::math::Matrix4d::Identity);
  if (_mesh->HasSkeleton())
    bindShapeMat = _mesh->MeshSkeleton()->BindShapeTransform();

  // read input elements. A vector of int is used because there can be
  // multiple TEXCOORD inputs.
  std::map<const unsigned int, std::set<int>> inputs;
  unsigned int inputSize = 0;
  while (polylistInputXml)
  {
    std::string semantic = polylistInputXml->Attribute("semantic");
    std::string source = polylistInputXml->Attribute("source");
    std::string offset = polylistInputXml->Attribute("offset");
    if (semantic == "VERTEX")
    {
      unsigned int count = norms.size();
      this->LoadVertices(source, _transform, verts, norms,
          positionDupMap, normalDupMap);
      if (norms.size() > count)
        combinedVertNorms = true;
      inputs[VERTEX].insert(gz::math::parseInt(offset));
    }
    else if (semantic == "NORMAL")
    {
      this->LoadNormals(source, _transform, norms, normalDupMap);
      combinedVertNorms = false;
      inputs[NORMAL].insert(gz::math::parseInt(offset));
    }
    else if (semantic == "TEXCOORD")
    {
      int offsetInt = gz::math::parseInt(offset);
      unsigned int set = 0u;
      auto setStr = polylistInputXml->Attribute("set");
      if (setStr)
        set = gz::math::parseInt(setStr);
      this->LoadTexCoords(source, texcoords[set], texDupMap[set]);
      inputs[TEXCOORD].insert(offsetInt);
      texcoordsOffsetToSet.push_back(std::make_pair(offsetInt, set));
    }
    else
    {
      inputs[otherSemantics++].insert(gz::math::parseInt(offset));
      gzwarn << "Polylist input semantic: '" << semantic << "' is currently"
          << " not supported" << std::endl;
    }

    polylistInputXml = polylistInputXml->NextSiblingElement("input");
  }

  for (const auto &input : inputs)
    inputSize += input.second.size();

  // read vcount
  // break poly into triangles
  // if vcount >= 4, anchor around 0 (note this is bad for concave elements)
  //   e.g. if vcount = 4, break into triangle 1: [0,1,2], triangle 2: [0,2,3]
  tinyxml2::XMLElement *vcountXml = _polylistXml->FirstChildElement("vcount");
  std::string vcountStr = vcountXml->GetText();
  std::vector<std::string> vcountStrs = split(vcountStr, " \t\r\n");
  std::vector<int> vcounts;
  for (unsigned int j = 0; j < vcountStrs.size(); ++j)
    vcounts.push_back(math::parseInt(vcountStrs[j]));

  // read p
  tinyxml2::XMLElement *pXml = _polylistXml->FirstChildElement("p");
  std::string pStr = pXml->GetText();

  // vertexIndexMap is a map of collada vertex index to Gazebo submesh vertex
  // indices, used for identifying vertices that can be shared.
  std::map<unsigned int, std::vector<GeometryIndices> > vertexIndexMap;
  unsigned int *values = new unsigned int[inputSize];
  memset(values, 0, inputSize);

  std::vector<std::string> strs = split(pStr, " \t\r\n");
  std::vector<std::string>::iterator strs_iter = strs.begin();
  for (unsigned int l = 0; l < vcounts.size(); ++l)
  {
    // put us at the beginning of the polygon list
    if (l > 0)
      strs_iter += inputSize * vcounts[l-1];

    for (unsigned int k = 2; k < static_cast<unsigned int>(vcounts[l]); ++k)
    {
      // if vcounts[l] = 5, then read 0,1,2, then 0,2,3, 0,3,4,...
      // here k = the last number in the series
      // j is the triangle loop
      for (unsigned int j = 0; j < 3; ++j)
      {
        // break polygon into triangles
        unsigned int triangle_index;

        if (j == 0)
          triangle_index = 0;
        if (j == 1)
          triangle_index = (k-1)*inputSize;
        if (j == 2)
          triangle_index = (k)*inputSize;

        for (unsigned int i = 0; i < inputSize; ++i)
        {
          values[i] = gz::math::parseInt(strs_iter[triangle_index+i]);
        }

        unsigned int daeVertIndex = 0;
        bool addIndex = inputs[VERTEX].empty();

        // find a set of vertex/normal/texcoord that can be reused
        // only do this if the mesh has vertices
        if (!inputs[VERTEX].empty())
        {
          // Get the vertex position index value. If it is a duplicate then use
          // the existing index instead
          daeVertIndex = values[*inputs[VERTEX].begin()];
          if (positionDupMap.find(daeVertIndex) != positionDupMap.end())
            daeVertIndex = positionDupMap[daeVertIndex];

          // if the vertex index has not been previously added then just add it.
          if (vertexIndexMap.find(daeVertIndex) == vertexIndexMap.end())
          {
            addIndex = true;
          }
          else
          {
            // if the vertex index was previously added, check to see if it has
            // the same normal and texcoord index values
            bool toDuplicate = true;
            unsigned int reuseIndex = 0;
            std::vector<GeometryIndices> inputValues =
                vertexIndexMap[daeVertIndex];

            for (unsigned int i = 0; i < inputValues.size(); ++i)
            {
              GeometryIndices iv = inputValues[i];
              bool normEqual = false;
              bool texEqual = false;

              if (!inputs[NORMAL].empty())
              {
                // Get the vertex normal index value. If the normal is a
                // duplicate then reset the index to the first instance of the
                // duplicated position
                unsigned int remappedNormalIndex =
                  values[*inputs[NORMAL].begin()];
                if (normalDupMap.find(remappedNormalIndex)
                    != normalDupMap.end())
                 {
                  remappedNormalIndex = normalDupMap[remappedNormalIndex];
                 }

                if (iv.normalIndex == remappedNormalIndex)
                  normEqual = true;
              }

              if (!inputs[TEXCOORD].empty())
              {
                texEqual = true;
                for (auto &pair : texcoordsOffsetToSet)
                {
                  unsigned int offset = pair.first;
                  unsigned int set = pair.second;
                  // Get the vertex texcoord index value. If the texcoord is a
                  // duplicate then reset the index to the first instance of the
                  // duplicated texcoord
                  unsigned int remappedTexcoordIndex =
                    values[offset];
                  auto &texDupMapSet = texDupMap[set];
                  auto texDupMapSetIt = texDupMapSet.find(
                      remappedTexcoordIndex);
                  if (texDupMapSetIt != texDupMapSet.end())
                    remappedTexcoordIndex = texDupMapSetIt->second;
                  if (iv.texcoordIndex[set] != remappedTexcoordIndex)
                  {
                    texEqual = false;
                    break;
                  }
                }
              }

              // if the vertex has matching normal and texcoord index values
              // then the vertex can be reused.
              if ((inputs[NORMAL].empty() || normEqual) &&
                  (inputs[TEXCOORD].empty() || texEqual))
              {
                // found a vertex that can be shared.
                toDuplicate = false;
                reuseIndex = iv.mappedIndex;
                subMesh->AddIndex(reuseIndex);
                break;
              }
            }
            addIndex = toDuplicate;
          }
        }

        // if the vertex index is new or can not be shared then add it
        if (addIndex)
        {
          GeometryIndices input;
          if (!inputs[VERTEX].empty())
          {
            subMesh->AddVertex(verts[daeVertIndex]);
            unsigned int newVertIndex = subMesh->VertexCount()-1;
            subMesh->AddIndex(newVertIndex);
            if (combinedVertNorms)
              subMesh->AddNormal(norms[daeVertIndex]);
            if (_mesh->HasSkeleton())
            {
              subMesh->SetVertex(newVertIndex, bindShapeMat *
                  subMesh->Vertex(newVertIndex));
              SkeletonPtr skel = _mesh->MeshSkeleton();
              for (unsigned int i = 0;
                  i < skel->VertNodeWeightCount(daeVertIndex); ++i)
              {
                std::pair<std::string, double> node_weight =
                  skel->VertNodeWeight(daeVertIndex, i);
                SkeletonNode *node =
                    _mesh->MeshSkeleton()->NodeByName(node_weight.first);
                subMesh->AddNodeAssignment(subMesh->VertexCount()-1,
                                node->Handle(), node_weight.second);
              }
            }
            input.vertexIndex = daeVertIndex;
            input.mappedIndex = newVertIndex;
          }
          if (!inputs[NORMAL].empty())
          {
            unsigned int inputRemappedNormalIndex =
              values[*inputs[NORMAL].begin()];

            if (normalDupMap.find(inputRemappedNormalIndex)
                != normalDupMap.end())
            {
              inputRemappedNormalIndex = normalDupMap[inputRemappedNormalIndex];
            }

            if (norms.size() > inputRemappedNormalIndex)
            {
              subMesh->AddNormal(norms[inputRemappedNormalIndex]);
              input.normalIndex = inputRemappedNormalIndex;
            }
          }

          if (!inputs[TEXCOORD].empty())
          {
            for (auto &pair : texcoordsOffsetToSet)
            {
              unsigned int offset = pair.first;
              unsigned int set = pair.second;

              unsigned int inputRemappedTexcoordIndex =
                values[offset];

              auto &texDupMapSet = texDupMap[set];
              auto texDupMapSetIt = texDupMapSet.find(
                  inputRemappedTexcoordIndex);
              if (texDupMapSetIt != texDupMapSet.end())
                inputRemappedTexcoordIndex = texDupMapSetIt->second;
              auto &texcoordsSet = texcoords[set];
              subMesh->AddTexCoordBySet(
                  texcoordsSet[inputRemappedTexcoordIndex].X(),
                  texcoordsSet[inputRemappedTexcoordIndex].Y(), set);
              input.texcoordIndex[set] = inputRemappedTexcoordIndex;
            }
          }

          // add the new Gazebo submesh vertex index to the map
          if (!inputs[VERTEX].empty())
          {
            std::vector<GeometryIndices> inputValues;
            inputValues.push_back(input);
            vertexIndexMap[daeVertIndex] = inputValues;
          }
        }
      }
    }
  }
  delete [] values;

  _mesh->AddSubMesh(std::move(subMesh));
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::LoadTriangles(
    tinyxml2::XMLElement *_trianglesXml,
    const gz::math::Matrix4d &_transform, Mesh *_mesh)
{
  std::unique_ptr<SubMesh> subMesh(new SubMesh);
  subMesh->SetName(this->currentNodeName);
  bool combinedVertNorms = false;

  subMesh->SetPrimitiveType(SubMesh::TRIANGLES);

  if (_trianglesXml->Attribute("material"))
  {
    std::map<std::string, std::string>::iterator iter;
    std::string matStr = _trianglesXml->Attribute("material");

    int matIndex = -1;
    iter = this->materialMap.find(matStr);
    if (iter != this->materialMap.end())
      matStr = iter->second;

    MaterialPtr mat = this->LoadMaterial(matStr);
    matIndex = _mesh->IndexOfMaterial(mat.get());
    if (matIndex < 0)
      matIndex = _mesh->AddMaterial(mat);

    if (matIndex < 0)
      gzwarn << "Unable to add material[" << matStr << "]\n";
    else
      subMesh->SetMaterialIndex(matIndex);
  }

  tinyxml2::XMLElement *trianglesInputXml =
      _trianglesXml->FirstChildElement("input");

  std::vector<gz::math::Vector3d> verts;
  std::vector<gz::math::Vector3d> norms;
  std::map<unsigned int, std::vector<gz::math::Vector2d>> texcoords;
  std::vector<std::pair<unsigned int, unsigned int>> texcoordsOffsetToSet;

  const unsigned int VERTEX = 0;
  const unsigned int NORMAL = 1;
  const unsigned int TEXCOORD = 2;
  unsigned int otherSemantics = TEXCOORD + 1;
  bool hasVertices = false;
  bool hasNormals = false;
  bool hasTexcoords = false;
  unsigned int offsetSize = 0;

  // read input elements. A vector of int is used because there can be
  // multiple TEXCOORD inputs.
  std::map<const unsigned int, std::set<int>> inputs;

  // look up table of position/normal/texcoord duplicate indices
  std::unordered_map<unsigned int, std::map<unsigned int, unsigned int>>
      texDupMap;
  std::map<unsigned int, unsigned int> normalDupMap;
  std::map<unsigned int, unsigned int> positionDupMap;

  while (trianglesInputXml)
  {
    std::string semantic = trianglesInputXml->Attribute("semantic");
    std::string source = trianglesInputXml->Attribute("source");
    std::string offset = trianglesInputXml->Attribute("offset");
    if (semantic == "VERTEX")
    {
      unsigned int count = norms.size();
      this->LoadVertices(source, _transform, verts, norms,
          positionDupMap, normalDupMap);
      if (norms.size() > count)
        combinedVertNorms = true;
      inputs[VERTEX].insert(gz::math::parseInt(offset));
      hasVertices = true;
    }
    else if (semantic == "NORMAL")
    {
      this->LoadNormals(source, _transform, norms, normalDupMap);
      combinedVertNorms = false;
      inputs[NORMAL].insert(gz::math::parseInt(offset));
      if (norms.size() > 0)
        hasNormals = true;
    }
    else if (semantic == "TEXCOORD")
    {
      int offsetInt = gz::math::parseInt(offset);
      unsigned int set = 0u;
      auto setStr = trianglesInputXml->Attribute("set");
      if (setStr)
        set = gz::math::parseInt(setStr);
      this->LoadTexCoords(source, texcoords[set], texDupMap[set]);
      inputs[TEXCOORD].insert(offsetInt);
      texcoordsOffsetToSet.push_back(std::make_pair(offsetInt, set));
      hasTexcoords = true;
    }
    else
    {
      inputs[otherSemantics++].insert(gz::math::parseInt(offset));
      gzwarn << "Triangle input semantic: '" << semantic << "' is currently"
          << " not supported" << std::endl;
    }
    trianglesInputXml = trianglesInputXml->NextSiblingElement("input");
  }

  for (const auto &input : inputs)
    offsetSize += input.second.size();

  tinyxml2::XMLElement *pXml = _trianglesXml->FirstChildElement("p");
  if (!pXml || !pXml->GetText())
  {
    int count = 1;
    if (_trianglesXml->Attribute("count"))
    {
      try
      {
        count = std::stoi(_trianglesXml->Attribute("count"));
      }
      catch(...)
      {
        // Do nothing. Messages are printed out below.
      }
    }

    // It's possible that the triangle count is zero. In this case, we
    // should not output an error message
    if (count)
    {
      gzerr << "Collada file[" << this->filename
        << "] is invalid. Loading what we can...\n";
    }
    else
    {
      gzlog << "Triangle input has a count of zero. "
        << "This is likely not desired\n";
    }

    return;
  }
  std::string pStr = pXml->GetText();

  // Collada format allows normals and texcoords to have their own set of
  // indices for more efficient storage of data but opengl only supports one
  // index buffer. So we need to reorder normals/texcoord to match the vertex
  // index and duplicate any vertices that have the same index but different
  // normal/texcoord.

  // vertexIndexMap is a map of collada vertex index to Gazebo submesh vertex
  // indices, used for identifying vertices that can be shared.
  std::map<unsigned int, std::vector<GeometryIndices> > vertexIndexMap;

  std::vector<unsigned int> values(offsetSize);
  std::vector<std::string> strs = split(pStr, " \t\r\n");

  for (unsigned int j = 0; j < strs.size(); j += offsetSize)
  {
    for (unsigned int i = 0; i < offsetSize; ++i)
      values.at(i) = gz::math::parseInt(strs[j+i]);

    unsigned int daeVertIndex = 0;
    bool addIndex = !hasVertices;

    // find a set of vertex/normal/texcoord that can be reused
    // only do this if the mesh has vertices
    if (hasVertices)
    {
      // Get the vertex position index value. If the position is a duplicate
      // then reset the index to the first instance of the duplicated position
      daeVertIndex = values.at(*inputs[VERTEX].begin());
      if (positionDupMap.find(daeVertIndex) != positionDupMap.end())
        daeVertIndex = positionDupMap[daeVertIndex];

      // if the vertex index has not been previously added then just add it.
      if (vertexIndexMap.find(daeVertIndex) == vertexIndexMap.end())
      {
        addIndex = true;
      }
      else
      {
        // if the vertex index was previously added, check to see if it has the
        // same normal and texcoord index values
        bool toDuplicate = true;
        unsigned int reuseIndex = 0;
        std::vector<GeometryIndices> inputValues = vertexIndexMap[daeVertIndex];

        for (unsigned int i = 0; i < inputValues.size(); ++i)
        {
          GeometryIndices iv = inputValues[i];
          bool normEqual = false;
          bool texEqual = false;
          if (hasNormals)
          {
            // Get the vertex normal index value. If the normal is a duplicate
            // then reset the index to the first instance of the duplicated
            // position
            unsigned int remappedNormalIndex = values.at(
                *inputs[NORMAL].begin());
            if (normalDupMap.find(remappedNormalIndex) != normalDupMap.end())
              remappedNormalIndex = normalDupMap[remappedNormalIndex];

            if (iv.normalIndex == remappedNormalIndex)
              normEqual = true;
          }
          if (hasTexcoords)
          {
            texEqual = true;
            for (auto &pair : texcoordsOffsetToSet)
            {
              unsigned int offset = pair.first;
              unsigned int set = pair.second;

              // Get the vertex texcoord index value. If the texcoord is a
              // duplicate then reset the index to the first instance of the
              // duplicated texcoord
              unsigned int remappedTexcoordIndex =
                  values.at(offset);
              auto &texDupMapSet = texDupMap[set];
              auto texDupMapSetIt = texDupMapSet.find(remappedTexcoordIndex);
              if (texDupMapSetIt != texDupMapSet.end())
                remappedTexcoordIndex = texDupMapSetIt->second;

              if (iv.texcoordIndex[set] != remappedTexcoordIndex)
              {
                texEqual = false;
                break;
              }
            }
          }

          // if the vertex has matching normal and texcoord index values then
          // the vertex can be reused.
          if ((!hasNormals || normEqual) && (!hasTexcoords || texEqual))
          {
            // found a vertex that can be shared.
            toDuplicate = false;
            reuseIndex = iv.mappedIndex;
            subMesh->AddIndex(reuseIndex);
            break;
          }
        }
        addIndex = toDuplicate;
      }
    }

    // if the vertex index is new or can not be shared then add it
    if (addIndex)
    {
      GeometryIndices input;
      if (hasVertices)
      {
        subMesh->AddVertex(verts[daeVertIndex]);
        unsigned int newVertIndex = subMesh->VertexCount()-1;
        subMesh->AddIndex(newVertIndex);

        if (combinedVertNorms)
          subMesh->AddNormal(norms[daeVertIndex]);
        if (_mesh->HasSkeleton())
        {
          SkeletonPtr skel = _mesh->MeshSkeleton();
          for (unsigned int i = 0;
              i < skel->VertNodeWeightCount(daeVertIndex); ++i)
          {
            std::pair<std::string, double> node_weight =
              skel->VertNodeWeight(daeVertIndex, i);
            SkeletonNode *node =
                _mesh->MeshSkeleton()->NodeByName(node_weight.first);
            if (nullptr == node)
            {
              gzerr << "Failed to find skeleton node named ["
                     << node_weight.first << "]" << std::endl;
              continue;
            }
            subMesh->AddNodeAssignment(subMesh->VertexCount()-1,
                            node->Handle(), node_weight.second);
          }
        }
        input.vertexIndex = daeVertIndex;
        input.mappedIndex = newVertIndex;
      }
      if (hasNormals)
      {
        unsigned int inputRemappedNormalIndex = values.at(
            *inputs[NORMAL].begin());
        if (normalDupMap.find(inputRemappedNormalIndex) != normalDupMap.end())
          inputRemappedNormalIndex = normalDupMap[inputRemappedNormalIndex];
        subMesh->AddNormal(norms[inputRemappedNormalIndex]);
        input.normalIndex = inputRemappedNormalIndex;
      }
      if (hasTexcoords)
      {
        for (auto &pair : texcoordsOffsetToSet)
        {
          unsigned int offset = pair.first;
          unsigned int set = pair.second;

          unsigned int inputRemappedTexcoordIndex =
              values.at(offset);

          auto &texDupMapSet = texDupMap[set];
          auto texDupMapSetIt = texDupMapSet.find(inputRemappedTexcoordIndex);
          if (texDupMapSetIt != texDupMapSet.end())
            inputRemappedTexcoordIndex = texDupMapSetIt->second;
          auto &texcoordsSet = texcoords[set];
          subMesh->AddTexCoordBySet(
              texcoordsSet[inputRemappedTexcoordIndex].X(),
              texcoordsSet[inputRemappedTexcoordIndex].Y(), set);
          input.texcoordIndex[set] = inputRemappedTexcoordIndex;
        }
      }

      // add the new Gazebo submesh vertex index to the map
      if (hasVertices)
      {
        std::vector<GeometryIndices> inputValues;
        inputValues.push_back(input);
        vertexIndexMap[daeVertIndex] = inputValues;
      }
    }
  }

  _mesh->AddSubMesh(std::move(subMesh));
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::LoadLines(tinyxml2::XMLElement *_xml,
    const gz::math::Matrix4d &_transform, Mesh *_mesh)
{
  std::unique_ptr<SubMesh> subMesh(new SubMesh);
  subMesh->SetName(this->currentNodeName);
  subMesh->SetPrimitiveType(SubMesh::LINES);

  tinyxml2::XMLElement *inputXml = _xml->FirstChildElement("input");
  // std::string semantic = inputXml->Attribute("semantic");
  std::string source = inputXml->Attribute("source");

  std::vector<gz::math::Vector3d> verts;
  std::vector<gz::math::Vector3d> norms;
  this->LoadVertices(source, _transform, verts, norms);

  tinyxml2::XMLElement *pXml = _xml->FirstChildElement("p");
  std::string pStr = pXml->GetText();
  std::istringstream iss(pStr);

  do
  {
    int a, b;
    iss >> a >> b;

    if (!iss)
      break;
    subMesh->AddVertex(verts[a]);
    subMesh->AddIndex(subMesh->VertexCount() - 1);
    subMesh->AddVertex(verts[b]);
    subMesh->AddIndex(subMesh->VertexCount() - 1);
  } while (iss);

  _mesh->AddSubMesh(std::move(subMesh));
}

/////////////////////////////////////////////////
float ColladaLoader::Implementation::LoadFloat(tinyxml2::XMLElement *_elem)
{
  float value = 0;

  if (_elem->FirstChildElement("float"))
  {
    value =
      gz::math::parseFloat(_elem->FirstChildElement("float")->GetText());
  }

  return value;
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::LoadTransparent(tinyxml2::XMLElement *_elem,
    MaterialPtr _mat)
{
  const char *opaqueCStr = _elem->Attribute("opaque");
  if (!opaqueCStr)
  {
    // no opaque mode, revert transparency to 0.0
    _mat->SetTransparency(0.0);
    return;
  }

  // https://www.khronos.org/files/collada_spec_1_5.pdf
  // Determining Transparency (Opacity) section:
  // opaque modes: RGB_ZERO, RGB_ONE, A_ONE
  if (_elem->FirstChildElement("texture"))
  {
    _mat->SetAlphaFromTexture(true);
    _mat->SetTransparency(0.0);
  }
  else if (_elem->FirstChildElement("color"))
  {
    const char *colorCStr = _elem->FirstChildElement("color")->GetText();
    if (!colorCStr)
    {
      gzerr << "No color string\n";
      return;
    }

    std::string opaqueStr = opaqueCStr;
    std::string colorStr = colorCStr;
    math::Color color;
    std::istringstream stream(colorStr);
    stream >> color;

    // src is the texel value and dst is the existing pixel value
    double srcFactor = 0;
    double dstFactor = 0;

    // If <transparency> tag exists, _mat->Transparency() should be set to
    // that value already. Otherwise, use default value of 1.0 as per
    // collada spec
    double transparency = 1.0;
    auto transparencyNode =
        _elem->Parent()->FirstChildElement("transparency");
    if (transparencyNode)
      transparency = _mat->Transparency();

    // Calculate alpha based on opaque mode.
    // Equations are extracted from collada spec
    // Make sure to update the final transparency value
    // final mat transparency = 1 - srcFactor = dstFactor
    if (opaqueStr == "RGB_ZERO")
    {
      // Lunimance based on ISO/CIE color standards ITU-R BT.709-4
      float luminance = 0.212671 * color.R() +
                        0.715160 * color.G() +
                        0.072169 * color.B();
      // result.a = fb.a * (luminance(transparent.rgb) * transparency) + mat.a *
      // (1.0f - luminance(transparent.rgb) * transparency)
      // where fb corresponds to the framebuffer (existing pixel) and
      // mat corresponds to material before transparency (texel)
      dstFactor = luminance * transparency;
      srcFactor = 1.0 - luminance * transparency;
      _mat->SetTransparency(dstFactor);
    }
    else if (opaqueStr == "RGB_ONE")
    {
      // Lunimance based on ISO/CIE color standards ITU-R BT.709-4
      float luminance = 0.212671 * color.R() +
                        0.715160 * color.G() +
                        0.072169 * color.B();

      // result.a = fb.a * (1.0f - luminance(transparent.rgb) * transparency) +
      // mat.a * (luminance(transparent.rgb) * transparency)
      // where fb corresponds to the framebuffer (existing pixel) and
      // mat corresponds to material before transparency (texel)
      dstFactor = 1.0 - luminance * transparency;
      srcFactor = luminance * transparency;
      _mat->SetTransparency(dstFactor);
    }
    else if (opaqueStr == "A_ONE")
    {
      // result.a = fb.a * (1.0f - transparent.a * transparency) + mat.a *
      // (transparent.a * transparency)
      // where fb corresponds to the framebuffer (existing pixel) and
      // mat corresponds to material before transparency (texel)
      dstFactor = 1.0 - color.A() * transparency;
      srcFactor = color.A() * transparency;
      _mat->SetTransparency(dstFactor);
    }
    else if (opaqueStr == "A_ZERO")
    {
      // result.a = fb.a * (transparent.a * transparency) + mat.a *
      // (1.0f - transparent.a * transparency)
      // where fb corresponds to the framebuffer (existing pixel) and
      // mat corresponds to material before transparency (texel)
      dstFactor = color.A() * transparency;
      srcFactor = 1.0 - color.A() * transparency;
      _mat->SetTransparency(dstFactor);
    }

    _mat->SetBlendFactors(srcFactor, dstFactor);
  }
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::MergeSkeleton(SkeletonPtr _skeleton,
    SkeletonNode *_mergeNode)
{
  if (nullptr == _skeleton)
  {
    gzerr << "Fail to merge null skeleton." << std::endl;
    return;
  }

  if (nullptr == _mergeNode)
  {
    gzerr << "Fail to merge null skeleton node." << std::endl;
    return;
  }

  if (_skeleton->NodeById(_mergeNode->Id()))
    return;

  if (nullptr == _skeleton->RootNode())
  {
    gzerr << "Skeleton missing root node." << std::endl;
    return;
  }

  SkeletonNode *currentRoot = _skeleton->RootNode();
  if (currentRoot->Id() == _mergeNode->Id())
    return;

  auto *rootInMergeNode = _mergeNode->ChildById(currentRoot->Id());

  if (rootInMergeNode != nullptr)
  {
    if (rootInMergeNode != currentRoot)
    {
      delete currentRoot;
    }
    _skeleton->RootNode(_mergeNode);
    return;
  }

  SkeletonNode *dummyRoot = nullptr;
  if (currentRoot->Id() == "dummy-root")
  {
    // Check if the node that will be merged contains the dummy-root
    // if so, replace dummyRoot
    bool mergeNodeContainsRoot = true;
    for (unsigned int i=0; i < currentRoot->ChildCount(); ++i)
    {
      if (_mergeNode->ChildById(currentRoot->Child(i)->Id()) == nullptr)
      {
        mergeNodeContainsRoot = false;
        break;
      }
    }
    if (mergeNodeContainsRoot)
    {
      std::function<void(SkeletonNode*)> delete_children;
      delete_children =
        [&delete_children](SkeletonNode *node)-> void {
          for (size_t ii = 0; ii < node->ChildCount(); ++ii)
          {
            auto *child = node->Child(ii);
            delete_children(child);
            delete child;
          }
      };

      // Since we are replacing the whole tree, recursively clean up
      // the existing skeleton nodes
      _skeleton->RootNode(_mergeNode);
      delete_children(currentRoot);
      delete currentRoot;
      return;
    }
    dummyRoot = currentRoot;
  }
  else
  {
    dummyRoot =
        new SkeletonNode(nullptr, "dummy-root", "dummy-root");
  }

  if (dummyRoot != currentRoot)
  {
    dummyRoot->AddChild(currentRoot);
    currentRoot->SetParent(dummyRoot);
  }
  dummyRoot->AddChild(_mergeNode);
  _mergeNode->SetParent(dummyRoot);
  dummyRoot->SetTransform(math::Matrix4d::Identity);
  _skeleton->RootNode(dummyRoot);
}

/////////////////////////////////////////////////
void ColladaLoader::Implementation::ApplyInvBindTransform(SkeletonPtr _skeleton)
{
  std::list<SkeletonNode *> queue;
  queue.push_back(_skeleton->RootNode());

  while (!queue.empty())
  {
    SkeletonNode *node = queue.front();
    queue.pop_front();
    if (nullptr == node)
      continue;

    if (node->HasInvBindTransform())
      node->SetModelTransform(node->InverseBindTransform().Inverse(), false);
    for (unsigned int i = 0; i < node->ChildCount(); i++)
      queue.push_back(node->Child(i));
  }
}
