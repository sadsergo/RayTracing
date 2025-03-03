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
};

class BVH
{
public:
  std::vector<BVHNode> Nodes;

  void Build(const std::vector<float4> vertices, const std::vector<uint32_t>& indices);
  void UpdateNodeBounds(uint32_t nodeIdx, const std::vector<BVHTriangle>& tris, const std::vector<uint32_t>& triIdx);
  void Subdivide(uint32_t nodeIdx, const std::vector<BVHTriangle>& tris, std::vector<uint32_t>& triIdx, uint32_t depth);
};

float safe_inverse(const float x);
float3 inv_dir(const float3& dir);