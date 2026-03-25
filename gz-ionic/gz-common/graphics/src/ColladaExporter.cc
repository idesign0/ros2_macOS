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
#include <gz/math/Vector3.hh>

#include <gz/common/Material.hh>
#include <gz/common/Mesh.hh>
#include <gz/common/SubMesh.hh>
#include <gz/common/Console.hh>
#include <gz/common/ColladaExporter.hh>
#include <gz/common/Filesystem.hh>

#include "tinyxml2.h"

#ifdef _WIN32
  static const char pathSeparator = '\\';
  #define snprintf _snprintf
#else
  static const char pathSeparator = '/';
#endif

using namespace gz;
using namespace common;

static void LogTinyXml2DocumentError(
  const std::string &_flavorText,
  const tinyxml2::XMLDocument &_doc)
{
  std::string warning = _flavorText + " | tinyxml2 error id (";
  if (_doc.Error())
  {
    warning += std::to_string(_doc.ErrorID()) + "): ";

    const char * error1 = _doc.ErrorStr();

    if (error1)
      warning += "str1=" + std::string(error1);
  }
  else
  {
    warning += "none)";
  }

  gzwarn << warning << "\n";
}

/// Private data for the ColladaExporter class
class gz::common::ColladaExporter::Implementation
{
  /// \brief Geometry types
  public: enum GeometryType {POSITION, NORMAL, UVMAP};

  /// \brief Export asset element
  /// \param[in] _assetXml Pointer to the asset XML instance
  public: void ExportAsset(tinyxml2::XMLElement *_assetXml);

  /// \brief Export geometry source
  /// \param[in] _subMesh Pointer to a submesh
  /// \param[in] _meshXml Pointer to the mesh XML instance
  /// \param[in] _type POSITION, NORMAL or UVMAP
  /// \param[in] _meshID Mesh ID (mesh_<number>)
  public: void ExportGeometrySource(
      const SubMesh *_subMesh,
      tinyxml2::XMLElement *_meshXml, GeometryType _type, const char *_meshID);

  /// \brief Export library geometries element
  /// \param[in] libraryGeometriesXml Pointer to the library geometries
  /// XML instance
  public: void ExportGeometries(tinyxml2::XMLElement *_libraryGeometriesXml);

  /// \brief Export library images element
  /// \param[in] _libraryImagesXml Pointer to the library images XML
  /// instance
  /// \return integer, number of images
  public: int ExportImages(tinyxml2::XMLElement *_libraryImagesXml);

  /// \brief Export library materials element
  /// \param[in] _libraryMaterialsXml Pointer to the library materials XML
  /// instance
  public: void ExportMaterials(tinyxml2::XMLElement *_libraryMaterialsXml);

  /// \brief Export library effects element
  /// \param[in] _libraryEffectsXml Pointer to the library effects XML
  /// instance
  public: void ExportEffects(tinyxml2::XMLElement *_libraryEffectsXml);

  /// \brief Export library visual scenes element
  /// \param[in] _libraryVisualScenesXml Pointer to the library visual
  /// scenes XML instance
  public: void ExportVisualScenes(
              tinyxml2::XMLElement *_libraryVisualScenesXml,
              const std::vector<math::Matrix4d> &_submeshToMatrix,
              const std::vector<ColladaLight> &_lights);

  /// \brief Export scene element
  /// \param[in] _sceneXml Pointer to the scene XML instance
  public: void ExportScene(tinyxml2::XMLElement *_sceneXml);

  /// \brief The mesh
  public: const Mesh *mesh;

  /// \brief Material count
  public: unsigned int materialCount;

  /// \brief SubMesh count
  public: unsigned int subMeshCount;

  /// \brief File path
  public: std::string path;

  /// \brief File name
  public: std::string filename;

  /// \brief True to export texture images to '../materials/textures'
  /// folder
  public: bool exportTextures;
};

//////////////////////////////////////////////////
ColladaExporter::ColladaExporter()
: MeshExporter(), dataPtr(gz::utils::MakeImpl<Implementation>())
{
}

//////////////////////////////////////////////////
ColladaExporter::~ColladaExporter()
{
}

//////////////////////////////////////////////////
void ColladaExporter::Export(const Mesh *_mesh, const std::string &_filename,
    bool _exportTextures)
{
  std::vector<math::Matrix4d> empty;
  std::vector<ColladaLight> empty_lights;
  this->Export(_mesh, _filename, _exportTextures, empty, empty_lights);
}

//////////////////////////////////////////////////
void ColladaExporter::Export(const Mesh *_mesh,
        const std::string &_filename, bool _exportTextures,
        const std::vector<math::Matrix4d> &_submeshToMatrix)
{
  std::vector<ColladaLight> empty_lights;
  this->Export(_mesh, _filename, _exportTextures,
    _submeshToMatrix, empty_lights);
}

