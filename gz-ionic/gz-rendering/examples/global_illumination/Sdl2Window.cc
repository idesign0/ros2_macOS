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

#include <mutex>

#include <gz/common/Console.hh>
#include <gz/rendering/Camera.hh>
#include <gz/rendering/GlobalIlluminationCiVct.hh>
#include <gz/rendering/GlobalIlluminationVct.hh>
#include <gz/rendering/Image.hh>
#include <gz/rendering/NativeWindow.hh>
#include <gz/rendering/OrbitViewController.hh>
#include <gz/rendering/RayQuery.hh>
#include <gz/rendering/RenderEngine.hh>
#include <gz/rendering/RenderPass.hh>
#include <gz/rendering/Scene.hh>

#include "GiConfig.hh"
#include "Sdl2Window.hh"

#include <SDL.h>
#include <SDL_syswm.h>

//////////////////////////////////////////////////
static unsigned int imgw = 0;
static unsigned int imgh = 0;

static std::vector<ir::CameraPtr> g_cameras;
static ir::CameraPtr g_camera;
static ir::CameraPtr g_currCamera;
static unsigned int g_cameraIndex = 0;

static SDL_Window *g_sdlWindow = nullptr;
static gz::rendering::NativeWindowPtr g_nativeWindow;

// view control variables
static ir::RayQueryPtr g_rayQuery;
static ir::OrbitViewController g_viewControl;
static ir::RayQueryResult g_target;
struct mouseButton
{
  int button = 0;
  int state = 0;
  int x = 0;
  int y = 0;
  int motionX = 0;
  int motionY = 0;
  int motionWheel = 0;
  int dragX = 0;
  int dragY = 0;
  int scroll = 0;
  bool buttonDirty = false;
  bool motionDirty = false;
};
static struct mouseButton g_mouse;

//////////////////////////////////////////////////
void mousePressed(const SDL_MouseButtonEvent &_arg)
{
  // ignore unknown mouse button numbers
  if (_arg.button >= 5)
    return;

  g_mouse.button = _arg.button;
  g_mouse.state = _arg.state;
  g_mouse.x = _arg.x;
  g_mouse.y = _arg.y;
  g_mouse.buttonDirty = true;
}

//////////////////////////////////////////////////
void mouseReleased(const SDL_MouseButtonEvent &_arg)
{
  // ignore unknown mouse button numbers
  if (_arg.button >= 5)
    return;

  g_mouse.button = _arg.button;
  g_mouse.state = _arg.state;
  g_mouse.x = _arg.x;
  g_mouse.y = _arg.y;
  g_mouse.buttonDirty = true;
}

//////////////////////////////////////////////////
void mouseMoved(const SDL_Event &_arg)
{
  g_mouse.motionX = _arg.motion.x;
  g_mouse.motionY = _arg.motion.y;

  if (!g_mouse.motionDirty)
  {
    g_mouse.dragX = 0;
    g_mouse.dragY = 0;
  }
  g_mouse.dragX += _arg.motion.xrel;
  g_mouse.dragY += _arg.motion.yrel;
  g_mouse.motionDirty = true;
}

//////////////////////////////////////////////////
void mouseWheel(const SDL_Event &_arg)
{
  g_mouse.motionWheel = -_arg.wheel.y;
}

