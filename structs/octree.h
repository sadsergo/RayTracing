#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <string>
#include <fstream>

struct SdfOctreeNode
{
  float values[8];
  unsigned offset; // offset for children (they are stored together). 0 offset means it's a leaf
};

struct SdfOctree
{
  std::vector<SdfOctreeNode> nodes;
};