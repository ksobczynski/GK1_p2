#pragma once

#include "Triangle.h"
#include <vector>

namespace model {

struct Mesh {
  std::vector<Triangle> tris;
};

} // namespace model