//////////////////////////////////////////////////
void handleMouse()
{
  // only ogre supports ray query for now so use
  // ogre camera located at camera index = 0.
  ir::CameraPtr rayCamera = g_cameras[0];
  if (!g_rayQuery)
  {
    g_rayQuery = rayCamera->Scene()->CreateRayQuery();
    if (!g_rayQuery)
    {
      gzerr << "Failed to create Ray Query" << std::endl;
      return;
    }
  }
  if (g_mouse.buttonDirty)
  {
    g_mouse.buttonDirty = false;

    // test mouse picking
    if (g_mouse.button == SDL_BUTTON_LEFT && g_mouse.state == SDL_PRESSED)
    {
      // Get visual using Selection Buffer from Camera
      ir::VisualPtr visual;
      gz::math::Vector2i mousePos(g_mouse.x, g_mouse.y);
      visual = rayCamera->VisualAt(mousePos);
      if (visual)
      {
        std::cout << "Selected visual at position: "        //
                  << g_mouse.x << " " << g_mouse.y << ": "  //
                  << visual->Name() << "\n";
      }
      else
      {
        std::cout << "No visual found at position: "  //
                  << g_mouse.x << " " << g_mouse.y << std::endl;
      }
    }

    // camera orbit
    double nx =
      2.0 * g_mouse.x / static_cast<double>(rayCamera->ImageWidth()) - 1.0;
    double ny =
      1.0 - 2.0 * g_mouse.y / static_cast<double>(rayCamera->ImageHeight());
    g_rayQuery->SetFromCamera(rayCamera, gz::math::Vector2d(nx, ny));
    g_target = g_rayQuery->ClosestPoint();
    if (!g_target)
    {
      // set point to be 10m away if no intersection found
      g_target.point = g_rayQuery->Origin() + g_rayQuery->Direction() * 10;
      return;
    }
  }

  // mouse wheel scroll zoom
  if (g_mouse.motionWheel != 0)
  {
    double scroll = g_mouse.motionWheel;
    double distance = rayCamera->WorldPosition().Distance(g_target.point);
    int factor = 1;
    double amount = -(scroll * factor) * (distance / 5.0);
    for (ir::CameraPtr camera : g_cameras)
    {
      g_viewControl.SetCamera(camera);
      g_viewControl.SetTarget(g_target.point);
      g_viewControl.Zoom(amount);
    }

    g_mouse.motionWheel = 0;
  }

  if (g_mouse.motionDirty)
  {
    g_mouse.motionDirty = false;
    auto drag = gz::math::Vector2d(g_mouse.dragX, g_mouse.dragY);

    // left mouse button pan
    if (g_mouse.button == SDL_BUTTON_LEFT && g_mouse.state == SDL_PRESSED)
    {
      for (ir::CameraPtr camera : g_cameras)
      {
        g_viewControl.SetCamera(camera);
        g_viewControl.SetTarget(g_target.point);
        g_viewControl.Pan(drag);
      }
    }
    else if (g_mouse.button == SDL_BUTTON_MIDDLE &&
             g_mouse.state == SDL_PRESSED)
    {
      for (ir::CameraPtr camera : g_cameras)
      {
        g_viewControl.SetCamera(camera);
        g_viewControl.SetTarget(g_target.point);
        g_viewControl.Orbit(drag);
      }
    }
    // right mouse button zoom
    else if (g_mouse.button == SDL_BUTTON_RIGHT && g_mouse.state == SDL_PRESSED)
    {
      double hfov = rayCamera->HFOV().Radian();
      double vfov = 2.0f * atan(tan(hfov / 2.0f) / rayCamera->AspectRatio());
      double distance = rayCamera->WorldPosition().Distance(g_target.point);
      double amount =
        ((-g_mouse.dragY / static_cast<double>(rayCamera->ImageHeight())) *
         distance * tan(vfov / 2.0) * 6.0);
      for (ir::CameraPtr camera : g_cameras)
      {
        g_viewControl.SetCamera(camera);
        g_viewControl.SetTarget(g_target.point);
        g_viewControl.Zoom(amount);
      }
    }
  }
}

//////////////////////////////////////////////////
void displayCB()
{
#if 0
  {
    // Example on how to download the image from GPU to CPU and access its data
    ir::Image image = g_camera->CreateImage();
    ir::ImagePtr g_image = std::make_shared<ir::Image>(image);
    g_cameras[g_cameraIndex]->Capture(*g_image);
    unsigned char *data = g_image->Data<unsigned char>();
  }
#endif
  g_cameras[g_cameraIndex]->Update();
  g_nativeWindow->Draw(g_cameras[g_cameraIndex]);
  handleMouse();
}

//////////////////////////////////////////////////
void keyPressed(const SDL_KeyboardEvent & /*_arg*/)
{
}

//////////////////////////////////////////////////
bool keyReleased(const SDL_KeyboardEvent &_arg)
{
  if (_arg.keysym.sym == SDLK_ESCAPE || _arg.keysym.sym == SDLK_q)
  {
    // return false to let the main loop know to quit
    return false;
  }
  else if (_arg.keysym.sym == SDLK_TAB)
  {
    g_cameraIndex = (g_cameraIndex + 1) % g_cameras.size();
  }
  else if (_arg.keysym.sym == SDLK_p)
  {
    // toggle all render passes
    for (ir::CameraPtr camera : g_cameras)
    {
      for (unsigned int i = 0; i < camera->RenderPassCount(); ++i)
      {
        ir::RenderPassPtr pass = camera->RenderPassByIndex(i);
        pass->SetEnabled(!pass->IsEnabled());
      }
    }
  }
  else if (_arg.keysym.sym == SDLK_s)
  {
    // toggle sky
    for (ir::CameraPtr camera : g_cameras)
    {
      camera->Scene()->SetSkyEnabled(!camera->Scene()->SkyEnabled());
    }
  }
  else if (_arg.keysym.sym == SDLK_d)
  {
    if (g_gi)
    {
#if GI_METHOD == 1
      if (g_gi->DebugVisualization() ==
          gz::rendering::GlobalIlluminationVct::DVM_None)
      {
        g_gi->SetDebugVisualization(
          gz::rendering::GlobalIlluminationVct::DVM_Lighting);
      }
      else
      {
        g_gi->SetDebugVisualization(
          gz::rendering::GlobalIlluminationVct::DVM_None);
      }
#else
      if (g_gi->DebugVisualization() ==
          gz::rendering::GlobalIlluminationCiVct::DVM_None)
      {
        g_gi->SetDebugVisualization(
          gz::rendering::GlobalIlluminationCiVct::DVM_Lighting);
      }
      else
      {
        g_gi->SetDebugVisualization(
          gz::rendering::GlobalIlluminationCiVct::DVM_None);
      }
#endif
    }
  }
  return true;
}

