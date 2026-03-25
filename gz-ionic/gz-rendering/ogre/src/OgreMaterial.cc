/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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

#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>

#include "gz/rendering/InstallationDirectories.hh"
#include "gz/rendering/ShaderParams.hh"
#include "gz/rendering/ogre/OgreMaterial.hh"
#include "gz/rendering/ogre/OgreConversions.hh"
#include "gz/rendering/ogre/OgreRenderEngine.hh"
#include "gz/rendering/ogre/OgreRTShaderSystem.hh"
#include "gz/rendering/ogre/OgreScene.hh"

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
OgreMaterial::OgreMaterial()
{
}

//////////////////////////////////////////////////
OgreMaterial::~OgreMaterial()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void OgreMaterial::Destroy()
{
  if (!this->Scene()->IsInitialized())
    return;
  std::string materialName;
  Ogre::MaterialManager &matManager = Ogre::MaterialManager::getSingleton();
#if OGRE_VERSION_LT_1_11_0
  if (!this->ogreMaterial.isNull())
  {
    materialName = this->ogreMaterial->getName();

    this->ogreTexState->setBlank();
    auto indexUnitStateToRemove =
      this->ogrePass->getTextureUnitStateIndex(this->ogreTexState);
    this->ogrePass->removeTextureUnitState(indexUnitStateToRemove);

    matManager.remove(this->ogreMaterial->getName());
    this->ogreMaterial.setNull();
  }
#else
  if (this->ogreMaterial)
  {
    matManager.remove(this->ogreMaterial->getName());
    this->ogreMaterial.reset();
  }
#endif
  auto &textureManager = Ogre::TextureManager::getSingleton();
  const auto iend = textureManager.getResourceIterator().end();
  for (auto i = textureManager.getResourceIterator().begin(); i != iend; ++i)
  {
    // A use count of 4 means that only RGM, RM and MeshManager have
    // references RGM has one (this one) and RM has 2 (by name and by handle)
    // and MeshManager keep another one int the template
    Ogre::Resource* res = i->second.get();
    if (i->second.useCount() == 4)
    {
      if (this->textureName == res->getName() &&
        res->getName().find(
          scene->Name() + "::RenderTexture") == std::string::npos)
      {
        OgreScenePtr s = std::dynamic_pointer_cast<OgreScene>(this->Scene());
        s->ClearMaterialsCache(this->textureName);
        this->Scene()->UnregisterMaterial(materialName);
        if (i->second.useCount() == 3)
        {
          textureManager.remove(res->getHandle());
        }
        break;
      }
    }
  }
}

//////////////////////////////////////////////////
bool OgreMaterial::LightingEnabled() const
{
  return this->ogrePass->getLightingEnabled();
}

//////////////////////////////////////////////////
void OgreMaterial::SetLightingEnabled(bool _enabled)
{
  this->ogrePass->setLightingEnabled(_enabled);
  this->UpdateColorOperation();
}

//////////////////////////////////////////////////
void OgreMaterial::SetDepthCheckEnabled(bool _enabled)
{
  this->ogrePass->setDepthCheckEnabled(_enabled);
}

//////////////////////////////////////////////////
void OgreMaterial::SetDepthWriteEnabled(bool _enabled)
{
  this->ogrePass->setDepthWriteEnabled(_enabled);
}

//////////////////////////////////////////////////
bool OgreMaterial::DepthCheckEnabled() const
{
  return this->ogrePass->getDepthCheckEnabled();
}

//////////////////////////////////////////////////
bool OgreMaterial::DepthWriteEnabled() const
{
  return this->ogrePass->getDepthWriteEnabled();
}

//////////////////////////////////////////////////
math::Color OgreMaterial::Ambient() const
{
  return OgreConversions::Convert(this->ogrePass->getAmbient());
}

//////////////////////////////////////////////////
void OgreMaterial::SetAmbient(const math::Color &_color)
{
  this->ogrePass->setAmbient(OgreConversions::Convert(_color));
  this->UpdateColorOperation();
  this->UpdateTransparency();
}

//////////////////////////////////////////////////
math::Color OgreMaterial::Diffuse() const
{
  return OgreConversions::Convert(this->ogrePass->getDiffuse());
}

//////////////////////////////////////////////////
void OgreMaterial::SetDiffuse(const math::Color &_color)
{
  this->ogrePass->setDiffuse(OgreConversions::Convert(_color));
}

//////////////////////////////////////////////////
math::Color OgreMaterial::Specular() const
{
  return OgreConversions::Convert(this->ogrePass->getSpecular());
}

