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

#include <functional>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <gz/common/Console.hh>
#include <gz/math/Vector3.hh>

#include "gz/rendering/CameraLens.hh"

/// \brief Private fields of camera lens
class gz::rendering::CameraLens::Implementation
{
  /// \brief Linear scale factor
  public: double c1 = 1.0;

  /// \brief Angle scale factor
  public: double c2 = 1.0;

  /// \brief Angle offset factor
  public: double c3 = 0.0;

  /// \brief Linear scale factor, may be adjusted in runtime
  public: double f = 1.0;

  /// \brief Visible field of view
  public: double cutOffAngle = GZ_PI * 0.5;

  /// \brief True to scale image to hfov, false to render as output according
  /// to projection parameters
  public: bool scaleToHFov = true;

  /// \brief Mapping function type
  public: MappingFunctionType type = MFT_GNOMONIC;

  /// \brief Enumeration of functions that can be casted to some other types
  public: class MapFunctionEnum
          {
            /// \brief Constructor
            /// \param[in] _fun Angle function MFT_SIN, MFT_TAN, or MFT_IDENTITY
            public: explicit MapFunctionEnum(AngleFunctionType _fun)
            {
              variants.push_back(std::make_tuple(AFT_SIN,
                    math::Vector3d::UnitX,
                    std::function<float (float)>(
                      static_cast<float (*)(float)>(&std::sin))));

              variants.push_back(std::make_tuple(AFT_TAN,
                    math::Vector3d::UnitY,
                    std::function<float (float)>(
                      static_cast<float (*)(float)>(&std::tan))));

              variants.push_back(std::make_tuple(AFT_IDENTITY,
                    math::Vector3d::UnitZ,
                    std::function<float (float)>(
                      [](float t) -> float
                      {
                      return t;
                      })));

              for (const auto &item : variants)
              {
                if (std::get<0>(item) == _fun)
                {
                  value = item;
                  return;
                }
              }

              // function provided is not in array
              throw std::invalid_argument("Unknown angle function");
            }

            /// \brief Cast to gz::math::Vector3d,
            ///   this vector is passed to shader to avoid branching
            /// \return Vector3 Vector whose one component is 1
            ///   and the rest are nulls
            public: math::Vector3d AsVector3d() const
            {
              return std::get<1>(value);
            }

            /// \brief Get the angle transformation function
            /// \return The same function which was passed to constructor
            public: AngleFunctionType AngleFunction() const
            {
              return std::get<0>(value);
            }

            /// \brief Apply function to a float value
            /// \param _f input float value
            /// \return The result of application
            public: float Apply(float _f)
            {
              return std::get<2>(value)(_f);
            }

            /// \brief Copy constructor
            public: MapFunctionEnum(const MapFunctionEnum &) = delete;

            /// \brief Assignment operator
            /// \param[in] _fun Rvalue
            /// \return Reference to (*this)
            public: MapFunctionEnum &operator=(const MapFunctionEnum &_fun)
            {
              // from https://github.com/gazebosim/gz-rendering/pull/1227
              // "`variants` is tmp variable and shouldn't be copied"
              this->value = _fun.value;
              return *this;
            }

            /// \brief List of all available functions
            ///   and its associated representations
            private: std::vector<std::tuple<AngleFunctionType,
                     math::Vector3d,
                       std::function<float (float)> > > variants;

            /// \brief Current value of enumeration
            private: decltype(variants)::value_type value;
          };

