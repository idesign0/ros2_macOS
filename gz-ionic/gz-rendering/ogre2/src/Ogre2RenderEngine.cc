/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#ifdef _WIN32
  // Ensure that Winsock2.h is included before Windows.h, which can get
  // pulled in by anybody (e.g., Boost).
  #include <Winsock2.h>
#endif
#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/StringUtils.hh>
#include <gz/common/SystemPaths.hh>
#include <gz/common/Util.hh>

#include <gz/plugin/Register.hh>

#include "gz/rendering/config.hh"
#include "gz/rendering/GraphicsAPI.hh"
#include "gz/rendering/InstallationDirectories.hh"
#include "gz/rendering/RenderEngineManager.hh"
#include "gz/rendering/ogre2/Ogre2Includes.hh"
#include "gz/rendering/ogre2/Ogre2NativeWindow.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2RenderTypes.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"
#include "gz/rendering/ogre2/Ogre2Storage.hh"

#include "Ogre2GzHlmsPbsPrivate.hh"
#include "Ogre2GzHlmsTerraPrivate.hh"
#include "Ogre2GzHlmsUnlitPrivate.hh"

#ifdef OGRE_BUILD_RENDERSYSTEM_VULKAN
#  include "vulkan/vulkan_core.h"
// Deal with old vulkan headers causing build errors
#  ifndef VK_NV_device_generated_commands
#    define VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV 0x00020000
#  endif
#  define VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR 0x00200000
// Needed headers to receive an external Vulkan device from Qt
// and inject it into OgreNext
#  include "RenderSystems/Vulkan/OgreVulkanDevice.h"
#  include "gz/rendering/RenderEngineVulkanExternalDeviceStructs.hh"
#endif
#include "Ogre2GzHlmsSphericalClipMinDistance.hh"
#include "Terra/Hlms/OgreHlmsTerra.h"
#include "Terra/Hlms/PbsListener/OgreHlmsPbsTerraShadows.h"
#include "Terra/TerraWorkspaceListener.h"

#if HAVE_GLX
# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <GL/glx.h>
# include <GL/glxext.h>
#endif

#if HAVE_EGL
  #include <EGL/egl.h>
#endif

#if defined(__APPLE__)
#include <OpenGL/CGLCurrent.h>
#endif

