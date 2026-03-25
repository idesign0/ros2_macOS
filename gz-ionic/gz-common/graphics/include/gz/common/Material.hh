/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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
#ifndef GZ_COMMON_MATERIAL_HH_
#define GZ_COMMON_MATERIAL_HH_

#include <iostream>
#include <memory>
#include <string>

#include <gz/math/Color.hh>

#include <gz/common/graphics/Export.hh>
#include <gz/common/EnumIface.hh>
#include <gz/common/Image.hh>
#include <gz/common/Pbr.hh>

#include <gz/utils/ImplPtr.hh>

namespace gz
{
  namespace common
  {
    /// \brief Encapsulates description of a material
    class GZ_COMMON_GRAPHICS_VISIBLE Material
    {
      /// \def Shade modes
      /// \brief Enumeration of shade mode types
      public: enum ShadeMode
      {
        /// \internal
        /// \brief Indicator used to create an iterator over the
        /// enum. Do not use this.
        SHADE_MODE_BEGIN = 0,

        /// \brief Flat shading
        FLAT = 0,

        /// \brief Gouraud shading
        GOURAUD = 1,

        /// \brief Phong shading
        PHONG = 2,

        /// \brief Blinn shading
        BLINN = 3,

        /// \internal
        /// \brief Indicator used to create an iterator over the
        /// enum. Do not use this.
        SHADE_MODE_END
      };

      /// \def Blend modes
      /// \brief Enumeration of blend mode types
      public: enum BlendMode
      {
        /// \internal
        /// \brief Indicator used to create an iterator over the
        /// enum. Do not use this.
        BLEND_MODE_BEGIN = 0,

        /// \brief Add mode
        ADD = 0,

        /// \brief Modulate mode
        MODULATE = 1,

        /// \brief Replace mode
        REPLACE = 2,

        /// \internal
        /// \brief Indicator used to create an iterator over the
        /// enum. Do not use this.
        BLEND_MODE_END
      };

      /// \brief Constructor
      public: Material();

      /// \brief Destructor
      public: ~Material();

      /// \brief Create a material with a default color
      /// \param[in] _clr Color of the material
      public: explicit Material(const math::Color &_clr);

      /// \brief Get the name of the material
      /// \return The name of the material
      public: std::string Name() const;

      /// \brief Set a texture image
      /// \param[in] _tex The name of the texture, which must be in the
      /// resource path or its name if _img is provided
      /// \param[in] _img The image containing the texture if image has been
      /// loaded in memory
      public: void SetTextureImage(const std::string &_tex,
                  const std::shared_ptr<const Image> &_img = nullptr);

      /// \brief Set a texture image
      /// \param[in] _tex The name of the texture
      /// \param[in] _resourcePath Path which contains _tex
      public: void SetTextureImage(const std::string &_tex,
                                   const std::string &_resourcePath);

      /// \brief Gets the texture image, if the texture was loaded from memory
      /// \return A pointer to the image that was loaded from memory
      public: std::shared_ptr<const Image> TextureData() const;

      /// \brief Get a texture image
      /// \return The name of the texture image (if one exists) or an empty
      /// string
      public: std::string TextureImage() const;

      /// \brief Set the ambient color
      /// \param[in] _clr The ambient color
      public: void SetAmbient(const math::Color &_clr);

      /// \brief Get the ambient color
      /// \return The ambient color
      public: math::Color Ambient() const;

      /// \brief Set the diffuse color
      /// \param[in] _clr The diffuse color
      public: void SetDiffuse(const math::Color &_clr);

      /// \brief Get the diffuse color
      /// \return The diffuse color
      public: math::Color Diffuse() const;

      /// \brief Set the specular color
      /// \param[in] _clr The specular color
      public: void SetSpecular(const math::Color &_clr);

      /// \brief Get the specular color
      /// \return The specular color
      public: math::Color Specular() const;

      /// \brief Set the emissive color
      /// \param[in] _clr The emissive color
      public: void SetEmissive(const math::Color &_clr);

      /// \brief Get the emissive color
      /// \return The emissive color
      public: math::Color Emissive() const;

      /// \brief Set the transparency percentage (0..1)
      /// \param[in] _t The amount of transparency (0..1) where a value of 1
      /// is fully transparent and 0 is not transparent.
      public: void SetTransparency(double _t);

      /// \brief Get the transparency percentage (0..1)
      /// A value of 1 is fully transparent and 0 is not transparent.
      /// \return The transparency percentage
      public: double Transparency() const;

