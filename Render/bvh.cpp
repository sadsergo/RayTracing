#include "bvh.h"

void BoundingBox::GrowToInclude(const float3 &point)
{
  Min = LiteMath::min(Min, point);
  Max = LiteMath::max(Max, point);
}

void BoundingBox::GrowToInclude(const BVHTriangle triangle)
{
  GrowToInclude(triangle.VertexA);
  GrowToInclude(triangle.VertexB);
  GrowToInclude(triangle.VertexC);
}

float3 BoundingBox::GetCenter() const
{
  return 0.5f * (Min + Max); 
}

float3 BoundingBox::Size() const
{
  return Max - Min;
}

void BVH::Build(const std::vector<float4> vertices, const std::vector<uint32_t> indices)
{
  BoundingBox bounds;

  for (const auto &vert : vertices)
  {
    bounds.GrowToInclude(LiteMath::to_float3(vert));
  }

  //  create triangles
  std::vector<BVHTriangle> triangles;

  for (int i = 0; i < indices.size(); i += 3)
  {
    float3 a = LiteMath::to_float3(vertices[indices[i + 0]]);
    float3 b = LiteMath::to_float3(vertices[indices[i + 1]]);
    float3 c = LiteMath::to_float3(vertices[indices[i + 2]]);

    triangles.push_back({a, b, c});
  }

  BVHNode root = BVHNode(bounds, triangles, 0);
  Nodes.push_back(root);

  Split(root);
}

void BVH::Split(BVHNode& parent, const uint32_t depth)
{
  if (depth == MAX_DEPTH)
  {
    return;
  }

  //  Choose split axis and position 
  float3 size = parent.Bounds.Size();
  int splitAxis = size.x > LiteMath::max(size.y, size.z) ? 0 : size.y > size.z ? 1 : 2;
  float splitPos = parent.Bounds.GetCenter()[splitAxis];

  //  Create child nodes (and store index in parents)
  parent.ChildIndex = Nodes.size();
  BVHNode childA, childB;

  for (const auto & tri : parent.Triangles)
  {
    bool inA = tri.GetCenter()[splitAxis] < splitPos;
    BVHNode& child = inA ? childA : childB;

    child.Triangles.push_back(tri);
    child.Bounds.GrowToInclude(tri);
  }

  Nodes.push_back(childA);
  Nodes.push_back(childB);

  Split(childA, depth + 1);
  Split(childB, depth + 1);
}

float safe_inverse(const float x)
{
  return std::fabs(x) <= std::numeric_limits<float>::epsilon() 
    ? std::copysign(1.0f / std::numeric_limits<float>::epsilon(), x) 
    : 1.0f / x;
}

float3 inv_dir(const float3 &dir)
{
  return float3(safe_inverse(dir.x), safe_inverse(dir.y), safe_inverse(dir.z));
}

void BVH::RayBoundingBox(const float3 &ray_orig, const float3 &ray_dir, const BoundingBox &bounds, HitInfo& hit) const
{
  float3 tMin = (bounds.Min - ray_orig) * inv_dir(ray_dir - ray_orig);
  float3 tMax = (bounds.Max - ray_orig) * inv_dir(ray_dir - ray_orig);
  float3 t1 = LiteMath::min(tMin, tMax);
  float3 t2 = LiteMath::max(tMin, tMax);
  float tNear = LiteMath::max(LiteMath::max(t1.x, t1.y), t1.z);
  float tFar = LiteMath::min(LiteMath::min(t2.x, t2.y), t2.z);

  hit.isHit = tFar >= tNear && tFar > 0;
  hit.t = hit.isHit ? tNear > 0 ? tNear : 0 : 1e10;
}

void BVH::RayTriangleTestBVH(const BVHNode &node, const float3 &ray_origin, const float3 &ray_dir, HitInfo &hit) const
{
  HitInfo boundsHit;


  if (!boundsHit.isHit)
  {
    return;
  }

  if (node.ChildIndex == 0)
  {
    for (const auto& tr : node.Triangles)
    {
      HitInfo trHit;
      
    }
  }
}