class GZ_RENDERING_OGRE2_HIDDEN
    gz::rendering::Ogre2RenderEnginePrivate
{
#if HAVE_GLX
  public: GLXFBConfig* dummyFBConfigs = nullptr;
#endif

  /// \brief The graphics API to use
  public: gz::rendering::GraphicsAPI graphicsAPI{GraphicsAPI::OPENGL};

  /// \brief A list of supported fsaa levels
  public: std::vector<unsigned int> fsaaLevels;

  /// \brief Controls Hlms customizations for both PBS and Unlit
  public: gz::rendering::Ogre2GzHlmsSphericalClipMinDistance
  sphericalClipMinDistance;

  /// \brief Pbs listener that adds terra shadows
  public: std::unique_ptr<Ogre::HlmsPbsTerraShadows> hlmsPbsTerraShadows;

  /// \brief Listener that needs to be in every workspace
  /// that wants terrain to cast shadows from spot and point lights
  public: std::unique_ptr<Ogre::TerraWorkspaceListener> terraWorkspaceListener;

  /// \brief Custom PBS modifications
  public: Ogre::Ogre2GzHlmsPbs *gzHlmsPbs{nullptr};

  /// \brief Custom Unlit modifications
  public: Ogre::Ogre2GzHlmsUnlit *gzHlmsUnlit{nullptr};

  /// \brief Custom Terra modifications
  public: Ogre::Ogre2GzHlmsTerra *gzHlmsTerra{nullptr};

#ifdef OGRE_BUILD_RENDERSYSTEM_VULKAN
  /// \brief Needed to receive an external Vulkan device from Qt
  /// and inject it into OgreNext.
  ///
  /// TODO(anyone): This struct doesn't need to live forever.
  /// It only needs to live until Root::loadPlugin( "RenderSystem_Vulkan" )
  /// is called.
  public: Ogre::VulkanExternalInstance vkExternalInstance{};

  /// \brief Needed to receive an external Vulkan device from Qt
  /// and inject it into OgreNext.
  ///
  /// TODO(anyone): This struct doesn't need to live forever.
  /// It only needs to live until the first Root::createRenderWindow
  /// is called.
  public: Ogre::VulkanExternalDevice vkExternalDevice{};
#endif
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2RenderEnginePlugin::Ogre2RenderEnginePlugin()
{
}

//////////////////////////////////////////////////
std::string Ogre2RenderEnginePlugin::Name() const
{
  return Ogre2RenderEngine::Instance()->Name();
}

//////////////////////////////////////////////////
RenderEngine *Ogre2RenderEnginePlugin::Engine() const
{
  return Ogre2RenderEngine::Instance();
}

//////////////////////////////////////////////////
Ogre2RenderEngine::Ogre2RenderEngine() :
  dataPtr(new Ogre2RenderEnginePrivate)
{
  this->dummyDisplay = nullptr;
  this->dummyContext = 0;
  this->dummyWindowId = 0;

  std::string ogrePath = std::string(OGRE2_RESOURCE_PATH);
  std::vector<std::string> paths = common::Split(ogrePath,
      common::SystemPaths::Delimiter());
  for (const auto &path : paths)
    this->ogrePaths.push_back(path);

  const char *env = std::getenv("OGRE2_RESOURCE_PATH");
  if (env)
  {
    paths = common::Split(std::string(env), common::SystemPaths::Delimiter());
    for (const auto &path : paths)
      this->ogrePaths.push_back(path);
  }
}

//////////////////////////////////////////////////
Ogre::Window * Ogre2RenderEngine::OgreWindow() const
{
  return this->window;
}

//////////////////////////////////////////////////
Ogre2RenderEngine::~Ogre2RenderEngine()
{
}

//////////////////////////////////////////////////
void Ogre2RenderEngine::Destroy()
{
  BaseRenderEngine::Destroy();

  if (this->scenes)
  {
    this->scenes->RemoveAll();
  }

#ifdef GZ_RENDERING_HAVE_OGRE2_OVERLAY
  delete this->ogreOverlaySystem;
  this->ogreOverlaySystem = nullptr;
#endif

  this->dataPtr->hlmsPbsTerraShadows.reset();

  if (this->ogreRoot)
  {
    // Clean up any textures that may still be in flight.
    Ogre::TextureGpuManager *mgr =
        this->ogreRoot->getRenderSystem()->getTextureGpuManager();

    auto entries = mgr->getEntries();
    for (const auto& [name, entry] : entries)
    {
      if (entry.resourceGroup == "General" && !entry.destroyRequested)
        mgr->destroyTexture(entry.texture);
    }

    try
    {
      // TODO(anyone): do we need to catch segfault on delete?
      delete this->ogreRoot;
    }
    catch (...)
    {
      gzerr << "Error deleting ogre root " << std::endl;
    }
    this->ogreRoot = nullptr;
  }

  delete this->ogreLogManager;
  this->ogreLogManager = nullptr;

#if HAVE_GLX
  if (this->dummyDisplay)
  {
    Display *x11Display = static_cast<Display*>(this->dummyDisplay);
    if (this->dummyContext)
    {
      GLXContext x11Context = static_cast<GLXContext>(this->dummyContext);
      glXDestroyContext(x11Display, x11Context);
      this->dummyContext = nullptr;
    }
    XDestroyWindow(x11Display, this->dummyWindowId);
    XCloseDisplay(x11Display);
    this->dummyDisplay = nullptr;
    if (this->dataPtr->dummyFBConfigs)
    {
      XFree(this->dataPtr->dummyFBConfigs);
      this->dataPtr->dummyFBConfigs = nullptr;
    }
  }
#endif

#if HAVE_EGL
  // release egl per-thread state otherwise this causes a crash on exit if
  // ogre is created and deleted in a thread
  // Do this only if we are using GL
  if (this->dataPtr->graphicsAPI == GraphicsAPI::OPENGL)
    eglReleaseThread();
#endif
}

//////////////////////////////////////////////////
bool Ogre2RenderEngine::IsEnabled() const
{
  return BaseRenderEngine::IsEnabled();
}

//////////////////////////////////////////////////
std::string Ogre2RenderEngine::Name() const
{
  return "ogre2";
}

//////////////////////////////////////////////////
void Ogre2RenderEngine::AddResourcePath(const std::string &_uri)
{
  if (_uri == "__default__" || _uri.empty())
    return;

  std::string path = common::findFilePath(_uri);

  if (path.empty())
  {
    gzerr << "URI doesn't exist[" << _uri << "]\n";
    return;
  }

  this->resourcePaths.push_back(path);

  try
  {
    if (!Ogre::ResourceGroupManager::getSingleton().resourceLocationExists(
          path, "General"))
    {
      Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          path, "FileSystem", "General", true);

      Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup(
          "General", false);
      // Parse all material files in the path if any exist
      if (common::isDirectory(path))
      {
        std::vector<std::string> paths;

        common::DirIter endIter;
        for (common::DirIter dirIter(path); dirIter != endIter; ++dirIter)
        {
          paths.push_back(*dirIter);
        }
        std::sort(paths.begin(), paths.end());

        // Iterate over all the models in the current gz-rendering path
        for (auto dIter = paths.begin(); dIter != paths.end(); ++dIter)
        {
          std::string fullPath = *dIter;
          std::string matExtension = fullPath.substr(fullPath.size()-9);
          if (matExtension == ".material")
          {
            Ogre::DataStreamPtr stream =
              Ogre::ResourceGroupManager::getSingleton().openResource(
                  fullPath, "General");

            // There is a material file under there somewhere, read the thing in
            try
            {
              Ogre::MaterialManager::getSingleton().parseScript(
                  stream, "General");
              Ogre::MaterialPtr matPtr =
                Ogre::MaterialManager::getSingleton().getByName(
                    fullPath);

              if (!matPtr.isNull())
              {
                // is this necessary to do here? Someday try it without
                matPtr->compile();
                matPtr->load();
              }
            }
            catch(Ogre::Exception&)
            {
              gzerr << "Unable to parse material file[" << fullPath << "]\n";
            }
            stream->close();
          }
        }
      }
    }
  }
  catch(Ogre::Exception &)
  {
    gzerr << "Unable to load Ogre Resources.\nMake sure the"
        "resources path in the world file is set correctly." << std::endl;
  }
}

//////////////////////////////////////////////////
Ogre::Root *Ogre2RenderEngine::OgreRoot() const
{
  return this->ogreRoot;
}

//////////////////////////////////////////////////
ScenePtr Ogre2RenderEngine::CreateSceneImpl(unsigned int _id,
    const std::string &_name)
{
  Ogre2ScenePtr scene = Ogre2ScenePtr(new Ogre2Scene(_id, _name));
  this->scenes->Add(scene);
  return scene;
}

//////////////////////////////////////////////////
SceneStorePtr Ogre2RenderEngine::Scenes() const
{
  return this->scenes;
}