//////////////////////////////////////////////////
void ColladaExporter::Export(const Mesh *_mesh, const std::string &_filename,
    bool _exportTextures, const std::vector<math::Matrix4d> &_submeshToMatrix,
    const std::vector<ColladaLight> &_lights)
{
  if ( _submeshToMatrix.size() > 0 &&
    (_mesh->SubMeshCount() != _submeshToMatrix.size()) )
  {
    gzerr << "_submeshToMatrix.size() : " << _mesh->SubMeshCount()
        << " , must be equal to SubMeshCount() : " << _mesh->SubMeshCount()
        << std::endl;
    return;
  }

  this->dataPtr->mesh = _mesh;
  this->dataPtr->materialCount = this->dataPtr->mesh->MaterialCount();
  this->dataPtr->subMeshCount = this->dataPtr->mesh->SubMeshCount();
  this->dataPtr->exportTextures = _exportTextures;

  // File name and path
  const std::string unix_filename = copyToUnixPath(_filename);
  unsigned int beginFilename = unix_filename.rfind("/")+1;

  this->dataPtr->path = unix_filename.substr(0, beginFilename);
  this->dataPtr->filename = unix_filename.substr(beginFilename);

  // Collada file
  tinyxml2::XMLDocument xmlDoc;

  // XML declaration
  xmlDoc.NewDeclaration();

  // Collada element
  tinyxml2::XMLElement *colladaXml = xmlDoc.NewElement("COLLADA");
  xmlDoc.LinkEndChild(colladaXml);
  colladaXml->SetAttribute("version", "1.4.1");
  colladaXml->SetAttribute("xmlns",
      "http://www.collada.org/2005/11/COLLADASchema");

  // Asset element
  tinyxml2::XMLElement *assetXml = xmlDoc.NewElement("asset");
  this->dataPtr->ExportAsset(assetXml);

  // Library geometries element
  tinyxml2::XMLElement *libraryGeometriesXml =
    xmlDoc.NewElement("library_geometries");
  this->dataPtr->ExportGeometries(libraryGeometriesXml);
  colladaXml->LinkEndChild(libraryGeometriesXml);

  if (this->dataPtr->materialCount != 0)
  {
    // Library images element
    tinyxml2::XMLElement *libraryImagesXml =
      xmlDoc.NewElement("library_images");
    int imageCount = this->dataPtr->ExportImages(libraryImagesXml);
    if (imageCount != 0)
    {
      colladaXml->LinkEndChild(libraryImagesXml);
    }

    // Library materials element
    tinyxml2::XMLElement *libraryMaterialsXml =
      xmlDoc.NewElement("library_materials");
    this->dataPtr->ExportMaterials(libraryMaterialsXml);
    colladaXml->LinkEndChild(libraryMaterialsXml);

    // Library effects element
    tinyxml2::XMLElement *libraryEffectsXml =
      xmlDoc.NewElement("library_effects");
    this->dataPtr->ExportEffects(libraryEffectsXml);
    colladaXml->LinkEndChild(libraryEffectsXml);
  }

  tinyxml2::XMLElement *libraryLightsXml =
      xmlDoc.NewElement("library_lights");
  for (const auto& light : _lights)
  {
    tinyxml2::XMLElement *lightXml =
      xmlDoc.NewElement("light");
    lightXml->SetAttribute("name", light.name.c_str());
    lightXml->SetAttribute("id", light.name.c_str());
    libraryLightsXml->LinkEndChild(lightXml);

    tinyxml2::XMLElement *techniqueCommonXml =
      xmlDoc.NewElement("technique_common");
    lightXml->LinkEndChild(techniqueCommonXml);

    tinyxml2::XMLElement *lightTypeXml =
      xmlDoc.NewElement(light.type.c_str());
    techniqueCommonXml->LinkEndChild(lightTypeXml);

    // color
    tinyxml2::XMLElement *colorXml =
      xmlDoc.NewElement("color");
    char color_str[100] = { 0 };
    snprintf(color_str, sizeof(color_str), "%g %g %g",
      light.diffuse.R(), light.diffuse.G(), light.diffuse.B());
    colorXml->SetText(color_str);
    lightTypeXml->LinkEndChild(colorXml);

    // attenuations
    if (light.type == "point" || light.type == "spot")
    {
      auto attenuation_tag = [&](const char* tagname, double value)
      {
        tinyxml2::XMLElement *attenXml =
          xmlDoc.NewElement(tagname);

        char str[100] = { 0 };
        snprintf(str, sizeof(str), "%g", value);
        attenXml->SetText(str);
        lightTypeXml->LinkEndChild(attenXml);
      };
      attenuation_tag("constant_attenuation", light.constantAttenuation);
      attenuation_tag("linear_attenuation", light.linearAttenuation);
      attenuation_tag("quadratic_attenuation", light.quadraticAttenuation);
    }

    // falloff
    if (light.type == "spot")
    {
      tinyxml2::XMLElement *falloffAngleXml =
        xmlDoc.NewElement("falloff_angle");
      char str[100] = { 0 };
      snprintf(str, sizeof(str), "%g", light.falloffAngleDeg);
      falloffAngleXml->SetText(str);
      lightTypeXml->LinkEndChild(falloffAngleXml);

      tinyxml2::XMLElement *falloffExpoXml =
        xmlDoc.NewElement("falloff_exponent");
      snprintf(str, sizeof(str), "%g", light.falloffExponent);
      falloffExpoXml->SetText(str);
      lightTypeXml->LinkEndChild(falloffExpoXml);
    }
  }
  colladaXml->LinkEndChild(libraryLightsXml);

  // Library visual scenes element
  tinyxml2::XMLElement *libraryVisualScenesXml =
      xmlDoc.NewElement("library_visual_scenes");
  this->dataPtr->ExportVisualScenes(libraryVisualScenesXml,
    _submeshToMatrix, _lights);
  colladaXml->LinkEndChild(libraryVisualScenesXml);

  // Scene element
  tinyxml2::XMLElement *sceneXml = xmlDoc.NewElement("scene");
  this->dataPtr->ExportScene(sceneXml);
  colladaXml->LinkEndChild(sceneXml);

  // Save file
  if (this->dataPtr->exportTextures)
  {
    const std::string directory = gz::common::joinPaths(
      this->dataPtr->path, this->dataPtr->filename, "meshes");

    createDirectories(directory);

    const std::string finalFilename = gz::common::joinPaths(
      this->dataPtr->path, this->dataPtr->filename, "meshes",
      this->dataPtr->filename + ".dae");

    const tinyxml2::XMLError error = xmlDoc.SaveFile(finalFilename.c_str());
    if (tinyxml2::XML_SUCCESS != error)
    {
      LogTinyXml2DocumentError(
        "Could not save colloda file with textures to [" + finalFilename
        + "]", xmlDoc);
    }
  }
  else
  {
    const std::string finalFilename = gz::common::joinPaths(
      this->dataPtr->path, this->dataPtr->filename + std::string(".dae"));

    const tinyxml2::XMLError error = xmlDoc.SaveFile(finalFilename.c_str());
    if (tinyxml2::XML_SUCCESS != error)
    {
      LogTinyXml2DocumentError(
        "Could not save collada file to [" + finalFilename + "]", xmlDoc);
    }
  }
}

