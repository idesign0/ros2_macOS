/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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
#include <gz/common/Mesh.hh>
#include <gz/common/MeshManager.hh>
#include <gz/common/SubMesh.hh>

#include "gz/rendering/ogre2/Ogre2Camera.hh"
#include "gz/rendering/ogre2/Ogre2Conversions.hh"
#include "gz/rendering/ogre2/Ogre2DepthCamera.hh"
#include "gz/rendering/ogre2/Ogre2ObjectInterface.hh"
#include "gz/rendering/ogre2/Ogre2RayQuery.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"
#include "gz/rendering/ogre2/Ogre2SegmentationCamera.hh"
#include "gz/rendering/ogre2/Ogre2SelectionBuffer.hh"
#include "gz/rendering/ogre2/Ogre2ThermalCamera.hh"
#include "gz/rendering/ogre2/Ogre2WideAngleCamera.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreCamera.h>
#include <OgreItem.h>
#include <OgreMesh2.h>
#include <OgreRay.h>
#include <OgreSceneManager.h>
#include <Threading/OgreUniformScalableTask.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

// Define this macro to force single threaded version (in case
// you suspect there's a bug caused by multithreading) or
// want to benchmark.
// #define SINGLE_THREADED

// Define this macro to use the old code. Only use this code if for some
// odd reason you suspect there is a regression (particularly with skeletally
// animated objects) or you want to benchmark.
// The new version should produce identical results (without accounting random
// floating point precision issues).
// #define SLOW_METHOD

/// \brief Private data class for Ogre2RayQuery
class gz::rendering::Ogre2RayQueryPrivate
{
  /// \brief Ogre ray scene query object for computing intersection.
  public: Ogre::RaySceneQuery *rayQuery = nullptr;

  //// \brief Pointer to camera
  public: Ogre2CameraPtr camera{nullptr};

  /// \brief Image pos to cast the ray from
  public: math::Vector2i imgPos = math::Vector2i::Zero;

  /// \brief thread that ray query is created in
  public: std::thread::id threadId;

