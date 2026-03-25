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

#version ogre_glsl_ver_330

vulkan_layout( OGRE_POSITION ) in vec4 vertex;

vulkan( layout( ogre_P0 ) uniform Params { )
  // aspect ratio
  uniform float ratio;
  uniform mat4 worldViewProj;
vulkan( }; )

vulkan_layout( location = 0 )
out block
{
  vec2 fragPos;
} outVs;

void main()
{
  gl_Position = worldViewProj * vertex;

  // get normalized fragment coordinate (3D to 2D window space transformation)
  outVs.fragPos = gl_Position.xy/gl_Position.w*vec2(-1.0,-1.0/ratio);
}