//////////////////////////////////////////////////
void OgreMaterial::SetSpecular(const math::Color &_color)
{
  this->ogrePass->setSpecular(OgreConversions::Convert(_color));
}

//////////////////////////////////////////////////
math::Color OgreMaterial::Emissive() const
{
#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR <= 7
  return this->emissiveColor;
#else
  return OgreConversions::Convert(this->ogrePass->getEmissive());
#endif
}

//////////////////////////////////////////////////
void OgreMaterial::SetEmissive(const math::Color &_color)
{
#if OGRE_VERSION_MAJOR == 1 && OGRE_VERSION_MINOR <= 7
  this->emissiveColor = _color;
#else
  this->ogrePass->setEmissive(OgreConversions::Convert(_color));
#endif
}

//////////////////////////////////////////////////
float OgreMaterial::RenderOrder() const
{
  return this->renderOrder;
}

//////////////////////////////////////////////////
void OgreMaterial::SetRenderOrder(const float _renderOrder)
{
  this->renderOrder = _renderOrder;
  this->ogrePass->setDepthBias(this->renderOrder);
}

//////////////////////////////////////////////////
double OgreMaterial::Shininess() const
{
  return this->shininess;
}

//////////////////////////////////////////////////
void OgreMaterial::SetShininess(const double _shininess)
{
  this->shininess = _shininess;
  this->ogrePass->setShininess(this->shininess);
}

//////////////////////////////////////////////////
double OgreMaterial::Transparency() const
{
  return this->transparency;
}

//////////////////////////////////////////////////
void OgreMaterial::SetTransparency(const double _transparency)
{
  this->transparency = std::min(std::max(_transparency, 0.0), 1.0);
  this->UpdateTransparency();
}

//////////////////////////////////////////////////
double OgreMaterial::Reflectivity() const
{
  return this->reflectivity;
}

//////////////////////////////////////////////////
void OgreMaterial::SetReflectivity(const double _reflectivity)
{
  this->reflectivity = std::min(std::max(_reflectivity, 0.0), 1.0);
}

//////////////////////////////////////////////////
bool OgreMaterial::CastShadows() const
{
  return this->castShadows;
}

//////////////////////////////////////////////////
void OgreMaterial::SetCastShadows(const bool _castShadows)
{
  // TODO(anyone): update RTShader
  this->castShadows = _castShadows;
}

//////////////////////////////////////////////////
bool OgreMaterial::ReceiveShadows() const
{
  return this->ogreMaterial->getReceiveShadows();
}

//////////////////////////////////////////////////
void OgreMaterial::SetReceiveShadows(const bool _receiveShadows)
{
  this->ogreMaterial->setReceiveShadows(_receiveShadows);
}

//////////////////////////////////////////////////
bool OgreMaterial::ReflectionEnabled() const
{
  return this->reflectionEnabled;
}

//////////////////////////////////////////////////
void OgreMaterial::SetReflectionEnabled(const bool _enabled)
{
  this->reflectionEnabled = _enabled;
}

//////////////////////////////////////////////////
bool OgreMaterial::HasTexture() const
{
  return !this->textureName.empty();
}

//////////////////////////////////////////////////
std::string OgreMaterial::Texture() const
{
  return this->textureName;
}

//////////////////////////////////////////////////
void OgreMaterial::SetTexture(const std::string &_name,
                              const std::shared_ptr<const common::Image> &_img)
{
  if (_name.empty())
  {
    this->ClearTexture();
    return;
  }

  this->textureName = _name;
  this->textureData = _img;
  if (_img == nullptr)
    this->SetTextureImpl(this->textureName);
  else
    this->SetTextureDataImpl(this->textureName, _img);
}

//////////////////////////////////////////////////
std::shared_ptr<const common::Image> OgreMaterial::TextureData() const
{
  return this->textureData;
}

//////////////////////////////////////////////////
void OgreMaterial::ClearTexture()
{
  this->textureName = "";
  this->textureData = nullptr;
  this->ogreTexState->setBlank();
  this->UpdateColorOperation();
}

//////////////////////////////////////////////////
bool OgreMaterial::HasNormalMap() const
{
  return !this->normalMapName.empty();
}

//////////////////////////////////////////////////
std::string OgreMaterial::NormalMap() const
{
  return this->normalMapName;
}

//////////////////////////////////////////////////
std::shared_ptr<const common::Image> OgreMaterial::NormalMapData() const
{
  return this->normalMapData;
}