//////////////////////////////////////////////////
bool Ogre2RenderEngine::LoadImpl(
    const std::map<std::string, std::string> &_params)
{
  // parse params
  auto it = _params.find("useCurrentGLContext");
  if (it != _params.end())
    std::istringstream(it->second) >> this->useCurrentGLContext;

  it = _params.find("headless");
  if (it != _params.end())
    std::istringstream(it->second) >> this->isHeadless;

  it = _params.find("winID");
  if (it != _params.end())
    std::istringstream(it->second) >> this->winID;

  it = _params.find("metal");
  if (it != _params.end())
  {
    bool useMetal;
    std::istringstream(it->second) >> useMetal;
    if(useMetal)
        this->dataPtr->graphicsAPI = GraphicsAPI::METAL;
  }

#ifdef OGRE_BUILD_RENDERSYSTEM_VULKAN
  this->dataPtr->vkExternalInstance.instance = nullptr;
  this->dataPtr->vkExternalDevice.physicalDevice = nullptr;
  this->dataPtr->vkExternalDevice.device = nullptr;
  this->dataPtr->vkExternalDevice.graphicsQueue = nullptr;
  this->dataPtr->vkExternalDevice.presentQueue = nullptr;
#endif

  it = _params.find("vulkan");
  if (it != _params.end())
  {
    bool useVulkan;
    std::istringstream(it->second) >> useVulkan;
    if(useVulkan)
    {
      this->dataPtr->graphicsAPI = GraphicsAPI::VULKAN;

#ifdef OGRE_BUILD_RENDERSYSTEM_VULKAN
      it = _params.find("external_instance");
      if (it != _params.end())
      {
        auto *gzExternalInstance =
          reinterpret_cast<const GzVulkanExternalInstance *>(
            Ogre::StringConverter::parseUnsignedLong(it->second));

        this->dataPtr->vkExternalInstance.instance =
          gzExternalInstance->instance;
        // This works as long as std::vector memory is actually contiguous
        this->dataPtr->vkExternalInstance.instanceLayers.appendPOD(
          &*gzExternalInstance->instanceLayers.begin(),
          &*gzExternalInstance->instanceLayers.end());
        this->dataPtr->vkExternalInstance.instanceExtensions.appendPOD(
          &*gzExternalInstance->instanceExtensions.begin(),
          &*gzExternalInstance->instanceExtensions.end());
      }

      it = _params.find("external_device");
      if (it != _params.end())
      {
        auto *gzExternalDevice =
          reinterpret_cast<const GzVulkanExternalDevice *>(
            Ogre::StringConverter::parseUnsignedLong(it->second));

        this->dataPtr->vkExternalDevice.physicalDevice =
          gzExternalDevice->physicalDevice;
        this->dataPtr->vkExternalDevice.device = gzExternalDevice->device;
        this->dataPtr->vkExternalDevice.graphicsQueue =
          gzExternalDevice->graphicsQueue;
        this->dataPtr->vkExternalDevice.presentQueue =
          gzExternalDevice->presentQueue;
        // This works as long as std::vector memory is actually contiguous
        this->dataPtr->vkExternalDevice.deviceExtensions.appendPOD(
          &*gzExternalDevice->deviceExtensions.begin(),
          &*gzExternalDevice->deviceExtensions.end());
      }
#endif
    }
  }

  try
  {
    this->LoadAttempt();
    this->loaded = true;
    return true;
  }
  catch (Ogre::Exception &ex)
  {
    gzerr << ex.what() << std::endl;
    return false;
  }
  catch (...)
  {
    gzerr << "Failed to load render-engine" << std::endl;
    return false;
  }
}

//////////////////////////////////////////////////
bool Ogre2RenderEngine::InitImpl()
{
  try
  {
    this->InitAttempt();
    return true;
  }
  catch (...)
  {
    gzerr << "Failed to initialize render-engine" << std::endl;
    gzerr << "Please see the troubleshooting page for possible fixes: "
          << "https://gazebosim.org/docs/latest/troubleshooting"
          << std::endl;
    return false;
  }
}

//////////////////////////////////////////////////
void Ogre2RenderEngine::LoadAttempt()
{
  this->CreateLogger();
  if (!this->useCurrentGLContext &&
      (this->dataPtr->graphicsAPI == GraphicsAPI::OPENGL ||
       this->dataPtr->graphicsAPI == GraphicsAPI::VULKAN))
  {
    this->CreateContext();
  }

  this->CreateRoot();
#ifdef GZ_RENDERING_HAVE_OGRE2_OVERLAY
  this->CreateOverlay();
#endif
  this->LoadPlugins();
  this->CreateRenderSystem();
  this->ogreRoot->initialise(false);
  this->CreateRenderWindow();
  this->CreateResources();
}

//////////////////////////////////////////////////
void Ogre2RenderEngine::CreateLogger()
{
  // create log file path
  std::string logPath;
  common::env(GZ_HOMEDIR, logPath);
  logPath = common::joinPaths(logPath, ".gz", "rendering");
  common::createDirectories(logPath);
  logPath = common::joinPaths(logPath, "ogre2.log");

  // create actual log
  this->ogreLogManager = new Ogre::LogManager();
  this->ogreLogManager->createLog(logPath, true, false, false);
}