//////////////////////////////////////////////////
void ColladaExporter::Implementation::ExportAsset(
    tinyxml2::XMLElement *_assetXml)
{
  tinyxml2::XMLElement *unitXml = _assetXml->GetDocument()->NewElement("unit");
  unitXml->SetAttribute("meter", "1");
  unitXml->SetAttribute("name", "meter");
  _assetXml->LinkEndChild(unitXml);

  tinyxml2::XMLElement *upAxisXml = _assetXml->GetDocument()->NewElement(
      "up_axis");
  upAxisXml->LinkEndChild(_assetXml->GetDocument()->NewText("Z_UP"));
  _assetXml->LinkEndChild(upAxisXml);
}

//////////////////////////////////////////////////
void ColladaExporter::Implementation::ExportGeometrySource(
    const gz::common::SubMesh *_subMesh,
    tinyxml2::XMLElement *_meshXml, GeometryType _type, const char *_meshID)
{
  char sourceId[100], sourceArrayId[107];
  std::ostringstream fillData;
  fillData.precision(8);
  fillData << std::fixed;
  int stride;
  unsigned int count = 0;

  if (_type == POSITION)
  {
    snprintf(sourceId, sizeof(sourceId), "%s-Positions", _meshID);
    count = _subMesh->VertexCount();
    stride = 3;
    gz::math::Vector3d vertex;
    for (unsigned int i = 0; i < count; ++i)
    {
      vertex = _subMesh->Vertex(i);
      fillData << vertex.X() << " " << vertex.Y() << " " << vertex.Z() << " ";
    }
  }
  if (_type == NORMAL)
  {
    snprintf(sourceId, sizeof(sourceId), "%s-Normals", _meshID);
    count = _subMesh->NormalCount();
    stride = 3;
    gz::math::Vector3d normal;
    for (unsigned int i = 0; i < count; ++i)
    {
      normal = _subMesh->Normal(i);
      fillData << normal.X() << " " << normal.Y() << " " << normal.Z() << " ";
    }
  }
  if (_type == UVMAP)
  {
    snprintf(sourceId, sizeof(sourceId), "%s-UVMap", _meshID);
    count = _subMesh->VertexCount();
    stride = 2;
    gz::math::Vector2d inTexCoord;
    for (unsigned int i = 0; i < count; ++i)
    {
      inTexCoord = _subMesh->TexCoordBySet(i, 0);
      fillData << inTexCoord.X() << " " << 1-inTexCoord.Y() << " ";
    }
  }
  tinyxml2::XMLElement *sourceXml = _meshXml->GetDocument()->NewElement(
      "source");
  _meshXml->LinkEndChild(sourceXml);
  sourceXml->SetAttribute("id", sourceId);
  sourceXml->SetAttribute("name", sourceId);

  snprintf(sourceArrayId, sizeof(sourceArrayId), "%s-array", sourceId);
  tinyxml2::XMLElement *floatArrayXml = _meshXml->GetDocument()->NewElement(
      "float_array");
  floatArrayXml->SetAttribute("count", count *stride);
  floatArrayXml->SetAttribute("id", sourceArrayId);
  floatArrayXml->LinkEndChild(
      _meshXml->GetDocument()->NewText(fillData.str().c_str()));
  sourceXml->LinkEndChild(floatArrayXml);

  tinyxml2::XMLElement *techniqueCommonXml =
    _meshXml->GetDocument()->NewElement("technique_common");
  sourceXml->LinkEndChild(techniqueCommonXml);

  snprintf(sourceArrayId, sizeof(sourceArrayId), "#%s-array", sourceId);
  tinyxml2::XMLElement *accessorXml = _meshXml->GetDocument()->NewElement(
      "accessor");
  accessorXml->SetAttribute("count", count);
  accessorXml->SetAttribute("source", sourceArrayId);
  accessorXml->SetAttribute("stride", stride);
  techniqueCommonXml->LinkEndChild(accessorXml);

  tinyxml2::XMLElement *paramXml = _meshXml->GetDocument()->NewElement("param");
  if (_type == POSITION || _type == NORMAL)
  {
    paramXml->SetAttribute("type", "float");
    paramXml->SetAttribute("name", "X");
    accessorXml->LinkEndChild(paramXml);

    paramXml = _meshXml->GetDocument()->NewElement("param");
    paramXml->SetAttribute("type", "float");
    paramXml->SetAttribute("name", "Y");
    accessorXml->LinkEndChild(paramXml);

    paramXml = _meshXml->GetDocument()->NewElement("param");
    paramXml->SetAttribute("type", "float");
    paramXml->SetAttribute("name", "Z");
    accessorXml->LinkEndChild(paramXml);
  }
  if (_type == UVMAP)
  {
    paramXml->SetAttribute("type", "float");
    paramXml->SetAttribute("name", "U");
    accessorXml->LinkEndChild(paramXml);

    paramXml = _meshXml->GetDocument()->NewElement("param");
    paramXml->SetAttribute("type", "float");
    paramXml->SetAttribute("name", "V");
    accessorXml->LinkEndChild(paramXml);
  }
}

