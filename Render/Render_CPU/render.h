#pragma once

#include "../../structs/mesh.h"
#include <LiteMath.h>
#include <Image2d.h>
#include "bvh.h"
#include "render_structs.h"
#include "omp.h"

#include <vector>

using namespace cmesh4;

using LiteMath::float2;
using LiteMath::float3;
using LiteMath::float4;
using LiteMath::int2;
using LiteMath::int3;
using LiteMath::int4;
using LiteMath::uint2;
using LiteMath::uint3;
using LiteMath::uint4;
using LiteImage::Image2D;
using LiteMath::normalize;
using LiteMath::cross;
using LiteMath::dot;

class Renderer
{
public:
  Renderer() {}

  std::vector<SimpleMesh> models;
  BVH bvh;
  
  void render(uint32_t* data, const uint32_t width, const uint32_t height, const Settings& settings, const Camera& camera, const Light& light) const;

private:
  void UnpackXY(const int index, const uint32_t width, uint32_t& x, uint32_t& y) const;
  void calcRayCollision(const float3& ray_origin, const float3& ray_dir, HitInfo& hit) const;
  void IntersectTriangle(const float3 &ray_origin, const float3 &ray_dir, const uint32_t model_ind, const uint32_t tr_ind, HitInfo &hit) const;
};