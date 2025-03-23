// Copyright 2020 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0
#version 450 

#extension GL_EXT_scalar_block_layout : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_debug_printf : require

#include "includes.glsl"

layout(local_size_x = 16, local_size_y = 8, local_size_z = 1) in;

// The scalar layout qualifier here means to align types according to the alignment
// of their scalar components, instead of e.g. padding them to std140 rules.
layout(binding = 0, set = 0, scalar) buffer storageBuffer
{
  vec3 imageData[];
};

layout(binding = 1, set = 0) uniform cameraBuffer
{
  vec3 position;
  vec3 target;
  float aspect;
  float fov;
} camera;

layout(binding = 2, set = 0) uniform lightBuffer
{
  vec3 position;
} light;

layout(binding = 3, set = 0) buffer sdfBuffer
{
  float sdf[];
};

const uint size = 32;

float eval_distance_sdf_grid(vec3 pos)
{
  //bbox for grid is a unit cube
  vec3 grid_size_f = vec3(size - 1);
  vec3 vox_f = grid_size_f*((pos-vec3(-1,-1,-1))/vec3(2,2,2));// - vec3(0.5, 0.5, 0.5);
  vox_f = min(max(vox_f, vec3(0.0f)), grid_size_f - vec3(1e-5f));
  uvec3 vox_u = uvec3(vox_f);
  vec3 dp = vox_f - vec3(vox_u);

  float res = 0;

  if (vox_u.x < size-1 && vox_u.y < size-1 && vox_u.z < size-1)
  {

    for (uint i=0;i<2;i++)
    {
      for (uint j=0;j<2;j++)
      {
        for (uint k=0;k<2;k++)
        {
          float qx = (1 - dp.x + i*(2*dp.x-1));
          float qy = (1 - dp.y + j*(2*dp.y-1));
          float qz = (1 - dp.z + k*(2*dp.z-1));   
          res += qx*qy*qz*sdf[(vox_u.z + k)*size*size + (vox_u.y + j)*size + (vox_u.x + i)];   
        }      
      }
    }
  }
  else
  {
    res += sdf[(vox_u.z)*size*size + (vox_u.y)*size + (vox_u.x)]; 
  }

  return res;
}

vec3 get_trilinear_normal(vec3 P)
{
  float EPS = 0.001;

  vec3 normal = vec3(eval_distance_sdf_grid(P + vec3(EPS, 0, 0)) - eval_distance_sdf_grid(P + vec3(-EPS, 0, 0)) / (2 * EPS), 
                    eval_distance_sdf_grid(P + vec3(0, EPS, 0)) - eval_distance_sdf_grid(P + vec3(0, -EPS, 0)) / (2 * EPS), 
                    eval_distance_sdf_grid(P + vec3(0, 0, EPS)) - eval_distance_sdf_grid(P + vec3(0, 0, -EPS)) / (2 * EPS));
  normal = normalize(normal);
  return normal;
}

void main()
{
  // The resolution of the buffer, which in this case is a hardcoded vector
  // of 2 unsigned integers:
  const uvec2 resolution = uvec2(800, 600);
  const uvec2 pixel = gl_GlobalInvocationID.xy;

  // If the pixel is outside of the image, don't do anything:
  if((pixel.x >= resolution.x) || (pixel.y >= resolution.y))
  {
    return;
  }

  vec2 P = vec2(pixel) / vec2(resolution);
  P = 2 * P - 1;

  const vec3 cameraOrigin = camera.position;
  const vec3 cameraDir = normalize(camera.target - camera.position);
  vec3 up = vec3(0, 1, 0);
  vec3 right = normalize(cross(cameraDir, up));
  up = normalize(cross(cameraDir, right));

  vec3 rayOrigin = cameraOrigin;
  vec3 rayDirection = normalize(cameraDir + right * P.x * tan(camera.fov / 2.0) * camera.aspect + up * P.y * tan(camera.fov / 2.0));

  vec3 min_pos = vec3(-1, -1, -1);
  vec3 max_pos = vec3(1, 1, 1);
  vec2 tNear_tFar = box_intersects(min_pos, max_pos, rayOrigin, rayDirection);

  float d = 1000;
  int MAX_ITER = 2000;
  int iter = 0;
  float EPS = 1e-6f;
  bool hit = false;

  float tNear = tNear_tFar.x, tFar = tNear_tFar.y;
  float t = tNear;

  vec3 Point = rayOrigin + t * rayDirection;
  d = eval_distance_sdf_grid(Point) + 1e-4;
  
  while (iter < MAX_ITER && d > EPS)
  {
    t += d;
    Point = rayOrigin + t * rayDirection;
    d = eval_distance_sdf_grid(Point);

    iter++;
  }

  // Create a vector of 3 floats with a different color per pixel.
  vec3 pixelColor = vec3(0, 0, 0);

  hit = (d <= EPS);

  if (!(!hit || t < tNear || t > tFar))
  {
    vec3 normal = get_trilinear_normal(rayOrigin + t * rayDirection);
    float c = max(0.1f, clamp(dot(normal, normalize(light.position)), 0.f, 1.f));
    pixelColor = vec3(c, c, c);
  }

  // debugPrintfEXT("%f\n", tNear);
  
  // Get the index of this invocation in the buffer:
  uint linearIndex = resolution.x * pixel.y + pixel.x;
  // Write the color to the buffer.
  imageData[linearIndex] = pixelColor;
}