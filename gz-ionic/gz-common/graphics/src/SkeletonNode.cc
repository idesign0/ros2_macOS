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

#include <list>

#include "gz/common/Console.hh"
#include "gz/common/SkeletonNode.hh"

using namespace gz;
using namespace common;

/// \brief SkeletonNode private data
class gz::common::SkeletonNode::Implementation
{
  /// \brief the name of the skeletal node
  public: std::string name;

  /// \brief a string identifier
  public: std::string id;

  /// \brief the type fo node
  public: SkeletonNode::SkeletonNodeType type;

  /// \brief the transform
  public: math::Matrix4d transform;

  /// \brief the initial transformation
  public: math::Matrix4d initialTransform;

  /// \brief the model transformation
  public: math::Matrix4d modelTransform;

  /// \brief the inverse of the bind pose skeletal transform
  public: math::Matrix4d invBindTransform;

  /// \brief the parent node
  public: SkeletonNode *parent;

  /// \brief the children nodes
  public: std::vector<SkeletonNode*> children;

  /// \brief handle index number
  public: unsigned int handle;

  /// \brief the raw transformation
  public: std::vector<NodeTransform> rawTransforms;
};

//////////////////////////////////////////////////
SkeletonNode::SkeletonNode(SkeletonNode *_parent)
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->parent = _parent;

  if (this->dataPtr->parent)
    this->dataPtr->parent->AddChild(this);
}

//////////////////////////////////////////////////
SkeletonNode::SkeletonNode(SkeletonNode *_parent,
    const std::string &_name, const std::string &_id,
    const SkeletonNodeType _type)
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->parent = _parent;

  if (this->dataPtr->parent)
    this->dataPtr->parent->AddChild(this);

  this->dataPtr->name = _name;
  this->dataPtr->id = _id;
  this->dataPtr->type = _type;
}

//////////////////////////////////////////////////
SkeletonNode::~SkeletonNode() = default;

//////////////////////////////////////////////////
void SkeletonNode::Name(const std::string &_name)
{
  this->dataPtr->name = _name;
}

//////////////////////////////////////////////////
std::string SkeletonNode::Name() const
{
  return this->dataPtr->name;
}

//////////////////////////////////////////////////
void SkeletonNode::Id(const std::string &_id)
{
  this->dataPtr->id = _id;
}

//////////////////////////////////////////////////
std::string SkeletonNode::Id() const
{
  return this->dataPtr->id;
}

//////////////////////////////////////////////////
void SkeletonNode::SetType(const SkeletonNodeType _type)
{
  this->dataPtr->type = _type;
}

//////////////////////////////////////////////////
bool SkeletonNode::IsJoint() const
{
  if (this->dataPtr->type == JOINT)
    return true;
  else
    return false;
}

//////////////////////////////////////////////////
void SkeletonNode::SetTransform(const math::Matrix4d &_trans,
    const bool _updateChildren)
{
  this->dataPtr->transform = _trans;

  if (this->dataPtr->parent == NULL)
  {
    this->dataPtr->modelTransform = _trans;
  }
  else
  {
    this->dataPtr->modelTransform =
      this->dataPtr->parent->ModelTransform() * _trans;
  }

  /// propagate the change to the children nodes
  if (_updateChildren)
    this->UpdateChildrenTransforms();
}

//////////////////////////////////////////////////
void SkeletonNode::SetInitialTransform(const math::Matrix4d &_trans)
{
  this->dataPtr->initialTransform = _trans;
  this->SetTransform(_trans);
}

//////////////////////////////////////////////////
void SkeletonNode::Reset(const bool _resetChildren)
{
  this->SetTransform(this->dataPtr->initialTransform);

  if (_resetChildren)
  {
    for (unsigned int i = 0; i < this->ChildCount(); ++i)
      this->Child(i)->Reset(true);
  }
}

//////////////////////////////////////////////////
void SkeletonNode::UpdateChildrenTransforms()
{
  std::list<SkeletonNode*> toVisit;
  for (unsigned int i = 0; i < this->dataPtr->children.size(); ++i)
    toVisit.push_back(this->dataPtr->children[i]);

  while (!toVisit.empty())
  {
    SkeletonNode *node = toVisit.front();
    toVisit.pop_front();

    for (int i = (node->ChildCount() - 1); i >= 0; --i)
      toVisit.push_front(node->Child(i));

    node->dataPtr->modelTransform =
        node->Parent()->dataPtr->modelTransform * node->dataPtr->transform;
  }
}

