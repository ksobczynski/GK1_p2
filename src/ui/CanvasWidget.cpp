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
  render::loadImage(lighting_.texture, "data/texture.png");
  render::loadImage(lighting_.normalMap, "data/normalmap.png");

  timer_.setInterval(16);
  connect(&timer_, &QTimer::timeout, this, [this] {
    lightS_ += 0.002f;
    if (lightS_ >= 1)
      lightS_ -= 1;
    update();
  });
  timer_.start();
}
QPointF CanvasWidget::toScreen(const QVector3D &r) const {
  return {r.x() * scale_, r.y() * scale_};
}

void CanvasWidget::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.fillRect(rect(), Qt::white);
  painter.setRenderHint(QPainter::Antialiasing);
  scale_ = 0.48f * std::min(width(), height()) / kSceneRadius;
  const QPointF L = curve_.point(lightS_);
  lighting_.lightPos = QVector3D(L.x(), L.y(), curve_.z);
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
  drawLightCurve(painter);
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
  if (event->button() != Qt::LeftButton)
    return;
  dragging_ = true;
  lastMousePos_ = event->position();
  setCursor(Qt::ClosedHandCursor);

  const QPointF p = toScene(event->position());
  dragPoint_ = -1;
  for (int i = 0; i < int(curve_.V.size()); i++)
    if (QLineF(p, curve_.V[i]).length() * scale_ < 8) {
      dragPoint_ = i;
      break;
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event) {
  if (!dragging_)
    return;

  if (dragPoint_ >= 0) {
    curve_.moveControl(dragPoint_, toScene(event->position()));
    update();
    return;
  }

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
    dragPoint_ = -1;
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
        const QVector3D P = b0 * t.v[0].Pr + b1 * t.v[1].Pr + b2 * t.v[2].Pr;
        QVector3D N =
            (b0 * t.v[0].Nr + b1 * t.v[1].Nr + b2 * t.v[2].Nr).normalized();
        const float u = b0 * t.v[0].u + b1 * t.v[1].u + b2 * t.v[2].u;
        const float v = b0 * t.v[0].v + b1 * t.v[1].v + b2 * t.v[2].v;

        if (lighting_.useNormalMap && !lighting_.normalMap.isNull()) {
          const QVector3D Pu =
              (b0 * t.v[0].Pur + b1 * t.v[1].Pur + b2 * t.v[2].Pur)
                  .normalized();
          const QVector3D Pv =
              (b0 * t.v[0].Pvr + b1 * t.v[1].Pvr + b2 * t.v[2].Pvr)
                  .normalized();
          const QVector3D Nt =
              2 * render::textureColor(lighting_.normalMap, u, v) -
              QVector3D(1, 1, 1);
          N = (Pu * Nt.x() + Pv * Nt.y() + N * Nt.z()).normalized();
        }

        QVector3D IO = lighting_.objectColor;
        if (lighting_.useTexture && !lighting_.texture.isNull())
          IO = render::textureColor(lighting_.texture, u, v);
        return render::shadePhong(lighting_, P, N, IO);
      });
    }
}

void CanvasWidget::drawLightCurve(QPainter &painter) const {
  const QColor c(230, 160, 0);
  const int N = int(curve_.V.size());

  QPolygonF curve;
  for (int i = 0; i <= 200; i++)
    curve << toScreen(curve_.point(i / 200.0f));
  painter.setPen(QPen(c, 1.5));
  painter.drawPolyline(curve);

  painter.setPen(QPen(c, 1, Qt::DashLine));
  for (int i = 0; i < N; i++)
    painter.drawLine(toScreen(curve_.V[i]), toScreen(curve_.V[(i + 1) % N]));

  painter.setBrush(c);
  for (const QPointF &v : curve_.V)
    painter.drawEllipse(toScreen(v), 4, 4);

  painter.setPen(QPen(Qt::black, 1));
  painter.setBrush(Qt::yellow);
  painter.drawEllipse(toScreen(curve_.point(lightS_)), 7, 7);
  painter.setBrush(Qt::NoBrush);
}

QPointF CanvasWidget::toScene(const QPointF &s) const {
  return {(s.x() - width() / 2.0) / scale_, -(s.y() - height() / 2.0) / scale_};
}