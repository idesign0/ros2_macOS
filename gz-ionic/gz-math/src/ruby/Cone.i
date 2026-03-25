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

%module cone
%{
#include <gz/math/Cone.hh>
#include <gz/math/config.hh>
#include <gz/math/MassMatrix3.hh>
#include <gz/math/Material.hh>
#include <gz/math/Quaternion.hh>
%}

namespace gz
{
  namespace math
  {
    template<typename Precision>
    class Cone
    {
      %rename("%(undercase)s", %$isfunction, notregexmatch$name="^[A-Z]*$") "";

      public: Cone() = default;

      public: Cone(const Precision _length, const Precision _radius,
                  const gz::math::Quaternion<Precision> &_rotOffset =
                  gz::math::Quaternion<Precision>::Identity);

      public: Cone(const Precision _length, const Precision _radius,
                  const gz::math::Material &_mat,
                  const gz::math::Quaternion<Precision> &_rotOffset =
                  gz::math::Quaternion<Precision>::Identity);

      public: ~Cone() = default;

      public: Precision Radius() const;

      public: void SetRadius(const Precision _radius);

      public: Precision Length() const;

      public: void SetLength(const Precision _length);

      public: gz::math::Quaternion<Precision> RotationalOffset() const;

      public: void SetRotationalOffset(
                  const gz::math::Quaternion<Precision> &_rotOffset);

      public: const gz::math::Material &Mat() const;

      public: void SetMat(const gz::math::Material &_mat);

      public: bool MassMatrix(gz::math::MassMatrix3<double> &_massMat) const;

      public: bool operator==(const Cone &_cone) const;

      public: Precision Volume() const;

      public: Precision DensityFromMass(const Precision _mass) const;

      public: bool SetDensityFromMass(const Precision _mass);
    };
    %template(Coned) Cone<double>;
  }
}
