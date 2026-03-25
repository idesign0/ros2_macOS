/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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
#ifndef GZ_COMMON_MESH_HH_
#define GZ_COMMON_MESH_HH_

#include <memory>
#include <string>
#include <vector>

#include <gz/math/Vector3.hh>
#include <gz/math/Vector2.hh>

#include <gz/utils/ImplPtr.hh>

#include <gz/common/graphics/Types.hh>
#include <gz/common/graphics/Export.hh>

namespace gz
{
  namespace common
  {
    class Material;
    class Skeleton;

    /// \class Mesh Mesh.hh gz/common/Mesh.hh
    /// \brief A 3D mesh
    class GZ_COMMON_GRAPHICS_VISIBLE Mesh
    {
      /// \brief Constructor
      public: Mesh();

      /// \brief Destructor
      public: virtual ~Mesh();

      /// \brief Set the path which contains the mesh resource
      /// \param[in] _path The file path
      public: void SetPath(const std::string &_path);

      /// \brief Get the path which contains the mesh resource
      /// \return The path to the mesh resource
      public: std::string Path() const;

      /// \brief Set the name of this mesh
      /// \param[in] _name The name to set
      public: void SetName(const std::string &_name);

      /// \brief Get the name of this mesh
      /// \return Name of the mesh.
      public: std::string Name() const;

      /// \brief Get the maximun X, Y, Z values
      /// \return The upper bounds of the bounding box
      public: gz::math::Vector3d Max() const;

      /// \brief Get the minimum X, Y, Z values
      /// \return The lower bounds of the bounding box
      public: gz::math::Vector3d Min() const;

      /// \brief Get the number of vertices
      /// \return The number of vertices
      public: unsigned int VertexCount() const;

      /// \brief Get the number of normals
      /// \return The number of normals
      public: unsigned int NormalCount() const;

      /// \brief Get the number of indices
      /// \return The number of indices
      public: unsigned int IndexCount() const;

      /// \brief Get the number of texture coordinates
      /// \return The number of texture coordinates
      public: unsigned int TexCoordCount() const;

      /// \brief Add a submesh mesh.
      /// This can be an expensive since _child is copied into this mesh.
      /// \sa AddSubMesh(std::unique_ptr<SubMesh> _child);
      /// \param[in] _child the submesh
      /// \return Weak pointer to the added submesh
      public: std::weak_ptr<SubMesh> AddSubMesh(const SubMesh &_child);

      /// \brief Add a submesh mesh. This transfers ownership of _child
      /// to this mesh. The value of _child after this call is nullptr.
      /// \param[in] _child the submesh
      /// \return Weak pointer to the added submesh
      public: std::weak_ptr<SubMesh> AddSubMesh(
                  std::unique_ptr<SubMesh> _child);

      /// \brief Get the number of child submeshes.
      /// \return The number of submeshes.
      public: unsigned int SubMeshCount() const;

      /// \brief Add a material to the mesh
      /// \param[in] _mat The material to add.
      /// \return Index of this material
      public: int AddMaterial(const MaterialPtr &_mat);

      /// \brief Get the number of materials
      /// \return The number of materials
      public: unsigned int MaterialCount() const;

      /// \brief Get a material by index
      /// \param[in] _index The index of the material.
      /// \return The material or NULL if the index is out of bounds
      public: MaterialPtr MaterialByIndex(const unsigned int _index) const;

      /// \brief Get the index of material
      /// \param[in] _mat The material
      /// \return The index of the material or -1 if not found, or _mat is
      /// null.
      public: int IndexOfMaterial(const Material *_mat) const;

      /// \brief Get a child submesh by index
      /// \param[in] _index Index of the submesh
      /// \return The submesh or nullptr if the index is out of bounds.
      public: std::weak_ptr<SubMesh> SubMeshByIndex(
                  const unsigned int _index) const;

      /// \brief Get a child submesh by name.
      /// \param[in] _name Name of the submesh.
      /// \return The submesh or nullptr if the _name is not found.
      public: std::weak_ptr<SubMesh> SubMeshByName(
                  const std::string &_name) const;

      /// \brief Put all the data into flat arrays
      /// \param[out] _vertArr the vertex array
      /// \param[out] _indArr the index array
      public: void FillArrays(double **_vertArr, int **_indArr) const;

      /// \brief Recalculate all the normals of each face defined by three
      /// indices.
      public: void RecalculateNormals();

      /// \brief Get axis-aligned bounding box in the mesh frame
      /// \param[out] _center Center of the bounding box
      /// \param[out] _minXYZ Bounding box minimum values
      /// \param[out] _maxXYZ Bounding box maximum values
      public: void AABB(gz::math::Vector3d &_center,
                        gz::math::Vector3d &_minXYZ,
                        gz::math::Vector3d &_maxXYZ) const;

      /// \brief Generate texture coordinates using spherical projection
      /// from center
      /// \param[in] _center Center of the projection
      public: void GenSphericalTexCoord(
                  const gz::math::Vector3d &_center);

      /// \brief Get the skeleton to which this mesh is attached.
      /// \return Pointer to skeleton or nullptr if none is present.
      public: SkeletonPtr MeshSkeleton() const;

      /// \brief Set the mesh skeleton
      /// \param[in] _skel Skeleton to attach to the mesh.
      public: void SetSkeleton(const SkeletonPtr &_skel);

      /// \brief Check if mesh is attached to a skeleton.
      /// \return True if mesh is attached to a skeleton.
      public: bool HasSkeleton() const;

      /// \brief Scale all vertices by _factor
      /// \param _factor Scaling factor
      public: void Scale(const gz::math::Vector3d &_factor);

      /// \brief Set the scale all vertices
      /// \param[in] _factor Scaling vector
      public: void SetScale(const gz::math::Vector3d &_factor);

      /// \brief Move the center of the mesh to the given coordinate in the
      /// mesh frame. This will move all the vertices in all submeshes.
      /// \param[in] _center Location of the mesh center.
      public: void Center(const gz::math::Vector3d &_center =
                          gz::math::Vector3d::Zero);

      /// \brief Move all vertices in all submeshes by _vec.
      /// \param[in] _vec Amount to translate vertices.
      public: void Translate(const gz::math::Vector3d &_vec);

      /// \brief Compute the volume of this mesh. The primitive type
      /// must be TRIANGLES.
      ///
      /// This function utilizes the mesh volume formula from
      /// "Efficient feature extraction for 2d/3d objects in mesh
      /// representation" by Cha Zhang and Tsuhan Chen. Link:
      /// http://chenlab.ece.cornell.edu/Publication/Cha/icip01_Cha.pdf.
      /// The formula does not check for a closed (water tight) mesh.
      ///
      /// \return The mesh's volume. The volume can be zero if
      /// the primitive type of the submeshes is not TRIANGLES,
      /// or there are no submeshes.
      public: double Volume() const;

      /// \brief Private data pointer.
      GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
    };
  }
}

#endif
