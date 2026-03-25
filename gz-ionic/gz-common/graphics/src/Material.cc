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
#include <algorithm>
#include <gz/math/Color.hh>

#include "gz/common/Material.hh"
#include "gz/common/Console.hh"

using namespace gz;
using namespace common;

GZ_ENUM(shadeModeIface, Material::ShadeMode,
    Material::SHADE_MODE_BEGIN, Material::SHADE_MODE_END,
    "FLAT", "GOURAUD", "PHONG", "BLINN")

GZ_ENUM(blendModeIface, Material::BlendMode,
    Material::BLEND_MODE_BEGIN, Material::BLEND_MODE_END,
    "ADD", "MODULATE", "REPLACE")

/// \brief Private data for Material
class gz::common::Material::Implementation
{
  /// \brief the name of the material
  public: std::string name;

  /// \brief the texture image file name
  public: std::string texImage;

  /// \brief Texture raw data
  public: std::shared_ptr<const Image> texData;

  /// \brief the ambient light color
  public: math::Color ambient;

  /// \brief the diffuse light color
  public: math::Color diffuse;

  /// \brief the specular light color
  public: math::Color specular;

  /// \brief the emissive light color
  public: math::Color emissive;

  /// \brief transparency value in the range 0 to 1
  public: double transparency = 0.0;

  // \brief Enable texture based alpha rendering
  public: bool textureAlphaEnabled = false;

  // \brief Cutoff value for alpha, values below threshold will not be rendered
  public: double alphaThreshold = 0.5;

  // \brief Enables two sided rendering
  public: bool twoSidedEnabled = false;

  /// \brief render order value
  public: float renderOrder = 0.0;

  /// \brief shininess value (0 to 1)
  public: double shininess = 0.0;

  /// \brief point size
  public: double pointSize;

  /// \brief blend mode
  public: Material::BlendMode blendMode;

  /// \brief the shade mode
  public: Material::ShadeMode shadeMode;

  /// \brief the total number of instantiated Material instances
  public: static unsigned int counter;

  /// \brief flag to perform depth buffer write
  public: bool depthWrite = true;

  /// \brief flag to indicate if lighting is enabled for this material.
  public: bool lighting = true;

  /// \brief source blend factor
  public: double srcBlendFactor;

  /// \brief destination blend factor
  public: double dstBlendFactor;

  /// \brief Physically Based Rendering (PBR) properties
  public: std::unique_ptr<Pbr> pbr;
};

unsigned int Material::Implementation::counter = 0;

//////////////////////////////////////////////////
Material::Material()
: dataPtr(gz::utils::MakeUniqueImpl<Implementation>())
{
  this->dataPtr->name = "gz_material_" +
    std::to_string(this->dataPtr->counter++);
  this->dataPtr->blendMode = REPLACE;
  this->dataPtr->shadeMode = GOURAUD;
  this->dataPtr->ambient.Set(0.4f, 0.4f, 0.4f, 1.0f);
  this->dataPtr->diffuse.Set(0.5f, 0.5f, 0.5f, 1.0f);
  this->dataPtr->specular.Set(0, 0, 0, 1);
  this->dataPtr->dstBlendFactor = this->dataPtr->srcBlendFactor = 1.0;
}

//////////////////////////////////////////////////
Material::Material(const math::Color &_clr)
: dataPtr(gz::utils::MakeUniqueImpl<Implementation>())
{
  this->dataPtr->name = "gz_material_" +
    std::to_string(this->dataPtr->counter++);
  this->dataPtr->blendMode = REPLACE;
  this->dataPtr->shadeMode = GOURAUD;
  this->dataPtr->ambient = _clr;
  this->dataPtr->diffuse = _clr;
}

//////////////////////////////////////////////////
Material::~Material()
{
}

//////////////////////////////////////////////////
std::string Material::Name() const
{
  return this->dataPtr->name;
}

//////////////////////////////////////////////////
void Material::SetTextureImage(const std::string &_tex,
    const std::shared_ptr<const Image> &_img)
{
  this->dataPtr->texImage = _tex;
  this->dataPtr->texData = _img;
}

//////////////////////////////////////////////////
std::shared_ptr<const Image> Material::TextureData() const
{
  return this->dataPtr->texData;
}

//////////////////////////////////////////////////
void Material::SetTextureImage(const std::string &_tex,
                               const std::string &_resourcePath)
{
  this->dataPtr->texImage = common::joinPaths(_resourcePath, _tex);
  this->dataPtr->texData = nullptr;

  // If the texture image doesn't exist then try the next most likely path.
  if (!exists(this->dataPtr->texImage))
  {
    // Try to resolve this texture image to a locally cached path in a
    // separate directory
    this->dataPtr->texImage = common::findFile(_tex);
    if (!exists(this->dataPtr->texImage))
    {
      this->dataPtr->texImage = common::joinPaths(_resourcePath, "..",
          "materials", "textures", _tex);
      if (!exists(this->dataPtr->texImage))
      {
        gzerr << "Unable to find texture [" << _tex << "] as a locally"
              " cached texture or in path ["<< _resourcePath << "]\n";
      }
    }
  }
}

