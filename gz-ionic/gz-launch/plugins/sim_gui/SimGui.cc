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

#include <fstream>

#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/sim/config.hh>
#include <gz/gui/MainWindow.hh>
#include "gz/sim/gui/Gui.hh"

#include "SimGui.hh"

using namespace gz;
using namespace gz::launch;

/////////////////////////////////////////////////
SimGui::SimGui()
  : launch::Plugin()
{
}

/////////////////////////////////////////////////
SimGui::~SimGui()
{
}

/////////////////////////////////////////////////
bool SimGui::Load(const tinyxml2::XMLElement *_elem)
{
  int argc;
  char **argv = nullptr;

  // Set default config file for Launch
  std::string defaultConfigPath;
  common::env(GZ_HOMEDIR, defaultConfigPath);
  defaultConfigPath = common::joinPaths(defaultConfigPath,
      ".gz", "launch");

  auto defaultConfigFile = common::joinPaths(defaultConfigPath,
      "gui.config");

  // Check if there's a default config file under
  // ~/.gz/launch and use that. If there isn't, create it
  if (!common::exists(defaultConfigFile))
  {
    common::createDirectories(defaultConfigPath);

    std::ofstream configFile(defaultConfigFile);
    if (configFile.is_open())
    {
      configFile <<
      "<window>\n" <<
      "  <width>1000</width>\n" <<
      "  <height>845</height>\n" <<
      "  <style\n" <<
      "    material_theme='Light'\n" <<
      "    material_primary='DeepOrange'\n" <<
      "    material_accent='LightBlue'\n" <<
      "    toolbar_color_light='#f3f3f3'\n" <<
      "    toolbar_text_color_light='#111111'\n" <<
      "    toolbar_color_dark='#414141'\n" <<
      "    toolbar_text_color_dark='#f3f3f3'\n" <<
      "    plugin_toolbar_color_light='#bbdefb'\n" <<
      "    plugin_toolbar_text_color_light='#111111'\n" <<
      "    plugin_toolbar_color_dark='#607d8b'\n" <<
      "    plugin_toolbar_text_color_dark='#eeeeee'\n" <<
      "  />\n" <<
      "  <menus>\n" <<
      "    <drawer default='false'>\n" <<
      "    </drawer>\n" <<
      "  </menus>\n" <<
      "</window>\n";
      configFile.close();
      gzmsg << "Saved file [" << defaultConfigFile << "]" << std::endl;
    }
    else
    {
      gzerr << "Unable to open file [" << defaultConfigFile << "]"
      << std::endl;
    }
  }

  auto app = sim::gui::createGui(argc, argv, defaultConfigFile.c_str(),
                                 defaultConfigFile.c_str(), false);

  auto win = app->findChild<gui::MainWindow *>()->QuickWindow();

  // Customize window
  std::string windowTitle{"Gazebo"};
  auto elem = _elem->FirstChildElement("window_title");
  if (elem)
    windowTitle = elem->GetText();
  win->setProperty("title", QString::fromStdString(windowTitle));

  auto iconElem = _elem->FirstChildElement("window_icon");
  if (iconElem)
  {
    win->setIcon(QIcon(iconElem->GetText()));
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
      gzerr << "A SimGui plugin is missing the name attribute. "
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
    app->LoadPlugin(file, elem);
  }

  gzdbg << "Running the SimGui plugin.\n";
  // This blocks until the window is closed or we receive a SIGINT
  app->exec();

  return false;
}
