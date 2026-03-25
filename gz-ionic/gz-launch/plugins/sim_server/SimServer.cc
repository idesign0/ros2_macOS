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

#include <gz/common/Console.hh>
#include <sdf/sdf.hh>
#include "SimServer.hh"

using namespace gz;
using namespace gz::launch;

/////////////////////////////////////////////////
void copyElement(sdf::ElementPtr _sdf, const tinyxml2::XMLElement *_xml)
{
  _sdf->SetName(_xml->Value());
  if (_xml->GetText() != nullptr)
    _sdf->AddValue("string", _xml->GetText(), "1");

  for (const tinyxml2::XMLAttribute *attribute = _xml->FirstAttribute();
       attribute; attribute = attribute->Next())
  {
    _sdf->AddAttribute(attribute->Name(), "string", "", 1, "");
    _sdf->GetAttribute(attribute->Name())->SetFromString(
        attribute->Value());
  }

  // Iterate over all the child elements
  const tinyxml2::XMLElement *elemXml = nullptr;
  for (elemXml = _xml->FirstChildElement(); elemXml;
      elemXml = elemXml->NextSiblingElement())
  {
    sdf::ElementPtr element(new sdf::Element);
    element->SetParent(_sdf);

    copyElement(element, elemXml);
    _sdf->InsertElement(element);
  }
}

/////////////////////////////////////////////////
SimServer::SimServer()
  : launch::Plugin()
{
}