//////////////////////////////////////////////////
math::Matrix4d SkeletonNode::Transform() const
{
  return this->dataPtr->transform;
}

//////////////////////////////////////////////////
void SkeletonNode::SetModelTransform(const math::Matrix4d &_trans,
    const bool _updateChildren)
{
  this->dataPtr->modelTransform = _trans;

  if (this->dataPtr->parent == NULL)
    this->dataPtr->transform = _trans;
  else
  {
    math::Matrix4d invParentTrans =
      this->dataPtr->parent->ModelTransform().Inverse();
    this->dataPtr->transform = invParentTrans * this->dataPtr->modelTransform;
  }

  if (_updateChildren)
    this->UpdateChildrenTransforms();
}

//////////////////////////////////////////////////
math::Matrix4d SkeletonNode::ModelTransform() const
{
  return this->dataPtr->modelTransform;
}

//////////////////////////////////////////////////
void SkeletonNode::SetParent(SkeletonNode *_parent)
{
  this->dataPtr->parent = _parent;
}

//////////////////////////////////////////////////
SkeletonNode *SkeletonNode::Parent() const
{
  return this->dataPtr->parent;
}

//////////////////////////////////////////////////
bool SkeletonNode::IsRootNode() const
{
  if (!this->dataPtr->parent)
    return true;
  else
    return false;
}

//////////////////////////////////////////////////
void SkeletonNode::AddChild(SkeletonNode *_child)
{
  this->dataPtr->children.push_back(_child);
}

//////////////////////////////////////////////////
unsigned int SkeletonNode::ChildCount() const
{
  return this->dataPtr->children.size();
}

//////////////////////////////////////////////////
SkeletonNode *SkeletonNode::Child(const unsigned int _index) const
{
  SkeletonNode *result = NULL;

  if (_index >= this->dataPtr->children.size())
    gzerr << "Index out of range[" << _index << "]\n";
  else
    result = this->dataPtr->children[_index];

  return result;
}

//////////////////////////////////////////////////
SkeletonNode *SkeletonNode::ChildByName(const std::string &_name) const
{
  for (unsigned int i = 0; i < this->dataPtr->children.size(); ++i)
  {
    if (this->dataPtr->children[i]->Name() == _name)
      return this->dataPtr->children[i];
  }

  return NULL;
}

//////////////////////////////////////////////////
SkeletonNode *SkeletonNode::ChildById(const std::string &_id) const
{
  for (unsigned int i = 0; i < this->dataPtr->children.size(); ++i)
  {
    if (this->dataPtr->children[i]->Id() == _id)
      return this->dataPtr->children[i];
  }

  return NULL;
}

//////////////////////////////////////////////////
void SkeletonNode::Handle(const unsigned int _handle)
{
  this->dataPtr->handle = _handle;
}

//////////////////////////////////////////////////
unsigned int SkeletonNode::Handle() const
{
  return this->dataPtr->handle;
}

//////////////////////////////////////////////////
void SkeletonNode::SetInverseBindTransform(const math::Matrix4d &_invBM)
{
  this->dataPtr->invBindTransform = _invBM;
}

//////////////////////////////////////////////////
math::Matrix4d SkeletonNode::InverseBindTransform() const
{
  return this->dataPtr->invBindTransform;
}

//////////////////////////////////////////////////
bool SkeletonNode::HasInvBindTransform() const
{
  return this->dataPtr->invBindTransform != math::Matrix4d::Zero;
}

//////////////////////////////////////////////////
std::vector<NodeTransform> SkeletonNode::RawTransforms() const
{
  return this->dataPtr->rawTransforms;
}

//////////////////////////////////////////////////
unsigned int SkeletonNode::RawTransformCount() const
{
  return this->dataPtr->rawTransforms.size();
}

//////////////////////////////////////////////////
NodeTransform SkeletonNode::RawTransform(const unsigned int _i) const
{
  return this->dataPtr->rawTransforms[_i];
}

//////////////////////////////////////////////////
void SkeletonNode::AddRawTransform(const NodeTransform &_t)
{
  this->dataPtr->rawTransforms.push_back(_t);
}

//////////////////////////////////////////////////
std::vector<NodeTransform> SkeletonNode::Transforms() const
{
  return this->dataPtr->rawTransforms;
}
