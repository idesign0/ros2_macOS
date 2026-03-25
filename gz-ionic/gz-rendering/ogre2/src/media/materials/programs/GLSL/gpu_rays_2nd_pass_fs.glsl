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

#version ogre_glsl_ver_330

vulkan_layout( location = 0 )
in block
{
  vec2 uv0;
} inPs;

// cubeUVTex packs information needed to sample from tex0-5
vulkan_layout( ogre_t0 ) uniform texture2D cubeUVTex;

// cubemap is constructed using z-up, x-forward, y-left
// index: face   axis
//     0: right  -y
//     1: left   +y
//     2: top    +z
//     3: bottom -z
//     4: front  +x
//     5: back   -x

// cube face 0 -y
vulkan_layout( ogre_t1 ) uniform texture2D tex0;

// cube face 1 +y
vulkan_layout( ogre_t2 ) uniform texture2D tex1;

// cube face 2 +z
vulkan_layout( ogre_t3 ) uniform texture2D tex2;

// cube face 3 -z
vulkan_layout( ogre_t4 ) uniform texture2D tex3;

// cube face 4 +x
vulkan_layout( ogre_t5 ) uniform texture2D tex4;

// cube face 5 -x
vulkan_layout( ogre_t6 ) uniform texture2D tex5;

vulkan( layout( ogre_s0 ) uniform sampler texSampler );

vulkan_layout( location = 0 )
out vec4 fragColor;

vec2 getRange(vec2 uv, texture2D tex)
{
  vec2 range = texture(vkSampler2D(tex,texSampler), uv).xy;
  return range;
}

void main()
{
  // get face index and uv coorodate data
  vec3 data = texture(vkSampler2D(cubeUVTex,texSampler), inPs.uv0).xyz;

  // which face to sample range data from
  float faceIdx = data.z;

  // uv coordinates on texture that stores the range data
  vec2 uv = data.xy;

  vec2 d;
  d.x = 0;
  d.y = 0;
  if (faceIdx == 0)
    d = getRange(uv, tex0);
  else if (faceIdx == 1)
    d = getRange(uv, tex1);
  else if (faceIdx == 2)
    d = getRange(uv, tex2);
  else if (faceIdx == 3)
    d = getRange(uv, tex3);
  else if (faceIdx == 4)
    d = getRange(uv, tex4);
  else if (faceIdx == 5)
    d = getRange(uv, tex5);

  float range = d.x;
  float retro = d.y;

  fragColor = vec4(range, retro, 0, 1.0);
  return;
}
