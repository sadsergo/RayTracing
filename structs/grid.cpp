#include "grid.h"

void save_sdf_grid(const SdfGrid &scene, const std::string &path)
{
  std::ofstream fs(path, std::ios::binary);
  fs.write((const char *)&scene.size, 3 * sizeof(unsigned));
  fs.write((const char *)scene.data.data(), scene.size.x * scene.size.y * scene.size.z * sizeof(float));
  fs.flush();
  fs.close();
}

void load_sdf_grid(SdfGrid &scene, const std::string &path)
{
  std::ifstream fs(path, std::ios::binary);
  fs.read((char *)&scene.size, 3 * sizeof(unsigned));
  scene.data.resize(scene.size.x * scene.size.y * scene.size.z);
  fs.read((char *)scene.data.data(), scene.size.x * scene.size.y * scene.size.z * sizeof(float));
  fs.close();
}

glm::vec3 closest_point_triangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    //implementation taken from Embree library
    const glm::vec3 ab = b - a;
    const glm::vec3 ac = c - a;
    const glm::vec3 ap = p - a;

    const float d1 = glm::dot(ab, ap);
    const float d2 = glm::dot(ac, ap);
    if (d1 <= 0.f && d2 <= 0.f) return a; //#1

    const glm::vec3 bp = p - b;
    const float d3 = glm::dot(ab, bp);
    const float d4 = glm::dot(ac, bp);
    if (d3 >= 0.f && d4 <= d3) return b; //#2

    const glm::vec3 cp = p - c;
    const float d5 = glm::dot(ab, cp);
    const float d6 = glm::dot(ac, cp);
    if (d6 >= 0.f && d5 <= d6) return c; //#3

    const float vc = d1 * d4 - d3 * d2;
    if (vc <= 0.f && d1 >= 0.f && d3 <= 0.f)
    {
        const float v = d1 / (d1 - d3);
        return a + v * ab; //#4
    }
        
    const float vb = d5 * d2 - d1 * d6;
    if (vb <= 0.f && d2 >= 0.f && d6 <= 0.f)
    {
        const float v = d2 / (d2 - d6);
        return a + v * ac; //#5
    }
        
    const float va = d3 * d6 - d5 * d4;
    if (va <= 0.f && (d4 - d3) >= 0.f && (d5 - d6) >= 0.f)
    {
        const float v = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + v * (c - b); //#6
    }

    const float denom = 1.f / (va + vb + vc);
    const float v = vb * denom;
    const float w = vc * denom;
    return a + v * ab + w * ac; //#0
}

SdfGrid mesh2Grid(const SimpleMesh& mesh, const glm::uvec3& size)
{
  SdfGrid grid;
  grid.size = size;
  grid.data.resize(size.x * size.y * size.z);

  float c = size.x - 1;

  for (uint32_t x = 0; x < size.x; x++)
  {
    for (uint32_t y = 0; y < size.y; y++)
    {
      for (uint32_t z = 0; z < size.z; z++)
      {
        glm::vec3 P = 2.f / c * glm::vec3(x, y, z) - glm::vec3(1);
        glm::vec3 P_nearest;
        glm::vec3 n;

        float dist = 1e6;

        for (int i = 0; i < mesh.IndicesNum(); i += 3)
        {
          uint32_t ind1 = mesh.indices[i + 0];
          uint32_t ind2 = mesh.indices[i + 1];
          uint32_t ind3 = mesh.indices[i + 2];

          glm::vec3 A = glm::vec3(mesh.vPos4f[ind1].x, mesh.vPos4f[ind1].y, mesh.vPos4f[ind1].z);
          glm::vec3 B = glm::vec3(mesh.vPos4f[ind2].x, mesh.vPos4f[ind2].y, mesh.vPos4f[ind2].z);
          glm::vec3 C = glm::vec3(mesh.vPos4f[ind3].x, mesh.vPos4f[ind3].y, mesh.vPos4f[ind3].z);

          glm::vec3 Pt = closest_point_triangle(P, A, B, C);

          float tmp_dist = glm::length(P - Pt);

          if (dist > tmp_dist)
          {
            dist = tmp_dist;
            n = glm::normalize(glm::cross(A - B, A - C));
            P_nearest = Pt;
          }
        }

        dist = glm::sign(glm::dot(n, P - P_nearest)) * dist;
        grid.data[(z * size.z + y) * size.y + x] = dist;
      }
    }
  }

  return grid;
}