
#pragma once

#include "model/Scene.h"
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

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  model::Scene &scene_;
  bool showControlPolygon_ = true;
  bool showWireframe_ = true;
};