//////////////////////////////////////////////////
void ColladaExporter::Implementation::ExportGeometries(
    tinyxml2::XMLElement *_libraryGeometriesXml)
{
  for (unsigned int i = 0; i < this->subMeshCount; ++i)
  {
    std::shared_ptr<SubMesh> subMesh = this->mesh->SubMeshByIndex(i).lock();
    if (!subMesh)
      continue;

    char meshId[100], materialId[100];
    snprintf(meshId, sizeof(meshId), "mesh_%u", i);
    if (subMesh->GetMaterialIndex())
    {
      snprintf(materialId, sizeof(materialId), "material_%u",
          subMesh->GetMaterialIndex().value());
    }

    tinyxml2::XMLElement *geometryXml =
      _libraryGeometriesXml->GetDocument()->NewElement("geometry");
    geometryXml->SetAttribute("id", meshId);
    _libraryGeometriesXml->LinkEndChild(geometryXml);

    tinyxml2::XMLElement *meshXml =
      _libraryGeometriesXml->GetDocument()->NewElement("mesh");
    geometryXml->LinkEndChild(meshXml);

    this->ExportGeometrySource(subMesh.get(), meshXml, POSITION, meshId);
    this->ExportGeometrySource(subMesh.get(), meshXml, NORMAL, meshId);
    if (subMesh->TexCoordCountBySet(0) != 0)
    {
      this->ExportGeometrySource(subMesh.get(), meshXml, UVMAP, meshId);
    }

    char attributeValue[111];

    tinyxml2::XMLElement *verticesXml =
      _libraryGeometriesXml->GetDocument()->NewElement("vertices");
    meshXml->LinkEndChild(verticesXml);
    snprintf(attributeValue, sizeof(attributeValue), "%s-Vertex", meshId);
    verticesXml->SetAttribute("id", attributeValue);
    verticesXml->SetAttribute("name", attributeValue);

    tinyxml2::XMLElement *inputXml =
      _libraryGeometriesXml->GetDocument()->NewElement("input");
    verticesXml->LinkEndChild(inputXml);
    inputXml->SetAttribute("semantic", "POSITION");
    snprintf(attributeValue, sizeof(attributeValue), "#%s-Positions", meshId);
    inputXml->SetAttribute("source", attributeValue);

    unsigned int indexCount = subMesh->IndexCount();

    tinyxml2::XMLElement *trianglesXml =
      _libraryGeometriesXml->GetDocument()->NewElement("triangles");
    meshXml->LinkEndChild(trianglesXml);
    trianglesXml->SetAttribute("count", indexCount/3);
    if (this->materialCount != 0)
    {
      trianglesXml->SetAttribute("material", materialId);
    }

    inputXml = _libraryGeometriesXml->GetDocument()->NewElement("input");
    trianglesXml->LinkEndChild(inputXml);
    inputXml->SetAttribute("offset", 0);
    inputXml->SetAttribute("semantic", "VERTEX");
    snprintf(attributeValue, sizeof(attributeValue), "#%s-Vertex", meshId);
    inputXml->SetAttribute("source", attributeValue);

    inputXml = _libraryGeometriesXml->GetDocument()->NewElement("input");
    trianglesXml->LinkEndChild(inputXml);
    inputXml->SetAttribute("offset", 1);
    inputXml->SetAttribute("semantic", "NORMAL");
    snprintf(attributeValue, sizeof(attributeValue), "#%s-Normals", meshId);
    inputXml->SetAttribute("source", attributeValue);

    if (subMesh->TexCoordCountBySet(0) != 0)
    {
      inputXml = _libraryGeometriesXml->GetDocument()->NewElement("input");
      trianglesXml->LinkEndChild(inputXml);
      inputXml->SetAttribute("offset", 2);
      inputXml->SetAttribute("semantic", "TEXCOORD");
      snprintf(attributeValue, sizeof(attributeValue), "#%s-UVMap", meshId);
      inputXml->SetAttribute("source", attributeValue);
    }

    std::ostringstream fillData;
    for (unsigned int j = 0; j < indexCount; ++j)
    {
      fillData << subMesh->Index(j) << " " << subMesh->Index(j) << " ";
      if (subMesh->TexCoordCountBySet(0) != 0)
      {
        fillData << subMesh->Index(j) << " ";
      }
    }

    tinyxml2::XMLElement *pXml =
      _libraryGeometriesXml->GetDocument()->NewElement("p");
    trianglesXml->LinkEndChild(pXml);
    pXml->LinkEndChild(_libraryGeometriesXml->GetDocument()->NewText(
          fillData.str().c_str()));
  }
}

