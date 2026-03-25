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
  vec3 cameraDir;
} inPs;

vulkan_layout( ogre_t0 ) uniform texture2D depthTexture;
vulkan_layout( ogre_t1 ) uniform texture2D colorTexture;
vulkan_layout( ogre_t2 ) uniform texture2D particleDepthTexture;
vulkan_layout( ogre_t3 ) uniform texture2D particleTexture;

vulkan( layout( ogre_s0 ) uniform sampler texSampler );

vulkan_layout( location = 0 )
out vec4 fragColor;

vulkan( layout( ogre_P0 ) uniform Params { )
	uniform vec2 projectionParams;
	uniform float near;
	uniform float far;
	uniform float min;
	uniform float max;

	uniform float particleStddev;
	uniform float particleScatterRatio;
	// rnd is a random number in the range of [0-1]
	uniform float rnd;
vulkan( }; )

// see gaussian_noise_fs.glsl for documentation on the rand and gaussrand
// functions

#define PI 3.14159265358979323846264

float rand(vec2 co)
{
  float r = fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
  // Make sure that we don't return 0.0
  if(r == 0.0)
    return 0.000000000001;
  else
    return r;
}

vec4 gaussrand(vec2 co, vec3 offsets, float stddev, float mean)
{
  float U, V, R, Z;
  U = rand(co + vec2(offsets.x, offsets.x));
  V = rand(co + vec2(offsets.y, offsets.y));
  R = rand(co + vec2(offsets.z, offsets.z));
  if(R < 0.5)
    Z = sqrt(-2.0 * log(U)) * sin(2.0 * PI * V);
  else
    Z = sqrt(-2.0 * log(U)) * cos(2.0 * PI * V);
  Z = Z * stddev + mean;
  return vec4(Z, Z, Z, 0.0);
}

void main()
{
  // get linear depth
  float fDepth = texture(vkSampler2D(depthTexture,texSampler), inPs.uv0).x;
  float d = projectionParams.y / (fDepth - projectionParams.x);

  // get retro
  float retro = texture(vkSampler2D(colorTexture, texSampler), inPs.uv0).x * 2000.0;

  // reconstruct 3d viewspace pos from depth
  vec3 viewSpacePos = inPs.cameraDir * d;

  // get length of 3d point, i.e.range
  float l = length(viewSpacePos);

  // particle mask - color and depth
  vec4 particle = texture(vkSampler2D(particleTexture,texSampler), inPs.uv0);
  float particleDepth = texture(vkSampler2D(particleDepthTexture,texSampler), inPs.uv0).x;
  float pd = projectionParams.y / (particleDepth - projectionParams.x);

  // check if need to apply scatter effect
  if (particle.x > 0.0 && pd < d)
  {
    // apply scatter effect so that only some of the smoke pixels are visible
    float r = rand(inPs.uv0 + vec2(rnd, rnd));
    if (r < particleScatterRatio)
    {
      vec3 point = inPs.cameraDir * pd;

      float rr = rand(inPs.uv0 + vec2(rnd, rnd)) - 0.5;

      // apply gaussian noise to particle range data
      // With large particles, the range returned are all from the first large
      // particle. So add noise with some mean values so that all the points are
      // shifted further out. This gives depth readings beyond the first few
      // particles and avoid too many early returns
      vec3 noise = gaussrand(inPs.uv0, vec3(rnd, rnd, rnd),
           particleStddev, rr*rr*particleStddev*0.5).xyz;
      float noiseLength = length(noise);

      // apply gaussian noise to particle depth data
      float newLength = length(point) + noiseLength;

      // make sure we do not produce values larger than the range of the first
      // non-particle obstacle, e.g. a box behind particle should still return
      // a hit
      if (newLength < l)
        l = newLength;
    }
  }

  if (l > far)
    l = max;
  else if (l < near)
    l = min;

  fragColor = vec4(l, retro, 0.0, 1.0);
}
