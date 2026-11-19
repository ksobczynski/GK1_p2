#pragma once

#include "BezierSurface.h"
#include <QMatrix4x4>
#include <QString>
#include <vector>

namespace model {

class Scene {
public:
  Scene();
  bool load(const QString &path, QString *error);
  void setResolution(int n);
  int resolution() const;
  const std::vector<BezierSurface> &surfaces() const;
  const std::vector<Mesh> &meshes() const;

  void rotate(float alphaDeg, float betaDeg);
  void resetOrientation();
  const QMatrix4x4 &rotationMatrix() const;

private:
  void retriangulate();
  void applyRotation();

  std::vector<BezierSurface> surfaces_;
  std::vector<Mesh> meshes_;
  int resolution_ = 10;

  QMatrix4x4 orientation_;
};

} // namespace model
