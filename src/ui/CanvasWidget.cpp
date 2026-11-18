#include "CanvasWidget.h"
#include <QPainter>

CanvasWidget::CanvasWidget(model::Scene &scene, QWidget *parent)
    : QWidget(parent), scene_(scene) {}

void CanvasWidget::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.fillRect(rect(), Qt::white);
  p.drawText(rect(), Qt::AlignCenter,
             QString("Siatek: %1").arg(scene_.meshes().size()));
}