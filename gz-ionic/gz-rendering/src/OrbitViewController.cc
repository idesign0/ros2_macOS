/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include <algorithm>
#include <cmath>

#include <gz/common/Console.hh>

#include "gz/rendering/OrbitViewController.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/Visual.hh"

class gz::rendering::OrbitViewControllerPrivate
{
  /// \brief Pointer to camera
  public: CameraPtr camera;

  /// \brief Target point for camera movements
  public: math::Vector3d target;

  /// \brief Keep yaw within limits
  /// \return Normalized yaw
  public: double NormalizeYaw(double _yaw);

  /// \brief Keep pitch within limits
  /// \return Normalized pitch
  public: double NormalizePitch(double _pitch);
};

using namespace gz;
using namespace rendering;

static constexpr double PITCH_LIMIT_LOW = -GZ_PI*0.5 + 0.001;
static constexpr double PITCH_LIMIT_HIGH = GZ_PI*0.5 - 0.001;

//////////////////////////////////////////////////
OrbitViewController::OrbitViewController()
    : dataPtr(new OrbitViewControllerPrivate)
{
}

//////////////////////////////////////////////////
OrbitViewController::OrbitViewController(const CameraPtr &_camera)
    : dataPtr(new OrbitViewControllerPrivate)
{
  this->dataPtr->camera = _camera;
}

//////////////////////////////////////////////////
OrbitViewController::~OrbitViewController() = default;

//////////////////////////////////////////////////
void OrbitViewController::SetCamera(const CameraPtr &_camera)
{
  this->dataPtr->camera = _camera;
  if (!this->dataPtr->camera)
    return;
  this->dataPtr->camera->SetProjectionType(CPT_PERSPECTIVE);
}

//////////////////////////////////////////////////
CameraPtr OrbitViewController::Camera() const
{
  return this->dataPtr->camera;
}

//////////////////////////////////////////////////
void OrbitViewController::SetTarget(const math::Vector3d &_target)
{
  this->dataPtr->target = _target;
}

//////////////////////////////////////////////////
const math::Vector3d &OrbitViewController::Target() const
{
  return this->dataPtr->target;
}

//////////////////////////////////////////////////
void OrbitViewController::Zoom(const double _value)
{
  if (!std::isfinite(_value))
  {
    gzerr << "Failed to zoom by non-finite value [" << _value << "]"
          << std::endl;
    return;
  }

  if (!this->dataPtr->camera)
  {
    gzerr << "Camera is NULL" << std::endl;
    return;
  }

  double distance =
      this->dataPtr->camera->WorldPose().Pos().Distance(this->dataPtr->target);

  distance -= _value;

  math::Vector3d delta = this->dataPtr->camera->WorldPosition() -
    this->dataPtr->target;
  delta.Normalize();
  delta *= distance;
  this->dataPtr->camera->SetWorldPosition(this->dataPtr->target + delta);
}

//////////////////////////////////////////////////
void OrbitViewController::Pan(const math::Vector2d &_value)
{
  if (!_value.IsFinite())
  {
    gzerr << "Failed to pan by non-finite value [" << _value << "]"
          << std::endl;
    return;
  }

  if (!this->dataPtr->camera)
  {
    gzerr << "Camera is NULL" << std::endl;
    return;
  }

  if (!this->dataPtr->camera->WorldPosition().IsFinite())
  {
    gzerr << "Camera world position isn't finite ["
          << this->dataPtr->camera->WorldPosition() << "]" << std::endl;
    return;
  }

  double viewportWidth = this->dataPtr->camera->ImageWidth();
  double viewportHeight = this->dataPtr->camera->ImageHeight();

  double distance =
      this->dataPtr->camera->WorldPosition().Distance(this->dataPtr->target);
  double hfov = this->dataPtr->camera->HFOV().Radian();
  double vfov = 2.0f * atan(tan(hfov / 2.0f) /
        this->dataPtr->camera->AspectRatio());

  math::Vector3d translation;

  double factor = 2.0;

  // Translate in the "y" "z" plane.
  translation.Set(0.0,
      (_value.X() / static_cast<double>(viewportWidth)) *
      distance * tan(hfov / 2.0) * factor,
      (_value.Y() / static_cast<double>(viewportHeight)) *
      distance * tan(vfov / 2.0) * factor);

  // Translate in the local coordinate frame
  this->dataPtr->camera->SetWorldPosition(
      this->dataPtr->camera->WorldPosition() +
      this->dataPtr->camera->WorldRotation() * translation);
}

//////////////////////////////////////////////////
void OrbitViewController::Orbit(const math::Vector2d &_value)
{
  if (!_value.IsFinite())
  {
    gzerr << "Failed to orbit by non-finite value [" << _value << "]"
          << std::endl;
    return;
  }

  if (!this->dataPtr->camera)
  {
    gzerr << "Camera is NULL" << std::endl;
    return;
  }

  double dy = 2 * GZ_PI * _value.X() / this->dataPtr->camera->ImageWidth();
  double dp = 2 * GZ_PI * _value.Y() / this->dataPtr->camera->ImageHeight();

  // translate to make target the origin for rotation
  this->dataPtr->camera->SetWorldPosition(
      this->dataPtr->camera->WorldPosition() - this->dataPtr->target);

  // rotate around world axis at target point
  math::Quaterniond yawQuat;
  yawQuat.SetFromAxisAngle(math::Vector3d::UnitZ, -dy);
  this->dataPtr->camera->SetWorldRotation(
      yawQuat * this->dataPtr->camera->WorldRotation());
  this->dataPtr->camera->SetWorldPosition(
      yawQuat * this->dataPtr->camera->WorldPosition());

  math::Quaterniond localPitchQuat;
  localPitchQuat.SetFromAxisAngle(
      this->dataPtr->camera->WorldRotation()*math::Vector3d::UnitY, dp);
  this->dataPtr->camera->SetWorldRotation(
      localPitchQuat * this->dataPtr->camera->WorldRotation());
  this->dataPtr->camera->SetWorldPosition(
      localPitchQuat * this->dataPtr->camera->WorldPosition());

  // translate camera back
  this->dataPtr->camera->SetWorldPosition(
      this->dataPtr->camera->WorldPosition() + this->dataPtr->target);
}

//////////////////////////////////////////////////
double OrbitViewControllerPrivate::NormalizeYaw(double _yaw)
{
  _yaw = fmod(_yaw, GZ_PI*2);
  if (_yaw < 0.0f)
  {
    _yaw = GZ_PI * 2 + _yaw;
  }

  return _yaw;
}

//////////////////////////////////////////////////
double OrbitViewControllerPrivate::NormalizePitch(double _pitch)
{
  return std::clamp(_pitch, PITCH_LIMIT_LOW, PITCH_LIMIT_HIGH);
}
