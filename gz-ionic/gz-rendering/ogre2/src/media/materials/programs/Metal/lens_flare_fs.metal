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
  // Viewport's aspect ratio
  float vpAspectRatio;

  // light pos in clip space
  float3 lightPos;

  // lens flare scale
  float scale;

  // lens flare color
  float3 color;
};

struct PS_INPUT
{
  float2 uv0;
};

inline float3 lensflare(float2 uv,float2 pos, constant Params &p)
{
  float2 main = uv-pos;

  float2 uvd = uv*length(uv);

  float dist = length(main); dist = pow(dist,.1);

  float f0 = 1.0/(length(uv-pos)*16.0/p.scale+1.0);

  float f2 = max(1.0/(1.0+32.0*pow(length(uvd+0.8*pos),2.0)),.0)*00.25;
  float f22 = max(1.0/(1.0+32.0*pow(length(uvd+0.85*pos),2.0)),.0)*00.23;
  float f23 = max(1.0/(1.0+32.0*pow(length(uvd+0.9*pos),2.0)),.0)*00.21;

  float2 uvx = mix(uv,uvd,-0.5);

  float f4 = max(0.01-pow(length(uvx+0.4*pos),2.4),.0)*6.0;
  float f42 = max(0.01-pow(length(uvx+0.45*pos),2.4),.0)*5.0;
  float f43 = max(0.01-pow(length(uvx+0.5*pos),2.4),.0)*3.0;

  uvx = mix(uv,uvd,-.4);

  float f5 = max(0.01-pow(length(uvx+0.2*pos),5.5),.0)*2.0;
  float f52 = max(0.01-pow(length(uvx+0.4*pos),5.5),.0)*2.0;
  float f53 = max(0.01-pow(length(uvx+0.6*pos),5.5),.0)*2.0;

  uvx = mix(uv,uvd,-0.5);

  float f6 = max(0.01-pow(length(uvx-0.3*pos),1.6),.0)*6.0;
  float f62 = max(0.01-pow(length(uvx-0.325*pos),1.6),.0)*3.0;
  float f63 = max(0.01-pow(length(uvx-0.35*pos),1.6),.0)*5.0;

  float3 c = float3(.0);

  c.r+=f2+f4+f5+f6; c.g+=f22+f42+f52+f62; c.b+=f23+f43+f53+f63;
  c *= min(p.scale, 0.2)/0.2;
  c+=float3(f0);

  return c;
}

fragment float4 main_metal
(
  PS_INPUT inPs [[stage_in]],
  texture2d<float> srcTex  [[texture(0)]],
  sampler texSampler [[sampler(0)]],
  constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{
  float4 fragColor;

  if (p.lightPos.z < 0.0)
  {
    // light is behind the view
    fragColor = srcTex.sample(texSampler, inPs.uv0.xy);
  }
  else
  {
    float3 pos = p.lightPos;

    // flip y
    pos.y *= -1.0;

    float2 uv = inPs.uv0.xy - 0.5;
    // scale p.lightPos to be same range as uv
    pos *= 0.5;
    // fix aspect ratio
    uv.x *= p.vpAspectRatio;
    pos.x *= p.vpAspectRatio;

    // compute lens flare
    float3 finalColor = p.color * lensflare(uv, pos.xy, p);

    // apply lens flare
    fragColor = srcTex.sample(texSampler, inPs.uv0.xy) +
                float4(finalColor, 1.0);
  }

  return fragColor;
}