//////////////////////////////////////////////////
std::string Material::TextureImage() const
{
  return this->dataPtr->texImage;
}

//////////////////////////////////////////////////
void Material::SetAmbient(const math::Color &_clr)
{
  this->dataPtr->ambient = _clr;
}

//////////////////////////////////////////////////
math::Color Material::Ambient() const
{
  return this->dataPtr->ambient;
}

//////////////////////////////////////////////////
void Material::SetDiffuse(const math::Color &_clr)
{
  this->dataPtr->diffuse = _clr;
}

//////////////////////////////////////////////////
math::Color Material::Diffuse() const
{
  return this->dataPtr->diffuse;
}

//////////////////////////////////////////////////
void Material::SetSpecular(const math::Color &_clr)
{
  this->dataPtr->specular = _clr;
}

//////////////////////////////////////////////////
math::Color Material::Specular() const
{
  return this->dataPtr->specular;
}

//////////////////////////////////////////////////
void Material::SetEmissive(const math::Color &_clr)
{
  this->dataPtr->emissive = _clr;
}

//////////////////////////////////////////////////
math::Color Material::Emissive() const
{
  return this->dataPtr->emissive;
}

//////////////////////////////////////////////////
void Material::SetTransparency(double _t)
{
  this->dataPtr->transparency = std::min(_t, 1.0);
  this->dataPtr->transparency = std::max(this->dataPtr->transparency, 0.0);
}

//////////////////////////////////////////////////
double Material::Transparency() const
{
  return this->dataPtr->transparency;
}

//////////////////////////////////////////////////
void Material::SetAlphaFromTexture(bool _enabled, double _alpha,
                                   bool _twoSided)
{
  this->dataPtr->textureAlphaEnabled = _enabled;
  this->dataPtr->alphaThreshold = _alpha;
  this->dataPtr->twoSidedEnabled = _twoSided;
}

//////////////////////////////////////////////////
bool Material::TextureAlphaEnabled() const
{
  return this->dataPtr->textureAlphaEnabled;
}

//////////////////////////////////////////////////
double Material::AlphaThreshold() const
{
  return this->dataPtr->alphaThreshold;
}

//////////////////////////////////////////////////
bool Material::TwoSidedEnabled() const
{
  return this->dataPtr->twoSidedEnabled;
}

//////////////////////////////////////////////////
void Material::SetShininess(double _s)
{
  this->dataPtr->shininess = _s;
}

//////////////////////////////////////////////////
double Material::Shininess() const
{
  return this->dataPtr->shininess;
}

//////////////////////////////////////////////////
void Material::SetRenderOrder(float _renderOrder)
{
  this->dataPtr->renderOrder = _renderOrder;
}

//////////////////////////////////////////////////
float Material::RenderOrder() const
{
  return this->dataPtr->renderOrder;
}

//////////////////////////////////////////////////
void Material::SetBlendFactors(double _srcFactor, double _dstFactor)
{
  this->dataPtr->srcBlendFactor = _srcFactor;
  this->dataPtr->dstBlendFactor = _dstFactor;
}

//////////////////////////////////////////////////
void Material::BlendFactors(double &_srcFactor, double &_dstFactor) const
{
  _srcFactor = this->dataPtr->srcBlendFactor;
  _dstFactor = this->dataPtr->dstBlendFactor;
}

//////////////////////////////////////////////////
void Material::SetBlend(Material::BlendMode _b)
{
  this->dataPtr->blendMode = _b;
}

//////////////////////////////////////////////////
Material::BlendMode Material::Blend() const
{
  return this->dataPtr->blendMode;
}

//////////////////////////////////////////////////
void Material::SetShade(Material::ShadeMode _s)
{
  this->dataPtr->shadeMode = _s;
}

//////////////////////////////////////////////////
Material::ShadeMode Material::Shade() const
{
  return this->dataPtr->shadeMode;
}

//////////////////////////////////////////////////
void Material::SetPointSize(double _size)
{
  this->dataPtr->pointSize = _size;
}

//////////////////////////////////////////////////
double Material::PointSize() const
{
  return this->dataPtr->pointSize;
}
//////////////////////////////////////////////////
void Material::SetDepthWrite(bool _value)
{
  this->dataPtr->depthWrite = _value;
}

//////////////////////////////////////////////////
bool Material::DepthWrite() const
{
  return this->dataPtr->depthWrite;
}

//////////////////////////////////////////////////
void Material::SetLighting(bool _value)
{
  this->dataPtr->lighting = _value;
}

//////////////////////////////////////////////////
bool Material::Lighting() const
{
  return this->dataPtr->lighting;
}

//////////////////////////////////////////////////
std::string Material::BlendStr() const
{
  return blendModeIface.Str(this->Blend());
}

//////////////////////////////////////////////////
std::string Material::ShadeStr() const
{
  return shadeModeIface.Str(this->Shade());
}

//////////////////////////////////////////////////
void Material::SetPbrMaterial(const Pbr &_pbr)
{
  this->dataPtr->pbr = std::make_unique<Pbr>(_pbr);
}

//////////////////////////////////////////////////
Pbr *Material::PbrMaterial() const
{
  return this->dataPtr->pbr.get();
}
