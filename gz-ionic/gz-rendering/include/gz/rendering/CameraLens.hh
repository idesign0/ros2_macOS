/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#ifndef GZ_RENDERING_CAMERALENS_HH_
#define GZ_RENDERING_CAMERALENS_HH_

#include <memory>

#include <gz/math/Vector3.hh>
#include <gz/utils/ImplPtr.hh>

#include "gz/rendering/config.hh"
#include "gz/rendering/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Enum for mapping function types
    enum GZ_RENDERING_VISIBLE MappingFunctionType
    {
      /// \brief Gnomonic
      MFT_GNOMONIC = 0,

      /// \brief Stereographic
      MFT_STEREOGRAPHIC = 1,

      /// \brief Equidistant
      MFT_EQUIDISTANT = 2,

      /// \brief Equisolid angle
      MFT_EQUISOLID_ANGLE = 3,

      /// \brief Orthographic
      MFT_ORTHOGRAPHIC = 4,

      /// \brief custom
      MFT_CUSTOM = 5
    };

    /// \brief Enum for angle function types
    enum GZ_RENDERING_VISIBLE AngleFunctionType
    {
      /// \brief identity
      AFT_IDENTITY = 0,

      /// \brief sin
      AFT_SIN = 1,

      /// \brief TAN
      AFT_TAN = 2
    };

    /// \brief Describes a lens of a camera
    ///   as amapping function of type r = c1*f*fun(theta/c2+c3)
    class GZ_RENDERING_VISIBLE CameraLens
    {
      /// \brief Constructor
      public: CameraLens();

      /// \brief Destructor
      public: virtual ~CameraLens();

      /// \brief Constructor
      /// \param[in] _other The other camera lens
      public: explicit CameraLens(const CameraLens &_other);

      /// \brief Set custom camera lens with specified parameters
      /// \param[in] _c1 Image scaling factor
      /// \param[in] _c2 Angle scaling factor
      /// \param[in] _fun Angle transform function
      /// \param[in] _f Focal length of the optical system
      /// \param[in] _c3 Angle shift parameter, should be 0 in most cases
      public: void SetCustomMappingFunction(double _c1, double _c2,
                        AngleFunctionType _fun, double _f, double _c3);

      /// \brief Get lens projection type
      /// \return Lens projection / mapping function type
      public: MappingFunctionType Type() const;

      /// \brief Checks if lens type is of the custom type
      /// \return True if this->Type() == MFT_CUSTOM
      public: bool IsCustom() const;

      /// \brief Gets c1 parameter
      /// \return c1 parameter
      public: double C1() const;

      /// \brief Gets c2 parameter
      /// \return c2 parameter
      public: double C2() const;

      /// \brief Gets c3 parameter
      /// \return c3 parameter
      public: double C3() const;

      /// \brief Gets f parameter
      /// \return f parameter
      public: double F() const;

      /// \brief Gets angle transform function
      /// \return Angle transform function
      public: AngleFunctionType AngleFunction() const;

      /// \brief Gets cut off angle
      /// \return Cut off angle
      public: double CutOffAngle() const;

      /// \brief Checks if image should be scaled to fit horizontal FOV
      /// \return True if the image will be scaled
      public: bool ScaleToHFOV() const;

      /// \brief Set lens projection type
      /// \param[in] _type Lens projection / mapping function type
      public: void SetType(MappingFunctionType _type);

      /// \brief Sets c1 parameter
      /// \param[in] _c c1 parameter
      public: void SetC1(double _c);

      /// \brief Sets c2 parameter
      /// \param[in] _c c2 parameter
      public: void SetC2(double _c);

      /// \brief Sets c3 parameter
      /// \param[in] _c c3 parameter
      public: void SetC3(double _c);

      /// \brief Sets f parameter
      /// \param[in] _f f parameter
      public: void SetF(double _f);

      /// \brief Sets angle transform function
      /// \param[in] _fun Angle transform function
      public: void SetAngleFunction(AngleFunctionType _fun);

      /// \brief Sets cut-off angle
      /// \param[in] _angle cut-off angle
      public: void SetCutOffAngle(double _angle);

      /// \brief Sets whether the image should be scaled to fit horizontal FOV
      /// If True, the projection will compute a new focal length for achieving
      /// the desired FOV
      /// \param[in] _scale true if it should,
      ///   note: c1 and f parameters are ignored in this case
      public: void SetScaleToHFOV(bool _scale);

      /// \brief Apply mapping function to input number
      /// \param[in] _f Input floating point number to apply the mapping
      /// function to.
      /// \return Result of the application
      public: float ApplyMappingFunction(float _f) const;

      /// \internal
      /// \brief Get mapping function as vector3d
      /// return unit vector, either unit x, y, or z.
      public: math::Vector3d MappingFunctionAsVector3d() const;

      /// \brief Assignment operator
      /// \param[in] _other The other camera lens
      public: CameraLens &operator=(const CameraLens &_other);

      /// \internal
      /// \brief Converts projection type from one of presets to `custom`
      private: void ConvertToCustom();

      /// \internal
      /// \brief Private data pointer
      GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
    };
    }
  }
}
#endif
