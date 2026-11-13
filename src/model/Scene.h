#pragma once

#include "BezierSurface.h"
#include <QString>
#include <vector>

namespace model {

class Scene {
public:
  bool load(const QString &path, QString *error);
  void setResolution(int n);
  int resolution() const;
  const std::vector<BezierSurface> &surfaces() const;
  const std::vector<Mesh> &meshes() const;

private:
  void retriangulate();
  std::vector<BezierSurface> surfaces_;
  std::vector<Mesh> meshes_;
  int resolution_ = 10;
};

} // namespace model
