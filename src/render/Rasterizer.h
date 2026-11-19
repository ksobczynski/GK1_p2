#pragma once

#include "FrameBuffer.h"
#include <algorithm>
#include <cmath>
#include <utility>

namespace render {
struct ScreenVertex {
  float x, y, z;
};

// pole trojkata apb podwojne
inline float edge(const ScreenVertex &a, const ScreenVertex &b, float px,
                  float py) {
  return (b.x - a.x) * (py - a.y) - (b.y - a.y) * (px - a.x);
}

template <class Shader>
void fillTriangle(FrameBuffer &fb, const ScreenVertex (&v)[3], Shader &&shade) {

  const float area = edge(v[0], v[1], v[2].x, v[2].y);
  if (std::fabs(area) < 1e-6f)
    return;
  const float invArea = 1.0f / area;

  const ScreenVertex *A = &v[0], *B = &v[1], *C = &v[2];
  if (A->y > B->y)
    std::swap(A, B);
  if (B->y > C->y)
    std::swap(B, C);
  if (A->y > B->y)
    std::swap(A, B);

  const int W = fb.width();
  const int H = fb.height();
  const int yStart = std::max(0, (int)std::ceil(A->y - 0.5f));
  const int yEnd = std::min(H - 1, (int)std::ceil(C->y - 0.5f) - 1);

  const float db0 = -(v[2].y - v[1].y) * invArea;
  const float db1 = -(v[0].y - v[2].y) * invArea;
  const float db2 = -(v[1].y - v[0].y) * invArea;
  const float dz = db0 * v[0].z + db1 * v[1].z + db2 * v[2].z;

  for (int y = yStart; y <= yEnd; ++y) {
    const float py = y + 0.5f;

    const float xLong = A->x + (C->x - A->x) * (py - A->y) / (C->y - A->y);
    const float xShort =
        (py < B->y) ? A->x + (B->x - A->x) * (py - A->y) / (B->y - A->y)
                    : B->x + (C->x - B->x) * (py - B->y) / (C->y - B->y);
    const float xl = std::min(xLong, xShort);
    const float xr = std::max(xLong, xShort);
    const int xs = std::max(0, (int)std::ceil(xl - 0.5f));
    const int xe = std::min(W - 1, (int)std::ceil(xr - 0.5f) - 1);
    if (xs > xe)
      continue;

    const float px = xs + 0.5f;
    float b0 = edge(v[1], v[2], px, py) * invArea;
    float b1 = edge(v[2], v[0], px, py) * invArea;
    float b2 = edge(v[0], v[1], px, py) * invArea;
    float z = b0 * v[0].z + b1 * v[1].z + b2 * v[2].z;

    QRgb *color = fb.colorRow(y);
    float *depth = fb.depthRow(y);
    for (int x = xs; x <= xe; ++x) {
      if (z > depth[x]) {
        depth[x] = z;
        color[x] = shade(b0, b1, b2);
      }
      b0 += db0;
      b1 += db1;
      b2 += db2;
      z += dz;
    }
  }
}
} // namespace render