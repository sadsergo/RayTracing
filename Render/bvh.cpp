#include "bvh.h"

void BVH::Build(const std::vector<float4> vertices, const std::vector<uint32_t>& indices)
{
  std::vector<BVHTriangle> tris;
  std::vector<uint32_t> triIdx;

  for (int i = 0; i < indices.size(); i += 3)
  {
    tris.push_back({to_float3(vertices[indices[i + 0]]), to_float3(vertices[indices[i + 1]]), to_float3(vertices[indices[i + 2]]), to_float3((vertices[indices[i + 0]] + vertices[indices[i + 1]] + vertices[indices[i + 2]]) * 0.3f)});
  }

  for (int i = 0; i < tris.size(); i++)
  {
    triIdx.push_back(i);
  }

  BVHNode root;
  root.leftNode = 0;
  root.firstTriIdx = 0;
  root.triCount = tris.size();

  Nodes.push_back(root);

  UpdateNodeBounds(0, tris, triIdx);
  Subdivide(0, tris, triIdx, 0);
}

void BVH::UpdateNodeBounds(uint32_t nodeIdx, const std::vector<BVHTriangle>& tris, const std::vector<uint32_t>& triIdx)
{
  BVHNode& node = Nodes[nodeIdx];
  node.aabbMin = float3(1e30);
  node.aabbMax = float3(-1e30);

  for (uint32_t first = node.firstTriIdx, i = 0; i < node.triCount; i++)
  {
    // printf("%u %u\n", first + i, triIdx.size());
    uint32_t leafTriIdx = triIdx[first + i];
    BVHTriangle leafTri = tris[leafTriIdx];
    node.aabbMin = LiteMath::min(node.aabbMin, leafTri.Vertex0);
    node.aabbMin = LiteMath::min(node.aabbMin, leafTri.Vertex1);
    node.aabbMin = LiteMath::min(node.aabbMin, leafTri.Vertex2);
    node.aabbMax = LiteMath::max(node.aabbMax, leafTri.Vertex0);
    node.aabbMax = LiteMath::max(node.aabbMax, leafTri.Vertex1);
    node.aabbMax = LiteMath::max(node.aabbMax, leafTri.Vertex2);
  }
}

void BVH::Subdivide(uint32_t nodeIdx, const std::vector<BVHTriangle>& tri, std::vector<uint32_t>& triIdx, uint32_t depth)
{
  if (depth >= MAX_DEPTH)
  {
    return;
  }

  BVHNode node = Nodes[nodeIdx];

  float3 extent = node.aabbMax - node.aabbMin;
  int axis = 0;

  if (extent.y > extent.x)
  {
    axis = 1;
  }
  if (extent.z > extent[axis])
  {
    axis = 2;
  }

  float splitPos = node.aabbMin[axis] + extent[axis] * 0.5;
  int i = node.firstTriIdx;
  
  int j = i + node.triCount - 1;

  while (i <= j)
  {
    if (tri[triIdx[i]].Centroid[axis] < splitPos)
    {
      i++;
    }
    else
    {
      std::swap(triIdx[i], triIdx[j--]);
    }
  }

  int leftCount = i - node.firstTriIdx;

  if (leftCount == 0 || leftCount == node.triCount)
  {
    return;
  }

  int leftChildIdx = Nodes.size();
  int rightChildIdx = leftChildIdx + 1;
  
  BVHNode leftNode, rightNode;
  Nodes.push_back(leftNode);
  Nodes.push_back(rightNode);
  
  Nodes[leftChildIdx].firstTriIdx = node.firstTriIdx;
  Nodes[leftChildIdx].triCount = leftCount;
  Nodes[rightChildIdx].firstTriIdx = i;
  Nodes[rightChildIdx].triCount = node.triCount - leftCount;
  
  Nodes[nodeIdx].leftNode = leftChildIdx;
  Nodes[nodeIdx].triCount = 0;

  UpdateNodeBounds(leftChildIdx, tri, triIdx);
  UpdateNodeBounds(rightChildIdx, tri, triIdx);

  Subdivide(leftChildIdx, tri, triIdx, depth + 1);
  Subdivide(rightChildIdx, tri, triIdx, depth + 1);
}