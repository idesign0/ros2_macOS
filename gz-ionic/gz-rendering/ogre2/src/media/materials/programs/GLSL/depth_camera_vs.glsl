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

vulkan_layout( OGRE_POSITION ) in vec4 vertex;
vulkan_layout( OGRE_NORMAL ) in vec3 normal;
vulkan_layout( OGRE_TEXCOORD0 ) in vec2 uv0;

vulkan( layout( ogre_P0 ) uniform Params { )
  uniform mat4 worldViewProj;
vulkan( }; )

out gl_PerVertex
{
  vec4 gl_Position;
};

vulkan_layout( location = 0 )
out block
{
  vec2 uv0;
  vec3 cameraDir;
} outVs;

void main()
{
  gl_Position = worldViewProj * vertex;
  outVs.uv0.xy = uv0.xy;

  // normal is used in fragment shader to reconstruct
  // viewspace pos from depth buffer data
  // see the `quad_normals` and `camera_far_corners_view_space`
  // param descriptions in
  // https://ogrecave.github.io/ogre/api/2.1/compositor.html
  outVs.cameraDir.xyz = normal.xyz;
}
