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

vulkan_layout( OGRE_POSITION ) in vec4 vertex;
vulkan_layout( OGRE_NORMAL ) in vec3 normal;

vulkan( layout( ogre_P0 ) uniform Params { )
  uniform mat4 worldViewProj;
  uniform float size;
vulkan( }; )

vulkan_layout( location = 0 )
out block
{
  vec3 ptColor;
} outVs;

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
};

void main()
{
  // Calculate output position
  gl_Position = worldViewProj * vertex;
  gl_PointSize = size;
  // We're abusing the normal variable to hold per-point colors
  outVs.ptColor = normal;
}