//////////////////////////////////////////////////
void OgreMaterial::SetNormalMap(const std::string &_name,
  const std::shared_ptr<const common::Image>& _img)
{
  if (_name.empty())
  {
    this->ClearNormalMap();
    return;
  }

  this->normalMapName = _name;
  this->normalMapData = _img;
  // TODO(anyone): implement
  // this->SetNormalMapImpl(texture);
}

//////////////////////////////////////////////////
void OgreMaterial::ClearNormalMap()
{
  this->normalMapName = "";
  this->normalMapData = nullptr;
}

//////////////////////////////////////////////////
enum ShaderType OgreMaterial::ShaderType() const
{
  return this->shaderType;
}

//////////////////////////////////////////////////
void OgreMaterial::SetShaderType(enum ShaderType _type)
{
  this->shaderType = (ShaderUtil::IsValid(_type)) ? _type : ST_PIXEL;
}

//////////////////////////////////////////////////
void OgreMaterial::PreRender()
{
  this->UpdateShaderParams();
}

//////////////////////////////////////////////////
void OgreMaterial::UpdateShaderParams()
{
  if (this->vertexShaderParams && this->vertexShaderParams->IsDirty())
  {
    Ogre::GpuProgramParametersSharedPtr ogreParams;
    ogreParams = this->ogrePass->getVertexProgramParameters();
    this->UpdateShaderParams(this->vertexShaderParams, ogreParams);
    this->vertexShaderParams->ClearDirty();
  }
  if (this->fragmentShaderParams && this->fragmentShaderParams->IsDirty())
  {
    Ogre::GpuProgramParametersSharedPtr ogreParams;
    ogreParams = this->ogrePass->getFragmentProgramParameters();
    this->UpdateShaderParams(this->fragmentShaderParams, ogreParams);
    this->fragmentShaderParams->ClearDirty();
  }
}

//////////////////////////////////////////////////
void OgreMaterial::UpdateShaderParams(ConstShaderParamsPtr _params,
    Ogre::GpuProgramParametersSharedPtr _ogreParams)
{
  for (const auto &name_param : *_params)
  {
    auto *constantDef =
        Ogre::GpuProgramParameters::getAutoConstantDefinition(name_param.first);
    if (constantDef)
    {
      _ogreParams->setNamedAutoConstant(name_param.first, constantDef->acType);
      continue;
    }

    if (!_ogreParams->_findNamedConstantDefinition(name_param.first) &&
        !(OgreRenderEngine::Instance()->GraphicsAPI() !=
            GraphicsAPI::OPENGL &&
            (ShaderParam::PARAM_TEXTURE == name_param.second.Type() ||
             ShaderParam::PARAM_TEXTURE_CUBE == name_param.second.Type())))
    {
      gzwarn << "Unable to find GPU program parameter: "
             << name_param.first << std::endl;
      continue;
    }

    if (ShaderParam::PARAM_FLOAT == name_param.second.Type())
    {
      float value;
      name_param.second.Value(&value);
      _ogreParams->setNamedConstant(name_param.first, value);
    }
    else if (ShaderParam::PARAM_INT == name_param.second.Type())
    {
      int value;
      name_param.second.Value(&value);
      _ogreParams->setNamedConstant(name_param.first, value);
    }
    else if (ShaderParam::PARAM_FLOAT_BUFFER == name_param.second.Type())
    {
      std::shared_ptr<void> buffer;
      name_param.second.Buffer(buffer);
      uint32_t count = name_param.second.Count();

      // multiple other than 4 is currently only supported by GLSL
      uint32_t multiple = 1;
      _ogreParams->setNamedConstant(name_param.first,
          reinterpret_cast<float*>(buffer.get()), count, multiple);
    }
    else if (ShaderParam::PARAM_INT_BUFFER == name_param.second.Type())
    {
      std::shared_ptr<void> buffer;
      name_param.second.Buffer(buffer);
      uint32_t count = name_param.second.Count();

      // multiple other than 4 is currently only supported by GLSL
      uint32_t multiple = 1;
      _ogreParams->setNamedConstant(name_param.first,
        reinterpret_cast<int*>(buffer.get()), count, multiple);
    }
    else if (ShaderParam::PARAM_TEXTURE == name_param.second.Type() ||
             ShaderParam::PARAM_TEXTURE_CUBE == name_param.second.Type())
    {
      // add the textures to the resource path
      std::string value;
      uint32_t uvSetIndex = 0;
      name_param.second.Value(value, uvSetIndex);
      ShaderParam::ParamType type = name_param.second.Type();

      std::string baseName = value;
      std::string dirPath = value;
      if (common::isFile(value))
      {
        baseName = common::basename(value);
        size_t idx = value.rfind(baseName);
        if (idx != std::string::npos)
        {
          dirPath = value.substr(0, idx);
          if (!dirPath.empty() &&
            !Ogre::ResourceGroupManager::getSingleton().resourceLocationExists(
            dirPath))
          {
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                dirPath, "FileSystem", this->ogreGroup);
          }
        }
      }
      else
      {
        gzerr << "Shader param texture not found: " << value << std::endl;
        continue;
      }

      // get the material and create the texture unit state if it does not exist
      auto texUnit = this->ogrePass->getTextureUnitState(name_param.first);
      if (!texUnit)
      {
        texUnit = this->ogrePass->createTextureUnitState();
        texUnit->setName(name_param.first);
      }
      // make sure to cast to int before calling setNamedConstant later
      // to set the texture index
      int texIndex = static_cast<int>(
          this->ogrePass->getTextureUnitStateIndex(texUnit));

      // set texture coordinate set
      texUnit->setTextureCoordSet(uvSetIndex);

      // regular 2d texture
      if (type == ShaderParam::ParamType::PARAM_TEXTURE)
      {
        texUnit->setTextureName(baseName);
      }
      // cube maps
      else if (type == ShaderParam::ParamType::PARAM_TEXTURE_CUBE)
      {
        texUnit->setCubicTextureName(baseName, true);
        // must apply this check for Metal rendering to work
        // (i.e. not segfault). See the discussion in:
        // https://github.com/gazebosim/gz-rendering/pull/541
        if (texUnit->isLoaded())
        {
          texUnit->_load();
        }
      }
      else
      {
        gzerr << "Unrecognized texture type set for shader param: "
               << name_param.first << std::endl;
        continue;
      }
      if (OgreRenderEngine::Instance()->GraphicsAPI() ==
          GraphicsAPI::OPENGL)
      {
        // set the texture map index
        _ogreParams->setNamedConstant(name_param.first, &texIndex, 1, 1);
      }
    }
  }
}

