/*
 * Copyright 2024 CogniPilot Foundation
 * Copyright 2024 Open Source Robotics Foundation
 * Copyright 2024 Rudis Laboratories
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
#ifndef GZ_MATH_DETAIL_CONE_HH_
#define GZ_MATH_DETAIL_CONE_HH_

#include <optional>
#include "gz/math/Cone.hh"

namespace gz::math
{
//////////////////////////////////////////////////
template<typename T>
Cone<T>::Cone(const T _length, const T _radius,
    const Quaternion<T> &_rotOffset)
{
  this->length = _length;
  this->radius = _radius;
  this->rotOffset = _rotOffset;
}

//////////////////////////////////////////////////
template<typename T>
Cone<T>::Cone(const T _length, const T _radius,
    const Material &_mat, const Quaternion<T> &_rotOffset)
{
  this->length = _length;
  this->radius = _radius;
  this->material = _mat;
  this->rotOffset = _rotOffset;
}

//////////////////////////////////////////////////
template<typename T>
T Cone<T>::Radius() const
{
  return this->radius;
}

//////////////////////////////////////////////////
template<typename T>
void Cone<T>::SetRadius(const T _radius)
{
  this->radius = _radius;
}

//////////////////////////////////////////////////
template<typename T>
T Cone<T>::Length() const
{
  return this->length;
}

//////////////////////////////////////////////////
template<typename T>
void Cone<T>::SetLength(const T _length)
{
  this->length = _length;
}

//////////////////////////////////////////////////
template<typename T>
Quaternion<T> Cone<T>::RotationalOffset() const
{
  return this->rotOffset;
}

//////////////////////////////////////////////////
template<typename T>
void Cone<T>::SetRotationalOffset(const Quaternion<T> &_rotOffset)
{
  this->rotOffset = _rotOffset;
}

//////////////////////////////////////////////////
template<typename T>
const Material &Cone<T>::Mat() const
{
  return this->material;
}

//////////////////////////////////////////////////
template<typename T>
void Cone<T>::SetMat(const Material &_mat)
{
  this->material = _mat;
}

//////////////////////////////////////////////////
template<typename T>
bool Cone<T>::operator==(const Cone &_cone) const
{
  return equal(this->radius, _cone.Radius()) &&
    equal(this->length, _cone.Length()) &&
    this->material == _cone.Mat();
}

//////////////////////////////////////////////////
template<typename T>
bool Cone<T>::MassMatrix(MassMatrix3d &_massMat) const
{
  return _massMat.SetFromConeZ(
      this->material, this->length,
      this->radius, this->rotOffset);
}

//////////////////////////////////////////////////
template<typename T>
std::optional < MassMatrix3<T> > Cone<T>::MassMatrix() const
{
  gz::math::MassMatrix3<T> _massMat;

  if(!_massMat.SetFromConeZ(
      this->material, this->length,
      this->radius, this->rotOffset))
  {
    return std::nullopt;
  }
  else
  {
    return std::make_optional(_massMat);
  }
}

//////////////////////////////////////////////////
template<typename T>
T Cone<T>::Volume() const
{
  return GZ_PI * std::pow(this->radius, 2) *
         this->length / 3.0;
}

//////////////////////////////////////////////////
template<typename T>
bool Cone<T>::SetDensityFromMass(const T _mass)
{
  T newDensity = this->DensityFromMass(_mass);
  if (newDensity > 0)
    this->material.SetDensity(newDensity);
  return newDensity > 0;
}

//////////////////////////////////////////////////
template<typename T>
T Cone<T>::DensityFromMass(const T _mass) const
{
  if (this->radius <= 0 || this->length <=0 || _mass <= 0)
    return -1.0;

  return _mass / this->Volume();
}

}  // namespace gz::math
#endif  // GZ_MATH_DETAIL_CONE_HH_
