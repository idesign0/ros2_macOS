/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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

#include "gz/math/Plane.hh"
#include "gz/math/Vector2.hh"
#include "gz/math/Vector3.hh"

#include "gz/rendering/Camera.hh"
#include "gz/rendering/GraphicsAPI.hh"
#include "gz/rendering/PixelFormat.hh"
#include "gz/rendering/RayQuery.hh"
#include "gz/rendering/Utils.hh"


namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {
//
/////////////////////////////////////////////////
math::Vector3d screenToScene(
    const math::Vector2i &_screenPos,
    const CameraPtr &_camera,
    const RayQueryPtr &_rayQuery,
    RayQueryResult &_rayResult,
    float _maxDistance)
{
  // Normalize point on the image
  double width = _camera->ImageWidth();
  double height = _camera->ImageHeight();

  double nx = 2.0 * _screenPos.X() / width - 1.0;
  double ny = 1.0 - 2.0 * _screenPos.Y() / height;

  // Make a ray query
  _rayQuery->SetFromCamera(
      _camera, math::Vector2d(nx, ny));

  _rayResult = _rayQuery->ClosestPoint();
  if (_rayResult)
    return _rayResult.point;

  // Set point to be maxDistance m away if no intersection found
  return _rayQuery->Origin() +
      _rayQuery->Direction() * _maxDistance;
}

/////////////////////////////////////////////////
math::Vector3d screenToScene(
    const math::Vector2i &_screenPos,
    const CameraPtr &_camera,
    const RayQueryPtr &_rayQuery,
    float _maxDistance)
{
  RayQueryResult rayResult;
  return screenToScene(_screenPos, _camera, _rayQuery, rayResult, _maxDistance);
}

/////////////////////////////////////////////////
math::Vector3d screenToPlane(
    const math::Vector2i &_screenPos,
    const CameraPtr &_camera,
    const RayQueryPtr &_rayQuery,
    const float _offset)
{
  // Normalize point on the image
  double width = _camera->ImageWidth();
  double height = _camera->ImageHeight();

  double nx = 2.0 * _screenPos.X() / width - 1.0;
  double ny = 1.0 - 2.0 * _screenPos.Y() / height;

  // Make a ray query
  _rayQuery->SetFromCamera(
      _camera, math::Vector2d(nx, ny));

  gz::math::Planed plane(gz::math::Vector3d(0, 0, 1), _offset);

  math::Vector3d origin = _rayQuery->Origin();
  math::Vector3d direction = _rayQuery->Direction();
  double distance = plane.Distance(origin, direction);
  return origin + direction * distance;
}

/////////////////////////////////////////////////
float screenScalingFactor()
{
  // The scaling factor seems to cause issues with mouse picking:
  // https://github.com/gazebosim/gz-sim/issues/147. The code to compute
  // the scaling factor was removed in
  // https://github.com/gazebosim/gz-rendering/pull/647.
  return 1.0;
}

/////////////////////////////////////////////////
gz::math::Matrix3d projectionToCameraIntrinsic(
    const gz::math::Matrix4d &_projectionMatrix,
    double _width, double _height)
{
  // Extracting the intrinsic matrix :
  // https://ogrecave.github.io/ogre/api/13/class_ogre_1_1_math.html
  double fX = (_projectionMatrix(0, 0) * _width) / 2.0;
  double fY = (_projectionMatrix(1, 1) * _height) / 2.0;
  double cX = (-1.0 * _width *
               (_projectionMatrix(0, 2) - 1.0)) / 2.0;
  double cY = _height + (_height *
               (_projectionMatrix(1, 2) - 1)) / 2.0;

  return gz::math::Matrix3d(fX, 0, cX,
                            0, fY, cY,
                            0, 0, 1);
}

/////////////////////////////////////////////////
gz::math::AxisAlignedBox transformAxisAlignedBox(
    const gz::math::AxisAlignedBox &_box,
    const gz::math::Pose3d &_pose)
{
  auto center = _box.Center();

  // Get the 8 corners of the bounding box.
  std::vector<gz::math::Vector3d> vertices;
  vertices.reserve(8);
  vertices.push_back(center + gz::math::Vector3d(-_box.XLength()/2.0,
                                                 _box.YLength()/2.0,
                                                 _box.ZLength()/2.0));
  vertices.push_back(center + gz::math::Vector3d(_box.XLength()/2.0,
                                                 _box.YLength()/2.0,
                                                 _box.ZLength()/2.0));
  vertices.push_back(center + gz::math::Vector3d(-_box.XLength()/2.0,
                                                 -_box.YLength()/2.0,
                                                 _box.ZLength()/2.0));
  vertices.push_back(center + gz::math::Vector3d(_box.XLength()/2.0,
                                                 -_box.YLength()/2.0,
                                                 _box.ZLength()/2.0));

  vertices.push_back(center + gz::math::Vector3d(-_box.XLength()/2.0,
                                                 _box.YLength()/2.0,
                                                 -_box.ZLength()/2.0));
  vertices.push_back(center + gz::math::Vector3d(_box.XLength()/2.0,
                                                 _box.YLength()/2.0,
                                                 -_box.ZLength()/2.0));
  vertices.push_back(center + gz::math::Vector3d(-_box.XLength()/2.0,
                                                 -_box.YLength()/2.0,
                                                 -_box.ZLength()/2.0));
  vertices.push_back(center + gz::math::Vector3d(_box.XLength()/2.0,
                                                 -_box.YLength()/2.0,
                                                 -_box.ZLength()/2.0));


  // Transform corners.
  for (size_t i = 0; i < vertices.size(); ++i)
  {
    auto &v = vertices[i];
    v = _pose.Rot() * v + _pose.Pos();
  }

  gz::math::Vector3d min = vertices[0];
  gz::math::Vector3d max = vertices[0];

  // find min / max of vertices
  for (size_t i = 1; i < vertices.size(); ++i)
  {
    auto &v = vertices[i];

    if (min.X() > v.X())
      min.X() = v.X();
    if (max.X() < v.X())
      max.X() = v.X();
    if (min.Y() > v.Y())
      min.Y() = v.Y();
    if (max.Y() < v.Y())
      max.Y() = v.Y();
    if (min.Z() > v.Z())
      min.Z() = v.Z();
    if (max.Z() < v.Z())
      max.Z() = v.Z();
  }
  return gz::math::AxisAlignedBox(min, max);
}

/////////////////////////////////////////////////
Image convertRGBToBayer(const Image &_image, PixelFormat _bayerFormat)
{
  const unsigned char *sourceImageData = _image.Data<unsigned char>();

  unsigned int width = _image.Width();
  unsigned int height = _image.Height();

  Image destImage(width, height, _bayerFormat);
  unsigned char *destImageData = destImage.Data<unsigned char>();

  if (_bayerFormat == PF_BAYER_RGGB8)
  {
    for (unsigned int i=0; i < width; i++)
    {
      for (unsigned int j=0; j < height; j++)
      {
        if (j%2)
        {
          if (i%2)
          {
            destImageData[i+j*width] = sourceImageData[i*3+j*width*3+2];
          }
          else
          {
            destImageData[i+j*width] = sourceImageData[i*3+j*width*3+1];
          }
        }
        else
        {
          if (i%2)
          {
            destImageData[i+j*width] = sourceImageData[i*3+j*width*3+1];
          }
          else
          {
            destImageData[i+j*width] = sourceImageData[i*3+j*width*3+0];
          }
        }
      }
    }
  }

  else if (_bayerFormat == PF_BAYER_BGGR8)
  {
    for (unsigned int i=0; i < width; i++)
    {
      for (unsigned int j=0; j < height; j++)
      {
        if (j%2)
        {
          if (i%2)
          {
            destImageData[i+j*width] = sourceImageData[i*3+j*width*3+0];
          }
          else
          {
            destImageData[i+j*width] = sourceImageData[i*3+j*width*3+1];
          }
        }
        else
        {
          if (i%2)
          {
            destImageData[i+j*width] = sourceImageData[i*3+j*width*3+1];
          }
          else
          {
            destImageData[i+j*width] = sourceImageData[i*3+j*width*3+2];
          }
        }
      }
    }
  }

  else if (_bayerFormat == PF_BAYER_GBRG8)
  {
    for (unsigned int i=0; i < width; i++)
    {
      for (unsigned int j=0; j < height; j++)
      {
        if (j%2)
        {
          if (i%2)
          {
            destImageData[i+j*width] = sourceImageData[i*3+j*width*3+1];
          }
          else
          {
            destImageData[i+j*width] = sourceImageData[i*3+j*width*3+2];
          }
        }
        else
        {
          if (i%2)
          {
            destImageData[i+j*width] = sourceImageData[i*3+j*width*3+0];
          }
          else
          {
            destImageData[i+j*width] = sourceImageData[i*3+j*width*3+1];
          }
        }
      }
    }
  }

  else if (_bayerFormat == PF_BAYER_GRBG8)
  {
    for (unsigned int i=0; i < width; i++)
    {
      for (unsigned int j=0; j < height; j++)
      {
        if (j%2)
        {
          if (i%2)
          {
            destImageData[i+j*width] = sourceImageData[i*3+j*width*3+1];
          }
          else
          {
            destImageData[i+j*width] = sourceImageData[i*3+j*width*3+0];
          }
        }
        else
        {
          if (i%2)
          {
            destImageData[i+j*width] = sourceImageData[i*3+j*width*3+2];
          }
          else
          {
            destImageData[i+j*width] = sourceImageData[i*3+j*width*3+1];
          }
        }
      }
    }
  }

  return destImage;
}

/////////////////////////////////////////////////
GraphicsAPI defaultGraphicsAPI()
{
#ifdef __APPLE__
  return GraphicsAPI::METAL;
#else
  return GraphicsAPI::OPENGL;
#endif
}

}
}
}
