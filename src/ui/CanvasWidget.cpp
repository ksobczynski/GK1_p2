#include "CanvasWidget.h"
#include "model/Triangle.h"
#include "render/Rasterizer.h"
#include <QMouseEvent>
#include <QPainter>
#include <algorithm>

namespace {
constexpr float kSceneRadius = 1.75f;
constexpr float kDegreesPerPixel = 0.5f;
} // namespace
CanvasWidget::CanvasWidget(model::Scene &scene, QWidget *parent)
    : QWidget(parent), scene_(scene) {
  setCursor(Qt::OpenHandCursor);
}
QPointF CanvasWidget::toScreen(const QVector3D &r) const {
  return {r.x() * scale_, r.y() * scale_};
}

void CanvasWidget::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.fillRect(rect(), Qt::white);
  painter.setRenderHint(QPainter::Antialiasing);
  scale_ = 0.48f * std::min(width(), height()) / kSceneRadius;
  if (showFilled_) {
    drawFilled();
    painter.drawImage(0, 0, fb_.image());
  }
  painter.translate(width() / 2.0, height() / 2.0);
  painter.scale(1, -1);

  if (showWireframe_)
    drawWireframe(painter);
  if (showControlPolygon_)
    drawControlPolygon(painter);
}

void CanvasWidget::drawWireframe(QPainter &painter) const {
  painter.setPen(QPen(QColor(90, 90, 90), 1));
  for (const model::Mesh &mesh : scene_.meshes()) {
    for (const model::Triangle &t : mesh.tris) {
      const QPointF a = toScreen(t.v[0].Pr);
      const QPointF b = toScreen(t.v[1].Pr);
      const QPointF c = toScreen(t.v[2].Pr);
      painter.drawLine(a, b);
      painter.drawLine(b, c);
      painter.drawLine(c, a);
    }
  }
}
void CanvasWidget::drawControlPolygon(QPainter &painter) const {
  const QMatrix4x4 &R = scene_.rotationMatrix();
  for (const model::BezierSurface &s : scene_.surfaces()) {
    QPointF pts[4][4];
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        pts[i][j] = toScreen(R.map(s.control(i, j)));

    painter.setPen(QPen(QColor(220, 40, 40), 1.5));
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        if (i + 1 < 4)
          painter.drawLine(pts[i][j], pts[i + 1][j]);
        if (j + 1 < 4)
          painter.drawLine(pts[i][j], pts[i][j + 1]);
      }
    }
    painter.setBrush(QColor(220, 40, 40));
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        painter.drawEllipse(pts[i][j], 3.5, 3.5);
    painter.setBrush(Qt::NoBrush);
  }
}

void CanvasWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    dragging_ = true;
    lastMousePos_ = event->position();
    setCursor(Qt::ClosedHandCursor);
  }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event) {
  if (!dragging_)
    return;

  const QPointF d = event->position() - lastMousePos_;
  lastMousePos_ = event->position();

  const float alpha = d.y() * kDegreesPerPixel;
  const float beta = d.x() * kDegreesPerPixel;

  scene_.rotate(alpha, beta);
  update();
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    dragging_ = false;
    setCursor(Qt::OpenHandCursor);
  }
}

void CanvasWidget::drawFilled() {
  fb_.resize(width(), height());
  fb_.clear(qRgb(255, 255, 255));
  const float cx = width() / 2.0f, cy = height() / 2.0f;

  for (const model::Mesh &mesh : scene_.meshes())
    for (const model::Triangle &t : mesh.tris) {
      render::ScreenVertex sv[3];
      for (int i = 0; i < 3; i++) {
        const QVector3D &p = t.v[i].Pr;
        sv[i] = {cx + p.x() * scale_, cy - p.y() * scale_, p.z()};
      }
      render::fillTriangle(fb_, sv, [&](float b0, float b1, float b2) {
        QVector3D n =
            (b0 * t.v[0].Nr + b1 * t.v[1].Nr + b2 * t.v[2].Nr).normalized();
        int g = int(255 * std::max(0.0f, n.z()));
        return qRgb(g, g, g);
      });
    }
}