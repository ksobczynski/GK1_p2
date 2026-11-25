#include "Scene.h"
#include "model/SurfaceLoader.h"
#include <algorithm>

namespace model {

namespace {
QMatrix4x4 initialOrientation() {
  QMatrix4x4 R;
  R.rotate(-60.0f, 1, 0, 0);
  R.rotate(30.0f, 0, 0, 1);
  return R;
}
} // namespace

Scene::Scene() : orientation_(initialOrientation()) {}

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
  applyRotation();
}

void Scene::rotate(float alphaDeg, float betaDeg) {
  QMatrix4x4 increment;
  increment.rotate(alphaDeg, 1, 0, 0);
  increment.rotate(betaDeg, 0, 1, 0);

  orientation_ = increment * orientation_;
  applyRotation();
}

void Scene::resetOrientation() {
  orientation_ = initialOrientation();
  applyRotation();
}
const QMatrix4x4 &Scene::rotationMatrix() const { return orientation_; }
void Scene::applyRotation() {
  for (Mesh &mesh : meshes_) {
    for (Triangle &t : mesh.tris) {
      for (Vertex &v : t.v) {
        v.Pr = orientation_.map(v.P);
        v.Pur = orientation_.mapVector(v.Pu);
        v.Pvr = orientation_.mapVector(v.Pv);
        v.Nr = orientation_.mapVector(v.N).normalized();
      }
    }
  }
}
} // namespace model