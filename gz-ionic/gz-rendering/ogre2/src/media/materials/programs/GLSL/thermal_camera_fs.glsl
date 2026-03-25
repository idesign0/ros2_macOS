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

#version ogre_glsl_ver_330

vulkan_layout( location = 0 )
in block
{
  vec2 uv0;
  vec3 cameraDir;
} inPs;

vulkan_layout( ogre_t0 ) uniform texture2D depthTexture;
vulkan_layout( ogre_t1 ) uniform texture2D colorTexture;

vulkan( layout( ogre_s0 ) uniform sampler texSampler );

vulkan_layout( location = 0 )
out vec4 fragColor;

vulkan( layout( ogre_P0 ) uniform Params { )
	uniform vec2 projectionParams;
	uniform float near;
	uniform float far;
	uniform float min;
	uniform float max;
	uniform float range;
	uniform float resolution;
	uniform float heatSourceTempRange;
	uniform float ambient;
	uniform int rgbToTemp;
	uniform int bitDepth;
vulkan( }; )

float getDepth(vec2 uv)
{
  float fDepth = texture(vkSampler2D(depthTexture,texSampler), uv).x;
  float linearDepth = projectionParams.y / (fDepth - projectionParams.x);
  return linearDepth;
}

void main()
{
  // temperature defaults to ambient
  float temp = ambient;
  float heatRange = range;
  int bitMaxValue = (1 << bitDepth) - 1;

  // dNorm value is between [0, 1]. It is used to for varying temperature
  // value of a fragment.
  // When dNorm = 1, temp = temp + heatRange*0.5.
  // When dNorm = 0, temp = temp - heatRange*0.5.
  float dNorm = 0.5;

  // check for heat source
  // heat source are objects with uniform temperature or heat signature
  // The custom heat source / signature shaders stores heat data in
  // a vec4 of [heat, 0, 0, 0] so we test to see if it is a heat
  // source by checking gba == 0. This is more of a hack but the idea is to
  // avoid having to render an extra pass to create a mask of heat source
  // objects
  vec4 rgba = texture(vkSampler2D(colorTexture, texSampler), inPs.uv0).rgba;
  bool isHeatSource = (rgba.g == 0.0 && rgba.b == 0.0 && rgba.a == 0.0);
  float heat = rgba.r;

  if (isHeatSource)
  {
    // heat is normalized so convert back to work in kelvin
    // for 16 bit camera and 0.01 resolution:
    //     ((1 << bitDepth) - 1) * resolution = 655.35
    temp = heat * bitMaxValue * resolution;

    // set temperature variation for heat source
    heatRange = heatSourceTempRange;
  }
  else
  {
    // other non-heat source objects are assigned ambient temperature
    temp = ambient;
  }

  // add temperature variation, either as a function of color or depth
  if (rgbToTemp == 1)
  {
    if (!isHeatSource)
    {
      // convert to grayscale: darker = warmer
      // (https://docs.opencv.org/3.4/de/d25/imgproc_color_conversions.html)
      float gray = rgba.r * 0.299 + rgba.g * 0.587 + rgba.b*0.114;
      dNorm = 1.0 - gray;
    }
  }
  else
  {
    // get depth
    float d = getDepth(inPs.uv0);
    // reconstruct 3d viewspace pos from depth
    vec3 viewSpacePos = inPs.cameraDir * d;
    d = -viewSpacePos.z;
    dNorm = (d-near) / (far-near);
  }

  // simulate temp variations
  float delta = (1.0 - dNorm) * heatRange;
  temp = temp - heatRange / 2.0 + delta;
  temp = clamp(temp, min, max);

  // apply resolution factor
  temp /= resolution;
  // normalize
  float denorm = float(bitMaxValue);
  temp /= denorm;

  fragColor = vec4(temp, 0, 0, 1.0);
}