//////////////////////////////////////////////////
void initCamera(ir::CameraPtr _camera)
{
  g_camera = _camera;
  imgw = g_camera->ImageWidth();
  imgh = g_camera->ImageHeight();
}

//////////////////////////////////////////////////
void printUsage()
{
  std::cout << "===============================" << std::endl;
  std::cout << "  TAB - Switch render engines  " << std::endl;
  std::cout << "  ESC - Exit                   " << std::endl;
  std::cout << "  P   - Toggle render pass     " << std::endl;
  std::cout << "  S   - Toggle skybox          " << std::endl;
  std::cout << "===============================" << std::endl;
}

void handleWindowEvent(const SDL_Event &evt)
{
  switch (evt.window.event)
  {
  case SDL_WINDOWEVENT_SIZE_CHANGED:
    int w, h;
    SDL_GetWindowSize(g_sdlWindow, &w, &h);
#ifdef __LINUX__
    g_nativeWindow->RequestResolution(static_cast<uint32_t>(w),
                                      static_cast<uint32_t>(h));
#endif
    g_nativeWindow->NotifyWindowMovedOrResized();
    break;
  case SDL_WINDOWEVENT_RESIZED:
#ifdef __LINUX__
    g_nativeWindow->RequestResolution(static_cast<uint32_t>(evt.window.data1),
                                      static_cast<uint32_t>(evt.window.data2));
#endif
    g_nativeWindow->NotifyWindowMovedOrResized();
    break;
  case SDL_WINDOWEVENT_SHOWN:
    g_nativeWindow->NotifyVisible(true);
    break;
  case SDL_WINDOWEVENT_HIDDEN:
    g_nativeWindow->NotifyVisible(false);
    break;
  case SDL_WINDOWEVENT_FOCUS_GAINED:
    g_nativeWindow->NotifyFocused(true);
    break;
  case SDL_WINDOWEVENT_FOCUS_LOST:
    g_nativeWindow->NotifyFocused(false);
    break;
  }
}

//////////////////////////////////////////////////
void run(std::vector<ir::CameraPtr> _cameras)
{
  if (_cameras.empty())
  {
    gzerr << "No cameras found. Scene will not be rendered" << std::endl;
    return;
  }

  g_sdlWindow = SDL_CreateWindow("Global Illumination Demo",  // window title
                                 0,              // initial x position
                                 0,              // initial y position
                                 1280,           // width, in pixels
                                 720,            // height, in pixels
                                 SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

  auto renderEngine = _cameras.back()->Scene()->Engine();

  std::string winHandle;
  // Get the native whnd
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version)

  if (SDL_GetWindowWMInfo(g_sdlWindow, &wmInfo) == SDL_FALSE)
  {
    std::cout << "Couldn't get WM Info! (SDL2)" << std::endl;
    abort();
  }

  switch (wmInfo.subsystem)
  {
#ifdef WIN32
  case SDL_SYSWM_WINDOWS:
    // Windows code
    winHandle = std::to_string((uintptr_t)wmInfo.info.win.window);
    break;
#elifdef __MACOSX__
  case SDL_SYSWM_COCOA:
    winHandle = std::to_string(WindowContentViewHandle(wmInfo));
    break;
#else
  case SDL_SYSWM_X11:
    if (renderEngine->GraphicsAPI() != gz::rendering::GraphicsAPI::VULKAN)
    {
      winHandle = std::to_string((uintptr_t)wmInfo.info.x11.window);
    }
    else
    {
      winHandle = std::to_string((uintptr_t)&wmInfo.info.x11);
    }
    break;
#endif
  default:
    std::cout << "Unexpected WM Info! (SDL2)" << std::endl;
    abort();
  }

  g_nativeWindow =
    renderEngine->CreateNativeWindow(winHandle, 1280u, 720u, 1.0);

  g_cameras = _cameras;
  initCamera(_cameras[0]);
  printUsage();

  bool bQuit = false;

  while (!bQuit)
  {
    SDL_Event evt;
    while (SDL_PollEvent(&evt))
    {
      switch (evt.type)
      {
      case SDL_WINDOWEVENT:
        handleWindowEvent(evt);
        break;
      case SDL_QUIT:
        bQuit = true;
        break;
      case SDL_MOUSEMOTION:
        mouseMoved(evt);
        break;
      case SDL_MOUSEWHEEL:
        mouseWheel(evt);
        break;
      case SDL_MOUSEBUTTONDOWN:
        mousePressed(evt.button);
        break;
      case SDL_MOUSEBUTTONUP:
        mouseReleased(evt.button);
        break;
      case SDL_KEYDOWN:
        if (!evt.key.repeat)
        {
          keyPressed(evt.key);
        }
        break;
      case SDL_KEYUP:
        if (!evt.key.repeat)
        {
          bQuit = !keyReleased(evt.key);
        }
        break;
      default:
        break;
      }
    }

    displayCB();
  }

  // Destroy window before RenderEngine deinitializes
  g_nativeWindow = nullptr;

  if (g_sdlWindow)
  {
    SDL_DestroyWindow(g_sdlWindow);
    g_sdlWindow = nullptr;
  }
}