//////////////////////////////////////////////////
void OgreMaterial::SetVertexShader(const std::string &_path)
{
  if (_path.empty())
    return;

  if (!common::exists(_path))
  {
    gzerr << "Vertex shader path does not exist: " << _path << std::endl;
    return;
  }

  Ogre::ResourceGroupManager::getSingleton().addResourceLocation(_path,
  "FileSystem", "General", false);

  Ogre::HighLevelGpuProgramPtr vertexShader =
    Ogre::HighLevelGpuProgramManager::getSingletonPtr()->createProgram(
        "__gz_rendering_vertex__" + _path,
        this->ogreGroup,
        "glsl", Ogre::GpuProgramType::GPT_VERTEX_PROGRAM);

  vertexShader->setSourceFile(_path);
  vertexShader->load();

  assert(vertexShader->isLoaded());
  assert(!(vertexShader->hasCompileError()));
  assert(vertexShader->isSupported());

  this->ogrePass->setVertexProgram(vertexShader->getName());

  this->ogreMaterial->compile();
  this->ogreMaterial->load();

  this->vertexShaderPath = _path;
  this->vertexShaderParams.reset(new ShaderParams);
}

//////////////////////////////////////////////////
std::string OgreMaterial::VertexShader() const
{
  return this->vertexShaderPath;
}

//////////////////////////////////////////////////
ShaderParamsPtr OgreMaterial::VertexShaderParams()
{
  return this->vertexShaderParams;
}

//////////////////////////////////////////////////
void OgreMaterial::SetFragmentShader(const std::string &_path)
{
  if (_path.empty())
    return;

  if (!common::exists(_path))
  {
    gzerr << "Fragment shader path does not exist: " << _path << std::endl;
    return;
  }

  Ogre::ResourceGroupManager::getSingleton().addResourceLocation(_path,
  "FileSystem", "General", false);

  Ogre::HighLevelGpuProgramPtr fragmentShader =
    Ogre::HighLevelGpuProgramManager::getSingleton().createProgram(
        "__gz_rendering_fragment__" + _path,
        this->ogreGroup,
        "glsl", Ogre::GpuProgramType::GPT_FRAGMENT_PROGRAM);

  fragmentShader->setSourceFile(_path);
  fragmentShader->load();

  assert(fragmentShader->isLoaded());
  assert(!(fragmentShader->hasCompileError()));
  assert(fragmentShader->isSupported());

  this->ogrePass->setFragmentProgram(fragmentShader->getName());

  this->ogreMaterial->setLightingEnabled(false);

  this->ogreMaterial->compile();
  this->ogreMaterial->load();

  this->fragmentShaderPath = _path;
  this->fragmentShaderParams.reset(new ShaderParams);
}

