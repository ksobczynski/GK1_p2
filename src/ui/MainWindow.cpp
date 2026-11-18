#include "MainWindow.h"
#include "CanvasWidget.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

MainWindow::MainWindow(model::Scene &scene, QWidget *parent)
    : QMainWindow(parent), scene_(scene),
      canvas_(new CanvasWidget(scene, this)) {
  setWindowTitle("Powierzchnie Beziera");
  resize(1200, 900);
  auto *central = new QWidget(this);
  auto *layout = new QHBoxLayout(central);
  layout->addWidget(canvas_, 1);
  layout->addWidget(createControlPanel(), 0);
  setCentralWidget(central);
}
QWidget *MainWindow::createControlPanel() {
  auto *panel = new QWidget(this);
  panel->setFixedWidth(260);
  auto *layout = new QVBoxLayout(panel);

  auto *triBox = new QGroupBox("Triangulacja", panel);
  auto *triLayout = new QVBoxLayout(triBox);

  auto *resLabel = new QLabel(triBox);
  auto *resSlider = new QSlider(Qt::Horizontal, triBox);
  resSlider->setRange(1, 50);
  resSlider->setValue(scene_.resolution());
  resLabel->setText(QString("Dokładność: %1").arg(scene_.resolution()));

  connect(resSlider, &QSlider::valueChanged, this, [this, resLabel](int n) {
    resLabel->setText(QString("Dokładność: %1").arg(n));
    scene_.setResolution(n);
    canvas_->update();
  });

  triLayout->addWidget(resLabel);
  triLayout->addWidget(resSlider);

  auto *showBox = new QGroupBox("Wyświetlanie", panel);
  auto *showLayout = new QVBoxLayout(showBox);

  auto *polygonCheck = new QCheckBox("Wielobok Béziera", showBox);
  auto *wireCheck = new QCheckBox("Siatka trójkątów", showBox);
  polygonCheck->setChecked(canvas_->showControlPolygon());
  wireCheck->setChecked(canvas_->showWireframe());

  connect(polygonCheck, &QCheckBox::toggled, canvas_,
          &CanvasWidget::setShowControlPolygon);
  connect(wireCheck, &QCheckBox::toggled, canvas_,
          &CanvasWidget::setShowWireframe);

  showLayout->addWidget(polygonCheck);
  showLayout->addWidget(wireCheck);

  layout->addWidget(triBox);
  layout->addWidget(showBox);
  layout->addStretch();
  return panel;
}