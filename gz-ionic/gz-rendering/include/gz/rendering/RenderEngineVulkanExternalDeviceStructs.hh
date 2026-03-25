/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_RENDERENGINE_VULKANEXTERNALDEVICESTRUCTS_HH_
#define GZ_RENDERING_RENDERENGINE_VULKANEXTERNALDEVICESTRUCTS_HH_

#ifndef __APPLE__


#include <vector>
#include "gz/rendering/Export.hh"
#include "gz/rendering/config.hh"

#ifdef GZ_RENDERING_HAVE_VULKAN

#include "vulkan/vulkan_core.h"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \brief Structure to encapsulate data needed by OgreNext to init Vulkan
    /// from Qt.
    /// Mirrors Ogre::VulkanExternalInstance without depending on OgreNext
    /// directly.
    struct GZ_RENDERING_VISIBLE GzVulkanExternalInstance
    {
      VkInstance instance;
      std::vector<VkLayerProperties> instanceLayers;
      std::vector<VkExtensionProperties> instanceExtensions;
    };

    /// \brief Structure to encapsulate data needed by OgreNext to init Vulkan
    /// from Qt.
    /// Mirrors Ogre::VulkanExternalDevice without depending on OgreNext
    /// directly.
    struct GZ_RENDERING_VISIBLE GzVulkanExternalDevice
    {
      VkPhysicalDevice physicalDevice;
      VkDevice device;
      std::vector<VkExtensionProperties> deviceExtensions;
      VkQueue graphicsQueue;
      VkQueue presentQueue;
    };
  }
  }
}

#endif
#endif
#endif
