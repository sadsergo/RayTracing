#pragma once

#include <LiteMath/LiteMath.h>
#include "../mesh.h"
#include "render_structs.h"
#include <vector>

using namespace cmesh4;
using LiteMath::float3;

const uint32_t MAX_DEPTH = 10;

struct BVHTriangle
{
  float3 VertexA;
  float3 VertexB;
  float3 VertexC;

  float3 GetCenter() const
  {
    return (VertexA + VertexB + VertexC) / 3.0f;
  }
};

class BoundingBox
{
public:
  float3 Min;
  float3 Max;

  void GrowToInclude(const float3& point);
  void GrowToInclude(const BVHTriangle triangle);
  float3 GetCenter() const;
  float3 Size() const;
};

struct BVHNode
{
  BoundingBox Bounds;
  std::vector<BVHTriangle> Triangles;

  uint32_t ChildIndex;

  BVHNode(const BoundingBox &bounds, const std::vector<BVHTriangle> &triangles, const uint32_t childIndex) : Bounds(bounds), Triangles(triangles), ChildIndex(childIndex) {}
  BVHNode() {}
};

class BVH
{
public:
  std::vector<BVHNode> Nodes;

  void Build(const std::vector<float4> vertices, const std::vector<uint32_t> indices);
  void Split(BVHNode& parent, const uint32_t depth = 0);
  void RayTriangleTestBVH(const BVHNode& node, const float3& ray_origin, const float3& ray_dir, HitInfo& hit) const;
  void RayBoundingBox(const float3 &ray_orig, const float3 &ray_dir, const BoundingBox &bounds, HitInfo &hit) const;
};

float safe_inverse(const float x);
float3 inv_dir(const float3& dir);