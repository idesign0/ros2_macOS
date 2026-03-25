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
#include <algorithm>
#include <fstream>
#include <sstream>

#include <gz/common/SystemPaths.hh>
#include <gz/common/Skeleton.hh>
#include <gz/common/SkeletonAnimation.hh>
#include <gz/common/Console.hh>
#include <gz/common/BVHLoader.hh>

using namespace gz;
using namespace common;

/////////////////////////////////////////////////
BVHLoader::BVHLoader()
{
}

/////////////////////////////////////////////////
BVHLoader::~BVHLoader()
{
}

/////////////////////////////////////////////////
std::unique_ptr<Skeleton> BVHLoader::Load(const std::string &_filename,
    const double _scale)
{
  std::string fullname = findFile(_filename);
  if (fullname.empty())
    return nullptr;

  std::unique_ptr<Skeleton> skeleton;
  std::ifstream file;
  file.open(fullname.c_str());
  std::vector<SkeletonNode*> nodes;
  std::vector<std::vector<std::string> > nodeChannels;
  unsigned int totalChannels = 0;
  std::string line;
  if (file.is_open())
  {
    getline(file, line);
    if (line.find("HIERARCHY") == std::string::npos)
    {
      file.close();
      return nullptr;
    }

    SkeletonNode *parent = nullptr;
    SkeletonNode *node = nullptr;
    while (!file.eof())
    {
      getline(file, line);
      trim(line);
      std::vector<std::string> words = split(line, " ");
      if (words[0] == "ROOT" || words[0] == "JOINT")
      {
        if (words.size() < 2)
        {
          file.close();
          return nullptr;
        }
        SkeletonNode::SkeletonNodeType type = SkeletonNode::JOINT;
        std::string name = words[1];
        node = new SkeletonNode(parent, name, name, type);
        if (words[0] != "End")
          nodes.push_back(node);
      }
      else
      {
        if (words[0] == "OFFSET")
        {
          if (words.size() < 4)
          {
            file.close();
            return nullptr;
          }
          math::Vector3d offset = math::Vector3d(
              math::parseFloat(words[1]) * _scale,
              math::parseFloat(words[2]) * _scale,
              math::parseFloat(words[3]) * _scale);
          math::Matrix4d transform(math::Matrix4d::Identity);
          transform.SetTranslation(offset);
          node->SetTransform(transform);
        }
        else
        {
          if (words[0] == "CHANNELS")
          {
            if (words.size() < 3 ||
                static_cast<size_t>(math::parseInt(words[1]) + 2) >
                 words.size())
            {
              file.close();
              return nullptr;
            }
            nodeChannels.push_back(words);
            totalChannels += math::parseInt(words[1]);
          }
          else
          {
            if (words[0] == "{")
            {
              parent = node;
            }
            else
            {
              if (words[0] == "}")
              {
                parent = parent->Parent();
              }
              else
              {
                if (words.size() == 2 && words[0] == "End"
                        && words[1] == "Site")
                {
                  /// ignore End Sites
                  getline(file, line);  /// read {
                  getline(file, line);  /// read OFFSET
                  getline(file, line);  /// read }
                }
                else
                {
                  if (nodes.empty())
                  {
                    file.close();
                    return nullptr;
                  }
                  skeleton.reset(new Skeleton(nodes[0]));
                  break;
                }
              }
            }
          }
        }
      }
    }
  }
  getline(file, line);
  trim(line);
  std::vector<std::string> words = split(line, " ");
  unsigned int frameCount = 0;
  double frameTime = 0.0;
  if (words[0] != "Frames:" || words.size() < 2)
  {
    file.close();
    return nullptr;
  }
  else
  {
    frameCount = static_cast<unsigned int>(math::parseInt(words[1]));
  }

  getline(file, line);
  words.clear();
  trim(line);
  words = split(line, " ");

  if (words.size() < 3 || words[0] != "Frame" || words[1] != "Time:")
  {
    file.close();
    return nullptr;
  }
  else
    frameTime = math::parseFloat(words[2]);

  double time = 0.0;
  unsigned int frameNo = 0;

  SkeletonAnimation *animation = new SkeletonAnimation(_filename);

  while (!file.eof())
  {
    getline(file, line);
    words.clear();
    trim(line);
    words = split(line, " ");
    if (words.size() < totalChannels)
    {
      gzwarn << "Frame " << frameNo << " invalid.\n";
      frameNo++;
      time += frameTime;
      continue;
    }

    unsigned int cursor = 0;
    for (unsigned int i = 0; i < nodes.size(); ++i)
    {
      SkeletonNode *node = nodes[i];
      std::vector<std::string> channels = nodeChannels[i];
      math::Vector3d translation = node->Transform().Translation();
      math::Vector3d xAxis(1, 0, 0);
      math::Vector3d yAxis(0, 1, 0);
      math::Vector3d zAxis(0, 0, 1);
      double xAngle = 0.0;
      double yAngle = 0.0;
      double zAngle = 0.0;
      math::Matrix4d transform(math::Matrix4d::Identity);
      std::vector<math::Matrix4d> mats;
      unsigned int chanCount = math::parseInt(channels[1]);
      for (unsigned int j = 2; j < (2 + chanCount); ++j)
      {
        double value = math::parseFloat(words[cursor]);
        cursor++;
        std::string channel = channels[j];
        if (channel == "Xposition")
          translation.X(value * _scale);
        else
        {
          if (channel == "Yposition")
          {
            translation.Y(value * _scale);
          }
          else
          {
            if (channel == "Zposition")
            {
              translation.Z(value * _scale);
            }
            else
            {
              if (channel == "Zrotation")
              {
                zAngle = GZ_DTOR(value);
                mats.push_back(math::Matrix4d(
                      math::Quaterniond(zAxis, zAngle)));
              }
              else
              {
                if (channel == "Xrotation")
                {
                  xAngle = GZ_DTOR(value);
                  mats.push_back(math::Matrix4d(
                    math::Quaterniond(xAxis, xAngle)));
                }
                else
                {
                  if (channel == "Yrotation")
                  {
                    yAngle = GZ_DTOR(value);
                    mats.push_back(math::Matrix4d(
                      math::Quaterniond(yAxis, yAngle)));
                  }
                }
              }
            }
          }
        }
      }
      while (!mats.empty())
      {
        transform = mats.back() * transform;
        mats.pop_back();
      }
      math::Matrix4d pos(math::Matrix4d::Identity);
      pos.SetTranslation(translation);
      transform = pos * transform;
      animation->AddKeyFrame(node->Name(), time, transform);
    }

    frameNo++;
    time += frameTime;
    if (frameNo == frameCount)
      break;
  }
  if (frameNo < frameCount - 1)
    gzwarn << "BVH file ended unexpectedly.\n";

  skeleton->AddAnimation(animation);

  file.close();
  return skeleton;
}
