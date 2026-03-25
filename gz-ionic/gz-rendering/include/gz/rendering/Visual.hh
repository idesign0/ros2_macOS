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
#ifndef GZ_RENDERING_VISUAL_HH_
#define GZ_RENDERING_VISUAL_HH_

#include <string>
#include <gz/math/AxisAlignedBox.hh>
#include "gz/rendering/config.hh"
#include "gz/rendering/Node.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \class Visual Visual.hh gz/rendering/Visual.hh
    /// \brief Represents a visual node in a scene graph. A Visual is the only
    /// node that can have Geometry and other Visual children.
    class GZ_RENDERING_VISIBLE Visual :
      public virtual Node
    {
      /// \brief Destructor
      public: virtual ~Visual();

      /// \brief Get the number of geometries attached to this visual
      /// \return The number of geometries attached to this visual
      public: virtual unsigned int GeometryCount() const = 0;

      /// \brief Determine if given geometry is attached to this visual
      /// \param[in] _geometry Geometry in question
      /// \return True if given node is attached to this visual
      public: virtual bool HasGeometry(ConstGeometryPtr _geometry) const = 0;

      /// \brief Get geometry at given index. If no geometry exists at given
      /// index, NULL will be returned.
      /// \param[in] _index Index of the desired geometry
      /// \return The specified geometry
      public: virtual GeometryPtr GeometryByIndex(
                  unsigned int _index) const = 0;

      /// \brief Add the given geometry to this visual. If the given node is
      /// already attached, no work will be done.
      /// \param[in] _geometry Geometry to be added
      public: virtual void AddGeometry(GeometryPtr _geometry) = 0;

      /// \brief Remove the given geometry from this visual. If the given node
      /// is not a child of this visual, no work will be done.
      /// \param[in] _geometry Geometry to be removed
      /// \return The removed geometry
      public: virtual GeometryPtr RemoveGeometry(GeometryPtr _geometry) = 0;

      /// \brief Remove the geometry at the given index from this visual. If the
      /// specified node is not attached this visual, no work will be done.
      /// \param[in] _index Index of the geometry to be removed
      /// \return The removed geometry
      public: virtual GeometryPtr RemoveGeometryByIndex(
                  unsigned int _index) = 0;

      /// \brief Remove all attached geometries from this visual
      public: virtual void RemoveGeometries() = 0;

      /// \brief Set the material for all attached visuals and geometries. The
      /// specified material will be retrieved from Scene::Material using
      /// the given material name. If no material is registered with the given
      /// name, no work will be done.
      /// \param[in] _name Name of the material to be assigned
      /// \param[in] _unique True if the specified material should be cloned
      public: virtual void SetMaterial(const std::string &_name,
                  bool _unique = true) = 0;

      /// \brief Set the material for all attached visuals and geometries
      /// \param[in] _material Name of the material to be assigned
      /// \param[in] _unique True if the specified material should be cloned
      public: virtual void SetMaterial(MaterialPtr _material,
                  bool _unique = true) = 0;

      /// \brief Set the material for all attached visuals only
      /// \param[in] _material Name of the material to be assigned
      /// \param[in] _unique True if the specified material should be cloned
      public: virtual void SetChildMaterial(MaterialPtr _material,
                  bool _unique = true) = 0;

      /// \brief Set the material for all attached geometries only
      /// \param[in] _material Name of the material to be assigned
      /// \param[in] _unique True if the specified material should be cloned
      public: virtual void SetGeometryMaterial(MaterialPtr _material,
                  bool _unique = true) = 0;

      /// \brief Get the material assigned to attached visuals and geometries.
      /// \return the Pointer to the material assigned to this visual. If the
      /// material is cloned at the time it is set to this visual, the cloned
      /// material will be returned.
      public: virtual MaterialPtr Material() const = 0;

      /// \brief Enable or disable wireframe
      /// \param[in] _show True to enable wireframe
      public: virtual void SetWireframe(bool _show) = 0;

      /// \brief Get whether wireframe is enabled for this visual.
      /// \return True if wireframe is enabled for this visual.
      public: virtual bool Wireframe() const = 0;

      /// \brief Specify if this visual is visible
      /// \param[in] _visible True if this visual should be made visible
      public: virtual void SetVisible(bool _visible) = 0;

      /// \brief Tells Render Engine this Visual will be static (i.e.
      /// won't move, rotate or scale)
      /// You can still move, rotate or scale the Visual; however doing so
      /// has a performance impact. How much of an impact will depend on the
      /// rendering backend being used.
      /// \remark Performance impact of switching staticness depends on
      /// on engine. In ogre2 it isn't expensive but it isn't
      /// free either. Try to minimize transitions.
      /// Raytracing engines may rely on this information for their
      /// BVH structures
      /// TODO(anyone): Staticness should be ideally be supplied
      /// during construction for maximum performance
      /// \remark ogre2 specific: OgreNext keeps 2 lists of objects.
      /// Dynamic and Static. Every frame OgreNext iterates through all nodes &
      /// items in the dynamic lists and forcefully updates them (regardless of
      /// whether they have actually changed).
      /// This may sound slow but Ogre 1.x worked by only updating those objects
      /// that have changed, and it was slower. CPUs care a lot more about
      /// processing in bulk and having all the data hot in cache.
      ///
      /// For the static list, it only iterates through the entire list in that
      /// frame when the whole list is tagged as dirty. If one element changes,
      /// then the whole list is dirty (OgreNext *might* be able to optimize it
      /// and only update a part of the list by narrowing).
      ///
      /// Changing a static node transform implies calling OgreNext's
      /// notifyStaticDirty. This is very cheap so it can be called whenever
      /// a static object changes. The only thing that needs to be taken into
      /// account is that notifyStaticDirty should be called as little as
      /// possible. And by "as little as possible" that means ideally it should
      /// not be called ever except when loading a scene or when something
      /// relevant changed. If it gets called e.g. once per frame or more then
      /// it's the same as having all objects dynamic.
      ///
      /// Thus if an object is static, make sure you don't keep moving around
      /// because it negates the performance of *all* static objects.
      ///
      /// \remark (INTERNAL) For implementations:
      ///   Dynamic Scene Node + Dynamic MovableObject = Valid
      ///   Static Scene Node  + Static MovableObject  = Valid
      ///   Static Scene Node  + Dynamic MovableObject = Valid, but
      ///     rarely makes sense
      ///   Dynamic Scene Node + Static MovableObject  = Invalid
      /// \param[in] _static True if this visual should be made static
      public: virtual void SetStatic(bool _static) = 0;

      /// \brief Get whether the visual is static.
      /// \return True if the visual is static, false otherwise
      /// \sa SetStatic
      public: virtual bool Static() const = 0;

      /// \brief Set visibility flags
      /// \param[in] _flags Visibility flags
      public: virtual void SetVisibilityFlags(uint32_t _flags) = 0;

      /// \brief Get visibility flags
      /// \return visibility flags
      public: virtual uint32_t VisibilityFlags() const = 0;

      /// \brief Add visibility flags
      /// \param[in] _flags Visibility flags
      public: virtual void AddVisibilityFlags(uint32_t _flags) = 0;

      /// \brief Remove visibility flags
      /// \param[in] _flags Visibility flags
      public: virtual void RemoveVisibilityFlags(uint32_t _flags) = 0;

      /// \brief Get the bounding box in world frame coordinates.
      /// \return The axis aligned bounding box
      public: virtual gz::math::AxisAlignedBox BoundingBox() const = 0;

      /// \brief Get the local bounding box of the visual.
      /// \return The local bounding box
      public: virtual gz::math::AxisAlignedBox LocalBoundingBox()
              const = 0;

      /// \brief Clone the visual (and its children) with a new name.
      /// \param[in] _name Name of the cloned Visual. Set this to an empty
      /// string to auto-generate a unique name for the cloned visual.
      /// \param[in] _newParent Parent of the cloned Visual. Set to nullptr if
      /// the cloned visual should have no parent.
      /// \return The visual. nullptr is returned if cloning failed.
      public: virtual VisualPtr Clone(const std::string &_name,
                  NodePtr _newParent) const = 0;
    };
    }
  }
}

#endif
