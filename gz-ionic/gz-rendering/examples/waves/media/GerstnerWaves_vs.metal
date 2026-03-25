// Copyright (c) 2016 The UUV Simulator Authors.
// All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.s


// Copyright (c) 2019 Rhys Mainwaring.
//
// Modified to accept vector parameters and use the form
// for Gerstner waves published in:
//
// Jerry Tessendorf, "Simulating Ocean Water", 1999-2004
//
// theta = k * dir . x - omega * t
//
// px = x - dir.x * a * k * sin(theta)
// py = y - dir.y * a * k * sin(theta)
// pz =         a * k * cos(theta)
//
// k is the wavenumber
// omega is the angular frequency
//
// The derivative terms (Tangent, Binormal, Normal) have been
// updated to be consistent with this convention.


// original concept:
// https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-1-effective-water-simulation-physical-models

#include <metal_stdlib>
using namespace metal;

struct VS_INPUT
{
  float4 position [[attribute(VES_POSITION)]];
  float2 uv0      [[attribute(VES_TEXTURE_COORDINATES0)]];
};

/////////// Input parameters //////////
// Waves
struct Params
{
  float4x4 worldviewproj_matrix;
  int Nwaves;
  float3 camera_position_object_space;
  float rescale;
  float2 bumpScale;
  float2 bumpSpeed;
  float t;
  float3 amplitude;
  float3 wavenumber;
  float3 omega;
  float3 steepness;
  float2 dir0;
  float2 dir1;
  float2 dir2;
  float tau;
};

/////////// Output variables to fragment shader //////////
struct PS_INPUT
{
  float4 gl_Position  [[position]];
  float2 uv0;
  float3 B;
  float3 T;
  float3 N;
  float3 eyeVec;
  float2 bumpCoord;
};

// Compute linear combination of Gerstner waves as described in
// GPU Gems, chapter 01: "Effective Water Simulation from Physical Models"
// http://http.developer.nvidia.com/GPUGems/gpugems_ch01.html

// Information regarding a single wave
struct WaveParameters {
  WaveParameters(float _k, float _a, float _omega, float2 _d, float _q) :
    k(_k), a(_a), omega(_omega), d(_d), q(_q) {}

  float k;      // wavenumber
  float a;      // amplitude
  float omega;  // phase constant of speed
  float2 d;     // horizontal direction of wave
  float q;      // steepness for Gerstner wave (q=0: rolling sine waves)
};

vertex PS_INPUT main_metal
(
  VS_INPUT input [[stage_in]],
  constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{
  PS_INPUT outVs;

  // Use combination of three waves. Values here are chosen rather arbitrarily.
  // Other parameters might lead to better-looking waves.

  WaveParameters waves[3] = {
    WaveParameters(p.wavenumber.x, p.amplitude.x, p.omega.x, p.dir0.xy, p.steepness.x),
    WaveParameters(p.wavenumber.y, p.amplitude.y, p.omega.y, p.dir1.xy, p.steepness.y),
    WaveParameters(p.wavenumber.z, p.amplitude.z, p.omega.z, p.dir2.xy, p.steepness.z)
  };

  float4 P = input.position;

  // Iteratively compute binormal, tangent, and normal vectors:
  float3 B = float3(1.0, 0.0, 0.0);
  float3 T = float3(0.0, 1.0, 0.0);
  float3 N = float3(0.0, 0.0, 1.0);

  // Wave synthesis using linear combination of Gerstner waves
  for(int i = 0; i < p.Nwaves; ++i)
  {
    // Evaluate wave equation:
    float k = waves[i].k;
    float a = waves[i].a * (1.0 - exp(-1.0*p.t/p.tau));
    float q = waves[i].q;
    float dx = waves[i].d.x;
    float dy = waves[i].d.y;
    float theta = dot(waves[i].d, P.xy)*k - p.t*waves[i].omega;
    float c = cos(theta);
    float s = sin(theta);

    // Displacement of point due to wave (Eq. 9)
    P.x -= q*a*dx*s;
    P.y -= q*a*dx*s;
    P.z += a*c;

    // Modify normals due to wave displacement (Eq. 10-12)
    float ka = a*k;
    float qkac = q*ka*c;
    float kas = ka*s;
    float dxy = dx*dy;

    B += float3(-qkac*dx*dx, -qkac*dxy, -kas*dx);
    T += float3(-qkac*dxy, -qkac*dy*dy, -kas*dy);
    N += float3(dx*kas, dy*kas, -qkac);
  }

  // Compute (Surf2World * Rescale) matrix
  B = normalize(B)*p.rescale;
  T = normalize(T)*p.rescale;
  N = normalize(N);
  // outVs won't accept float3x3, so pass components 
  outVs.B = B;
  outVs.T = T;
  outVs.N = N;
 
  outVs.gl_Position = p.worldviewproj_matrix * P;

  // Compute texture coordinates for bump map
  outVs.bumpCoord = input.uv0.xy * p.bumpScale + p.t * p.bumpSpeed;

  outVs.eyeVec = P.xyz - p.camera_position_object_space; // eye position in vertex space

  // Pass texture coordinates to frag shader
  outVs.uv0 = input.uv0.xy;

  return outVs;
}