//////////////////////////////////////////////////
int ColladaExporter::Implementation::ExportImages(
    tinyxml2::XMLElement *_libraryImagesXml)
{
  int imageCount = 0;
  for (unsigned int i = 0; i < this->materialCount; ++i)
  {
    const gz::common::MaterialPtr material =
      this->mesh->MaterialByIndex(i);
    std::string imageString = material->TextureImage();

    if (imageString.find(pathSeparator) != std::string::npos)
    {
      char id[100];
      snprintf(id, sizeof(id), "image_%u", i);

      tinyxml2::XMLElement *imageXml =
        _libraryImagesXml->GetDocument()->NewElement("image");
      imageXml->SetAttribute("id", id);
      _libraryImagesXml->LinkEndChild(imageXml);

      tinyxml2::XMLElement *initFromXml =
        _libraryImagesXml->GetDocument()->NewElement("init_from");
      const auto imageName = imageString.substr(
          imageString.rfind(pathSeparator));
      const auto imagePath = gz::common::joinPaths("..", "materials",
        "textures", imageName);
      initFromXml->LinkEndChild(
        _libraryImagesXml->GetDocument()->NewText(imagePath.c_str()));
      imageXml->LinkEndChild(initFromXml);

      if (this->exportTextures)
      {
        std::string textureDir = joinPaths(this->path, this->filename,
            "materials", "textures");
        std::string destFilename = joinPaths(textureDir, imageString.substr(
              imageString.rfind(pathSeparator)));
        createDirectories(textureDir);
        copyFile(imageString, destFilename);
      }

      imageCount++;
    }
  }

  return imageCount;
}

//////////////////////////////////////////////////
void ColladaExporter::Implementation::ExportMaterials(
    tinyxml2::XMLElement *_libraryMaterialsXml)
{
  for (unsigned int i = 0; i < this->materialCount; ++i)
  {
    char id[100];
    snprintf(id, sizeof(id), "material_%u", i);

    tinyxml2::XMLElement *materialXml =
      _libraryMaterialsXml->GetDocument()->NewElement("material");
    materialXml->SetAttribute("id", id);
    _libraryMaterialsXml->LinkEndChild(materialXml);

    snprintf(id, sizeof(id), "#material_%u_fx", i);
    tinyxml2::XMLElement *instanceEffectXml =
      _libraryMaterialsXml->GetDocument()->NewElement("instance_effect");
    instanceEffectXml->SetAttribute("url", id);
    materialXml->LinkEndChild(instanceEffectXml);
  }
}

