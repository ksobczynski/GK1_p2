#pragma once
#include "model/LightCurve.h"
#include "model/Scene.h"
#include "render/FrameBuffer.h"
#include "render/Lighting.h"
#include <QMatrix4x4>
#include <QTimer>
#include <QWidget>

class CanvasWidget : public QWidget {
public:
  explicit CanvasWidget(model::Scene &scene, QWidget *parent = nullptr);

  bool showControlPolygon() const { return showControlPolygon_; }
  bool showWireframe() const { return showWireframe_; }
  void setShowControlPolygon(bool on) {
    showControlPolygon_ = on;
    update();
  }
  void setShowWireframe(bool on) {
    showWireframe_ = on;
    update();
  }
  bool showFilled() const { return showFilled_; }
  void setShowFilled(bool on) {
    showFilled_ = on;
    update();
  }
  render::Lighting &lighting() { return lighting_; }
  model::LightCurve &lightCurve() { return curve_; }
  void setAnimating(bool on) {
    if (on)
      timer_.start();
    else
      timer_.stop();
  }

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  void drawFilled();
  render::FrameBuffer fb_;
  bool showFilled_ = true;
  void drawWireframe(QPainter &painter) const;
  void drawControlPolygon(QPainter &painter) const;
  QPointF toScreen(const QVector3D &rotated) const;

  model::Scene &scene_;
  float scale_ = 1.0f;
  QPointF lastMousePos_;
  bool dragging_ = false;
  bool showControlPolygon_ = true;
  bool showWireframe_ = true;
  render::Lighting lighting_;
  void drawLightCurve(QPainter &painter) const;
  QPointF toScreen(const QPointF &p) const { return p * scale_; }
  QPointF toScene(const QPointF &widgetPos) const;
  model::LightCurve curve_;
  float lightS_ = 0;
  QTimer timer_;
  int dragPoint_ = -1;
};