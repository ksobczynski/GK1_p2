#include "Scene.h"
#include "model/BezierSurface.h"
#include "model/SurfaceLoader.h"
#include <algorithm>

namespace model {
bool Scene::load(const QString &path, QString *error) {
  std::vector<BezierSurface> surfaces;
  if (!loadSurfaces(path, surfaces, error))
    return false;

  surfaces_ = std::move(surfaces);
  retriangulate();
  return true;
}

void Scene::setResolution(int n) {
  n = std::max(1, n);
  if (n == resolution_)
    return;
  resolution_ = n;
  retriangulate();
}

int Scene::resolution() const { return resolution_; }

const std::vector<BezierSurface> &Scene::surfaces() const { return surfaces_; }
const std::vector<Mesh> &Scene::meshes() const { return meshes_; }

void Scene::retriangulate() {
  meshes_.clear();
  meshes_.reserve(surfaces_.size());
  for (const BezierSurface &s : surfaces_)
    meshes_.push_back(s.triangulate(resolution_));
}

}; // namespace model