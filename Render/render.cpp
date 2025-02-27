#include "render.h"

void Renderer::render(Image2D<float>& image, const Settings& settings, const Camera& camera) const
{
  const uint32_t width = image.width();
  const uint32_t height = image.height();

  const float AR = camera.aspect;
  float3 camera_dir = normalize(camera.target - camera.position);
  float3 up {0, 1, 0};
  float3 right = normalize(cross(camera_dir, up));

  up = normalize(cross(camera_dir, right));

  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      float3 color_vec {0, 0, 0};

      float2 P{x, y};
      P /= float2(width, height);
      P = 2 * P - 1;

      float3 ray_orig = camera.position;
      float3 ray_dir = normalize(camera_dir + right * P.x * std::tan(camera.fov / 2) * camera.aspect + up * P.y * std::tan(camera.fov / 2));
      
      HitInfo minHit;

      for (int i = 0; i < meshes[0].IndicesNum(); i += 3)
      {
        HitInfo hit;
        
        IntersectTriangle(ray_orig, ray_dir, i, hit);

        if (hit.isHit && minHit.t > hit.t)
        {
          minHit.isHit = true;
          minHit.t = hit.t;
        } 
      }

      if (minHit.isHit)
      {
        color_vec = float3(1, 1, 1);
        image.data()[width * y + x] = 0xffffff;
      }

      
    }
  }
}

void Renderer::IntersectTriangle(const float3& ray_origin, const float3& ray_dir, const uint32_t ind, HitInfo& hit) const
{
  uint32_t ind1 = meshes[0].indices[ind + 0];
  uint32_t ind2 = meshes[0].indices[ind + 1];
  uint32_t ind3 = meshes[0].indices[ind + 2];

  float3 v0 = float3(meshes[0].vPos4f[ind1].x, meshes[0].vPos4f[ind1].y, meshes[0].vPos4f[ind1].z);
  float3 v1 = float3(meshes[0].vPos4f[ind2].x, meshes[0].vPos4f[ind2].y, meshes[0].vPos4f[ind2].z);
  float3 v2 = float3(meshes[0].vPos4f[ind3].x, meshes[0].vPos4f[ind3].y, meshes[0].vPos4f[ind3].z);

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
}