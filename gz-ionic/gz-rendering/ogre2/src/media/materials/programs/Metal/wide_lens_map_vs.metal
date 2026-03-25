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

struct VS_INPUT
{
  float4 position [[attribute(VES_POSITION)]];
};

struct Params
{
  // aspect ratio
  float ratio;
  float4x4 worldViewProj;
};

struct PS_INPUT
{
  float2 fragPos;
  float4 gl_Position [[position]];
};

vertex PS_INPUT main_metal
(
  VS_INPUT input [[stage_in]],
  constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{
  PS_INPUT outVs;

  outVs.gl_Position = p.worldViewProj * input.position;

  // get normalized fragment coordinate (3D to 2D window space transformation)
  outVs.fragPos = outVs.gl_Position.xy/outVs.gl_Position.w*float2(-1.0,1.0/p.ratio);

  return outVs;
}
