#include "LightCurve.h"
#include <algorithm>
#include <cmath>

namespace model {

LightCurve::LightCurve() {

  const float r = 1.2f, h = r / 2;
  V = {{r, 0},  {r, h},   {h, r},   {0, r},  {-h, r}, {-r, h},
       {-r, 0}, {-r, -h}, {-h, -r}, {0, -r}, {h, -r}, {r, -h}};
}

QPointF LightCurve::point(float s) const {
  const int n = segments();
  s -= std::floor(s);
  const int k = std::min(int(s * n), n - 1);
  const float t = s * n - k;
  const QPointF &V0 = V[3 * k], &V1 = V[3 * k + 1], &V2 = V[3 * k + 2];
  const QPointF &V3 = V[(3 * k + 3) % V.size()];
  const float u = 1 - t;
  return V0 * (u * u * u) + V1 * (3 * t * u * u) + V2 * (3 * t * t * u) +
         V3 * (t * t * t);
}

void LightCurve::moveControl(int i, QPointF p) {
  const int N = int(V.size());
  if (i % 3 == 0) {
    const QPointF d = p - V[i];
    V[i] = p;
    V[(i + 1) % N] += d;
    V[(i + N - 1) % N] += d;
  } else {
    const int joint = (i % 3 == 1) ? i - 1 : (i + 1) % N;
    const int other = (i % 3 == 1) ? (i + N - 2) % N : (i + 2) % N;
    V[i] = p;
    V[other] = 2 * V[joint] - p;
  }
}

} // namespace model