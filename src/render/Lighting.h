#pragma once

#include <QColor>
#include <QVector3D>
#include <algorithm>
#include <cmath>

namespace render {
struct Lighting {
  QVector3D lightPos{0.5f, 0.5f, 2.0f};
  QVector3D lightColor{1, 1, 1};
  QVector3D objectColor{0.9f, 0.6f, 0.2f};
  float kd = 0.6f, ks = 0.4f;
  float m = 20;
};
inline QRgb shadePhong(const Lighting &l, const QVector3D &P,
                       const QVector3D &N, const QVector3D &IO) {
  const QVector3D V(0, 0, 1);
  const QVector3D L = (l.lightPos - P).normalized();
  const float NL = QVector3D::dotProduct(N, L);
  const QVector3D R = (2 * NL * N - L).normalized();

  const float diff = std::max(0.0f, NL);
  const float spec = std::pow(std::max(0.0f, QVector3D::dotProduct(V, R)), l.m);

  const QVector3D I = l.lightColor * IO * (l.kd * diff + l.ks * spec);

  auto to255 = [](float c) { return std::min(255, int(c * 255)); };
  return qRgb(to255(I.x()), to255(I.y()), to255(I.z()));
}
} // namespace render