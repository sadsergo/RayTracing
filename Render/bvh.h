#pragma once

#include <LiteMath/LiteMath.h>
#include "../mesh.h"
#include "render_structs.h"
#include <vector>

using namespace cmesh4;
using LiteMath::float3;

const uint32_t MAX_DEPTH = 10;

struct BVHNode
{
  float3 aabbMin, aabbMax;
  uint32_t leftNode, firstTriIdx, triCount;

  bool IsLeaf() const { return triCount > 0; }
};

struct BVHTriangle
{
  float3 Vertex0, Vertex1, Vertex2, Centroid;
  float3 normal;
};

class BVH
{
public:
  std::vector<BVHNode> Nodes;
  std::vector<BVHTriangle> tri;
  std::vector<uint32_t> triIdx;

  void Build(const SimpleMesh& mesh);
  void UpdateNodeBounds(uint32_t nodeIdx);
  void Subdivide(uint32_t nodeIdx, uint32_t depth);
  void IntersectBVH(const float3& ray_origin, const float3& ray_dir, const uint32_t nodeIdx, HitInfo& hit) const;
  void IntersectAABB(const float3& ray_origin, const float3& ray_dir, const float3& bmin, const float3& bmax, HitInfo& hit) const;
  void IntersectTriangle(const float3& ray_origin, const float3& ray_dir, const BVHTriangle& tri, HitInfo& hit) const;
};

float safe_inverse(const float x);
float3 inv_dir(const float3& dir);