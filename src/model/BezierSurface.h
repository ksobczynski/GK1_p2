#pragma once

#include "Mesh.h"
#include <QVector3D>
#include <array>

namespace model {

class BezierSurface {
private:
  QVector3D P[4][4];

public:
  explicit BezierSurface(const std::array<QVector3D, 16> &pts);
  QVector3D point(float u, float v) const;
  QVector3D du(float u, float v) const;
  QVector3D dv(float u, float v) const;
  Mesh triangulate(int n) const;

  const QVector3D &control(int i, int j) const;
};

} // namespace model
