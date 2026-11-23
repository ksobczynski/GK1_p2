#pragma once

#include <QColor>
#include <QImage>
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
  QImage texture;
  bool useTexture = false;
  QImage normalMap;
  bool useNormalMap = false;
};

inline bool loadImage(QImage &dst, const QString &path) {
  QImage img(path);
  if (img.isNull())
    return false;
  dst = img.convertToFormat(QImage::Format_RGB32);
  return true;
}

inline QVector3D textureColor(const QImage &tex, float u, float v) {
  const int x = int(std::clamp(u, 0.0f, 1.0f) * (tex.width() - 1) + 0.5f);
  const int y = int(std::clamp(v, 0.0f, 1.0f) * (tex.height() - 1) + 0.5f);
  const QRgb c = reinterpret_cast<const QRgb *>(tex.constScanLine(y))[x];
  return QVector3D(qRed(c), qGreen(c), qBlue(c)) / 255.0f;
}

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