//////////////////////////////////////////////////
void ColladaExporter::Implementation::ExportEffects(
    tinyxml2::XMLElement *_libraryEffectsXml)
{
  for (unsigned int i = 0; i < this->materialCount; ++i)
  {
    char id[100];
    snprintf(id, sizeof(id), "material_%u_fx", i);

    tinyxml2::XMLElement *effectXml =
      _libraryEffectsXml->GetDocument()->NewElement("effect");
    effectXml->SetAttribute("id", id);
    _libraryEffectsXml->LinkEndChild(effectXml);

    tinyxml2::XMLElement *profileCommonXml =
      _libraryEffectsXml->GetDocument()->NewElement("profile_COMMON");
    effectXml->LinkEndChild(profileCommonXml);

    // Image
    const gz::common::MaterialPtr material =
        this->mesh->MaterialByIndex(i);
    std::string imageString = material->TextureImage();

    if (imageString.find(pathSeparator) != std::string::npos)
    {
      tinyxml2::XMLElement *newParamXml =
        _libraryEffectsXml->GetDocument()->NewElement("newparam");
      snprintf(id, sizeof(id), "image_%u_surface", i);
      newParamXml->SetAttribute("sid", id);
      profileCommonXml->LinkEndChild(newParamXml);

      tinyxml2::XMLElement *surfaceXml =
        _libraryEffectsXml->GetDocument()->NewElement("surface");
      surfaceXml->SetAttribute("type", "2D");
      newParamXml->LinkEndChild(surfaceXml);

      tinyxml2::XMLElement *initFromXml =
        _libraryEffectsXml->GetDocument()->NewElement("init_from");
      snprintf(id, sizeof(id), "image_%u", i);
      initFromXml->LinkEndChild(_libraryEffectsXml->GetDocument()->NewText(id));
      surfaceXml->LinkEndChild(initFromXml);

      newParamXml = _libraryEffectsXml->GetDocument()->NewElement("newparam");
      snprintf(id, sizeof(id), "image_%u_sampler", i);
      newParamXml->SetAttribute("sid", id);
      profileCommonXml->LinkEndChild(newParamXml);

      tinyxml2::XMLElement *sampler2dXml =
        _libraryEffectsXml->GetDocument()->NewElement("sampler2D");
      newParamXml->LinkEndChild(sampler2dXml);

      tinyxml2::XMLElement *sourceXml =
        _libraryEffectsXml->GetDocument()->NewElement("source");
      snprintf(id, sizeof(id), "image_%u_surface", i);
      sourceXml->LinkEndChild(_libraryEffectsXml->GetDocument()->NewText(id));
      sampler2dXml->LinkEndChild(sourceXml);

      tinyxml2::XMLElement *minFilterXml =
        _libraryEffectsXml->GetDocument()->NewElement("minfilter");
      minFilterXml->LinkEndChild(
          _libraryEffectsXml->GetDocument()->NewText("LINEAR"));
      sampler2dXml->LinkEndChild(minFilterXml);

      tinyxml2::XMLElement *magFilterXml =
        _libraryEffectsXml->GetDocument()->NewElement("magfilter");
      magFilterXml->LinkEndChild(
          _libraryEffectsXml->GetDocument()->NewText("LINEAR"));
      sampler2dXml->LinkEndChild(magFilterXml);
    }

    tinyxml2::XMLElement *techniqueXml =
      _libraryEffectsXml->GetDocument()->NewElement("technique");
    techniqueXml->SetAttribute("sid", "COMMON");
    profileCommonXml->LinkEndChild(techniqueXml);

    // Using phong for now
    tinyxml2::XMLElement *phongXml =
      _libraryEffectsXml->GetDocument()->NewElement("phong");
    techniqueXml->LinkEndChild(phongXml);

    // ambient
    unsigned int RGBAcolor = material->Ambient().AsRGBA();
    float r = ((RGBAcolor >> 24) & 0xFF) / 255.0f;
    float g = ((RGBAcolor >> 16) & 0xFF) / 255.0f;
    float b = ((RGBAcolor >> 8) & 0xFF) / 255.0f;
    float a = (RGBAcolor & 0xFF) / 255.0f;

    tinyxml2::XMLElement *ambientXml =
      _libraryEffectsXml->GetDocument()->NewElement("ambient");
    phongXml->LinkEndChild(ambientXml);

    tinyxml2::XMLElement *colorXml =
      _libraryEffectsXml->GetDocument()->NewElement("color");
    snprintf(id, sizeof(id), "%f %f %f %f", r, g, b, a);
    colorXml->LinkEndChild(_libraryEffectsXml->GetDocument()->NewText(id));
    ambientXml->LinkEndChild(colorXml);

    // emission
    RGBAcolor = material->Emissive().AsRGBA();
    r = ((RGBAcolor >> 24) & 0xFF) / 255.0f;
    g = ((RGBAcolor >> 16) & 0xFF) / 255.0f;
    b = ((RGBAcolor >> 8) & 0xFF) / 255.0f;
    a = (RGBAcolor & 0xFF) / 255.0f;

    tinyxml2::XMLElement *emissionXml =
      _libraryEffectsXml->GetDocument()->NewElement("emission");
    phongXml->LinkEndChild(emissionXml);

    colorXml = _libraryEffectsXml->GetDocument()->NewElement("color");
    snprintf(id, sizeof(id), "%f %f %f %f", r, g, b, a);
    colorXml->LinkEndChild(_libraryEffectsXml->GetDocument()->NewText(id));
    emissionXml->LinkEndChild(colorXml);

    // diffuse
    tinyxml2::XMLElement *diffuseXml =
      _libraryEffectsXml->GetDocument()->NewElement("diffuse");
    phongXml->LinkEndChild(diffuseXml);

    if (imageString.find(pathSeparator) != std::string::npos)
    {
      tinyxml2::XMLElement *textureXml =
        _libraryEffectsXml->GetDocument()->NewElement("texture");
      snprintf(id, sizeof(id), "image_%u_sampler", i);
      textureXml->SetAttribute("texture", id);
      textureXml->SetAttribute("texcoord", "UVSET0");
      diffuseXml->LinkEndChild(textureXml);
    }
    else
    {
      RGBAcolor = material->Diffuse().AsRGBA();
      r = ((RGBAcolor >> 24) & 0xFF) / 255.0f;
      g = ((RGBAcolor >> 16) & 0xFF) / 255.0f;
      b = ((RGBAcolor >> 8) & 0xFF) / 255.0f;
      a = (RGBAcolor & 0xFF) / 255.0f;

      colorXml = _libraryEffectsXml->GetDocument()->NewElement("color");
      snprintf(id, sizeof(id), "%f %f %f %f", r, g, b, a);
      colorXml->LinkEndChild(_libraryEffectsXml->GetDocument()->NewText(id));
      diffuseXml->LinkEndChild(colorXml);
    }

    // specular
    RGBAcolor = material->Specular().AsRGBA();
    r = ((RGBAcolor >> 24) & 0xFF) / 255.0f;
    g = ((RGBAcolor >> 16) & 0xFF) / 255.0f;
    b = ((RGBAcolor >> 8) & 0xFF) / 255.0f;
    a = (RGBAcolor & 0xFF) / 255.0f;

    tinyxml2::XMLElement *specularXml =
      _libraryEffectsXml->GetDocument()->NewElement("specular");
    phongXml->LinkEndChild(specularXml);

    colorXml = _libraryEffectsXml->GetDocument()->NewElement("color");
    snprintf(id, sizeof(id), "%f %f %f %f", r, g, b, a);
    colorXml->LinkEndChild(_libraryEffectsXml->GetDocument()->NewText(id));
    specularXml->LinkEndChild(colorXml);

    // transparency
    // In Collada, a transparency value of 1 is not transparent and 0 is
    // fully transparent.
    double transp = 1.0 - material->Transparency();

    tinyxml2::XMLElement *transparencyXml =
      _libraryEffectsXml->GetDocument()->NewElement("transparency");
    phongXml->LinkEndChild(transparencyXml);

    tinyxml2::XMLElement *floatXml =
      _libraryEffectsXml->GetDocument()->NewElement("float");
    snprintf(id, sizeof(id), "%f", transp);
    floatXml->LinkEndChild(_libraryEffectsXml->GetDocument()->NewText(id));
    transparencyXml->LinkEndChild(floatXml);

    // shininess
    double shine = material->Shininess();

    tinyxml2::XMLElement *shininessXml =
      _libraryEffectsXml->GetDocument()->NewElement("shininess");
    phongXml->LinkEndChild(shininessXml);

    colorXml = _libraryEffectsXml->GetDocument()->NewElement("color");
    snprintf(id, sizeof(id), "%f", shine);
    colorXml->LinkEndChild(_libraryEffectsXml->GetDocument()->NewText(id));
    shininessXml->LinkEndChild(colorXml);
  }
}