//////////////////////////////////////////////////
std::string OgreMaterial::FragmentShader() const
{
  return this->fragmentShaderPath;
}

//////////////////////////////////////////////////
ShaderParamsPtr OgreMaterial::FragmentShaderParams()
{
  return this->fragmentShaderParams;
}

//////////////////////////////////////////////////
Ogre::MaterialPtr OgreMaterial::Material() const
{
  return this->ogreMaterial;
}

//////////////////////////////////////////////////
void OgreMaterial::LoadOneImage(const std::string &_name, Ogre::Image &_image)
{
  try
  {
    if (Ogre::ResourceGroupManager::getSingleton().resourceExists(
        this->ogreGroup, _name))
    {
      _image.load(_name, this->ogreGroup);
    }
    else
    {
      std::string path = common::findFile(_name);
      if (!path.empty())
      {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
            path, "FileSystem", this->ogreGroup);
        _image.load(path, this->ogreGroup);
      }
      else
        gzerr << "Unable to find texture image: " << _name << std::endl;
    }
  }
  catch (const Ogre::Exception &ex)
  {
    gzerr << "Unable to load texture image: " << ex.what() << std::endl;
  }
}

//////////////////////////////////////////////////
void OgreMaterial::SetTextureImpl(const std::string &_texture)
{
  if (!Ogre::ResourceGroupManager::getSingleton().resourceExists(
      this->ogreGroup, _texture))
  {
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        _texture, "FileSystem", this->ogreGroup);
  }

  this->ogreTexState->setTextureName(_texture);
  this->UpdateColorOperation();
}

//////////////////////////////////////////////////
void OgreMaterial::SetTextureDataImpl(const std::string &_texture,
  const std::shared_ptr<const common::Image> &_img)
{
  // Create the texture only if it was not created already
  if (!Ogre::ResourceGroupManager::getSingleton().resourceExists(
      this->ogreGroup, _texture))
  {
    auto ogreTexture =
    Ogre::TextureManager::getSingleton().createManual(
        _texture,
        "General",
        Ogre::TEX_TYPE_2D,
        _img->Width(),
        _img->Height(),
        0,
        Ogre::PF_R8G8B8A8);
    Ogre::HardwarePixelBufferSharedPtr pixelBuffer = ogreTexture->getBuffer();
    pixelBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
    const Ogre::PixelBox &pixelBox = pixelBuffer->getCurrentLock();

    auto data = _img->RGBAData();
    // TODO(anyone) Why we need to switch red and blue again for OGRE1?
    for (unsigned int r = 0; r < _img->Height(); ++r)
    {
      for (unsigned int c = 0; c < _img->Width(); ++c)
      {
        int pixIdx = (r * _img->Width() + c) * 4;
        std::swap(data[pixIdx], data[pixIdx + 2]);
      }
    }

    memcpy(pixelBox.data, &data[0], data.size());
    pixelBuffer->unlock();
  }

  this->ogreTexState->setTextureName(_texture);
  this->UpdateColorOperation();
}

//////////////////////////////////////////////////
Ogre::TexturePtr OgreMaterial::Texture(const std::string &_name)
{
  Ogre::TextureManager &texManager = Ogre::TextureManager::getSingleton();

  if (texManager.resourceExists(_name))
  {
    return texManager.getByName(_name);
  }

  return this->CreateTexture(_name);
}

//////////////////////////////////////////////////
Ogre::TexturePtr OgreMaterial::CreateTexture(const std::string &_name)
{
  Ogre::Image image;
  Ogre::TexturePtr texture;

  this->LoadOneImage(_name, image);

  if (image.getWidth() == 0)
  {
    #if OGRE_VERSION_LT_1_11_0
    texture.setNull();
    #else
    texture.reset();
    #endif
    return texture;
  }

  texture = Ogre::TextureManager::getSingleton().createManual(_name,
      this->ogreGroup, Ogre::TEX_TYPE_2D, image.getWidth(),
      image.getHeight(), 0, Ogre::PF_X8R8G8B8);

  texture->loadImage(image);
  return texture;
}