//////////////////////////////////////////////////
void Ogre2RenderEngine::CreateContext()
{
#ifdef OGRE_BUILD_RENDERSYSTEM_VULKAN
  if (this->dataPtr->vkExternalInstance.instance)
  {
    return;
  }
#endif
#if !defined(__APPLE__) && !defined(_WIN32)
  if (this->Headless())
  {
    // Nothing to do
    return;
  }
#if HAVE_GLX
  // create X11 display
  this->dummyDisplay = XOpenDisplay(0);
  Display *x11Display = static_cast<Display*>(this->dummyDisplay);

  if (!this->dummyDisplay)
  {
    // Not able to create a Xwindow, try to run in headless mode
    this->SetHeadless(true);
    gzwarn << "Unable to open display: " << XDisplayName(0)
            << ". Trying to run in headless mode." << std::endl;
    return;
  }

  // create X11 visual
  int screenId = DefaultScreen(x11Display);

  int attributeList[] = {
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_DOUBLEBUFFER, True,
    GLX_DEPTH_SIZE, 16,
    GLX_STENCIL_SIZE, 8,
    None
  };

  if (this->dataPtr->graphicsAPI == GraphicsAPI::OPENGL)
  {
    int nelements = 0;
    this->dataPtr->dummyFBConfigs =
      glXChooseFBConfig(x11Display, screenId, attributeList, &nelements);

    if (nelements <= 0)
    {
      gzerr << "Unable to create glx fbconfig" << std::endl;
      return;
    }
  }

  // create X11 context
  this->dummyWindowId = XCreateSimpleWindow(x11Display,
      RootWindow(this->dummyDisplay, screenId), 0, 0, 1, 1, 0, 0, 0);

  if (this->dataPtr->graphicsAPI == GraphicsAPI::OPENGL)
  {
    PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB =
      (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress(
        (const GLubyte *)"glXCreateContextAttribsARB");

    if (glXCreateContextAttribsARB)
    {
      int contextAttribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,  //
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,  //
        None                               //
      };
      // clang-format on
      this->dummyContext =
        glXCreateContextAttribsARB(x11Display, this->dataPtr->dummyFBConfigs[0],
                                   nullptr, 1, contextAttribs);
    }
    else
    {
      gzwarn << "glXCreateContextAttribsARB() not found" << std::endl;
      this->dummyContext =
        glXCreateNewContext(x11Display, this->dataPtr->dummyFBConfigs[0],
                            GLX_RGBA_TYPE, nullptr, 1);
    }

    GLXContext x11Context = static_cast<GLXContext>(this->dummyContext);

    if (!this->dummyContext)
    {
      gzerr << "Unable to create glx context" << std::endl;
      return;
    }

    // select X11 context
    glXMakeCurrent(x11Display, this->dummyWindowId, x11Context);
  }
#endif
#endif
}

//////////////////////////////////////////////////
void Ogre2RenderEngine::CreateRoot()
{
  try
  {
    this->ogreRoot = new Ogre::Root("", "", "");
  }
  catch (Ogre::Exception &)
  {
    gzerr << "Unable to create Ogre root" << std::endl;
  }
}

#ifdef GZ_RENDERING_HAVE_OGRE2_OVERLAY
//////////////////////////////////////////////////
void Ogre2RenderEngine::CreateOverlay()
{
  this->ogreOverlaySystem = new Ogre::v1::OverlaySystem();
}
#endif

//////////////////////////////////////////////////
void Ogre2RenderEngine::LoadPlugins()
{
  for (auto iter = this->ogrePaths.begin();
       iter != this->ogrePaths.end(); ++iter)
  {
    std::string path(*iter);
    if (!common::isDirectory(path))
      continue;

    struct PluginName
    {
      std::string name;
      bool bOptional;
    };

    std::vector<PluginName> plugins;

#ifdef __APPLE__
    std::string extension = ".dylib";
#elif _WIN32
    std::string extension = ".dll";
#else
    std::string extension = ".so";
#endif
    std::string p = common::joinPaths(path, "RenderSystem_GL3Plus");
    plugins.push_back({ p, false });
    p = common::joinPaths(path, "Plugin_ParticleFX");
    plugins.push_back({ p, false });

    if (this->dataPtr->graphicsAPI == GraphicsAPI::VULKAN)
    {
      p = common::joinPaths(path, "RenderSystem_Vulkan");
      // Vulkan is very recent, so mark it as optional, as it
      // can easily fail to load
      plugins.push_back({ p, true });
    }

    if (this->dataPtr->graphicsAPI == GraphicsAPI::METAL)
    {
      p = common::joinPaths(path, "RenderSystem_Metal");
      plugins.push_back({ p, false });
    }

    Ogre::NameValuePairList pluginParams;

    for (std::vector<PluginName>::iterator piter = plugins.begin();
         piter != plugins.end(); ++piter)
    {
      // check if plugin library exists
#ifndef DEBUG
      std::string filename = piter->name + extension;
#else
      std::string filename = piter->name + "_d" + extension;
#endif

      if (!common::exists(filename))
      {
        filename = filename + "." + std::string(OGRE2_VERSION);
        if (!common::exists(filename))
        {
          if (piter->name.find("RenderSystem") != std::string::npos)
          {
            gzerr << "Unable to find Ogre Plugin[" << piter->name
                   << "]. Rendering will not be possible."
                   << "Make sure you have installed OGRE properly.\n";
          }
          continue;
        }
      }

      // load the plugin
      try
      {
        pluginParams.clear();

#ifdef OGRE_BUILD_RENDERSYSTEM_VULKAN
        if (this->dataPtr->vkExternalInstance.instance &&
            filename.find("RenderSystem_Vulkan") != std::string::npos)
        {
          pluginParams["external_instance"] = std::to_string(
            reinterpret_cast<uintptr_t>(&this->dataPtr->vkExternalInstance));
        }
#endif

#if HAVE_GLX
        if (filename.find("RenderSystem_GL3Plus") != std::string::npos)
        {
          // Store the current GLX context in case OGRE plugin init changes it
          const auto context = glXGetCurrentContext();
          const auto display = glXGetCurrentDisplay();
          const auto drawable = glXGetCurrentDrawable();
          this->ogreRoot->loadPlugin(filename, piter->bOptional, &pluginParams);
          // Restore GLX context
          glXMakeCurrent(display, drawable, context);
        }
        else
#endif
        {
          // Load the plugin into OGRE
          this->ogreRoot->loadPlugin(filename, piter->bOptional, &pluginParams);
        }
      }
      catch(Ogre::Exception &)
      {
        if (piter->name.find("RenderSystem") != std::string::npos)
        {
          gzerr << "Unable to load Ogre Plugin[" << piter->name
                 << "]. Rendering will not be possible."
                 << "Make sure you have installed OGRE properly.\n";
        }
      }
    }
  }
}

