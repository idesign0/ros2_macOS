/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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
#ifndef GZ_MSGS_CONVERT_INERTIAL_HH_
#define GZ_MSGS_CONVERT_INERTIAL_HH_

#include <gz/msgs/config.hh>
#include <gz/msgs/convert/Pose.hh>
#include <gz/msgs/convert/Vector3.hh>

// Message Headers
#include "gz/msgs/inertial.pb.h"

// Data Headers
#include <gz/math/Inertial.hh>
#include <gz/math/MassMatrix3.hh>

namespace gz::msgs {
// Inline bracket to help doxygen filtering.
inline namespace GZ_MSGS_VERSION_NAMESPACE {

/////////////////////////////////
inline void Set(gz::msgs::Inertial *_msg, const gz::math::MassMatrix3d &_data)
{
  _msg->set_mass(_data.Mass());
  _msg->set_ixx(_data.Ixx());
  _msg->set_iyy(_data.Iyy());
  _msg->set_izz(_data.Izz());
  _msg->set_ixy(_data.Ixy());
  _msg->set_ixz(_data.Ixz());
  _msg->set_iyz(_data.Iyz());
  _msg->mutable_pose()->mutable_orientation()->set_w(1);
}

inline void Set(gz::math::MassMatrix3d *_data, const gz::msgs::Inertial &_msg)
{
  _data->SetMass(_msg.mass());
  _data->SetIxx(_msg.ixx());
  _data->SetIyy(_msg.iyy());
  _data->SetIzz(_msg.izz());
  _data->SetIxy(_msg.ixy());
  _data->SetIxz(_msg.ixz());
  _data->SetIyz(_msg.iyz());
}

inline gz::msgs::Inertial Convert(const gz::math::MassMatrix3d &_data)
{
  gz::msgs::Inertial ret;
  Set(&ret, _data);
  return ret;
}

/////////////////////////////////
inline void Set(gz::msgs::Inertial *_msg, const gz::math::Inertiald &_data)
{
  Set(_msg, _data.MassMatrix());
  Set(_msg->mutable_pose(), _data.Pose());

  if (_data.FluidAddedMass().has_value())
  {
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(0, 0));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(0, 1));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(0, 2));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(0, 3));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(0, 4));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(0, 5));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(1, 1));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(1, 2));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(1, 3));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(1, 4));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(1, 5));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(2, 2));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(2, 3));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(2, 4));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(2, 5));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(3, 3));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(3, 4));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(3, 5));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(4, 4));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(4, 5));
    _msg->add_fluid_added_mass(_data.FluidAddedMass().value()(5, 5));
  }
}

inline void Set(gz::math::Inertiald *_data, const gz::msgs::Inertial &_msg)
{
  gz::math::MassMatrix3d to_set = _data->MassMatrix();
  Set(&to_set, _msg);
  _data->SetMassMatrix(to_set);

  gz::math::Pose3d pose_to_set = _data->Pose();
  Set(&pose_to_set, _msg.pose());
  _data->SetPose(pose_to_set);

  if (_msg.fluid_added_mass_size() == 21)
  {
    math::Matrix6d addedMass{
      _msg.fluid_added_mass(0),
      _msg.fluid_added_mass(1),
      _msg.fluid_added_mass(2),
      _msg.fluid_added_mass(3),
      _msg.fluid_added_mass(4),
      _msg.fluid_added_mass(5),

      _msg.fluid_added_mass(1),
      _msg.fluid_added_mass(6),
      _msg.fluid_added_mass(7),
      _msg.fluid_added_mass(8),
      _msg.fluid_added_mass(9),
      _msg.fluid_added_mass(10),

      _msg.fluid_added_mass(2),
      _msg.fluid_added_mass(7),
      _msg.fluid_added_mass(11),
      _msg.fluid_added_mass(12),
      _msg.fluid_added_mass(13),
      _msg.fluid_added_mass(14),

      _msg.fluid_added_mass(3),
      _msg.fluid_added_mass(8),
      _msg.fluid_added_mass(12),
      _msg.fluid_added_mass(15),
      _msg.fluid_added_mass(16),
      _msg.fluid_added_mass(17),

      _msg.fluid_added_mass(4),
      _msg.fluid_added_mass(9),
      _msg.fluid_added_mass(13),
      _msg.fluid_added_mass(16),
      _msg.fluid_added_mass(18),
      _msg.fluid_added_mass(19),

      _msg.fluid_added_mass(5),
      _msg.fluid_added_mass(10),
      _msg.fluid_added_mass(14),
      _msg.fluid_added_mass(17),
      _msg.fluid_added_mass(19),
      _msg.fluid_added_mass(20)
    };
    _data->SetFluidAddedMass(addedMass);
  }
}

inline gz::msgs::Inertial Convert(const gz::math::Inertiald &_data)
{
  gz::msgs::Inertial ret;
  Set(&ret, _data);
  return ret;
}

inline gz::math::Inertiald Convert(const gz::msgs::Inertial &_msg)
{
  gz::math::Inertiald ret;
  Set(&ret, _msg);
  return ret;
}
}  // namespace
}  // namespace gz::msgs
#endif  // GZ_MSGS_CONVERT_VECTOR3_HH_