  //// \brief See RayQuery::SetPreferGpu
  public: bool preferGpu = true;
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2RayQuery::Ogre2RayQuery()
    : dataPtr(new Ogre2RayQueryPrivate)
{
  this->dataPtr->threadId = std::this_thread::get_id();
}

//////////////////////////////////////////////////
Ogre2RayQuery::~Ogre2RayQuery()
{
  if (!this->Scene()->IsInitialized())
    return;

  if (this->dataPtr->rayQuery)
  {
    Ogre2ScenePtr ogreScene =
        std::dynamic_pointer_cast<Ogre2Scene>(this->Scene());
    if (ogreScene)
    {
      ogreScene->OgreSceneManager()->destroyQuery(this->dataPtr->rayQuery);
      this->dataPtr->rayQuery = nullptr;
    }
  }
}

//////////////////////////////////////////////////
void Ogre2RayQuery::SetFromCamera(const CameraPtr &_camera,
    const math::Vector2d &_coord)
{
  // convert to normalized screen pos for ogre
  math::Vector2d screenPos((_coord.X() + 1.0) / 2.0, (_coord.Y() - 1.0) / -2.0);

  Ogre2CameraPtr camera = std::dynamic_pointer_cast<Ogre2Camera>(_camera);
  if (camera)
  {
    this->dataPtr->camera = camera;
  }

  Ogre2ObjectInterfacePtr ogre2ObjectInterface =
      std::dynamic_pointer_cast<Ogre2ObjectInterface>(_camera);
  if (!ogre2ObjectInterface)
  {
    gzwarn << "Camera does not support ray query\n";
    return;
  }

  Ogre::Ray ray = ogre2ObjectInterface->OgreCamera()->getCameraToViewportRay(
      screenPos.X(), screenPos.Y());

  auto originMath = Ogre2Conversions::Convert(ray.getOrigin());
  if (originMath.IsFinite())
  {
    this->origin = originMath;
  }
  else
  {
    gzwarn << "Attempted to set non-finite origin from camera ["
           << camera->Name() << "]" << std::endl;
  }

  auto directionMath = Ogre2Conversions::Convert(ray.getDirection());
  if (directionMath.IsFinite())
  {
    this->direction = directionMath;
  }
  else
  {
    gzwarn << "Attempted to set non-finite direction from camera ["
           << camera->Name() << "]" << std::endl;
  }

  this->dataPtr->imgPos.X() = static_cast<int>(
      screenPos.X() * _camera->ImageWidth());
  this->dataPtr->imgPos.Y() = static_cast<int>(
      screenPos.Y() * _camera->ImageHeight());
}

//////////////////////////////////////////////////
void Ogre2RayQuery::SetFromCamera(const WideAngleCameraPtr &_camera,
                                  uint32_t _faceIdx,
                                  const math::Vector2d &_coord)
{
  // convert to normalized screen pos for ogre
  math::Vector2d screenPos((_coord.X() + 1.0) / 2.0, (_coord.Y() - 1.0) / -2.0);

  Ogre2WideAngleCameraPtr camera =
    std::dynamic_pointer_cast<Ogre2WideAngleCamera>(_camera);
  this->dataPtr->camera.reset();

  Ogre::Ray ray = camera->CameraToViewportRay(screenPos, _faceIdx);

  auto originMath = Ogre2Conversions::Convert(ray.getOrigin());
  if (originMath.IsFinite())
  {
    this->origin = originMath;
  }
  else
  {
    gzwarn << "Attempted to set non-finite origin from camera ["
           << camera->Name() << "]" << std::endl;
  }

  auto directionMath = Ogre2Conversions::Convert(ray.getDirection());
  if (directionMath.IsFinite())
  {
    this->direction = directionMath;
  }
  else
  {
    gzwarn << "Attempted to set non-finite direction from camera ["
           << camera->Name() << "]" << std::endl;
  }

  this->dataPtr->imgPos.X() =
    static_cast<int>(screenPos.X() * _camera->ImageWidth());
  this->dataPtr->imgPos.Y() =
    static_cast<int>(screenPos.Y() * _camera->ImageHeight());
}

//////////////////////////////////////////////////
void Ogre2RayQuery::SetPreferGpu(bool _preferGpu)
{
  this->dataPtr->preferGpu = _preferGpu;
}

//////////////////////////////////////////////////
bool Ogre2RayQuery::UsesGpu() const
{
  if (!this->dataPtr->preferGpu ||         //
      !this->dataPtr->camera ||            //
      !this->dataPtr->camera->Parent() ||  //
      std::this_thread::get_id() != this->dataPtr->threadId)
  {
    return false;
  }

  return true;
}

//////////////////////////////////////////////////
RayQueryResult Ogre2RayQuery::ClosestPoint(bool _forceSceneUpdate)
{
  if (!this->UsesGpu())
  {
    // use legacy method for backward compatibility if no camera is set or
    // camera is not attached in the scene tree or
    // this function is called from non-rendering thread
    return this->ClosestPointByIntersection(_forceSceneUpdate);
  }
  else
  {
    // the VisualAt function is a hack to force creation of the selection
    // buffer object
    // todo(anyone) Make Camera::SetSelectionBuffer function public?
    if (!this->dataPtr->camera->SelectionBuffer())
      this->dataPtr->camera->VisualAt(math::Vector2i(0, 0));

    return this->ClosestPointBySelectionBuffer();
  }
}

//////////////////////////////////////////////////
RayQueryResult Ogre2RayQuery::ClosestPointBySelectionBuffer()
{
  // update selection buffer dimension in case window is resized
  this->dataPtr->camera->SelectionBuffer()->SetDimensions(
    this->dataPtr->camera->ImageWidth(), this->dataPtr->camera->ImageHeight());

  RayQueryResult result;
  Ogre::MovableObject *ogreObj = nullptr;
  math::Vector3d point;
  bool success = this->dataPtr->camera->SelectionBuffer()->ExecuteQuery(
      this->dataPtr->imgPos.X(), this->dataPtr->imgPos.Y(), ogreObj, point);
  result.distance = -1;

  if (success)
  {
    double distance = this->dataPtr->camera->WorldPosition().Distance(point)
        - this->dataPtr->camera->NearClipPlane();
    unsigned int objectId = 0;
    if (ogreObj)
    {
      if (!ogreObj->getUserObjectBindings().getUserAny().isEmpty() &&
          ogreObj->getUserObjectBindings().getUserAny().getType() ==
          typeid(unsigned int))
      {
        auto userAny = ogreObj->getUserObjectBindings().getUserAny();
        objectId = Ogre::any_cast<unsigned int>(userAny);
      }
    }
    if (!std::isinf(distance))
    {
      result.distance = distance;
      result.point = point;
      result.objectId = objectId;
    }
  }
  return result;
}

//////////////////////////////////////////////////

/// \brief This class performs a Triangle-level raycast over the broadphase
/// results returned by OgreNext spreading the work as evenly as possible
/// across multiple threads
///
/// On multicore machines this can lead to considerable speed ups, specially
/// if the scene has a lot of triangles.
class GZ_RENDERING_OGRE2_HIDDEN ThreadedTriRay final
  : public Ogre::UniformScalableTask
{
  /// \brief Stores the result Ogre2RayQueryPrivate::rayQuery->execute
  /// that we will iterate for triangle intersection matches.
  private: Ogre::RaySceneQueryResult ogreResult;

  /// \brief Raycast's origin
  private: const Ogre::Vector3 rayOrigin;

  /// \brief Raycast's direction
  private: const Ogre::Vector3 rayDir;

  /// \brief Stores the results of our triangle intersections.
  /// One entry per thread.
  public: std::vector<RayQueryResult> collectedResults;

  /// \brief Constructor
  /// \param[in, out] _ogreResult Ray Query done by Ogre that we will iterate
  /// We take ownership of its internal pointer, thus it becomes empty
  /// afterwards
  /// \param[in] _rayOrigin Raycast's origin
  /// \param[in] _rayDir Raycast's direction
  /// \param[in] _numThreads Number of worker threads
  public: ThreadedTriRay(Ogre::RaySceneQueryResult &_ogreResult,
                         const Ogre::Vector3 &_rayOrigin,
                         const Ogre::Vector3 &_rayDir,
                         size_t _numThreads) :
      rayOrigin(_rayOrigin),
      rayDir(_rayDir)
  {
    this->ogreResult.swap(_ogreResult);
    this->collectedResults.resize(_numThreads);
  }

  // Documentation inherited
  public: void execute(size_t _threadId, size_t _numThreads) override;

  /// \brief To be run after parallel execution is done. It will iterate
  /// through the results of every thread and return the closest match
  /// \return The closest match (it may be empty)
  public: RayQueryResult CollapseCollectedResults();
};

//////////////////////////////////////////////////
void ThreadedTriRay::execute(size_t _threadId, size_t _numThreads)
{
  const unsigned int numThreads = static_cast<unsigned int>(_numThreads);
  const unsigned int threadId = static_cast<unsigned int>(_threadId);

  double distance = std::numeric_limits<double>::max();

  RayQueryResult result;

  // Iterate over all the results.
  for (auto iter = this->ogreResult.begin(); iter != this->ogreResult.end();
       ++iter)
  {
    if (iter->distance <= 0.0)
      continue;

    if (!iter->movable || !iter->movable->getVisible())
      continue;

    auto userAny = iter->movable->getUserObjectBindings().getUserAny();
    if (!userAny.isEmpty() && userAny.getType() == typeid(unsigned int) &&
        iter->movable->getMovableType() == "Item")
    {
      Ogre::Item *ogreItem = static_cast<Ogre::Item *>(iter->movable);

      // mesh factory creates name with ::CENTER or ::ORIGINAL depending on
      // the params passed in the MeshDescriptor when loading the mesh
      // so strip off the suffix
      std::string meshName = ogreItem->getMesh()->getName();
      size_t idx = meshName.find("::");
      if (idx != std::string::npos)
        meshName.resize(idx);

      const common::Mesh *mesh =
        common::MeshManager::Instance()->MeshByName(meshName);

      if (!mesh)
        continue;

      Ogre::Matrix4 transform = ogreItem->_getParentNodeFullTransform();

      const bool bIsAffine = transform.isAffine();

      Ogre::Ray mouseRay;
#ifndef SLOW_METHOD
      if (bIsAffine)
      {
        Ogre::Matrix4 invTransform = transform.inverse();
        Ogre::Matrix3 invTransform3x3;
        invTransform.extract3x3Matrix(invTransform3x3);
        mouseRay = Ogre::Ray(invTransform * this->rayOrigin,
                             (invTransform3x3 * this->rayDir).normalisedCopy());
      }
      else
#endif
      {
        mouseRay = Ogre::Ray(this->rayOrigin, this->rayDir);
      }

      // test for hitting individual triangles on the mesh
      for (unsigned int j = 0; j < mesh->SubMeshCount(); ++j)
      {
        auto s = mesh->SubMeshByIndex(j);
        auto submesh = s.lock();
        if (!submesh || submesh->VertexCount() < 3u)
          continue;
        const unsigned int indexCount = submesh->IndexCount();

        const math::Vector3d *RESTRICT_ALIAS vertices =
          submesh->VertexPtr();
        const unsigned int *RESTRICT_ALIAS indices = submesh->IndexPtr();

        std::pair<bool, Ogre::Real> bestHit = {
          false, std::numeric_limits<Ogre::Real>::max()
        };

        // Round up to next multiple of numThreads and divide by it
        unsigned int indexCountPerThread =
          (indexCount + (numThreads - 1u)) / numThreads;
        // indexCountPerThread must be multiple of 3
        indexCountPerThread = ((indexCountPerThread + 2u) / 3u) * 3u;

        unsigned int indexStart =
          std::min(indexCountPerThread * threadId, indexCount);
        unsigned int indexEnd =
          std::min(indexCountPerThread * (threadId + 1u), indexCount);

        for (unsigned int k = indexStart; k < indexEnd; k += 3)
        {
          if (indexCount <= k + 2)
            continue;

#ifdef SLOW_METHOD
          math::Vector3d vertexA = submesh->Vertex(submesh->Index(k));
          math::Vector3d vertexB = submesh->Vertex(submesh->Index(k + 1));
          math::Vector3d vertexC = submesh->Vertex(submesh->Index(k + 2));

          Ogre::Vector3 worldVertexA =
            transform * Ogre2Conversions::Convert(vertexA);
          Ogre::Vector3 worldVertexB =
            transform * Ogre2Conversions::Convert(vertexB);
          Ogre::Vector3 worldVertexC =
            transform * Ogre2Conversions::Convert(vertexC);
#else
          Ogre::Vector3 worldVertexA, worldVertexB, worldVertexC;

          if (bIsAffine)
          {
            worldVertexA = Ogre2Conversions::Convert(vertices[indices[k]]);
            worldVertexB = Ogre2Conversions::Convert(vertices[indices[k + 1]]);
            worldVertexC = Ogre2Conversions::Convert(vertices[indices[k + 2]]);
          }
          else
          {
            worldVertexA =
              transform * Ogre2Conversions::Convert(vertices[indices[k]]);
            worldVertexB =
              transform * Ogre2Conversions::Convert(vertices[indices[k + 1]]);
            worldVertexC =
              transform * Ogre2Conversions::Convert(vertices[indices[k + 2]]);
          }
#endif

          // check for a hit against this triangle
          std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(
            mouseRay, worldVertexA, worldVertexB, worldVertexC, true, false);

          // if it was a hit check if its the closest
          if (hit.first && hit.second < bestHit.second)
          {
            bestHit = hit;
          }
        }

        if (bestHit.first && distance > bestHit.second)
        {
          // this is the closest so far, save it off
          distance = bestHit.second;
          result.distance = distance;
          if (bIsAffine)
          {
            result.point = Ogre2Conversions::Convert(
              transform * mouseRay.getPoint(distance));
          }
          else
          {
            result.point =
              Ogre2Conversions::Convert(mouseRay.getPoint(distance));
          }
          result.objectId = Ogre::any_cast<unsigned int>(userAny);
        }
      }
    }
  }

  collectedResults[_threadId] = result;
}

//////////////////////////////////////////////////
RayQueryResult ThreadedTriRay::CollapseCollectedResults()
{
  RayQueryResult result;
  for (const RayQueryResult &entry : this->collectedResults)
  {
    if (entry)
    {
      if (!result || entry.distance < result.distance)
      {
        result = entry;
      }
    }
  }
  return result;
}

//////////////////////////////////////////////////
RayQueryResult Ogre2RayQuery::ClosestPointByIntersection(bool _forceSceneUpdate)
{
  RayQueryResult result;
  Ogre2ScenePtr ogreScene =
      std::dynamic_pointer_cast<Ogre2Scene>(this->Scene());
  if (!ogreScene)
    return result;

  Ogre::SceneManager *ogreSceneManager = ogreScene->OgreSceneManager();

  if (_forceSceneUpdate)
  {
    ogreSceneManager->updateSceneGraph();
  }

  const Ogre::Vector3 rayOrigin = Ogre2Conversions::Convert(this->origin);
  const Ogre::Vector3 rayDir = Ogre2Conversions::Convert(this->direction);

  Ogre::Ray mouseRay(rayOrigin, rayDir);

  if (!this->dataPtr->rayQuery)
  {
    this->dataPtr->rayQuery = ogreSceneManager->createRayQuery(mouseRay);
  }
  this->dataPtr->rayQuery->setSortByDistance(true);
  this->dataPtr->rayQuery->setRay(mouseRay);

  // Perform the scene query
  Ogre::RaySceneQueryResult &ogreResult = this->dataPtr->rayQuery->execute();

#ifndef SINGLE_THREADED
  ThreadedTriRay rayTask(ogreResult, rayOrigin, rayDir,
                         ogreSceneManager->getNumWorkerThreads());
  ogreSceneManager->executeUserScalableTask(&rayTask, true);
#else
  ThreadedTriRay rayTask(ogreResult, rayOrigin, rayDir, 1u);
  rayTask.execute(0u, 1u);
#endif

  result = rayTask.CollapseCollectedResults();

  return result;
}
