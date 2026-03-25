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

#include <metal_stdlib>
using namespace metal;

#define M_PI 3.1415926535897932384626433832795

struct PS_INPUT
{
  float4 interpolatedPosition;
};

struct Params
{
  int u_seed;
  float2 u_resolution;
  float3 u_color;
  float4x4 u_adjustments;
};

float random(float2 uv, float seed) {
  return fract(sin(fmod(dot(uv, float2(12.9898, 78.233))
          + 1113.1 * seed, M_PI)) * 43758.5453);
}

fragment float4 main_metal
(
  PS_INPUT inPs [[stage_in]],
  constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{
  // Metal matrices are row major: a[row][col]
  float a0 = p.u_adjustments[0][0];
  float a1 = p.u_adjustments[0][1];
  float a2 = p.u_adjustments[0][2];
  float a3 = p.u_adjustments[0][3];

  float3 a = float3(a0, a1, a2);
  float2 b = float2(distance(float3(inPs.interpolatedPosition.xyw), a)) * a3;
  float2 normalizedFragCoord = b / p.u_resolution;

  float color = random(normalizedFragCoord, float(p.u_seed));
  return float4(color * p.u_color, 1.0);
}
