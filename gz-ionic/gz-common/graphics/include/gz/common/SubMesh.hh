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
#ifndef GZ_COMMON_SUBMESH_HH_
#define GZ_COMMON_SUBMESH_HH_

#include <memory>
#include <optional>
#include <string>

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
    class NodeAssignment;

    /// \brief A child mesh
    class GZ_COMMON_GRAPHICS_VISIBLE SubMesh
    {
      /// \brief An enumeration of the geometric mesh primitives
      public: enum PrimitiveType
              {
                /// \brief Point. 1 vertex per point
                POINTS,
                /// \brief Line. 2 vertices per line
                LINES,
                /// \brief Connected lines. 2 vertices for the first line and
                /// every vertex specified afterwards creates a new line.
                LINESTRIPS,
                /// \brief Triangle type. 3 vertices per triangle.
                TRIANGLES,
                /// \brief Connected triangle in fan shape. 3 vertices for
                ///  the first triangle and every vertex specified afterwards
                /// creates a new triangle.
                TRIFANS,
                /// \brief Connected triangle. 3 vertices for the first
                /// triangle and every vertex specified afterwards creates a
                /// new triangle.
                TRISTRIPS
              };

      /// \brief Constructor
      public: SubMesh();

      /// \brief Constructor
      /// \param _name Name of the submesh.
      public: explicit SubMesh(const std::string &_name);

      /// \brief Destructor
      public: virtual ~SubMesh();

      /// \brief Set the name of this mesh
      /// \param[in] _n The name to set
      public: void SetName(const std::string &_name);

      /// \brief Get the name of this mesh
      /// \return The name
      public: std::string Name() const;

      /// \brief Set the primitive type
      /// \param[in] _type The primitive type
      public: void SetPrimitiveType(PrimitiveType _type);

      /// \brief Get the primitive type
      /// \return The primitive type
      public: PrimitiveType SubMeshPrimitiveType() const;

      /// \brief Add an index to the mesh
      /// \param[in] _index The new vertex index
      public: void AddIndex(const unsigned int _index);

      /// \brief Add a vertex to the mesh
      /// \param[in] _v The new position
      public: void AddVertex(const gz::math::Vector3d &_v);

      /// \brief Add a vertex to the mesh
      /// \param[in] _x Position along x
      /// \param[in] _y Position along y
      /// \param[in] _z Position along z
      public: void AddVertex(const double _x, const double _y, const double _z);

      /// \brief Add a normal to the mesh
      /// \param[in] _n The normal
      public: void AddNormal(const gz::math::Vector3d &_n);

      /// \brief Add a normal to the mesh
      /// \param[in] _x Position along x
      /// \param[in] _y Position along y
      /// \param[in] _z Position along z
      public: void AddNormal(const double _x, const double _y, const double _z);

      /// \brief Add a texture coord to the mesh. If multiple texture
      /// coordinate sets exist, this function adds it to the first texture
      /// coordinate set in the submesh. If no previous texture coordinates
      /// exist, it is added to set 0.
      /// \param[in] _u Position along u
      /// \param[in] _v Position along v
      /// \sa AddTexCoordBySet
      public: void AddTexCoord(const double _u, const double _v);

      /// \brief Add a texture coordinate to the mesh. If multiple texture
      /// coordinate sets exist, this function adds it to the first texture
      /// coordinate set in the submesh. If no previous texture coordinates
      /// exist, it is added to set 0.
      /// \param[in] _uv The texture coordinate
      /// \sa AddTexCoordBySet
      public: void AddTexCoord(const gz::math::Vector2d &_uv);

      /// \brief Add a texture coord to a texture coordinate set of the mesh
      /// \param[in] _u Position along u
      /// \param[in] _v Position along v
      /// \param[in] _setIndex Texture coordinate set index
      public: void AddTexCoordBySet(double _u, double _v,
          unsigned int _setIndex);

      /// \brief Add a texture coord to a texture coordinate set of the mesh
      /// \param[in] _u Position along u
      /// \param[in] _v Position along v
      /// \param[in] _setIndex Texture coordinate set index
      public: void AddTexCoordBySet(const gz::math::Vector2d &_uv,
          unsigned int _setIndex);

      /// \brief Add a vertex - skeleton node assignment
      /// \param[in] _vertex The vertex index
      /// \param[in] _node The node index
      /// \param[in] _weight The weight (between 0 and 1)
      public: void AddNodeAssignment(const unsigned int _vertex,
                                     const unsigned int _node,
                                     const float _weight);
      /// \brief Get a vertex
      /// \param[in] _index Index of the vertex
      /// \return Coordinates of the vertex or gz::math::Vector3d::Zero
      /// if the index is out of bounds.
      /// \sa bool HasVertex(const unsigned int) const
      public: gz::math::Vector3d Vertex(const unsigned int _index) const;

      /// \brief Get the raw vertex pointer. This is unsafe, it is the
      /// caller's responsibility to ensure it's not indexed out of bounds.
      /// The valid range is [0; VertexCount())
      /// \return Raw vertices
      public: const gz::math::Vector3d* VertexPtr() const;

      /// \brief Set a vertex
      /// \param[in] _index Index of the vertex
      /// \param[in] _v The new vertex coordinate
      public: void SetVertex(const unsigned int _index,
                             const gz::math::Vector3d &_v);

      /// \brief Get a normal
      /// \param[in] _index The normal index
      /// \return The normal direction or gz::math::Vector3d::Zero
      ///  if index is out of bounds.
      /// \sa bool HasNormal(const unsigned int _index);
      public: gz::math::Vector3d Normal(const unsigned int _index) const;

      /// \brief Set a normal
      /// \param[in] _index Index of the normal that will be set.
      /// \param[in] _n The new normal direction
      public: void SetNormal(const unsigned int _index,
                  const gz::math::Vector3d &_n);

      /// \brief Get a texture coordinate
      /// \param[in] _index the texture index
      /// \return The texture coordinate or gz::math::Vector2d::Zero
      /// if index is out of bounds.
      /// \sa bool HasTexCoord(const unsigned int _index) const
      public: gz::math::Vector2d TexCoord(
                  const unsigned int _index) const;

      /// \brief Get a texture coordinate for a texture coordinate set
      /// \param[in] _index the texture index
      /// \return The texture coordinate or gz::math::Vector2d::Zero
      /// if index is out of bounds.
      /// \param[in] _setIndex Texture coordinate set index
      /// \sa bool HasTexCoordBySet(unsigned int _index, unsigned int _setIndex)
      /// const
      public: gz::math::Vector2d TexCoordBySet(
                  unsigned int _index,
                  unsigned int _setIndex) const;

      /// \brief Set a texture coordinate. If multiple texture
      /// coordinate sets exist, this function sets the texture
      /// coordinate in the first texture coordinate set in the submesh.
      /// \param[in] _index Index of the texture coordinate that will be set.
      /// \param[in] _uv The new texture coordinate
      /// \sa SetTexCoordBySet
      public: void SetTexCoord(const unsigned int _index,
                               const gz::math::Vector2d &_uv);

      /// \brief Set a texture coordinate for a texture coordinate set
      /// \param[in] _index Index of the texture coordinate that will be set.
      /// \param[in] _uv The new texture coordinate
      /// \param[in] _setIndex Texture coordinate set index
      public: void SetTexCoordBySet(unsigned int _index,
                               const gz::math::Vector2d &_uv,
                               unsigned int _setIdex);

      /// \brief Get an index value from the index array
      /// \param[in] _index Array index.
      /// \return The index, or -1 if the _index is out of bounds.
      public: int Index(const unsigned int _index) const;

      /// \brief Get the raw index pointer. This is unsafe, it is the
      /// caller's responsibility to ensure it's not indexed out of bounds.
      /// The valid range is [0; IndexCount())
      /// \return Raw indices
      public: const unsigned int* IndexPtr() const;

      /// \brief Set an index
      /// \param[in] _index Index of the indices
      /// \param[in] _i The new index value to set to
      public: void SetIndex(const unsigned int _index,
          const unsigned int _i);

      /// \brief Get a vertex - skeleton node assignment
      /// \param[in] _index The index of the assignment
      /// \return The skeleton node assignment, or a
      /// default constructed skeleton node assignment when _index is
      /// invalid.
      /// \sa bool HasNodeAssignment(const unsigned int _index) const;
      public: NodeAssignment NodeAssignmentByIndex(
          const unsigned int _index) const;

      /// \brief Get the maximum X, Y, Z values from all the vertices
      /// \return Max X,Y,Z values from all vertices in submesh
      public: gz::math::Vector3d Max() const;

      /// \brief Get the minimum X, Y, Z values from all the vertices
      /// \return Min X,Y,Z values from all vertices in submesh
      public: gz::math::Vector3d Min() const;

      /// \brief Get the number of vertices
      /// \return The number of vertices.
      public: unsigned int VertexCount() const;

      /// \brief Return the number of normals
      /// \return The number of normals.
      public: unsigned int NormalCount() const;

      /// \brief Return the number of indices
      /// \return The number of indices.
      public: unsigned int IndexCount() const;

      /// \brief Return the number of texture coordinates. If multiple
      /// texture coordinate sets exist, this function checks the first
      /// texture coordinate set in the submesh, which by default is set 0,
      /// unless AddTexCoordBySet is called with a different set index number
      /// the first time a texture coordinate is added.
      /// \return The number of texture coordinates.
      /// \sa TexCoordCountBySet
      public: unsigned int TexCoordCount() const;

      /// \brief Return the number of texture coordinates for a texture
      /// coordinate set
      /// \param[in] _setIndex Texture coordinate set index
      /// \return The number of texture coordinates.
      public: unsigned int TexCoordCountBySet(unsigned int _setIndex) const;

      /// \brief Return the number of texture coordinate sets
      /// \return The number of texture coordinates sets.
      public: unsigned int TexCoordSetCount() const;

      /// \brief Get the number of vertex-skeleton node assignments
      /// \return The number of vertex-skeleton node assignments
      public: unsigned int NodeAssignmentsCount() const;

      /// \brief Get the highest value in the index array.
      /// \return The highest index value.
      public: unsigned int MaxIndex() const;

      /// \brief Set the material index. Relates to the parent mesh material
      /// list.
      /// \param[in] _index Index to set the material to.
      public: void SetMaterialIndex(const unsigned int _index);

      /// \brief Get the material index
      /// \return The assigned material index. Nullopt is returned if the
      /// submesh has no assigned material index
      public: std::optional<unsigned int> GetMaterialIndex() const;

      /// \brief Return true if this submesh has the vertex
      /// \param[in] _v Vertex coordinate
      /// \return Return true if this submesh has the vertex
      public: bool HasVertex(const gz::math::Vector3d &_v) const;

      /// \brief Return true if this submesh has the vertex with the given
      /// index
      /// \param[in] _index Vertex index
      /// \return Return true if this submesh has the vertex with the given
      /// _index.
      public: bool HasVertex(const unsigned int _index) const;

      /// \brief Return true if this submesh has the normal with the given
      /// index
      /// \param[in] _index Normal index
      /// \return Return true if this submesh has the normal with the given
      /// _index.
      public: bool HasNormal(const unsigned int _index) const;

      /// \brief Return true if this submesh has the texture coordinate with
      /// the given index. If multiple texture coordinate sets exist, this
      /// function checks the first texture coordinate set in the submesh,
      /// which by default is set 0 unless AddTexCoordBySet is called with
      /// a different set index number the first time a texture coordinate is
      /// added.
      /// \param[in] _index Texture coordinate index
      /// \return Return true if this submesh has the texture coordinate with
      /// the given _index.
      /// \sa HasTexCoordBySet
      public: bool HasTexCoord(const unsigned int _index) const;

      /// \brief Return true if this submesh has the texture coordinate with
      /// the given index in a texture coordinate set
      /// \param[in] _index Texture coordinate index
      /// \param[in] _setIndex Texture coordinate set index
      /// \return Return true if this submesh has the texture coordinate with
      /// the given _index.
      public: bool HasTexCoordBySet(unsigned int _index, unsigned int _setIndex)
          const;

      /// \brief Return true if this submesh has the node assignment with
      /// the given index
      /// \param[in] _index Node assignment index
      /// \return Return true if this submesh has the node assignment with
      /// the given _index.
      public: bool HasNodeAssignment(const unsigned int _index) const;

      /// \brief Get the index of the vertex
      /// \param[in] _v Vertex to check
      /// \return Index of the vertex that matches _v.
      public: int IndexOfVertex(const gz::math::Vector3d &_v) const;

      /// \brief Put all the data into flat arrays
      /// \param[in] _verArr The vertex array to be filled.
      /// \param[in] _indexndArr The index array to be filled.
      public: void FillArrays(double **_vertArr, int **_indexndArr) const;

      /// \brief Recalculate all the normals.
      public: void RecalculateNormals();

      /// \brief Generate texture coordinates using spherical projection
      /// from center
      /// \param[in] _center Center of the projection.
      public: void GenSphericalTexCoord(
                  const gz::math::Vector3d &_center);

      /// \brief Generate texture coordinates for a texture coordinate set
      ///  using spherical projection from center
      /// \param[in] _center Center of the projection.
      /// \param[in] _setIndex Texture coordinate set index
      public: void GenSphericalTexCoordBySet(
                  const gz::math::Vector3d &_center,
                  unsigned int _setIndex);

      /// \brief Scale all vertices by _factor
      /// \param[in] _factor Scaling factor
      public: void Scale(const gz::math::Vector3d &_factor);

      /// \brief Scale all vertices by _factor
      /// \param[in] _factor Scaling factor
      public: void Scale(const double &_factor);

      /// \brief Move the center of the submesh to the given coordinate. This
      /// will move all the vertices.
      /// \param[in] _center Location of the mesh center.
      public: void Center(const gz::math::Vector3d &_center =
                          gz::math::Vector3d::Zero);

      /// \brief Move all vertices by _vec.
      /// \param[in] _vec Amount to translate vertices.
      public: void Translate(const gz::math::Vector3d &_vec);

      /// \brief Compute the volume of this submesh. The primitive type
      /// must be TRIANGLES.
      ///
      /// This function utilizes the mesh volume formula from
      /// "Efficient feature extraction for 2d/3d objects in mesh
      /// representation" by Cha Zhang and Tsuhan Chen. Link:
      /// http://chenlab.ece.cornell.edu/Publication/Cha/icip01_Cha.pdf.
      /// The formula does not check for a closed (water tight) mesh.
      ///
      /// \return The submesh's volume. The volume can be zero if
      /// the primitive type is not TRIANGLES, or there are no triangles.
      public: double Volume() const;

      /// \brief Verify that all indices point to a valid vertex in the submesh
      /// \return True if all values of indices are valid, false otherwise.
      public: bool HasValidIndices() const;

      /// \brief Private data pointer.
      GZ_UTILS_IMPL_PTR(dataPtr)
    };

    /// \brief Vertex to node weighted assignment for skeleton animation
    /// visualization
    class GZ_COMMON_GRAPHICS_VISIBLE NodeAssignment
    {
      /// \brief Constructor.
      public: NodeAssignment();

      /// \brief Index of the vertex
      public: unsigned int vertexIndex;

      /// \brief Node (or bone) index
      public: unsigned int nodeIndex;

      /// \brief The weight (between 0 and 1). This determines how much the
      /// node (bone) affects the vertex in animation.
      public: float weight;
    };
  }
}

#endif
