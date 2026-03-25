/*
 * Copyright (C) 2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <list>
#include <gz/common/Console.hh>
#include <gz/common/SkeletonAnimation.hh>
#include <gz/common/Skeleton.hh>
#include <gz/common/BVHLoader.hh>

using namespace gz;
using namespace common;

/// Private data class
class gz::common::Skeleton::Implementation
{
  typedef std::vector<std::vector<std::pair<std::string, double> > >
    RawNodeWeights;

  /// \brief the root node
  public: SkeletonNode *root{nullptr};

  /// \brief The dictionary of nodes, indexed by name
  public: SkeletonNodeMap nodes;

  /// \brief the bind pose skeletal transform
  public: math::Matrix4d bindShapeTransform{math::Matrix4d::Identity};

  /// \brief the node weight table
  public: RawNodeWeights rawNodeWeights;

  /// \brief the array of animations
  public: std::vector<SkeletonAnimation *> anims;

  /// \brief Animation node to skin node map
  /// * Map holding:
  ///     * Skeleton node names from animation
  ///     * Skeleton node names from skin
  /// \sa skelAnimation
  public: std::vector<std::map<std::string, std::string>> mapAnimSkin;

  /// \brief Translation alignment from animation to skin
  /// * Map holding:
  ///     * Skeleton node names from animation
  ///     * Transformations to translate
  public: std::vector<std::map<std::string, math::Matrix4d>> alignTranslate;

  /// \brief Rotation alignment from animation to skin
  /// * Map holding:
  ///     * Skeleton node names from animation
  ///     * Transformations to rotate
  public: std::vector<std::map<std::string, math::Matrix4d>> alignRotate;
};

//////////////////////////////////////////////////
Skeleton::Skeleton()
: dataPtr(gz::utils::MakeUniqueImpl<Implementation>())
{
}

//////////////////////////////////////////////////
Skeleton::Skeleton(SkeletonNode *_root)
: Skeleton()
{
  this->RootNode(_root);
}

//////////////////////////////////////////////////
Skeleton::~Skeleton()
{
  for (auto &kv : this->dataPtr->nodes)
    delete kv.second;
  this->dataPtr->nodes.clear();

  for (auto &a : this->dataPtr->anims)
    delete a;
  this->dataPtr->anims.clear();
}

//////////////////////////////////////////////////
void Skeleton::RootNode(SkeletonNode *_node)
{
  this->dataPtr->root = _node;
  this->BuildNodeMap();
}

//////////////////////////////////////////////////
SkeletonNode *Skeleton::RootNode() const
{
  return this->dataPtr->root;
}

//////////////////////////////////////////////////
SkeletonNode *Skeleton::NodeByName(const std::string &_name) const
{
  for (SkeletonNodeMap::const_iterator iter =
      this->dataPtr->nodes.begin(); iter != this->dataPtr->nodes.end(); ++iter)
  {
    if (iter->second->Name() == _name)
      return iter->second;
  }

  return NULL;
}

//////////////////////////////////////////////////
SkeletonNode *Skeleton::NodeById(const std::string &_id) const
{
  for (SkeletonNodeMap::const_iterator iter =
      this->dataPtr->nodes.begin(); iter != this->dataPtr->nodes.end(); ++iter)
  {
    if (iter->second->Id() == _id)
      return iter->second;
  }

  return NULL;
}

//////////////////////////////////////////////////
SkeletonNode *Skeleton::NodeByHandle(const unsigned int _handle) const
{
  SkeletonNodeMap::const_iterator iter = this->dataPtr->nodes.find(_handle);
  return iter != this->dataPtr->nodes.end() ? iter->second : NULL;
}

//////////////////////////////////////////////////
unsigned int Skeleton::NodeCount() const
{
  return this->dataPtr->nodes.size();
}

//////////////////////////////////////////////////
unsigned int Skeleton::JointCount() const
{
  unsigned int c = 0;
  for (SkeletonNodeMap::const_iterator iter =
      this->dataPtr->nodes.begin(); iter != this->dataPtr->nodes.end(); ++iter)
  {
    if (iter->second->IsJoint())
      c++;
  }

  return c;
}

//////////////////////////////////////////////////
void Skeleton::Scale(const double _scale)
{
  if (nullptr == this->dataPtr->root)
  {
    gzerr << "Failed to scale skeleton, null root." << std::endl;
    return;
  }

  //  scale skeleton structure
  for (SkeletonNodeMap::iterator iter = this->dataPtr->nodes.begin();
       iter != this->dataPtr->nodes.end(); ++iter)
  {
    SkeletonNode *node = iter->second;
    math::Matrix4d trans = node->Transform();
    math::Vector3d pos = trans.Translation();
    trans.SetTranslation(pos * _scale);
    node->SetTransform(trans, false);
  }

  //  update the nodes' model transforms
  this->dataPtr->root->UpdateChildrenTransforms();

  //  scale the animation data
  for (unsigned int i = 0; i < this->dataPtr->anims.size(); ++i)
    this->dataPtr->anims[i]->Scale(_scale);
}

//////////////////////////////////////////////////
void Skeleton::BuildNodeMap()
{
  std::list<SkeletonNode *> toVisit;
  toVisit.push_front(this->dataPtr->root);

  unsigned int handle = 0;

  while (!toVisit.empty())
  {
    SkeletonNode *node = toVisit.front();
    toVisit.pop_front();

    if (nullptr == node)
      continue;

    for (int i = (node->ChildCount() - 1); i >= 0; --i)
      toVisit.push_front(node->Child(i));

    node->Handle(handle);
    this->dataPtr->nodes[handle] = node;
    handle++;
  }
}

//////////////////////////////////////////////////
void Skeleton::SetBindShapeTransform(const math::Matrix4d &_trans)
{
  this->dataPtr->bindShapeTransform = _trans;
}

//////////////////////////////////////////////////
math::Matrix4d Skeleton::BindShapeTransform() const
{
  return this->dataPtr->bindShapeTransform;
}

//////////////////////////////////////////////////
void Skeleton::PrintTransforms() const
{
  for (SkeletonNodeMap::const_iterator iter =
      this->dataPtr->nodes.begin(); iter != this->dataPtr->nodes.end(); ++iter)
  {
    const SkeletonNode *node = iter->second;
    std::cout << "---------------\n" << node->Name() << "\n";

    for (unsigned int i = 0; i < node->RawTransformCount(); ++i)
    {
      NodeTransform nt = node->RawTransform(i);
      std::cout << "\t" << nt.SID();
      if (nt.Type() == NodeTransformType::MATRIX)
      {
        std::cout << " MATRIX\n";
      }
      else
      {
        if (nt.Type() == NodeTransformType::TRANSLATE)
        {
          std::cout << " TRANSLATE\n";
        }
        else
        {
          if (nt.Type() == NodeTransformType::ROTATE)
            std::cout << " ROTATE\n";
          else
            std::cout << " SCALE\n";
        }
      }
      std::cout << nt() << "\n+++++++++++\n";
    }

    std::cout << node->ModelTransform() << "\n";

    if (node->IsJoint())
      std::cout << node->InverseBindTransform() << "\n";
  }
}

//////////////////////////////////////////////////
const SkeletonNodeMap &Skeleton::Nodes() const
{
  return this->dataPtr->nodes;
}

//////////////////////////////////////////////////
void Skeleton::SetNumVertAttached(const unsigned int _vertices)
{
  this->dataPtr->rawNodeWeights.clear();
  this->dataPtr->rawNodeWeights.resize(_vertices);
}

//////////////////////////////////////////////////
void Skeleton::AddVertNodeWeight(
    const unsigned int _vertex, const std::string &_node,
    const double _weight)
{
  if (_vertex < this->dataPtr->rawNodeWeights.size())
  {
    this->dataPtr->rawNodeWeights[_vertex].push_back(
        std::make_pair(_node, _weight));
  }
}

//////////////////////////////////////////////////
unsigned int Skeleton::VertNodeWeightCount(const unsigned int _vertex) const
{
  if (_vertex < this->dataPtr->rawNodeWeights.size())
    return this->dataPtr->rawNodeWeights[_vertex].size();
  else
    return 0;
}

//////////////////////////////////////////////////
std::pair<std::string, double> Skeleton::VertNodeWeight(
    const unsigned int _v, const unsigned int _i) const
{
  std::pair<std::string, double> result;

  if (_v < this->dataPtr->rawNodeWeights.size() &&
      _i < this->dataPtr->rawNodeWeights[_v].size())
  {
    result = this->dataPtr->rawNodeWeights[_v][_i];
  }

  return result;
}

//////////////////////////////////////////////////
unsigned int Skeleton::AnimationCount() const
{
  return this->dataPtr->anims.size();
}

//////////////////////////////////////////////////
SkeletonAnimation *Skeleton::Animation(const unsigned int _i) const
{
  if (_i < this->dataPtr->anims.size())
    return this->dataPtr->anims[_i];
  else
    return NULL;
}

//////////////////////////////////////////////////
void Skeleton::AddAnimation(SkeletonAnimation *_anim)
{
  this->dataPtr->mapAnimSkin.push_back(std::map<std::string, std::string>());
  this->dataPtr->alignTranslate.push_back(
      std::map<std::string, math::Matrix4d>());
  this->dataPtr->alignRotate.push_back(std::map<std::string, math::Matrix4d>());
  this->dataPtr->anims.push_back(_anim);
}

//////////////////////////////////////////////////
bool Skeleton::AddBvhAnimation(const std::string &_bvhFile, double _scale)
{
  BVHLoader loader;
  auto skel = loader.Load(_bvhFile, _scale);
  if (nullptr == skel)
    return false;

  bool compatible = true;

  std::map<std::string, std::string> skelMap;
  if (this->NodeCount() != skel->NodeCount())
  {
    compatible = false;
  }
  else
  {
    for (unsigned int i = 0; i < this->NodeCount(); ++i)
    {
      SkeletonNode *skinNode = this->NodeByHandle(i);
      SkeletonNode *animNode = skel->NodeByHandle(i);
      if (skinNode->ChildCount() != animNode->ChildCount())
      {
        compatible = false;
        break;
      }
      else
      {
        skelMap[animNode->Name()] = skinNode->Name();
      }
    }
  }

  if (!compatible)
  {
    return false;
  }

  // align frames
  std::map<std::string, math::Matrix4d> translations;
  std::map<std::string, math::Matrix4d> rotations;

  for (unsigned int i = 0; i < skel->NodeCount(); ++i)
  {
    SkeletonNode *animNode = skel->NodeByHandle(i);
    SkeletonNode *skinNode = this->NodeByHandle(i);

    if (animNode->Parent() != nullptr)
    {
      if (animNode->Parent()->ChildCount() > 1
              && skinNode->Transform().Translation() ==
              math::Vector3d::Zero)
      {
        translations[animNode->Name()] = math::Matrix4d::Identity;
      }
    }

    if (animNode->ChildCount() == 0)
    {
      continue;
    }

    if (this->RootNode()->Name() == skelMap[animNode->Name()])
    {
      translations[animNode->Name()] = math::Matrix4d(
        this->RootNode()->Transform().Rotation());
      math::Matrix4d tmp(translations[animNode->Name()]);
      tmp.SetTranslation(animNode->Transform().Translation());
      animNode->SetTransform(tmp, true);
    }

    if (animNode->ChildCount() > 1)
    {
      continue;
    }

    math::Vector3d relativeBVH(
          animNode->Child(0)->ModelTransform().Translation()
              - animNode->ModelTransform().Translation());

    math::Vector3d relativeSkin(
          skinNode->Child(0)->ModelTransform().Translation()
              - skinNode->ModelTransform().Translation());

    if (relativeBVH == math::Vector3d::Zero ||
            relativeSkin == math::Vector3d::Zero)
    {
      continue;
    }

    math::Vector3d n(relativeBVH.Cross(relativeSkin));
    double theta = asin(n.Length() /
          (relativeSkin.Length() * relativeBVH.Length()));

    translations[animNode->Child(0)->Name()] =
        math::Matrix4d(skinNode->ModelTransform().Rotation()).Inverse()
        * math::Matrix4d(math::Quaterniond(n.Normalize(), theta))
        * math::Matrix4d(animNode->ModelTransform().Rotation());
    math::Matrix4d tmp(animNode->ModelTransform().Rotation());
    tmp = tmp.Inverse()
        * math::Matrix4d(math::Quaterniond(n.Normalize(), theta))
        * math::Matrix4d(animNode->ModelTransform().Rotation());
    tmp.SetTranslation(animNode->Transform().Translation());
    animNode->SetTransform(tmp, true);
  }

  for (unsigned int i = 0; i < skel->NodeCount(); ++i)
  {
    SkeletonNode *animNode = skel->NodeByHandle(i);
    SkeletonNode *skinNode = this->NodeByHandle(i);

    if (skelMap[animNode->Name()] == this->RootNode()->Name())
    {
      rotations[animNode->Name()] = math::Matrix4d::Identity;
      continue;
    }

    if (translations[animNode->Name()] == math::Matrix4d::Zero)
    {
        translations[animNode->Name()] = math::Matrix4d::Identity;
    }

    rotations[animNode->Name()] =
          math::Matrix4d(animNode->Transform().Rotation()).Inverse()
          * translations[animNode->Name()].Inverse()
          * math::Matrix4d(skinNode->Transform().Rotation());
  }

  // Copy pointer from temp skeleton before it's deleted
  auto newAnim = new SkeletonAnimation(skel->Animation(0u)->Name());
  *newAnim = *skel->Animation(0u);
  this->dataPtr->anims.push_back(newAnim);
  this->dataPtr->mapAnimSkin.push_back(skelMap);
  this->dataPtr->alignTranslate.push_back(translations);
  this->dataPtr->alignRotate.push_back(rotations);
  return true;
}

//////////////////////////////////////////////////
std::string Skeleton::NodeNameAnimToSkin(unsigned int _index,
      const std::string &_animNodeName)
{
  if (this->dataPtr->mapAnimSkin[_index].find(_animNodeName)
        != this->dataPtr->mapAnimSkin[_index].end())
  {
    return this->dataPtr->mapAnimSkin[_index][_animNodeName];
  }
  return _animNodeName;
}

//////////////////////////////////////////////////
math::Matrix4d Skeleton::AlignTranslation(unsigned int _index,
      const std::string &_animNodeName)
{
  if (this->dataPtr->alignTranslate[_index].find(_animNodeName)
        != this->dataPtr->alignTranslate[_index].end())
  {
    return this->dataPtr->alignTranslate[_index][_animNodeName];
  }
  return math::Matrix4d::Identity;
}

//////////////////////////////////////////////////
math::Matrix4d Skeleton::AlignRotation(unsigned int _index,
      const std::string &_animNodeName)
{
  if (this->dataPtr->alignRotate[_index].find(_animNodeName)
        != this->dataPtr->alignRotate[_index].end())
  {
    return this->dataPtr->alignRotate[_index][_animNodeName];
  }
  return math::Matrix4d::Identity;
}