//////////////////////////////////////////////////
void ColladaExporter::Implementation::ExportVisualScenes(
    tinyxml2::XMLElement *_libraryVisualScenesXml,
    const std::vector<math::Matrix4d> &_submeshToMatrix,
    const std::vector<ColladaLight> &_lights)
{
  tinyxml2::XMLElement *visualSceneXml =
    _libraryVisualScenesXml->GetDocument()->NewElement("visual_scene");
  _libraryVisualScenesXml->LinkEndChild(visualSceneXml);
  visualSceneXml->SetAttribute("name", "Scene");
  visualSceneXml->SetAttribute("id", "Scene");

  for (unsigned int i = 0; i < this->subMeshCount; ++i)
  {
    char meshId[100], attributeValue[101], nodeId[106];

    snprintf(meshId, sizeof(meshId), "mesh_%u", i);
    snprintf(nodeId, sizeof(nodeId), "node_%u", i);

    tinyxml2::XMLElement *nodeXml =
      _libraryVisualScenesXml->GetDocument()->NewElement("node");
    visualSceneXml->LinkEndChild(nodeXml);

    nodeXml->SetAttribute("name", nodeId);
    nodeXml->SetAttribute("id", nodeId);

    if (i < _submeshToMatrix.size())
    {
      std::ostringstream fillData;
      fillData.precision(8);
      fillData << std::fixed;

      fillData << _submeshToMatrix.at(i);

      tinyxml2::XMLElement *matrixXml =
        _libraryVisualScenesXml->GetDocument()->NewElement("matrix");
      nodeXml->LinkEndChild(matrixXml);
      matrixXml->LinkEndChild(_libraryVisualScenesXml->GetDocument()->NewText(
            fillData.str().c_str()));
    }

    tinyxml2::XMLElement *instanceGeometryXml =
      _libraryVisualScenesXml->GetDocument()->NewElement("instance_geometry");
    nodeXml->LinkEndChild(instanceGeometryXml);
    snprintf(attributeValue, sizeof(attributeValue), "#%s", meshId);
    instanceGeometryXml->SetAttribute("url", attributeValue);

    const auto subMesh = this->mesh->SubMeshByIndex(i).lock();
    if (!subMesh->GetMaterialIndex())
      continue;
    const auto materialIndex = subMesh->GetMaterialIndex().value();
    const gz::common::MaterialPtr material =
      this->mesh->MaterialByIndex(materialIndex);

    if (material)
    {
      char materialId[100];

      snprintf(materialId, sizeof(materialId), "material_%u", materialIndex);

      tinyxml2::XMLElement *bindMaterialXml =
        _libraryVisualScenesXml->GetDocument()->NewElement("bind_material");
      instanceGeometryXml->LinkEndChild(bindMaterialXml);

      tinyxml2::XMLElement *techniqueCommonXml =
        _libraryVisualScenesXml->GetDocument()->NewElement("technique_common");
      bindMaterialXml->LinkEndChild(techniqueCommonXml);

      tinyxml2::XMLElement *instanceMaterialXml =
        _libraryVisualScenesXml->GetDocument()->NewElement("instance_material");
      techniqueCommonXml->LinkEndChild(instanceMaterialXml);
      instanceMaterialXml->SetAttribute("symbol", materialId);
      snprintf(attributeValue, sizeof(attributeValue), "#%s", materialId);
      instanceMaterialXml->SetAttribute("target", attributeValue);

      std::string imageString = material->TextureImage();

      if (imageString.find(pathSeparator) != std::string::npos)
      {
        tinyxml2::XMLElement *bindVertexInputXml =
          _libraryVisualScenesXml->GetDocument()->NewElement(
              "bind_vertex_input");
        instanceMaterialXml->LinkEndChild(bindVertexInputXml);
        bindVertexInputXml->SetAttribute("semantic", "UVSET0");
        bindVertexInputXml->SetAttribute("input_semantic", "TEXCOORD");
      }
    }
  }

  for (const auto& light : _lights)
  {
    tinyxml2::XMLElement *nodeXml =
      _libraryVisualScenesXml->GetDocument()->NewElement("node");
    visualSceneXml->LinkEndChild(nodeXml);

    nodeXml->SetAttribute("name", light.name.c_str());

    tinyxml2::XMLElement *lightInstXml =
      _libraryVisualScenesXml->GetDocument()->NewElement("instance_light");
    char lightId[100] = { 0 };
    snprintf(lightId, sizeof(lightId), "#%s", light.name.c_str());
    lightInstXml->SetAttribute("url", lightId);
    nodeXml->LinkEndChild(lightInstXml);

    const auto& position = light.position;
    // translation
    {
      tinyxml2::XMLElement *translateXml =
        _libraryVisualScenesXml->GetDocument()->NewElement("translate");
      translateXml->SetAttribute("sid", "translate");

      char tx_value[100] = { 0 };
      snprintf(tx_value, sizeof(tx_value), "%f %f %f",
        position.X(), position.Y(), position.Z());
      translateXml->SetText(tx_value);
      nodeXml->LinkEndChild(translateXml);
    }

    // rotation; blender starts off with light direction (0,0,-1),
    // we output an axis-angle rotation to our desired direction
    {
      auto lightdir_norm = light.direction.Normalized();
      math::Vector3d initial_dir(0, 0, -1);
      math::Quaterniond q;
      q.SetFrom2Axes(initial_dir, lightdir_norm);

      math::Vector3d axis;
      double angle = 0.0;
      q.AxisAngle(axis, angle);

      tinyxml2::XMLElement *rotateXml =
        _libraryVisualScenesXml->GetDocument()->NewElement("rotate");

      char str[100] = { 0 };
      snprintf(str, sizeof(str), "%g %g %g %g",
        axis.X(), axis.Y(), axis.Z(), angle / GZ_PI * 180.0);
      rotateXml->SetText(str);
      nodeXml->LinkEndChild(rotateXml);
    }
  }
}

//////////////////////////////////////////////////
void ColladaExporter::Implementation::ExportScene(
    tinyxml2::XMLElement *_sceneXml)
{
  tinyxml2::XMLElement *instanceVisualSceneXml =
      _sceneXml->GetDocument()->NewElement("instance_visual_scene");
  _sceneXml->LinkEndChild(instanceVisualSceneXml);
  instanceVisualSceneXml->SetAttribute("url", "#Scene");
}
