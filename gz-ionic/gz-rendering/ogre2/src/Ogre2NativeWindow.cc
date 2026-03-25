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

#include "gz/rendering/ogre2/Ogre2NativeWindow.hh"

#include "gz/rendering/ogre2/Ogre2Camera.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2RenderTarget.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif
#include <Compositor/OgreCompositorWorkspace.h>
#include <OgreRoot.h>
#include <OgreWindow.h>
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

class GZ_RENDERING_OGRE2_HIDDEN gz::rendering::Ogre2NativeWindowPrivate
{
  // clang-format off
  /// \brief The native Ogre window handle
  public: Ogre::Window *window{ nullptr };

  /// \brief The workspace used by Ogre2NativeWindow::Draw
  public: Ogre::CompositorWorkspace *workspace{ nullptr };
  // clang-format on
};

using namespace gz;
using namespace rendering;

static const char *kWorkspaceName = "NativeWindow Copy";

//////////////////////////////////////////////////
Ogre2NativeWindow::Ogre2NativeWindow(Ogre::Window *_window) :
  dataPtr(new Ogre2NativeWindowPrivate)
{
  this->dataPtr->window = _window;

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  if (!ogreCompMgr->hasWorkspaceDefinition(kWorkspaceName))
  {
    Ogre::CompositorNodeDef *nodeDef =
      ogreCompMgr->addNodeDefinition("Native Window Copy Node");
    // Input texture
    nodeDef->addTextureSourceName("rt_window", 0,
                                  Ogre::TextureDefinitionBase::TEXTURE_INPUT);
    nodeDef->addTextureSourceName("rt_input", 1,
                                  Ogre::TextureDefinitionBase::TEXTURE_INPUT);
    nodeDef->setNumTargetPass(1);

    Ogre::CompositorTargetDef *targetDef = nodeDef->addTargetPass("rt_window");
    targetDef->setNumPasses(1u);
    {
      {
        auto *passQuad = static_cast<Ogre::CompositorPassQuadDef *>(
          targetDef->addPass(Ogre::PASS_QUAD));

        passQuad->mMaterialName = "Ogre/Copy/4xFP32";
        passQuad->addQuadTextureSource(0u, "rt_input");

        passQuad->setAllLoadActions(Ogre::LoadAction::DontCare);
        passQuad->setAllStoreActions(Ogre::StoreAction::DontCare);
        passQuad->mStoreActionColour[0] = Ogre::StoreAction::StoreOrResolve;
      }
    }

    Ogre::CompositorWorkspaceDef *workspaceDef =
      ogreCompMgr->addWorkspaceDefinition(kWorkspaceName);
    workspaceDef->connectExternal(0, nodeDef->getName(), 0);
    workspaceDef->connectExternal(1, nodeDef->getName(), 1);
  }
}

//////////////////////////////////////////////////
Ogre2NativeWindow::~Ogre2NativeWindow()
{
  if (this->dataPtr->workspace)
  {
    auto ogreCompMgr = this->dataPtr->workspace->getCompositorManager();
    ogreCompMgr->removeWorkspace(this->dataPtr->workspace);
    this->dataPtr->workspace = nullptr;
  }
  if (this->dataPtr->window)
  {
    // todo(anyone) the call below causes a crash with the error message:
    // "pure virtual method called"
    Ogre::Root::getSingleton().getRenderSystem()->destroyRenderWindow(
      this->dataPtr->window);
    this->dataPtr->window = nullptr;
  }
}

//////////////////////////////////////////////////
void Ogre2NativeWindow::NotifyFocused(bool _focused)
{
  this->dataPtr->window->setFocused(_focused);
}

//////////////////////////////////////////////////
void Ogre2NativeWindow::NotifyVisible(bool _visible)
{
  this->dataPtr->window->_setVisible(_visible);
}

//////////////////////////////////////////////////
void Ogre2NativeWindow::NotifyWindowMovedOrResized()
{
  this->dataPtr->window->windowMovedOrResized();
}

//////////////////////////////////////////////////
void Ogre2NativeWindow::RequestResolution(uint32_t _width, uint32_t _height)
{
  this->dataPtr->window->requestResolution(_width, _height);
}

//////////////////////////////////////////////////
void Ogre2NativeWindow::Draw(CameraPtr _camera)
{
  Ogre2Camera *camera = dynamic_cast<Ogre2Camera *>(_camera.get());

  if (!camera)
  {
    gzwarn << "Using Camera from a different RenderEngine with the wrong "
              "NativeWindow\n"
              "Display may be wrong\n";
    return;
  }

  Ogre2RenderTargetPtr renderTarget = camera->renderTexture;

  Ogre2Scene *scene = dynamic_cast<Ogre2Scene *>(camera->Scene().get());

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  Ogre::TextureGpu *texture = renderTarget->RenderTarget();
  if (!this->dataPtr->workspace ||
      this->dataPtr->workspace->getExternalRenderTargets()[1] != texture)
  {
    if (this->dataPtr->workspace)
    {
      ogreCompMgr->removeWorkspace(this->dataPtr->workspace);
      this->dataPtr->workspace = nullptr;
    }

    Ogre::CompositorChannelVec channels{ this->dataPtr->window->getTexture(),
                                         texture };

    this->dataPtr->workspace =
      ogreCompMgr->addWorkspace(scene->OgreSceneManager(), channels,
                                camera->OgreCamera(), kWorkspaceName, false);

    // This is a bit of a hack. You're not really supposed to draw
    // to a window by hand. Vulkan needs
    // CompositorManager2::prepareRenderWindowsForPresent to be called,
    // but this won't happen because the workspace is disabled
    //
    // So the first frame performs this hack to ensure the Vulkan
    // workspace is setup to prepare the window for present
    this->dataPtr->workspace->setEnabled(true);
    ogreCompMgr->_update();
    this->dataPtr->workspace->setEnabled(false);
    scene->FlushGpuCommandsAndStartNewFrame(1u, true);
  }
  else
  {
    this->dataPtr->workspace->_validateFinalTarget();
    this->dataPtr->workspace->_beginUpdate(false);
    this->dataPtr->workspace->_update();
    this->dataPtr->workspace->_endUpdate(false);

    Ogre::vector<Ogre::TextureGpu *>::type swappedTargets;
    swappedTargets.reserve(2u);
    this->dataPtr->workspace->_swapFinalTarget(swappedTargets);

    scene->FlushGpuCommandsAndStartNewFrame(1u, true);
  }
}
