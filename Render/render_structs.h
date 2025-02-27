#pragma once

#include "LiteMath/LiteMath.h"
#include "LiteMath/Image2d.h"

using LiteMath::float2;
using LiteMath::float3;
using LiteMath::float4;
using LiteMath::int2;
using LiteMath::int3;
using LiteMath::int4;
using LiteMath::uint2;
using LiteMath::uint3;
using LiteMath::uint4;

struct HitInfo
{
  bool isHit;
  float t;
  float3 normal;

  HitInfo(const bool isHit = false, const float t = 1e10, const float3& normal = float3(0, 0, 0)) : isHit(isHit), t(t), normal(normal) {}
};

struct Settings
{
  uint32_t spp;
};

struct Camera
{
  float3 position;
  float3 target;

  float fov;
  float aspect;
};