//////////////////////////////////////////////////
void Ogre2RenderEngine::CreateRenderSystem()
{
  std::string targetRenderSysName("OpenGL 3+ Rendering Subsystem");
  if (this->dataPtr->graphicsAPI == GraphicsAPI::VULKAN)
  {
    targetRenderSysName = "Vulkan Rendering Subsystem";
  }
  if (this->dataPtr->graphicsAPI == GraphicsAPI::METAL)
  {
    targetRenderSysName = "Metal Rendering Subsystem";
  }

  Ogre::RenderSystem *renderSys = nullptr;
  for (auto *render : this->ogreRoot->getAvailableRenderers())
  {
    renderSys = render;
    if (renderSys && renderSys->getName() == targetRenderSysName)
      break;
  }

  if (renderSys == nullptr)
  {
    gzerr << "unable to find " << targetRenderSysName << ". OGRE is probably "
            "installed incorrectly. Double check the OGRE cmake output, "
            "and make sure OpenGL is enabled." << std::endl;
  }

  if (!this->Headless())
  {

    // We operate in windowed mode
    renderSys->setConfigOption("Full Screen", "No");

    if (this->dataPtr->graphicsAPI == GraphicsAPI::OPENGL)
    {
      /// We used to allow the user to set the RTT mode to PBuffer,
      /// FBO, or Copy.
      ///   Copy is slow, and there doesn't seem to be a good reason to use it
      ///   PBuffer limits the size of the renderable area of the RTT to the
      ///           size of the first window created.
      ///   FBO is the only good option
      renderSys->setConfigOption("RTT Preferred Mode", "FBO");
    }
  }
  else
  {
    try
    {
        // This may fail if Ogre was *only* build with EGL support, but in that
        // case we can ignore the error
        renderSys->setConfigOption( "Interface", "Headless EGL / PBuffer" );
    }
    catch( Ogre::Exception & )
    {
      std::cerr << "Unable to setup EGL (headless mode)" << '\n';
    }
  }

  // get all supported fsaa values
  Ogre::ConfigOptionMap configMap = renderSys->getConfigOptions();
  auto fsaaOoption = configMap.find("FSAA");

  if (fsaaOoption != configMap.end())
  {
    auto values = (*fsaaOoption).second.possibleValues;
    for (auto const &str : values)
    {
      int value = 0;
      try
      {
        value = std::stoi(str);
      }
      catch(...)
      {
        continue;
      }
      this->dataPtr->fsaaLevels.push_back(value);
    }
  }
  std::sort(this->dataPtr->fsaaLevels.begin(), this->dataPtr->fsaaLevels.end());

  // check if target fsaa is supported
  unsigned int fsaa = 0;
  unsigned int targetFSAA = 4;
  auto const it = std::find(this->dataPtr->fsaaLevels.begin(),
      this->dataPtr->fsaaLevels.end(), targetFSAA);
  if (it != this->dataPtr->fsaaLevels.end())
    fsaa = targetFSAA;

  renderSys->setConfigOption("FSAA", std::to_string(fsaa));

  this->ogreRoot->setRenderSystem(renderSys);
}

