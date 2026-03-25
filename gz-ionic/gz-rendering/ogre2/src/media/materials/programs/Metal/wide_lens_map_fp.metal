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

#include <metal_stdlib>
using namespace metal;

struct Params
{
  float cutOffAngle;

  // focal length
  float f;

  // linear scaling constant
  float c1;

  // angle scaling constant
  float c2;

  // angle offset constant
  float c3;

  // unit axis
  // depends on the type of math function sin (X), tan (Y), or identity (Z)
  float3 fun;
};

struct PS_INPUT
{
  float2 fragPos;
};

float3 map(float th, float2 fragPos, float r)
{
  // spherical to cartesian conversion
  return float3(-sin(th)*fragPos.x/r, sin(th)*fragPos.y/r, cos(th));
}

fragment float4 main_metal
(
  PS_INPUT inPs [[stage_in]],
  texturecube<float> envMap  [[texture(0)]],
  sampler texSampler [[sampler(0)]],
  constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{
  float4 fragColor;

  float r = length(inPs.fragPos);
  // calculate angle from optical axis based on the mapping function specified
  float param = r/(p.c1*p.f);
  float theta = 0.0;
  if (p.fun.x > 0)
    theta = asin(param);
  else if (p.fun.y > 0)
    theta = atan(param);
  else if (p.fun.z > 0)
    theta = param;
  theta = (theta-p.c3)*p.c2;

  // compute the direction vector that will be used to sample from the cubemap
  float3 tc = map(theta, inPs.fragPos, r);

  // sample and set resulting color
  fragColor = float4(envMap.sample(texSampler, tc).rgb, 1);

  // limit to visible fov
  //TODO: move to vertex shader
  float param2 = p.cutOffAngle/p.c2+p.c3;
  float cutRadius = p.c1*p.f*(p.fun.x*sin(param2)+p.fun.y*tan(param2)+p.fun.z*param2);

  // smooth edges
  // gl_FragColor.rgb *= 1.0-step(cutRadius,r);
  fragColor.rgb *= 1.0-smoothstep(cutRadius-0.02,cutRadius,r);

  return fragColor;
}
