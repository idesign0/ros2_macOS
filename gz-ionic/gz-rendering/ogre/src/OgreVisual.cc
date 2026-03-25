/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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

#include "gz/rendering/ogre/OgreVisual.hh"
#include "gz/rendering/ogre/OgreWireBox.hh"
#include "gz/rendering/ogre/OgreConversions.hh"
#include "gz/rendering/ogre/OgreStorage.hh"
#include "gz/rendering/Utils.hh"

using namespace gz;
using namespace rendering;

/// \brief Private data for the Ogre2Visual class
class gz::rendering::OgreVisualPrivate
{
  /// \brief True if wireframe mode is enabled
  public: bool wireframe;
};

//////////////////////////////////////////////////
OgreVisual::OgreVisual()
  : dataPtr(new OgreVisualPrivate)
{
  this->dataPtr->wireframe = false;
}

//////////////////////////////////////////////////
OgreVisual::~OgreVisual()
{
}

//////////////////////////////////////////////////
void OgreVisual::SetWireframe(bool _show)
{
  if (this->dataPtr->wireframe == _show)
    return;

  if (!this->ogreNode)
    return;

  this->dataPtr->wireframe = _show;
  for (size_t i = 0; i < this->ogreNode->numAttachedObjects(); ++i)
  {
    Ogre::MovableObject *obj = this->ogreNode->getAttachedObject(i);
    Ogre::Entity *entity = dynamic_cast<Ogre::Entity *>(obj);

    if (!entity)
      continue;

    for (size_t j = 0; j < entity->getNumSubEntities(); ++j)
    {
      Ogre::SubEntity *subEntity = entity->getSubEntity(j);
      Ogre::MaterialPtr entityMaterial = subEntity->getMaterial();
      if (entityMaterial.isNull())
        continue;

      for (size_t techniqueCount = 0;
           techniqueCount < entityMaterial->getNumTechniques();
           ++techniqueCount)
      {
        Ogre::Technique *technique =
            entityMaterial->getTechnique(techniqueCount);

        for (size_t passCount = 0;
             passCount < technique->getNumPasses(); ++passCount)
        {
          Ogre::Pass *pass = technique->getPass(passCount);
          if (_show)
            pass->setPolygonMode(Ogre::PM_WIREFRAME);
          else
            pass->setPolygonMode(Ogre::PM_SOLID);
        }
      }
    }
  }
}

//////////////////////////////////////////////////
bool OgreVisual::Wireframe() const
{
  return this->dataPtr->wireframe;
}

//////////////////////////////////////////////////
void OgreVisual::SetVisible(bool _visible)
{
  if (!this->ogreNode)
    return;

  this->ogreNode->setVisible(_visible);
}

//////////////////////////////////////////////////
void OgreVisual::SetVisibilityFlags(uint32_t _flags)
{
  BaseVisual::SetVisibilityFlags(_flags);

  if (!this->ogreNode)
    return;

  for (size_t i = 0; i < this->ogreNode->numAttachedObjects(); ++i)
    this->ogreNode->getAttachedObject(i)->setVisibilityFlags(_flags);
}

//////////////////////////////////////////////////
GeometryStorePtr OgreVisual::Geometries() const
{
  return this->geometries;
}

//////////////////////////////////////////////////
bool OgreVisual::AttachGeometry(GeometryPtr _geometry)
{
  if (!_geometry)
  {
    gzerr << "Cannot attach null geometry." << std::endl;

    return false;
  }

  if (!this->ogreNode)
  {
    gzerr << "Cannot attach geometry, null Ogre node." << std::endl;
    return false;
  }

  OgreGeometryPtr derived =
      std::dynamic_pointer_cast<OgreGeometry>(_geometry);

  if (!derived)
  {
    gzerr << "Cannot attach geometry created by another render-engine"
          << std::endl;

    return false;
  }

  // Some geometries, like heightmaps, may not have an OgreObject
  Ogre::MovableObject *ogreObj = derived->OgreObject();
  if (ogreObj)
  {
    // set user data for mouse queries
    ogreObj->getUserObjectBindings().setUserAny(
        Ogre::Any(this->Id()));
    ogreObj->setVisibilityFlags(this->visibilityFlags);
    this->ogreNode->attachObject(ogreObj);
  }

  derived->SetParent(this->SharedThis());
  return true;
}

