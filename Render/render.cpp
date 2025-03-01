#include "render.h"

void Renderer::render(uint32_t* data, const uint32_t width, const uint32_t height, const Settings &settings, const Camera &camera, const Light &light) const
{
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

      float2 P{(float)x, (float)y};
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
          minHit.normal = hit.normal;
          minHit.t = hit.t;
        } 
      }

      if (minHit.isHit)
      {
        float3 hitPoint = ray_orig + minHit.t * ray_dir;
        float3 light_dir = normalize(light.pos - hitPoint);

        float ambientStrength = 0.1f;
        float3 ambient = ambientStrength * light.color;

        float3 objectColor{165, 42, 42};

        float diff = LiteMath::max(dot(minHit.normal, light_dir), 0.1f);
        float3 diffuse = diff * light.color;

        float specularStrenght = 0.5f;
        float3 reflectDir = LiteMath::reflect(light_dir, minHit.normal);
        float spec = std::pow(LiteMath::max(dot(ray_dir, reflectDir), 0.0f), 32);
        float3 specular = specularStrenght * spec * light.color;

        float d = LiteMath::length(light_dir), K_c = 1.f, K_t = 0.09f, K_q = 0.032f;
        float F_att = 1.0 / (K_c + K_t * d + K_q * d * d);

        color_vec = F_att * (ambient + diffuse + specular) * objectColor;
        
        data[width * y + x] = 0xff << 24 | (uint8_t)color_vec.x << 16 | (uint8_t)color_vec.y << 8 | (uint8_t)color_vec.z;
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

  hit.normal = normalize(LiteMath::to_float3(meshes[0].vNorm4f[ind1]));

  // if (dot(hit.normal, ray_dir) > 0)
  // {
  //   hit.normal *= -1;
  // }
}