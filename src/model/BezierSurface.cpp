#include "BezierSurface.h"
#include <algorithm>

namespace model {
namespace {
std::array<float, 4> bernstein(float t) {
  const float s = 1 - t;
  return {s * s * s,     // B0 = (1-t)^3
          3 * t * s * s, // B1 = 3t(1-t)^2
          3 * t * t * s, // B2 = 3t^2(1-t)
          t * t * t};    // B3 = t^3
}

std::array<float, 4> bernsteinDerivative(float t) {
  const float s = 1 - t;
  return {-3 * s * s,            // B0' = -3(1-t)^2
          3 * s * s - 6 * t * s, // B1' = 3(1-t)^2 - 6t(1-t)
          6 * t * s - 3 * t * t, // B2' = 6t(1-t) - 3t^2
          3 * t * t};            // B3' = 3t^2
}
} // namespace
BezierSurface::BezierSurface(const std::array<QVector3D, 16> &pts) {
  for (int k = 0; k < 16; k++) {
    P[k / 4][k % 4] = pts[k];
  }
}

const QVector3D &BezierSurface::control(int i, int j) const { return P[i][j]; }

QVector3D BezierSurface::point(float u, float v) const {
  std::array<float, 4> bu = bernstein(u);
  std::array<float, 4> bv = bernstein(v);
  QVector3D res{};
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      res += P[i][j] * bu[i] * bv[j];
    }
  }
  return res;
}

QVector3D BezierSurface::du(float u, float v) const {
  const auto bu = bernsteinDerivative(u);
  const auto bv = bernstein(v);
  QVector3D res{};
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      res += P[i][j] * bu[i] * bv[j];
    }
  }
  return res;
}

QVector3D BezierSurface::dv(float u, float v) const {
  const auto bu = bernstein(u);
  const auto bv = bernsteinDerivative(v);
  QVector3D res{};
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      res += P[i][j] * bu[i] * bv[j];
    }
  }
  return res;
}

Mesh BezierSurface::triangulate(int n) const {
  n = std::max(1, n);
  const int side = n + 1;
  std::vector<Vertex> grid;
  grid.reserve(side * side);
  for (int i = 0; i <= n; i++) {
    for (int j = 0; j <= n; j++) {
      Vertex vx;
      vx.u = static_cast<float>(i) / n;
      vx.v = static_cast<float>(j) / n;
      vx.P = point(vx.u, vx.v);
      vx.Pu = du(vx.u, vx.v);
      vx.Pv = dv(vx.u, vx.v);
      vx.N = QVector3D::crossProduct(vx.Pu, vx.Pv).normalized();

      vx.Pr = vx.P;
      vx.Pur = vx.Pu;
      vx.Pvr = vx.Pv;
      vx.Nr = vx.N;

      grid.push_back(vx);
    }
  }
  auto at = [&](int i, int j) -> const Vertex & { return grid[i * side + j]; };

  Mesh mesh;
  mesh.tris.reserve(2 * n * n);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      const Vertex &A = at(i, j);
      const Vertex &B = at(i + 1, j);
      const Vertex &C = at(i + 1, j + 1);
      const Vertex &D = at(i, j + 1);
      mesh.tris.push_back({{A, B, C}});
      mesh.tris.push_back({{A, C, D}});
    }
  }
  return mesh;
}
} // namespace model