//////////////////////////////////////////////////
void OgreMaterial::UpdateTransparency()
{
  Ogre::ColourValue color = this->ogrePass->getAmbient();
  double alpha = (1 - this->transparency) * color.a;

  if (alpha < 1 || this->textureAlphaEnabled)
  {
    this->ogrePass->setDepthWriteEnabled(false);
    this->ogrePass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);

    if (!this->textureAlphaEnabled)
    {
      this->ogreTexState->setAlphaOperation(Ogre::LBX_SOURCE1, Ogre::LBS_MANUAL,
          Ogre::LBS_CURRENT, alpha);
    }
  }
  else
  {
    this->ogrePass->setDepthWriteEnabled(true);
    this->ogrePass->setDepthCheckEnabled(true);
    this->ogrePass->setSceneBlending(Ogre::SBT_REPLACE);
  }
  if (this->twoSidedEnabled)
    this->ogrePass->setCullingMode(Ogre::CULL_NONE);
  else
    this->ogrePass->setCullingMode(Ogre::CULL_CLOCKWISE);
}

//////////////////////////////////////////////////
void OgreMaterial::SetAlphaFromTexture(bool _enabled,
  double _alpha, bool _twoSided)
{
  // TODO(anyone) Implement alpha testing for shadow caster pass
  BaseMaterial::SetAlphaFromTexture(_enabled, _alpha, _twoSided);
  this->UpdateTransparency();
}

//////////////////////////////////////////////////
void OgreMaterial::SetDepthMaterial(const double _far,
  const double _near)
{
  // Configure Ogre Pass settings for Depth
  this->ogrePass->setDepthCheckEnabled(false);
  this->ogrePass->setDepthWriteEnabled(false);
  this->ogrePass->setLightingEnabled(false);
  this->ogrePass->setFog(true, Ogre::FOG_NONE);

  // TODO(anyone): convert depth configuration into a ShaderType
  // Get shader parameters path
  const char *env = std::getenv("GZ_RENDERING_RESOURCE_PATH");

  std::string resourcePath = (env) ? std::string(env) :
      gz::rendering::getResourcePath();

  // path to look for vertex and fragment shader parameters
  std::string depth_vertex_shader_path = common::joinPaths(
      resourcePath, "ogre", "media", "materials", "programs",
      depth_vertex_shader_file);

  std::string depth_fragment_shader_path = common::joinPaths(
      resourcePath, "ogre", "media", "materials", "programs",
      depth_fragment_shader_file);

  this->SetVertexShader(depth_vertex_shader_path);
  this->SetFragmentShader(depth_fragment_shader_path);

  // Configure fragment shader variables
  // Note: MSVC was not happy with one line commands, be sure
  // of checking it if you change the lines below
  auto farShaderParams = (*this->fragmentShaderParams)["pfar"];
  auto nearShaderParams = (*this->fragmentShaderParams)["pnear"];
  farShaderParams = static_cast<float>(_far);
  nearShaderParams = static_cast<float>(_near);
}

//////////////////////////////////////////////////
void OgreMaterial::UpdateColorOperation()
{
  Ogre::LayerBlendOperationEx operation;
  Ogre::LayerBlendSource source1;
  Ogre::LayerBlendSource source2;
  Ogre::ColourValue color;

  bool texOff = !this->HasTexture();
  bool lightOff = !this->LightingEnabled();

  operation = (texOff) ? Ogre::LBX_SOURCE1 : Ogre::LBX_MODULATE;
  source1 = (texOff && lightOff) ? Ogre::LBS_MANUAL : Ogre::LBS_CURRENT;
  source2 = Ogre::LBS_TEXTURE;
  color = this->ogrePass->getAmbient();

  this->ogreTexState->setColourOperationEx(operation, source1, source2, color);
}

//////////////////////////////////////////////////
void OgreMaterial::Init()
{
  BaseMaterial::Init();
  this->ogreGroup = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME;
  Ogre::MaterialManager &matManager = Ogre::MaterialManager::getSingleton();
  this->ogreMaterial = matManager.create(this->name, this->ogreGroup);
  this->ogreTechnique = this->ogreMaterial->getTechnique(0);
  this->ogrePass = this->ogreTechnique->getPass(0);
  this->ogreTexState = this->ogrePass->createTextureUnitState();
  this->ogreTexState->setBlank();
  this->Reset();

  // TODO(anyone): provide function interface
  this->ogreMaterial->setTextureAnisotropy(8);
}

//////////////////////////////////////////////////
