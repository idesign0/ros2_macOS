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
#ifndef GZ_COMMON_SKELETON_NODE_HH_
#define GZ_COMMON_SKELETON_NODE_HH_

#include <map>
#include <string>
#include <vector>

#include <gz/utils/ImplPtr.hh>

#include <gz/common/NodeTransform.hh>
#include <gz/common/graphics/Export.hh>

namespace gz
{
  namespace common
  {
    /// \class SkeletonNode Skeleton.hh gz/common/SkeletonNode.hh
    /// \brief A skeleton node
    class GZ_COMMON_GRAPHICS_VISIBLE SkeletonNode
    {
      /// \brief enumeration of node types
      public: enum SkeletonNodeType {NODE, JOINT};

      /// \brief Constructor
      /// \param[in] _parent The parent node
      public: explicit SkeletonNode(SkeletonNode *_parent);

      /// \brief Constructor
      /// \param[in] _parent the parent node
      /// \param[in] _name name of node
      /// \param[in] _id Id of node
      /// \param[in] _type The type of this node
      public: SkeletonNode(SkeletonNode *_parent,
                  const std::string &_name,
                  const std::string &_id,
                  const SkeletonNodeType _type = JOINT);

      /// \brief Destructor
      public: virtual ~SkeletonNode();

      /// \brief Change the name
      /// \param[in] _name the new name
      public: void Name(const std::string &_name);

      /// \brief Returns the name
      /// \return the name
      public: std::string Name() const;

      /// \brief Change the id string
      /// \param[in] _id the new id string
      public: void Id(const std::string &_id);

      /// \brief Returns the index
      /// \return the id string
      public: std::string Id() const;

      /// \brief Change the skeleton node type
      /// \param[in] _type the new type
      public: void SetType(const SkeletonNodeType _type);

      /// \brief Is a joint query
      /// \return true if the skeleton type is a joint, false otherwise
      public: bool IsJoint() const;

      /// \brief Set a transformation
      /// \param[in] _trans the transformation
      /// \param[in] _updateChildren when true the UpdateChildrenTransforms
      /// operation is performed
      public: void SetTransform(const math::Matrix4d &_trans,
                 const bool _updateChildren = true);

      /// \brief Set the model transformation
      /// \param[in] _trans the transformation
      /// \param[in] _updateChildren when true the UpdateChildrenTransforms
      /// operation is performed
      public: void SetModelTransform(const math::Matrix4d &_trans,
                  const bool _updateChildren = true);

      /// \brief Apply model transformations in order for each node in the tree
      public: void UpdateChildrenTransforms();

      /// \brief Sets the initial transformation
      /// \param[in] _trans the transfromation matrix
      public: void SetInitialTransform(const math::Matrix4d &_trans);

      /// \brief Reset the transformation to the initial transformation
      /// \param[in] _resetChildren when true, performs the operation for every
      /// node in the tree
      public: void Reset(const bool _resetChildren);

      /// \brief Get transform relative to parent
      public: math::Matrix4d Transform() const;

      /// \brief Set the parent node
      /// \param[in] _parent the new parent
      public: void SetParent(SkeletonNode *_parent);

      /// \brief Returns the parent node
      /// \return the parent
      public: SkeletonNode *Parent() const;

      /// \brief Queries wether a node has no parent parent
      /// \return true if the node has no parent, fasle otherwise
      public: bool IsRootNode() const;

      /// \brief Add a new child
      /// \param[in] _child a child
      public: void AddChild(SkeletonNode *_child);

      /// \brief Returns the children count
      /// \return the count
      public: unsigned int ChildCount() const;

      /// \brief Find a child by index
      /// \param[in] _index the index
      /// \return the child skeleton.
      public: SkeletonNode *Child(const unsigned int _index) const;

      /// \brief Get child by name
      /// \param[in] _name the name of the child skeleton
      /// \return the skeleton, or NULL if not found
      public: SkeletonNode *ChildByName(const std::string &_name) const;

      /// \brief Get child by string id
      /// \param[in] _id the string id
      /// \return the child skeleton or NULL if not found
      public: SkeletonNode *ChildById(const std::string &_id) const;

      /// \brief Assign a handle number
      /// \param[in] _h the handle
      public: void Handle(const unsigned int _h);

      /// \brief Get the handle index
      /// \return the handle index
      public: unsigned int Handle() const;

      /// \brief Assign the inverse of the bind pose skeletal transform
      /// \param[in] _invBM the transform
      public: void SetInverseBindTransform(const math::Matrix4d &_invBM);

      /// \brief Retrieve the inverse of the bind pose skeletal transform
      /// \return the transform
      public: math::Matrix4d InverseBindTransform() const;

      /// \brief Returns true if the node has inv bind transform.
      /// \detail to keep ABI compatibility, it checks if the inv bind transform
      /// is the default value of zero.
      /// \return true if the node has inv bind transform
      public: bool HasInvBindTransform() const;

      /// \brief Retrieve the model transform
      /// \return the transform
      public: math::Matrix4d ModelTransform() const;

      /// \brief Retrieve the raw transformations
      /// \return an array of transformations
      public: std::vector<NodeTransform> RawTransforms() const;

      /// \brief Return the raw transformations count
      /// \return the count
      public: unsigned int RawTransformCount() const;

      /// \brief Find a raw transformation
      /// \param[in] _i the index of the transformation
      /// \return the node transform.
      public: NodeTransform RawTransform(const unsigned int _i) const;

      /// \brief Add a raw transform
      /// \param[in] _t the transform
      public: void AddRawTransform(const NodeTransform &_t);

      /// \brief Returns a copy of the array of transformations.
      /// \return the array of transform (These are the same as the raw trans)
      public: std::vector<NodeTransform> Transforms() const;

      /// \brief Private data pointer.
      GZ_UTILS_IMPL_PTR(dataPtr)
    };

    typedef std::map<unsigned int, SkeletonNode*> SkeletonNodeMap;
  }
}
#endif
