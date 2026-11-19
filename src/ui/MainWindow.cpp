#include "MainWindow.h"
#include "CanvasWidget.h"
#include "render/Lighting.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QColorDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <functional>

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
  auto *fillCheck = new QCheckBox("Wypełnione trójkąty", showBox);

  auto *polygonCheck = new QCheckBox("Wielobok Béziera", showBox);
  auto *wireCheck = new QCheckBox("Siatka trójkątów", showBox);
  polygonCheck->setChecked(canvas_->showControlPolygon());
  wireCheck->setChecked(canvas_->showWireframe());
  fillCheck->setChecked(canvas_->showFilled());

  connect(polygonCheck, &QCheckBox::toggled, canvas_,
          &CanvasWidget::setShowControlPolygon);
  connect(wireCheck, &QCheckBox::toggled, canvas_,
          &CanvasWidget::setShowWireframe);
  connect(fillCheck, &QCheckBox::toggled, canvas_,
          &CanvasWidget::setShowFilled);

  showLayout->addWidget(polygonCheck);
  showLayout->addWidget(wireCheck);
  showLayout->addWidget(fillCheck);

  layout->addWidget(triBox);
  layout->addWidget(showBox);

  auto addSlider = [this](QBoxLayout *lay, const QString &name, int min,
                          int max, int val, std::function<void(int)> onChange) {
    auto *label = new QLabel(QString("%1: %2").arg(name).arg(val));
    auto *s = new QSlider(Qt::Horizontal);
    s->setRange(min, max);
    s->setValue(val);
    connect(s, &QSlider::valueChanged, this, [=](int v) {
      label->setText(QString("%1: %2").arg(name).arg(v));
      onChange(v);
      canvas_->update();
    });
    lay->addWidget(label);
    lay->addWidget(s);
  };

  auto *lightBox = new QGroupBox("Oświetlenie", panel);
  auto *lightLayout = new QVBoxLayout(lightBox);
  render::Lighting &L = canvas_->lighting();
  addSlider(lightLayout, "kd (%)", 0, 100, int(L.kd * 100),
            [&L](int v) { L.kd = v / 100.0f; });
  addSlider(lightLayout, "ks (%)", 0, 100, int(L.ks * 100),
            [&L](int v) { L.ks = v / 100.0f; });
  addSlider(lightLayout, "m", 1, 100, int(L.m), [&L](int v) { L.m = v; });

  auto addColorButton = [this](QBoxLayout *lay, const QString &name,
                               QVector3D &target) {
    auto *b = new QPushButton(name);
    connect(b, &QPushButton::clicked, this, [this, &target] {
      QColor c = QColorDialog::getColor(
          QColor::fromRgbF(target.x(), target.y(), target.z()), this);
      if (!c.isValid()) // Anuluj
        return;
      target = QVector3D(c.redF(), c.greenF(), c.blueF());
      canvas_->update();
    });
    lay->addWidget(b);
  };
  addColorButton(lightLayout, "Kolor światła", L.lightColor);
  addColorButton(lightLayout, "Kolor obiektu", L.objectColor);

  layout->addWidget(lightBox);

  auto *srcBox = new QGroupBox("Źródło światła", panel);
  auto *srcLayout = new QVBoxLayout(srcBox);

  auto *animCheck = new QCheckBox("Animacja", srcBox);
  animCheck->setChecked(true);
  connect(animCheck, &QCheckBox::toggled, canvas_, &CanvasWidget::setAnimating);
  srcLayout->addWidget(animCheck);

  model::LightCurve &C = canvas_->lightCurve();
  addSlider(srcLayout, "z (×0.1)", -30, 30, int(C.z * 10),
            [&C](int v) { C.z = v / 10.0f; });
  layout->addWidget(srcBox);

  layout->addStretch();
  return panel;
}