/////////////////////////////////////////////////
bool SimServer::Load(const tinyxml2::XMLElement *_elem)
{
  sim::ServerConfig serverConfig;
  const tinyxml2::XMLElement *elem;

  // Get the world file
  elem = _elem->FirstChildElement("world_file");
  if (elem)
    serverConfig.SetSdfFile(elem->GetText());

  // Set whether to use levels
  elem = _elem->FirstChildElement("levels");
  if (elem)
  {
    std::string str = elem->GetText();
    serverConfig.SetUseLevels(str == "1" ||
        common::lowercase(str) == "true");
  }

  elem = _elem->FirstChildElement("record");
  // Path for logs
  std::string recordPathMod = serverConfig.LogRecordPath();
  if (elem)
  {
    const tinyxml2::XMLElement *enabled = elem->FirstChildElement("enabled");
    if (enabled)
    {
      std::string str = enabled->GetText();
      serverConfig.SetUseLogRecord(str == "1" ||
          common::lowercase(str) == "true");
    }

    // Add topics to record, if present.
    for (const tinyxml2::XMLElement *recordTopic =
        elem->FirstChildElement("record_topic"); recordTopic;
        recordTopic = recordTopic->NextSiblingElement("record_topic"))
    {
      std::string topic = recordTopic->GetText();
      serverConfig.AddLogRecordTopic(topic);
    }

    const auto *resources = elem->FirstChildElement("resources");
    if (resources)
    {
      std::string str = resources->GetText();
      serverConfig.SetLogRecordResources(str == "1" ||
          common::lowercase(str) == "true");
    }

    bool overwrite{false};
    const auto *overwriteElem = elem->FirstChildElement("overwrite");
    if (overwriteElem)
    {
      std::string str = overwriteElem->GetText();
      overwrite = (str == "1" || common::lowercase(str) == "true");
    }

    // Update compressed file path to name of recording directory path
    std::string cmpPath = std::string(recordPathMod);
    if (!std::string(1, cmpPath.back()).compare(common::separator("")))
      // Remove the separator at end of path
      cmpPath = cmpPath.substr(0, cmpPath.length() - 1);
    cmpPath += ".zip";

    const auto *pathElem = elem->FirstChildElement("path");
    if (pathElem)
    {
      recordPathMod = pathElem->GetText();
    }

    // Initialize console log
    if (!recordPathMod.empty())
    {
      // Update compressed file path to name of recording directory path
      cmpPath = std::string(recordPathMod);
      if (!std::string(1, cmpPath.back()).compare(common::separator(
        "")))
        // Remove the separator at end of path
        cmpPath = cmpPath.substr(0, cmpPath.length() - 1);
      cmpPath += ".zip";

      // Check if path or compressed file with same prefix exists
      if (common::exists(recordPathMod) ||
        common::exists(cmpPath))
      {
        // Overwrite if flag specified
        if (overwrite)
        {
          bool recordMsg = false, cmpMsg = false;
          // Remove files before initializing console log files on top of them
          if (common::exists(recordPathMod))
          {
            recordMsg = true;
            common::removeAll(recordPathMod);
          }
          if (common::exists(cmpPath))
          {
            cmpMsg = true;
            common::removeFile(cmpPath);
          }

          // Create log file before printing any messages so they can be logged
          gzLogInit(recordPathMod, "server_console.log");

          if (recordMsg)
          {
            gzmsg << "Log path already exists on disk! Existing files will be "
              << "overwritten." << std::endl;
            gzmsg << "Removing existing path [" << recordPathMod << "]\n";
          }

          if (cmpMsg)
          {
            gzwarn << "Compressed log path already exists on disk! Existing "
              << "files will be overwritten." << std::endl;
            gzmsg << "Removing existing compressed file [" << cmpPath << "]\n";
          }
        }
        // Otherwise rename to unique path
        else
        {
          // Remove the separator at end of path
          if (!std::string(1, recordPathMod.back()).compare(
            common::separator("")))
            recordPathMod = recordPathMod.substr(0, recordPathMod.length() - 1);
          recordPathMod = common::uniqueDirectoryPath(recordPathMod);

          cmpPath = std::string(recordPathMod);
          // Remove the separator at end of path
          if (!std::string(1, cmpPath.back()).compare(
            common::separator("")))
            cmpPath = cmpPath.substr(0, cmpPath.length() - 1);
          cmpPath += ".zip";

          // If compressed file exists, rename again
          if (common::exists(cmpPath))
          {
            cmpPath = common::uniqueFilePath(recordPathMod, "zip");

            size_t extIdx = cmpPath.find_last_of(".");
            recordPathMod = cmpPath.substr(0, extIdx);
          }

          gzLogInit(recordPathMod, "server_console.log");
          gzwarn << "Log path already exists on disk! "
            << "Recording instead to [" << recordPathMod << "]" << std::endl;
          gzwarn << "Compressed log path already exists on disk! "
            << "Recording instead to [" << cmpPath << "]" << std::endl;
        }
      }
      else
      {
        gzLogInit(recordPathMod, "server_console.log");
      }
    }
    else
    {
      gzLogInit(recordPathMod, "server_console.log");
    }
    serverConfig.SetLogRecordPath(recordPathMod);

    bool compress{false};
    const auto *compressElem = elem->FirstChildElement("compress");
    if (compressElem)
    {
      std::string str = compressElem->GetText();
      compress = (str == "1" || common::lowercase(str) == "true");

      if (compress)
        serverConfig.SetLogRecordCompressPath(cmpPath);
    }

    gzmsg << "Logging to [" << recordPathMod << "]" << std::endl;
  }

  if (serverConfig.UseLogRecord())
    gzmsg << "Recording to [" << recordPathMod << "]\n";

  // Set whether to use a custom random seed
  elem = _elem->FirstChildElement("seed");
  if (elem)
  {
    unsigned seed;
    auto result = elem->QueryUnsignedText(&seed);
    if (result == tinyxml2::XML_SUCCESS)
    {
      serverConfig.SetSeed(seed);
      gzmsg << "Using seed [" << seed << "]" << std::endl;
    }
    else
    {
      gzerr << "Unable to parse [" << elem->GetText() << "] as a seed."
             << " Make sure it's an unsigned int" << std::endl;
    }
  }

  // Set update rate
  elem = _elem->FirstChildElement("update_rate");
  if (elem)
  {
    unsigned updateRate;
    auto result = elem->QueryUnsignedText(&updateRate);
    if (result == tinyxml2::XML_SUCCESS)
    {
      serverConfig.SetUpdateRate(updateRate);
      gzmsg << "Using update rate [" << updateRate << "]" << std::endl;
    }
    else
    {
      gzerr << "Unable to parse [" << elem->GetText() << "] as an update rate."
             << " Make sure it's an unsigned int" << std::endl;
    }
  }

  // Get whether simulation should start paused.
  bool run = false;
  elem = _elem->FirstChildElement("run");
  if (elem)
  {
    std::string str = elem->GetText();
    run = str == "1" || common::lowercase(str) == "true";
  }

  // Process all the plugins.
  for (elem = _elem->FirstChildElement("plugin"); elem;
       elem = elem->NextSiblingElement("plugin"))
  {
    // Get the plugin's name
    const char *nameStr = elem->Attribute("name");
    std::string name = nameStr == nullptr ? "" : nameStr;
    if (name.empty())
    {
      gzerr << "A SimServer plugin is missing the name attribute. "
        << "Skipping this plugin.\n";
      continue;
    }

    // Get the plugin's filename
    const char *fileStr = elem->Attribute("filename");
    std::string file = fileStr == nullptr ? "" : fileStr;
    if (file.empty())
    {
      gzerr << "A SimServer plugin with name[" << name << "] is "
        << "missing the filename attribute. Skipping this plugin.\n";
      continue;
    }

    // Get the plugin's entity name attachment information.
    const char *entityNameStr = elem->Attribute("entity_name");
    std::string entityName = entityNameStr == nullptr ? "" : entityNameStr;
    if (entityName.empty())
    {
      gzerr << "A SimServer plugin with name[" << name << "] and "
        << "filename[" << file << "] is missing the entity_name attribute. "
        << "Skipping this plugin.\n";
      continue;
    }

    // Get the plugin's entity type attachment information.
    const char *entityTypeStr = elem->Attribute("entity_type");
    std::string entityType = entityTypeStr == nullptr ? "" : entityTypeStr;
    if (entityType.empty())
    {
      gzerr << "A SimServer plugin with name[" << name << "] and "
        << "filename[" << file << "] is missing the entity_type attribute. "
        << "Skipping this plugin.\n";
      continue;
    }

    // Create an SDF element of the plugin
    sdf::Plugin plugin;
    plugin.SetFilename(file);
    plugin.SetName(name);

    sdf::ElementPtr sdf(new sdf::Element);
    copyElement(sdf, elem);
    plugin.InsertContent(sdf);

    // Add the plugin to the server config
    serverConfig.AddPlugin({entityName, entityType, plugin});
  }

  // Set headless rendering
  elem = _elem->FirstChildElement("headless_rendering");
  if (elem)
  {
    std::string str = elem->GetText();
    serverConfig.SetHeadlessRendering(str == "1" ||
        common::lowercase(str) == "true");
  }

  // Create and run the simulation server
  this->server.reset(new sim::Server(serverConfig));
  this->server->Run(false, 0, !run);

  gzdbg << "Loaded SimServer plugin.\n";
  return true;
}