      /// \brief Set the material to use the alpha channel from the textures
      /// \param[in] _enabled Enable alpha channel based rendering
      /// \param[in] _alpha Set the alpha threshold value
      /// \param[in] _twoSided Enable two sided rendering
      public: void SetAlphaFromTexture(bool _enabled,
                  double _alpha = 0.5, bool _twoSided = true);

      /// \brief Get the enable alpha from textures
      /// \return The enable alpha value
      public: bool TextureAlphaEnabled() const;

      /// \brief Get the alpha threshold
      /// \return The alpha threshold value
      public: double AlphaThreshold() const;

      /// \brief Get the enable two sided rendering
      /// \return The enable two sided rendering value
      public: bool TwoSidedEnabled() const;

      /// \brief Set the render order. The higher value will be rendered on top
      /// of the other coplanar polygons.
      /// \param[in] _renderOrder The render order value
      public: void SetRenderOrder(float _renderOrder);

      /// \brief Get the render order
      /// \return The render order value
      public: float RenderOrder() const;

      /// \brief Set the shininess
      /// \param[in] _t The shininess value
      public: void SetShininess(double _t);

      /// \brief Get the shininess
      /// \return The shininess value
      public: double Shininess() const;

      /// \brief Set the blend factors. Will be interpreted as:
      ///        (texture * _srcFactor) + (scene_pixel * _dstFactor)
      /// \param[in] _srcFactor The source factor
      /// \param[in] _dstFactor The destination factor
      public: void SetBlendFactors(double _srcFactor, double _dstFactor);

      /// \brief Get the blend factors
      /// \param[in] _srcFactor Source factor is returned in this variable
      /// \param[in] _dstFactor Destination factor is returned in this variable
      public: void BlendFactors(double &_srcFactor, double &_dstFactor) const;

      /// \brief Set the blending mode
      /// \param[in] _b the blend mode
      public: void SetBlend(BlendMode _b);

      /// \brief Get the blending mode
      /// \return the blend mode
      public: BlendMode Blend() const;

      /// \brief Get the blend mode string
      public: std::string BlendStr() const;

      /// \brief Get the shade mode string
      public: std::string ShadeStr() const;

      /// \brief Set the shading mode
      /// param[in] the shading mode
      public: void SetShade(ShadeMode _b);

      /// \brief Get the shading mode
      /// \return the shading mode
      public: ShadeMode Shade() const;

      /// \brief Set the point size
      /// \param[in] _size the size
      public: void SetPointSize(double _size);

      /// \brief Get the point size
      /// \return the point size
      public: double PointSize() const;

      /// \brief Set depth write
      /// \param[in] _value the depth write enabled state
      public: void SetDepthWrite(bool _value);

      /// \brief Get depth write
      /// \return the depth write enabled state
      public: bool DepthWrite() const;

      /// \brief Set lighting enabled
      /// \param[in] _value the lighting enabled state
      public: void SetLighting(bool _value);

      /// \brief Get lighting enabled
      /// \return the lighting enabled state
      public: bool Lighting() const;

      /// \brief Set the Physically Based Rendering (PBR) material
      /// \return The PBR material to set to.
      public: void SetPbrMaterial(const Pbr &_pbr);

      /// \brief Get the Physically Based Rendering (PBR) material
      /// \return Pointer to the PBR material. Null if it does not exist.
      public: Pbr *PbrMaterial() const;

      /// \brief Stream insertion operator
      /// param[in] _out the output stream to extract from
      /// param[out] _m the material information
      public: friend std::ostream &operator<<(std::ostream &_out,
                  const gz::common::Material &_m)
              {
                _out << "Material:\n";
                _out << "  Name: " << _m.Name() << "\n";
                _out << "  Texture: " << _m.TextureImage() << "\n";
                _out << "  Ambient: " << _m.Ambient() << "\n";
                _out << "  Diffuse: " << _m.Diffuse() << "\n";
                _out << "  Specular: " << _m.Specular() << "\n";
                _out << "  Emissive: " << _m.Emissive() << "\n";
                _out << "  Transparency: " << _m.Transparency() << "\n";
                _out << "  Shininess: " << _m.Shininess() << "\n";
                _out << "  Render order: " << _m.RenderOrder() << "\n";
                _out << "  BlendMode: " << _m.BlendStr() << "\n";
                _out << "  ShadeMode: " << _m.ShadeStr() << "\n";
                _out << "  DepthWrite: " << _m.DepthWrite() << "\n";
                return _out;
              }

      /// \brief Private data pointer.
      GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
    };
  }
}
#endif