void Ogre2RenderEngine::RegisterHlms()
{
  const char *env = std::getenv("GZ_RENDERING_RESOURCE_PATH");

  std::string resourcePath = (env) ? std::string(env) :
      gz::rendering::getResourcePath();

  // install path
  std::string mediaPath = common::joinPaths(resourcePath, "ogre2", "media");
  if (!common::exists(mediaPath))
  {
    // src path
    mediaPath = common::joinPaths(resourcePath, "ogre2", "src", "media");
  }

  // register PbsMaterial resources
  Ogre::String rootHlmsFolder = mediaPath;
  Ogre::String pbsCompositorFolder = common::joinPaths(
      rootHlmsFolder, "2.0", "scripts", "Compositors");
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
      pbsCompositorFolder, "FileSystem", "General");
  Ogre::String commonMaterialFolder = common::joinPaths(
      rootHlmsFolder, "2.0", "scripts", "materials", "Common");
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
      commonMaterialFolder, "FileSystem", "General");
  Ogre::String commonGLSLMaterialFolder = common::joinPaths(
      rootHlmsFolder, "2.0", "scripts", "materials", "Common", "GLSL");
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
      commonGLSLMaterialFolder, "FileSystem", "General");
  Ogre::String terraMaterialFolder = common::joinPaths(
      rootHlmsFolder, "2.0", "scripts", "materials", "Terra");
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
      terraMaterialFolder, "FileSystem", "General");
  Ogre::String terraGLSLMaterialFolder = common::joinPaths(
      rootHlmsFolder, "2.0", "scripts", "materials", "Terra", "GLSL");
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
      terraGLSLMaterialFolder, "FileSystem", "General");

  if (this->dataPtr->graphicsAPI == GraphicsAPI::METAL)
  {
    Ogre::String commonMetalMaterialFolder = common::joinPaths(
        rootHlmsFolder, "2.0", "scripts", "materials", "Common", "Metal");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        commonMetalMaterialFolder, "FileSystem", "General");
    Ogre::String terraMetalMaterialFolder = common::joinPaths(
        rootHlmsFolder, "2.0", "scripts", "materials", "Terra", "Metal");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        terraMetalMaterialFolder, "FileSystem", "General");
  }

  // The following code is taken from the registerHlms() function in ogre2
  // samples framework
  if (rootHlmsFolder.empty())
    rootHlmsFolder = "./";
  else if (*(rootHlmsFolder.end() - 1) != '/')
    rootHlmsFolder += "/";

  // At this point rootHlmsFolder should be a valid path to the Hlms data folder

  // For retrieval of the paths to the different folders needed
  Ogre::String mainFolderPath;
  Ogre::StringVector libraryFoldersPaths;
  Ogre::StringVector::const_iterator libraryFolderPathIt;
  Ogre::StringVector::const_iterator libraryFolderPathEn;

  Ogre::ArchiveManager &archiveManager = Ogre::ArchiveManager::getSingleton();

  Ogre::Archive *customizationsArchiveLibrary =
      archiveManager.load(common::joinPaths(rootHlmsFolder, "Hlms", "Gz"),
      "FileSystem", true);

  {
    Ogre::Ogre2GzHlmsUnlit *hlmsUnlit = 0;
    // Create & Register HlmsUnlit
    // Get the path to all the subdirectories used by HlmsUnlit
    Ogre::Ogre2GzHlmsUnlit::getDefaultPaths(mainFolderPath,
                                             libraryFoldersPaths);
    Ogre::Archive *archiveUnlit = archiveManager.load(
        rootHlmsFolder + mainFolderPath, "FileSystem", true);
    Ogre::ArchiveVec archiveUnlitLibraryFolders;
    libraryFolderPathIt = libraryFoldersPaths.begin();
    libraryFolderPathEn = libraryFoldersPaths.end();
    while (libraryFolderPathIt != libraryFolderPathEn)
    {
      Ogre::Archive *archiveLibrary =
          archiveManager.load(rootHlmsFolder + *libraryFolderPathIt,
          "FileSystem", true);
      archiveUnlitLibraryFolders.push_back(archiveLibrary);
      ++libraryFolderPathIt;
    }

    archiveUnlitLibraryFolders.push_back(customizationsArchiveLibrary);

    // Create and register the unlit Hlms
    hlmsUnlit = OGRE_NEW Ogre::Ogre2GzHlmsUnlit(
      archiveUnlit, &archiveUnlitLibraryFolders,
      &this->dataPtr->sphericalClipMinDistance);
    Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlmsUnlit);

    // disable writing debug output to disk
    hlmsUnlit->setDebugOutputPath(false, false);
    hlmsUnlit->setListener(hlmsUnlit);

    this->dataPtr->gzHlmsUnlit = hlmsUnlit;
  }

  {
    Ogre::Ogre2GzHlmsPbs *hlmsPbs = 0;
    // Create & Register HlmsPbs
    // Do the same for HlmsPbs:
    Ogre::Ogre2GzHlmsPbs::GetDefaultPaths(mainFolderPath, libraryFoldersPaths);
    Ogre::Archive *archivePbs = archiveManager.load(
        rootHlmsFolder + mainFolderPath, "FileSystem", true);

    // Get the library archive(s)
    Ogre::ArchiveVec archivePbsLibraryFolders;

    {
      archivePbsLibraryFolders.push_back(archiveManager.load(
        rootHlmsFolder + common::joinPaths("Hlms", "Terra", "GLSL",
        "PbsTerraShadows"), "FileSystem", true ));
      this->dataPtr->hlmsPbsTerraShadows.reset(new Ogre::HlmsPbsTerraShadows());
    }

    libraryFolderPathIt = libraryFoldersPaths.begin();
    libraryFolderPathEn = libraryFoldersPaths.end();
    while (libraryFolderPathIt != libraryFolderPathEn)
    {
      Ogre::Archive *archiveLibrary =
          archiveManager.load(rootHlmsFolder + *libraryFolderPathIt,
          "FileSystem", true);
      archivePbsLibraryFolders.push_back(archiveLibrary);
      ++libraryFolderPathIt;
    }

    archivePbsLibraryFolders.push_back(customizationsArchiveLibrary);

    // Create and register
    hlmsPbs =
      OGRE_NEW Ogre::Ogre2GzHlmsPbs(archivePbs, &archivePbsLibraryFolders,
                                     &this->dataPtr->sphericalClipMinDistance,
                                     this->dataPtr->hlmsPbsTerraShadows.get());
    Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlmsPbs);

    // disable writing debug output to disk
    hlmsPbs->setDebugOutputPath(false, false);
    hlmsPbs->setListener(hlmsPbs);

    dataPtr->gzHlmsPbs = hlmsPbs;
  }

  {
    Ogre::Ogre2GzHlmsTerra *hlmsTerra = 0;
    // Create & Register HlmsPbs
    // Do the same for HlmsPbs:
    Ogre::Ogre2GzHlmsTerra::GetDefaultPaths(mainFolderPath,
                                             libraryFoldersPaths);
    Ogre::Archive *archiveTerra = archiveManager.load(
        rootHlmsFolder + mainFolderPath, "FileSystem", true);

    // Get the library archive(s)
    Ogre::ArchiveVec archiveTerraLibraryFolders;
    libraryFolderPathIt = libraryFoldersPaths.begin();
    libraryFolderPathEn = libraryFoldersPaths.end();
    while (libraryFolderPathIt != libraryFolderPathEn)
    {
      Ogre::Archive *archiveLibrary =
          archiveManager.load(rootHlmsFolder + *libraryFolderPathIt,
          "FileSystem", true);
      archiveTerraLibraryFolders.push_back(archiveLibrary);
      ++libraryFolderPathIt;
    }

    // Create and register
    hlmsTerra = OGRE_NEW Ogre::Ogre2GzHlmsTerra(
      archiveTerra, &archiveTerraLibraryFolders,
      &this->dataPtr->sphericalClipMinDistance);
    Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlmsTerra);

    // disable writing debug output to disk
    hlmsTerra->setDebugOutputPath(false, false);
    hlmsTerra->setListener(hlmsTerra);

    this->dataPtr->terraWorkspaceListener.reset(
      new Ogre::TerraWorkspaceListener(hlmsTerra));

    this->dataPtr->gzHlmsTerra = hlmsTerra;
  }
}

