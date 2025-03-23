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
        glm::vec3 point = 2.f / c * glm::vec3(x, y, z) - glm::vec3(1);

        float dist = 1e6;

        for (int i = 0; i < mesh.IndicesNum(); i += 3)
        {
          uint32_t ind1 = mesh.indices[i + 0];
          uint32_t ind2 = mesh.indices[i + 1];
          uint32_t ind3 = mesh.indices[i + 2];

          glm::vec3 v0 = glm::vec3(mesh.vPos4f[ind1].x, mesh.vPos4f[ind1].y, mesh.vPos4f[ind1].z);
          glm::vec3 v1 = glm::vec3(mesh.vPos4f[ind2].x, mesh.vPos4f[ind2].y, mesh.vPos4f[ind2].z);
          glm::vec3 v2 = glm::vec3(mesh.vPos4f[ind3].x, mesh.vPos4f[ind3].y, mesh.vPos4f[ind3].z);

          
        }
      }
    }
  }

  return grid;
}