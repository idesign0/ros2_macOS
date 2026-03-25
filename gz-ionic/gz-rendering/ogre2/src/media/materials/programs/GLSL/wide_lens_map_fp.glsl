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

vulkan( layout( ogre_P0 ) uniform Params { )
  uniform float cutOffAngle;

  // focal length
  uniform float f;

  // linear scaling constant
  uniform float c1;

  // angle scaling constant
  uniform float c2;

  // angle offset constant
  uniform float c3;

  // unit axis
  // depends on the type of math function sin (X), tan (Y), or identity (Z)
  uniform vec3 fun;
vulkan( }; )

vulkan_layout( location = 0 )
in block
{
  vec2 fragPos;
} inPs;

vulkan_layout( ogre_t0 ) uniform textureCube envMap;
vulkan( layout( ogre_s0 ) uniform sampler texSampler );

vec3 map(float th, vec2 fragPos, float r)
{
  // spherical to cartesian conversion
  return vec3(-sin(th)*fragPos.x/r, sin(th)*fragPos.y/r, cos(th));
}

vulkan_layout( location = 0 )
out vec4 fragColor;

void main()
{
  float r = length(inPs.fragPos);
  // calculate angle from optical axis based on the mapping function specified
  float param = r/(c1*f);
  float theta = 0.0;
  if (fun.x > 0)
    theta = asin(param);
  else if (fun.y > 0)
    theta = atan(param);
  else if (fun.z > 0)
    theta = param;
  theta = (theta-c3)*c2;

  // compute the direction vector that will be used to sample from the cubemap
  vec3 tc = map(theta, inPs.fragPos, r);

  // sample and set resulting color
  fragColor = vec4(texture(vkSamplerCube(envMap, texSampler), tc).rgb, 1);

  // limit to visible fov
  //TODO: move to vertex shader
  float param2 = cutOffAngle/c2+c3;
  float cutRadius = c1*f*(fun.x*sin(param2)+fun.y*tan(param2)+fun.z*param2);

  // smooth edges
  // gl_FragColor.rgb *= 1.0-step(cutRadius,r);
  fragColor.rgb *= 1.0-smoothstep(cutRadius-0.02,cutRadius,r);
}