//////////////////////////////////////////////////
bool OgreVisual::DetachGeometry(GeometryPtr _geometry)
{
  if (!this->ogreNode)
  {
    gzerr << "Cannot detach geometry, null Ogre node." << std::endl;
    return false;
  }

  OgreGeometryPtr derived =
      std::dynamic_pointer_cast<OgreGeometry>(_geometry);

  if (!derived)
  {
    gzerr << "Cannot detach geometry created by another render-engine"
          << std::endl;

    return false;
  }

  if (nullptr != derived->OgreObject())
    this->ogreNode->detachObject(derived->OgreObject());
  derived->SetParent(nullptr);
  return true;
}

//////////////////////////////////////////////////
gz::math::AxisAlignedBox OgreVisual::LocalBoundingBox() const
{
  gz::math::AxisAlignedBox box;
  this->BoundsHelper(box, true /* local frame */);
  return box;
}

//////////////////////////////////////////////////
gz::math::AxisAlignedBox OgreVisual::BoundingBox() const
{
  gz::math::AxisAlignedBox box;
  this->BoundsHelper(box, false /* world frame */);
  return box;
}

//////////////////////////////////////////////////
void OgreVisual::BoundsHelper(gz::math::AxisAlignedBox &_box,
    bool _local) const
{
  this->BoundsHelper(_box, _local, this->WorldPose());
}

//////////////////////////////////////////////////
void OgreVisual::BoundsHelper(gz::math::AxisAlignedBox &_box,
    bool _local, const gz::math::Pose3d &_pose) const
{
  if (!this->ogreNode)
    return;

  this->ogreNode->_updateBounds();
  this->ogreNode->_update(false, true);

  gz::math::Vector3d scale = this->WorldScale();

  for (size_t i = 0; i < this->ogreNode->numAttachedObjects(); i++)
  {
    Ogre::MovableObject *obj = this->ogreNode->getAttachedObject(i);

    if (obj->isVisible() && obj->getVisibilityFlags() != GZ_VISIBILITY_GUI)
    {
      Ogre::AxisAlignedBox bb = obj->getBoundingBox();

      gz::math::Vector3d min(0, 0, 0);
      gz::math::Vector3d max(0, 0, 0);
      gz::math::AxisAlignedBox box(min, max);

      // Ogre does not return a valid bounding box for lights.
      if (obj->getMovableType() == Ogre::LightFactory::FACTORY_TYPE_NAME)
      {
        box.Min() = gz::math::Vector3d(-0.5, -0.5, -0.5);
        box.Max()  = gz::math::Vector3d(0.5, 0.5, 0.5);
      }
      else
      {
        Ogre::Vector3 ogreMin = bb.getMinimum();
        Ogre::Vector3 ogreMax = bb.getMaximum();

        // Get ogre bounding boxes and size to object's scale
        min = scale * gz::math::Vector3d(ogreMin.x, ogreMin.y, ogreMin.z);
        max = scale * gz::math::Vector3d(ogreMax.x, ogreMax.y, ogreMax.z);
        box.Min() = min,
        box.Max() = max;

        // Assume world transform
        gz::math::Pose3d transform = _pose;

        // If local frame, calculate transform matrix and set
        if (_local)
        {
          gz::math::Pose3d worldPose = this->WorldPose();
          gz::math::Vector3d parentPos = _pose.Pos();
          gz::math::Quaternion parentRotInv = _pose.Rot().Inverse();
          gz::math::Pose3d localTransform =
            gz::math::Pose3d(
                (parentRotInv * (worldPose.Pos() - parentPos)),
                (parentRotInv * worldPose.Rot()));
          transform = localTransform;
        }

        // Transform to world or local space
        box = transformAxisAlignedBox(box, transform);
      }
      _box.Merge(box);
    }
  }

  auto childNodes = std::dynamic_pointer_cast<OgreNodeStore>(this->Children());
  if (!childNodes)
    return;

  for (auto it = childNodes->Begin(); it != childNodes->End(); ++it)
  {
    OgreVisualPtr visual = std::dynamic_pointer_cast<OgreVisual>(*it);
    if (visual)
      visual->BoundsHelper(_box, _local, _pose);
  }
}

//////////////////////////////////////////////////
void OgreVisual::Init()
{
  BaseVisual::Init();
  this->geometries = OgreGeometryStorePtr(new OgreGeometryStore);
}

//////////////////////////////////////////////////
OgreVisualPtr OgreVisual::SharedThis()
{
  ObjectPtr object = shared_from_this();
  return std::dynamic_pointer_cast<OgreVisual>(object);
}
