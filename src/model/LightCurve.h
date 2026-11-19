#pragma once
#include <QPointF>
#include <vector>

namespace model {

struct LightCurve {
  std::vector<QPointF> V;
  float z = 1.5f;

  LightCurve();
  int segments() const { return int(V.size()) / 3; }
  QPointF point(float s) const;
  void moveControl(int i, QPointF p);
};

} // namespace model