//////////////////////////////////////////////////
void Ogre2RenderEngine::CreateResources()
{
  const char *env = std::getenv("GZ_RENDERING_RESOURCE_PATH");

  std::string resourcePath = (env) ? std::string(env) :
      gz::rendering::getResourcePath();
  // install path
  std::string mediaPath = common::joinPaths(resourcePath, "ogre2", "media");
  if (!common::exists(mediaPath))
  {
    // src path
    mediaPath = common::joinPaths(resourcePath, "ogre2", "src", "media");
  }

  // register low level materials (ogre v1 materials)
  // and compute shader stuff too
  std::vector< std::pair<std::string, std::string> > archNames;
  std::string p = mediaPath;
  if (common::isDirectory(p))
  {
    archNames.push_back(
        std::make_pair(p, "General"));
    archNames.push_back(
        std::make_pair(p + "/materials/programs", "General"));
    archNames.push_back(
        std::make_pair(p + "/materials/programs/GLSL", "General"));
    archNames.push_back(
        std::make_pair(p + "/materials/programs/Metal", "General"));
    archNames.push_back(
        std::make_pair(p + "/materials/scripts", "General"));
    archNames.push_back(
        std::make_pair(p + "/materials/textures", "General"));
    archNames.push_back(
        std::make_pair(p + "/Compute/Tools", "General"));
    archNames.push_back(
        std::make_pair(p + "/Compute/Tools/Any", "General"));
    archNames.push_back(
        std::make_pair(p + "/Compute/Tools/GLSL", "General"));
    archNames.push_back(
        std::make_pair(p + "/Compute/Tools/HLSL", "General"));
    archNames.push_back(
        std::make_pair(p + "/Compute/Tools/Metal", "General"));
    archNames.push_back(
        std::make_pair(p + "/VCT", "General"));
    archNames.push_back(
        std::make_pair(p + "/VCT/ImageVoxelizer", "General"));
    archNames.push_back(
        std::make_pair(p + "/VCT/Visualizer", "General"));
    archNames.push_back(
        std::make_pair(p + "/Hlms/Common/Any", "General"));
    archNames.push_back(
        std::make_pair(p + "/Hlms/Common/GLSL", "General"));
    archNames.push_back(
        std::make_pair(p + "/Hlms/Pbs/Any", "General"));

    for (auto aiter = archNames.begin(); aiter != archNames.end(); ++aiter)
    {
      try
      {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
            aiter->first, "FileSystem", aiter->second);
      }
      catch(Ogre::Exception &/*_e*/)
      {
        gzerr << "Unable to load Ogre Resources. Make sure the resources "
            "path in the world file is set correctly." << std::endl;
      }
    }
  }
}

//////////////////////////////////////////////////
void Ogre2RenderEngine::CreateRenderWindow()
{
  // create dummy window
  std::string handle;

  struct SDLx11
  {
    void *display;    // The X11 display
    uint64_t window;  // The X11 window
  };

  SDLx11 vulkanX11Data;

  if (this->dataPtr->graphicsAPI == GraphicsAPI::VULKAN)
  {
    if(this->dummyWindowId && this->dummyDisplay)
    {
      vulkanX11Data.window = this->dummyWindowId;
      vulkanX11Data.display = this->dummyDisplay;
      handle = std::to_string((uintptr_t)&vulkanX11Data);
    }
  }
  else
  {
    handle = std::to_string(this->dummyWindowId);
  }

  auto res = this->CreateRenderWindow(handle, 1, 1, 1, 0);
  if (res.empty())
  {
    gzerr << "Failed to create dummy render window." << std::endl;
    gzerr << "Please see the troubleshooting page for possible fixes: "
          << "https://gazebosim.org/docs/fortress/troubleshooting"
          << std::endl;
  }
}

//////////////////////////////////////////////////
std::string Ogre2RenderEngine::CreateRenderWindow(const std::string &_handle,
    const unsigned int _width, const unsigned int _height,
    const double _ratio, const unsigned int _antiAliasing)
{
  Ogre::StringVector paramsVector;
  Ogre::NameValuePairList params;
  this->window = nullptr;

  if (this->dataPtr->graphicsAPI == GraphicsAPI::VULKAN)
  {
    if (!_handle.empty())
    {
      params["SDL2x11"] = _handle;
    }
  }
  else
  {
    // if use current gl then don't include window handle params
    if (!this->useCurrentGLContext)
    {
      // Mac and Windows *must* use externalWindow handle.
#if defined(__APPLE__) || defined(_MSC_VER)
      params["externalWindowHandle"] = _handle;
      params["hidden"] = "true";
#else
      params["parentWindowHandle"] = _handle;
#endif
    }
  }

  params["FSAA"] = std::to_string(_antiAliasing);
  params["stereoMode"] = "Frame Sequential";

  // TODO(anyone): determine api without qt

#if defined(__APPLE__)
  // Set the macAPI for Ogre based on the Qt implementation
  params["macAPI"] = "cocoa";
  params["macAPICocoaUseNSView"] = "true";
#endif

  // Hide window if dimensions are less than or equal to one.
  params["border"] = "none";

  std::ostringstream stream;
  stream << "OgreWindow(0)" << "_" << _handle;

  // Needed for retina displays
  params["contentScalingFactor"] = std::to_string(_ratio);

  // Ogre 2 PBS expects gamma correction
  params["gamma"] = "Yes";

#ifdef OGRE_BUILD_RENDERSYSTEM_VULKAN
  if (this->dataPtr->vkExternalDevice.device)
  {
    params.insert({ "windowType", "null" });
    params.insert({ "surface_less", "Yes" });
    params.insert(
      { "external_device", std::to_string(reinterpret_cast<uintptr_t>(
                             &this->dataPtr->vkExternalDevice)) });
  }
#endif

  if (this->useCurrentGLContext)
  {
    params["externalGLControl"] = "true";
    params["currentGLContext"] = "true";

#if defined(_MSC_VER)
    if (!this->winID.empty())
    {
      params["externalWindowHandle"] = this->winID;
    }
    else
    {
      gzerr << "useCurrentGLContext option specified, "
            << "but no winID specified." << std::endl
            << "On Windows if useCurrentGLContext is specified, "
            << "the ogre2 rendering requires that also winID is specified."
            << std::endl;
      return std::string();
    }
#endif
  }

#if !defined(__APPLE__) && !defined(_MSC_VER)
  if (!this->winID.empty())
  {
    params["parentWindowHandle"] = this->winID;
  }
#endif

  int attempts = 0;
  while (this->window == nullptr && (attempts++) < 10)
  {
    try
    {
      this->window = Ogre::Root::getSingleton().createRenderWindow(
          stream.str(), _width, _height, false, &params);
    }
    catch(const std::exception &_e)
    {
      gzerr << " Unable to create the rendering window: " << _e.what()
             << std::endl;
      this->window = nullptr;
    }
  }

  if (attempts >= 10)
  {
    gzerr << "Unable to create the rendering window after [" << attempts
           << "] attempts." << std::endl;
    return std::string();
  }

  this->RegisterHlms();

  if (this->window)
  {
    this->window->_setVisible(true);

    // Windows needs to reposition the render window to 0,0.
    this->window->reposition(0, 0);
  }
  return stream.str();
}

