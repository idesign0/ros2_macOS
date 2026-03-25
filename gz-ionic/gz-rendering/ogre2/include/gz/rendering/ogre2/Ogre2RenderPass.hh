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
#ifndef GZ_RENDERING_OGRE2_OGRE2RENDERPASS_HH_
#define GZ_RENDERING_OGRE2_OGRE2RENDERPASS_HH_

#include <memory>
#include <string>

#include "gz/rendering/base/BaseRenderPass.hh"
#include "gz/rendering/ogre2/Export.hh"
#include "gz/rendering/ogre2/Ogre2Object.hh"

namespace Ogre
{
  class CompositorWorkspace;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    // forward declaration
    class Ogre2RenderPassPrivate;

    /* \class Ogre2RenderPass Ogre2RenderPass.hh \
     * gz/rendering/ogre2/Ogre2RenderPass.hh
     */
    /// \brief Ogre2 Implementation of a render pass.
    ///
    /// The ogre2 compositor chain in gz-rendering is set up as follows:
    ///  * Base scene pass -> [0..N] RenderPass'es -> Final compositor pass.
    /// This is set up by Ogre2RenderTarget which loads the compositor workspace
    /// definition from script. The base scene pass node is responsible for
    /// rendering the initial scene and passes its output to any RenderPass'es
    /// that are added to the RenderTarget. Each RenderPass has its own ogre
    /// compositor node that receives the output from the previous RenderPass
    /// as input, applies its own pass over the input, and sends the result to
    /// the next RenderPass. Note that the Ogre2RenderPass class provides the
    /// node definition only and the actual node creation work is done in the
    /// Ogre2RenderTarget class when the whole workspace is constructed.
    class GZ_RENDERING_OGRE2_VISIBLE Ogre2RenderPass :
      public BaseRenderPass<Ogre2Object>
    {
      /// \brief Constructor
      protected: Ogre2RenderPass();

      /// \brief Destructor
      public: virtual ~Ogre2RenderPass();

      // Documentation inherited.
      public: void Destroy() override;

      /// \brief Get the ogre compositor node definition name for this
      /// render pass
      public: std::string OgreCompositorNodeDefinitionName() const;

      /// \brief Notifies Ogre2RenderPass that a new workspace has been created
      /// \param[in] _workspace workspace that was created
      public: virtual void WorkspaceAdded(
            Ogre::CompositorWorkspace *_workspace);

      /// \brief Notifies Ogre2RenderPass that a workspace will be destroyed
      /// \param[in] _workspace workspace about to be destroyed
      public: virtual void WorkspaceRemoved(
            Ogre::CompositorWorkspace *_workspace);

      /// \brief Create the render pass using ogre compositor
      public: virtual void CreateRenderPass();

      /// \brief Name of the ogre compositor node definition
      protected: std::string ogreCompositorNodeDefName;

      /// \brief Pointer to private data class
      private: std::unique_ptr<Ogre2RenderPassPrivate> dataPtr;
    };
    }
  }
}
#endif
