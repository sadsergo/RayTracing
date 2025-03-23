#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <string>
#include <fstream>
#include "mesh.h"

using namespace cmesh4;

struct SdfGrid
{
  glm::uvec3 size;
  std::vector<float> data; // size.x*size.y*size.z values
};

void save_sdf_grid(const SdfGrid &scene, const std::string &path);
void load_sdf_grid(SdfGrid &scene, const std::string &path);

SdfGrid mesh2Grid(const SimpleMesh& mesh, const glm::uvec3& size);