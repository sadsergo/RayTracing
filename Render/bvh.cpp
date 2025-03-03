#include "bvh.h"

void BVH::Build(const SimpleMesh& mesh)
{
  std::vector<float4> vertices = mesh.vPos4f;
  std::vector<uint32_t> indices = mesh.indices;
  std::vector<float4> normals = mesh.vNorm4f;
  

  for (int i = 0; i < indices.size(); i += 3)
  {
    tri.push_back({to_float3(vertices[indices[i + 0]]), 
      to_float3(vertices[indices[i + 1]]), 
      to_float3(vertices[indices[i + 2]]), 
      to_float3((vertices[indices[i + 0]] + vertices[indices[i + 1]] + vertices[indices[i + 2]]) * 0.3f), 
      to_float3(normals[indices[i + 0]])});
  }

  for (int i = 0; i < tri.size(); i++)
  {
    triIdx.push_back(i);
  }

  BVHNode root;
  root.leftNode = 0;
  root.firstTriIdx = 0;
  root.triCount = tri.size();

  Nodes.push_back(root);

  UpdateNodeBounds(0);
  Subdivide(0, 0);
}

void BVH::UpdateNodeBounds(uint32_t nodeIdx)
{
  BVHNode& node = Nodes[nodeIdx];
  node.aabbMin = float3(1e30);
  node.aabbMax = float3(-1e30);

  for (uint32_t first = node.firstTriIdx, i = 0; i < node.triCount; i++)
  {
    // printf("%u %u\n", first + i, triIdx.size());
    uint32_t leafTriIdx = triIdx[first + i];
    BVHTriangle leafTri = tri[leafTriIdx];
    node.aabbMin = LiteMath::min(node.aabbMin, leafTri.Vertex0);
    node.aabbMin = LiteMath::min(node.aabbMin, leafTri.Vertex1);
    node.aabbMin = LiteMath::min(node.aabbMin, leafTri.Vertex2);
    node.aabbMax = LiteMath::max(node.aabbMax, leafTri.Vertex0);
    node.aabbMax = LiteMath::max(node.aabbMax, leafTri.Vertex1);
    node.aabbMax = LiteMath::max(node.aabbMax, leafTri.Vertex2);
  }
}

void BVH::Subdivide(uint32_t nodeIdx, uint32_t depth)
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

  UpdateNodeBounds(leftChildIdx);
  UpdateNodeBounds(rightChildIdx);

  Subdivide(leftChildIdx, depth + 1);
  Subdivide(rightChildIdx, depth + 1);
}

void BVH::IntersectBVH(const float3& ray_origin, const float3& ray_dir, const uint32_t nodeIdx, HitInfo& hit) const
{
  const BVHNode& node = Nodes[nodeIdx];

  HitInfo hitBVH;
  IntersectAABB(ray_origin, ray_dir, node.aabbMin, node.aabbMax, hitBVH);

  if (!hitBVH.isHit)
  {
    return;
  }

  if (node.IsLeaf())
  {
    for (int i = 0; i < node.triCount; i++)
    {
      HitInfo triHit;
      IntersectTriangle(ray_origin, ray_dir, tri[triIdx[node.firstTriIdx + i]], triHit);

      if (triHit.isHit && triHit.t < hit.t)
      {
        hit = triHit;
      }
    }
  }
  else
  {
    HitInfo h1;
    IntersectBVH(ray_origin, ray_dir, node.leftNode, h1);

    if (h1.isHit && h1.t < hit.t)
    {
      hit = h1;
    }

    HitInfo h2;
    IntersectBVH(ray_origin, ray_dir, node.leftNode + 1, h2);

    if (h2.isHit && h2.t < hit.t)
    {
      hit = h2;
    }
  }
}

void BVH::IntersectTriangle(const float3& ray_origin, const float3& ray_dir, const BVHTriangle& tri, HitInfo& hit) const
{
  float3 v0 = tri.Vertex0;
  float3 v1 = tri.Vertex1;
  float3 v2 = tri.Vertex2;

  float3 e1 = v1 - v0;
  float3 e2 = v2 - v0;

  float3 pvec = cross(ray_dir, e2);
  float det = dot(e1, pvec);

  //  ray is parralel with triangle
  if (det < 1e-8 && det > -1e-8)
  {
    return;
  }

  float inv_det = 1 / det;
  float3 tvec = ray_origin - v0;
  float u = dot(tvec, pvec) * inv_det;

  if (u < 0 || u > 1)
  {
    return;
  }

  float3 qvec = cross(tvec, e1);
  float v = dot(ray_dir, qvec) * inv_det;

  if (v < 0 || u + v > 1)
  {
    return;
  }

  hit.isHit = true;
  hit.t = dot(e2, qvec) * inv_det;

  hit.normal = normalize(tri.normal);
}

void BVH::IntersectAABB(const float3& ray_origin, const float3& ray_dir, const float3& bmin, const float3& bmax, HitInfo& hit) const
{
  float tx1 = (bmin.x - ray_origin.x) / ray_dir.x, tx2 = (bmax.x - ray_origin.x) / ray_dir.x;
  float tmin = LiteMath::min(tx1, tx2), tmax = LiteMath::max(tx1, tx2);
  float ty1 = (bmin.y - ray_origin.y) / ray_dir.y, ty2 = (bmax.y - ray_origin.y) / ray_dir.y;

  tmin = LiteMath::max(tmin, LiteMath::min(ty1, ty2));
  tmax = LiteMath::min(tmax, LiteMath::max(ty1, ty2));

  float tz1 = (bmin.z - ray_origin.z) / ray_dir.z, tz2 = (bmax.z - ray_origin.z) / ray_dir.z;

  tmin = LiteMath::max(tmin, LiteMath::min(tz1, tz2));
  tmax = LiteMath::min(tmax, LiteMath::max(tz1, tz2));

  hit.isHit = tmax >= tmin && tmin < hit.t && tmax > 0;
}