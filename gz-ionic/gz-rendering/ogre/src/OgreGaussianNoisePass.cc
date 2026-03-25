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

#include "gz/rendering/RenderPassSystem.hh"
#include "gz/rendering/ogre/OgreIncludes.hh"
#include "gz/rendering/ogre/OgreGaussianNoisePass.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    // We'll create an instance of this class for each camera, to be used to
    // inject random values on each render call.
    class GaussianNoiseCompositorListener
      : public Ogre::CompositorInstance::Listener
    {
      /// \brief Constructor, setting mean and standard deviation.
      public: GaussianNoiseCompositorListener(const double &_mean,
                                              const double &_stddev):
          mean(_mean), stddev(_stddev) {}

      /// \brief Callback that OGRE will invoke for us on each render call
      /// \param[in] _passID OGRE material pass ID.
      /// \param[in] _mat Pointer to OGRE material.
      public: virtual void notifyMaterialRender(unsigned int _passId,
                                                Ogre::MaterialPtr &_mat)
      {
#if OGRE_VERSION_LT_1_11_0
        GZ_ASSERT(!_mat.isNull(), "Null OGRE material");
#else
        GZ_ASSERT(_mat, "Null OGRE material");
#endif
        // modify material here (wont alter the base material!), called for
        // every drawn geometry instance (i.e. compositor render_quad)

        // Sample three values within the range [0,1.0] and set them for use in
        // the fragment shader, which will interpret them as offsets from (0,0)
        // to use when computing pseudo-random values.
        Ogre::Vector3 offsets(gz::math::Rand::DblUniform(0.0, 1.0),
                              gz::math::Rand::DblUniform(0.0, 1.0),
                              gz::math::Rand::DblUniform(0.0, 1.0));
        // These calls are setting parameters that are declared in two places:
        // 1. media/materials/scripts/gaussian_noise.material, in
        //    fragment_program GaussianNoiseFS
        // 2. media/materials/scripts/gaussian_noise_fs.glsl
        Ogre::Technique *technique = _mat->getTechnique(0);
        GZ_ASSERT(technique, "Null OGRE material technique");
        Ogre::Pass *pass = technique->getPass(_passId);
        GZ_ASSERT(pass, "Null OGRE material pass");
        Ogre::GpuProgramParametersSharedPtr params =
            pass->getFragmentProgramParameters();
#if OGRE_VERSION_LT_1_11_0
        GZ_ASSERT(!params.isNull(), "Null OGRE material GPU parameters");
#else
        GZ_ASSERT(params, "Null OGRE material GPU parameters");
#endif
        params->setNamedConstant("offsets", offsets);
        params->setNamedConstant("mean", static_cast<Ogre::Real>(this->mean));
        params->setNamedConstant("stddev",
            static_cast<Ogre::Real>(this->stddev));
      }

      /// \brief Mean that we'll pass down to the GLSL fragment shader.
      private: const double &mean;
      /// \brief Standard deviation that we'll pass down to the GLSL fragment
      /// shader.
      private: const double &stddev;
    };
    }
  }
}

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
OgreGaussianNoisePass::OgreGaussianNoisePass()
{
}

//////////////////////////////////////////////////
OgreGaussianNoisePass::~OgreGaussianNoisePass()
{
}

//////////////////////////////////////////////////
void OgreGaussianNoisePass::PreRender()
{
  if (!this->gaussianNoiseInstance[0])
    return;

  for (size_t i = 0u; i < kMaxOgreRenderPassCameras; ++i)
  {
    if (this->gaussianNoiseInstance[i] &&
        this->enabled != this->gaussianNoiseInstance[i]->getEnabled())
    {
      this->gaussianNoiseInstance[i]->setEnabled(this->enabled);
    }
  }
}

//////////////////////////////////////////////////
void OgreGaussianNoisePass::CreateRenderPass()
{
  if (!this->ogreCamera[0])
  {
    gzerr << "No camera set for applying Gaussian Noise Pass" << std::endl;
    return;
  }

  if (this->gaussianNoiseInstance[0] || this->gaussianNoiseCompositorListener)
  {
    gzerr << "Gaussian Noise pass already created. " << std::endl;
    return;
  }

  this->gaussianNoiseCompositorListener.reset(
    new GaussianNoiseCompositorListener(this->mean, this->stdDev));
  for (size_t i = 0u; i < kMaxOgreRenderPassCameras; ++i)
  {
    if (this->ogreCamera[i])
    {
      // create compositor instance
      this->gaussianNoiseInstance[i] =
        Ogre::CompositorManager::getSingleton().addCompositor(
          this->ogreCamera[i]->getViewport(), "RenderPass/GaussianNoise");
      this->gaussianNoiseInstance[i]->setEnabled(this->enabled);

      // add listener that injects random noise over time
      this->gaussianNoiseInstance[i]->addListener(
        this->gaussianNoiseCompositorListener.get());
    }
  }
}

//////////////////////////////////////////////////
void OgreGaussianNoisePass::Destroy()
{
  for (size_t i = 0u; i < kMaxOgreRenderPassCameras; ++i)
  {
    if (this->gaussianNoiseInstance[i])
    {
      this->gaussianNoiseInstance[i]->setEnabled(false);
      if (this->gaussianNoiseCompositorListener)
      {
        this->gaussianNoiseInstance[i]->removeListener(
          this->gaussianNoiseCompositorListener.get());
      }
      Ogre::CompositorManager::getSingleton().removeCompositor(
        this->ogreCamera[i]->getViewport(), "RenderPass/GaussianNoise");

      this->gaussianNoiseInstance[i] = nullptr;
    }
  }
  this->gaussianNoiseCompositorListener.reset();
}

GZ_RENDERING_REGISTER_RENDER_PASS(OgreGaussianNoisePass, GaussianNoisePass)