  /// \brief Angle function component of the mapping function,
  /// \see CameraLens description
  public: MapFunctionEnum fun = MapFunctionEnum(AFT_IDENTITY);
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
CameraLens::CameraLens()
  : dataPtr(utils::MakeUniqueImpl<Implementation>())
{
}

//////////////////////////////////////////////////
CameraLens::CameraLens(const CameraLens &_other)
  : dataPtr(utils::MakeUniqueImpl<Implementation>())
{
  // Avoid incorrect cppcheck error about dataPtr being assigned in constructor
  CameraLens::Implementation &dp = *(this->dataPtr);
  dp = *(_other.dataPtr);
}

//////////////////////////////////////////////////
CameraLens::~CameraLens() = default;

//////////////////////////////////////////////////
CameraLens &CameraLens::operator=(const CameraLens &_other)
{
  *(this->dataPtr) = *(_other.dataPtr);
  return *this;
}

//////////////////////////////////////////////////
void CameraLens::SetCustomMappingFunction(double _c1, double _c2,
    AngleFunctionType _fun, double _f, double _c3)
{
  this->dataPtr->c1 = _c1;
  this->dataPtr->c2 = _c2;
  this->dataPtr->c3 = _c3;
  this->dataPtr->f = _f;

  try
  {
    this->dataPtr->fun = CameraLens::Implementation::MapFunctionEnum(_fun);
  }
  catch(const std::exception &ex)
  {
    gzerr << "Angle functionis not known, "
           << "[tan] will be used instead" << std::endl;

    this->dataPtr->fun = CameraLens::Implementation::MapFunctionEnum(AFT_TAN);
  }
  if (!this->IsCustom())
    this->ConvertToCustom();
}

//////////////////////////////////////////////////
MappingFunctionType CameraLens::Type() const
{
  return this->dataPtr->type;
}

//////////////////////////////////////////////////
bool CameraLens::IsCustom() const
{
  return this->Type() == MFT_CUSTOM;
}

//////////////////////////////////////////////////
double CameraLens::C1() const
{
  return this->dataPtr->c1;
}

//////////////////////////////////////////////////
double CameraLens::C2() const
{
  return this->dataPtr->c2;
}

//////////////////////////////////////////////////
double CameraLens::C3() const
{
  return this->dataPtr->c3;
}

//////////////////////////////////////////////////
double CameraLens::F() const
{
  return this->dataPtr->f;
}

//////////////////////////////////////////////////
AngleFunctionType CameraLens::AngleFunction() const
{
  return this->dataPtr->fun.AngleFunction();
}

//////////////////////////////////////////////////
double CameraLens::CutOffAngle() const
{
  return this->dataPtr->cutOffAngle;
}

//////////////////////////////////////////////////
bool CameraLens::ScaleToHFOV() const
{
  return this->dataPtr->scaleToHFov;
}

//////////////////////////////////////////////////
void CameraLens::SetType(MappingFunctionType _type)
{
  // c1, c2, c3, f, angle function
  std::map<MappingFunctionType, std::tuple<double, double, double,
      double, AngleFunctionType> > funTypes = {
    {MFT_GNOMONIC,        std::make_tuple(1.0, 1.0, 0.0, 1.0, AFT_TAN)},
    {MFT_STEREOGRAPHIC,   std::make_tuple(2.0, 2.0, 0.0, 1.0, AFT_TAN)},
    {MFT_EQUIDISTANT,     std::make_tuple(1.0, 1.0, 0.0, 1.0, AFT_IDENTITY)},
    {MFT_EQUISOLID_ANGLE, std::make_tuple(2.0, 2.0, 0.0, 1.0, AFT_SIN)},
    {MFT_ORTHOGRAPHIC,    std::make_tuple(1.0, 1.0, 0.0, 1.0, AFT_SIN)}};

  funTypes.emplace(MFT_CUSTOM,
      std::make_tuple(this->C1(), this->C2(), this->C3(), this->F(),
      this->AngleFunction()));

  decltype(funTypes)::mapped_type params;

  try
  {
    params = funTypes.at(_type);
  }
  catch(...)
  {
    gzerr << "Unknown lens type." << std::endl;
    return;
  }

  this->dataPtr->type = _type;

  if (_type == MFT_CUSTOM)
  {
    this->SetC1(std::get<0>(params));
    this->SetC2(std::get<1>(params));
    this->SetC3(std::get<2>(params));
    this->SetF(std::get<3>(params));
    this->SetAngleFunction(std::get<4>(params));
  }
  else
  {
    this->dataPtr->c1 = std::get<0>(params);
    this->dataPtr->c2 = std::get<1>(params);
    this->dataPtr->c3 = std::get<2>(params);
    this->dataPtr->f = std::get<3>(params);

    try
    {
      this->dataPtr->fun =
          CameraLens::Implementation::MapFunctionEnum(std::get<4>(params));
    }
    catch(const std::exception &ex)
    {
      gzerr << "`fun` value [" << std::get<4>(params)
             << "] is not known, keeping the old one" << std::endl;
    }
  }
}

//////////////////////////////////////////////////
void CameraLens::SetC1(double _c)
{
  this->dataPtr->c1 = _c;

  if (!this->IsCustom())
    this->ConvertToCustom();
}

//////////////////////////////////////////////////
void CameraLens::SetC2(double _c)
{
  this->dataPtr->c2 = _c;

  if (!this->IsCustom())
    this->ConvertToCustom();
}

//////////////////////////////////////////////////
void CameraLens::SetC3(double _c)
{
  this->dataPtr->c3 = _c;

  if (!this->IsCustom())
    this->ConvertToCustom();
}

//////////////////////////////////////////////////
void CameraLens::SetF(double _f)
{
  this->dataPtr->f = _f;

  if (!this->IsCustom())
    this->ConvertToCustom();
}

//////////////////////////////////////////////////
void CameraLens::SetAngleFunction(AngleFunctionType _fun)
{
  if (!this->IsCustom())
    this->ConvertToCustom();

  try
  {
    this->dataPtr->fun = CameraLens::Implementation::MapFunctionEnum(_fun);
  }
  catch(const std::exception &ex)
  {
    gzerr << "`Fun` value [" << _fun << "] is not known, "
           << "keeping the old one" << std::endl;
    return;
  }
}

//////////////////////////////////////////////////
void CameraLens::SetCutOffAngle(const double _angle)
{
  this->dataPtr->cutOffAngle = _angle;
}

//////////////////////////////////////////////////
void CameraLens::SetScaleToHFOV(bool _scale)
{
  this->dataPtr->scaleToHFov = _scale;
}

//////////////////////////////////////////////////
void CameraLens::ConvertToCustom()
{
  this->SetType(MFT_CUSTOM);
}

//////////////////////////////////////////////////
float CameraLens::ApplyMappingFunction(float _f) const
{
  return this->dataPtr->fun.Apply(_f);
}

//////////////////////////////////////////////////
math::Vector3d CameraLens::MappingFunctionAsVector3d() const
{
  return this->dataPtr->fun.AsVector3d();
}
