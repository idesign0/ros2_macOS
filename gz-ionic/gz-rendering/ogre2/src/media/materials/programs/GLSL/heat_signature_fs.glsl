/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

// The input texture, which is set up by the Ogre Compositor infrastructure.
vulkan_layout( ogre_t0 ) uniform texture2D RT;
vulkan( layout( ogre_s0 ) uniform sampler texSampler );

// input params from vertex shader
vulkan_layout( location = 0 )
in block
{
  vec2 uv0;
} inPs;

// final output color
vulkan_layout( location = 0 )
out vec4 fragColor;

vulkan( layout( ogre_P0 ) uniform Params { )
  // The minimum and maximum temprature values (in Kelvin) that the
  // heat signature texture should be normalized to
  // (users can override these defaults)
  uniform float minTemp;
  uniform float maxTemp;

  uniform int bitDepth;
  uniform float resolution;
vulkan( }; )

// map a temperature from the [min, max] range to the user defined
// [minTemp, maxTemp] range
float mapNormalized(float num)
{
  float mappedKelvin = ((maxTemp - minTemp) * num) + minTemp;
  return mappedKelvin / (((1 << bitDepth) - 1.0) * resolution);
}

void main()
{
  float heat = texture(vkSampler2D(RT,texSampler), inPs.uv0.xy).x;

  // set g, b, a to 0. This will be used by thermal_camera_fs.glsl to determine
  // if a particular fragment is a heat source or not
  fragColor = vec4(mapNormalized(heat), 0, 0, 0.0);
}
