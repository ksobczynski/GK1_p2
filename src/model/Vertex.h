#pragma once

#include <QVector3D>

namespace model {

struct Vertex {
  QVector3D P, Pu, Pv, N;
  QVector3D Pr, Pur, Pvr, Nr;
  float u, v;
};

} // namespace model