//////////////////////////////////////////////////
GraphicsAPI Ogre2RenderEngine::GraphicsAPI() const
{
  return this->dataPtr->graphicsAPI;
}

//////////////////////////////////////////////////
NativeWindowPtr Ogre2RenderEngine::CreateNativeWindow(
  const std::string &_winHandle, const uint32_t _width, const uint32_t _height,
  const double _ratio)
{
  Ogre::StringVector paramsVector;
  Ogre::NameValuePairList params;
  Ogre::Window *ogreWindow = nullptr;

  if (this->dataPtr->graphicsAPI == GraphicsAPI::VULKAN)
  {
    if (!_winHandle.empty())
    {
      params["SDL2x11"] = _winHandle;
    }
  }
  else
  {
    if (!_winHandle.empty())
    {
      // Mac and Windows *must* use externalWindow handle.
#if defined(__APPLE__) || defined(_MSC_VER)
      params["externalWindowHandle"] = _winHandle;
#else
      params["parentWindowHandle"] = _winHandle;
#endif
    }
  }

  if (this->dataPtr->graphicsAPI == GraphicsAPI::OPENGL)
  {
#ifdef _WIN32
    params["externalGLContext"] =
      std::to_string((size_t)wglGetCurrentContext());
#elif defined(__APPLE__)
    params["externalGLContext"] =
      std::to_string((size_t)CGLGetCurrentContext());
#else
    params["currentGLContext"] = "true";
#endif

    // All GL windows must match the same params (i.e. FSAA and pixel format)
    this->window->getSampleDescription().getFsaaDesc(params["FSAA"]);
  }

  std::ostringstream stream;
  stream << "OgreNativeWindow(0)" << "_" << _winHandle;

  // Needed for retina displays
  params["contentScalingFactor"] = std::to_string(_ratio);

  // Ogre 2 PBS expects gamma correction
  params["gamma"] = "Yes";

  int attempts = 0;
  while (ogreWindow == nullptr && (attempts++) < 10)
  {
    try
    {
      ogreWindow = Ogre::Root::getSingleton().createRenderWindow(
          stream.str(), _width, _height, false, &params);
    }
    catch(const std::exception &_e)
    {
      gzerr << " Unable to create the rendering window: " << _e.what()
            << std::endl;
      ogreWindow = nullptr;
    }
  }

  if (attempts >= 10)
  {
    gzerr << "Unable to create the rendering window after [" << attempts
          << "] attempts." << std::endl;
    return NativeWindowPtr(nullptr);
  }

  if (ogreWindow)
  {
    // Windows needs to reposition the render window to 0,0.
    ogreWindow->reposition(0, 0);
  }
  return NativeWindowPtr(new Ogre2NativeWindow(ogreWindow));
}

//////////////////////////////////////////////////
void Ogre2RenderEngine::InitAttempt()
{
  this->initialized = false;

  // init the resources
  Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(false);

  this->scenes = Ogre2SceneStorePtr(new Ogre2SceneStore);
}

/////////////////////////////////////////////////
std::vector<unsigned int> Ogre2RenderEngine::FSAALevels() const
{
  return this->dataPtr->fsaaLevels;
}

/////////////////////////////////////////////////
Ogre2GzHlmsSphericalClipMinDistance& Ogre2RenderEngine::
SphericalClipMinDistance()
{
  return this->dataPtr->sphericalClipMinDistance;
}

#ifdef GZ_RENDERING_HAVE_OGRE2_OVERLAY
/////////////////////////////////////////////////
Ogre::v1::OverlaySystem *Ogre2RenderEngine::OverlaySystem() const
{
  return this->ogreOverlaySystem;
}
#endif

/////////////////////////////////////////////////
void Ogre2RenderEngine::SetGzOgreRenderingMode(
  GzOgreRenderingMode renderingMode)
{
  this->dataPtr->gzHlmsPbs->gzOgreRenderingMode = renderingMode;
  this->dataPtr->gzHlmsUnlit->gzOgreRenderingMode = renderingMode;
  this->dataPtr->gzHlmsTerra->gzOgreRenderingMode = renderingMode;
}

/////////////////////////////////////////////////
Ogre::HlmsPbsTerraShadows *Ogre2RenderEngine::HlmsPbsTerraShadows() const
{
  return this->dataPtr->hlmsPbsTerraShadows.get();
}

/////////////////////////////////////////////////
Ogre::CompositorWorkspaceListener *Ogre2RenderEngine::TerraWorkspaceListener()
  const
{
  return this->dataPtr->terraWorkspaceListener.get();
}

//////////////////////////////////////////////////
Ogre2RenderEngine *Ogre2RenderEngine::Instance()
{
  return gz::common::SingletonT<Ogre2RenderEngine>::Instance();
}

// Register this plugin
GZ_ADD_PLUGIN(rendering::Ogre2RenderEnginePlugin,
              rendering::RenderEnginePlugin)
GZ_ADD_PLUGIN_ALIAS(rendering::Ogre2RenderEnginePlugin,
                    "gz::rendering::ogre2::Plugin")
