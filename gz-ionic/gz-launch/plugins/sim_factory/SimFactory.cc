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

#include <gz/msgs/boolean.pb.h>
#include <gz/msgs/entity_factory.pb.h>
#include <gz/msgs/stringmsg.pb.h>

#include <gz/common/Util.hh>
#include <gz/common/Console.hh>
#include <gz/msgs/Utility.hh>
#include "SimFactory.hh"

using namespace gz;
using namespace gz::launch;

/////////////////////////////////////////////////
SimFactory::SimFactory()
  : launch::Plugin()
{
}

/////////////////////////////////////////////////
void SimFactory::ProcessSpawn(const tinyxml2::XMLElement *_elem)
{
  const tinyxml2::XMLElement *elem;

  // Get the sdf field, otherwise return.
  const tinyxml2::XMLElement *sdfElem = _elem->FirstChildElement("sdf");
  if (!sdfElem)
  {
    // Return early if there is no <sdf> element.
    return;
  }

  // Get a user-defined world name
  elem = _elem->FirstChildElement("world");
  std::string worldName;
  if (elem)
  {
    worldName = elem->GetText();
  }
  // Otherwise get a world name using transport.
  else
  {
    std::vector<std::string> topics;
    std::set<std::string> worlds;

    // Get all the topics
    this->node.TopicList(topics);

    // Find the topics that start with "/world/".
    for (const std::string &topic : topics)
    {
      if (topic.find("/world/") == 0)
      {
        // Get what we would assume is the world name
        std::vector<std::string> split = common::split(topic, "/");
        worlds.insert(split[1]);
      }
    }

    // Error if no world was found.
    if (worlds.empty())
    {
      gzerr << "No simulation worlds were found. Unable to run the factory. "
        << "Is Gazebo running?\n";
      return;
    }

    // Warning if multiple worlds were found.
    if (worlds.size() > 1)
    {
      gzwarn << "Multiple simulation worlds were found. Using the first, "
        << " which has the name[" << *worlds.begin() << "]\n";
    }

    worldName = *worlds.begin();
  }

  msgs::EntityFactory *req = this->worldFactoryMsgs[worldName].add_data();

  tinyxml2::XMLPrinter printer;
  sdfElem->Accept(&printer);
  req->set_sdf(printer.CStr());

  // Get <name>
  elem = _elem->FirstChildElement("name");
  if (elem)
    req->set_name(elem->GetText());

  // Get <is_performer>, by default we assume a spawned model is a performer
  bool isPerformer = true;
  elem = _elem->FirstChildElement("is_performer");
  if (elem)
  {
    std::string str = elem->GetText();
    isPerformer = str == "1" || common::lowercase(str) == "true";
  }
  if (isPerformer)
    this->worldPerformers[worldName].push_back(req->name());

  // Get <allow_renaming>
  elem = _elem->FirstChildElement("allow_renaming");
  if (elem)
  {
    std::string str = elem->GetText();
    req->set_allow_renaming(str == "1" || common::lowercase(str) == "true");
  }

  // Get the pose
  elem = _elem->FirstChildElement("pose");
  if (elem)
  {
    math::Pose3d pose;
    std::stringstream stream;
    stream << elem->GetText();
    stream >> pose;
    msgs::Set(req->mutable_pose(), pose);
  }
}

/////////////////////////////////////////////////
bool SimFactory::Load(const tinyxml2::XMLElement *_elem)
{
  const tinyxml2::XMLElement *elem;

  // Process each <spawn> ... </spawn> element.
  elem = _elem->FirstChildElement("spawn");
  while (elem)
  {
    this->ProcessSpawn(elem);
    elem = elem->NextSiblingElement("spawn");
  }

  // Process the case where there is no <spawn> .. </spawn> entity, and only
  // one "spawn" request is embedded in the plugin.
  this->ProcessSpawn(_elem);

  for (const auto &msg : this->worldFactoryMsgs)
  {
    uint32_t timeout = 2000;
    msgs::Boolean rep;
    bool result;

    std::string topic = "/world/";
    topic += msg.first + "/create_multiple";

    // Send the request.
    bool executed = this->node.Request(topic, msg.second, timeout, rep, result);

    if (executed && result && rep.data())
    {
      gzdbg << "Factory service call succeeded.\n";
      if (!this->worldPerformers[msg.first].empty())
      {
        GZ_SLEEP_S(2);
        topic = std::string("/world/") + msg.first + "/level/set_performer";

        for (const auto &perf : this->worldPerformers[msg.first])
        {
          msgs::StringMsg performerReq;
          performerReq.set_data(perf);
          // \todo(anyone) Setting the size to 2,2,2 is a hack. Gazebo should
          // calculate the bounding box based on the model information.
          executed = this->node.Request(
              topic, performerReq, timeout, rep, result);

          // msgs::Performer performerReq;
          // performerReq.set_name(req.name());
          // // \todo(anyone) Setting the size to 2,2,2 is a hack. Gazebo should
          // // calculate the bounding box based on the model information.
          //  msgs::Set(performerReq.mutable_geometry()->mutable_box()->mutable_size(), math::Vector3d(2, 2, 2));
          // executed = this->node.Request(topic, performerReq, timeout, rep, result);
        }
      }
    }
    else
    {
      if (executed)
      {
        if (result && !rep.data())
        {
          gzerr << "Factory service call completed, but returned a false value."
            << "You may have an invalid request. Check the configuration.\n";
        }
        else
        {
          gzerr << "Factory service call failed.\n";
        }
      }
      else
      {
        gzerr << "Factory service call timed out.\n";
      }
    }
  }



  return false;
}
