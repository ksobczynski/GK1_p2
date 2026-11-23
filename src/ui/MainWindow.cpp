#include "MainWindow.h"
#include "CanvasWidget.h"
#include "render/Lighting.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
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
  auto *fillCheck = new QCheckBox("Wypełnione trójkąty (F)", showBox);
  fillCheck->setShortcut(Qt::Key_F);

  auto *polygonCheck = new QCheckBox("Wielobok Béziera (B)", showBox);
  polygonCheck->setShortcut(Qt::Key_B);
  auto *wireCheck = new QCheckBox("Siatka trójkątów (S)", showBox);
  wireCheck->setShortcut(Qt::Key_S);
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

  auto *ioBox = new QGroupBox("Kolor obiektu IO", panel);
  auto *ioLayout = new QVBoxLayout(ioBox);
  auto *colorRadio = new QRadioButton("Jednolity kolor (C)", ioBox);
  auto *texRadio = new QRadioButton("Tekstura (T)", ioBox);
  colorRadio->setShortcut(Qt::Key_C);
  texRadio->setShortcut(Qt::Key_T);
  colorRadio->setChecked(!L.useTexture);
  texRadio->setChecked(L.useTexture);
  connect(texRadio, &QRadioButton::toggled, this, [this, &L](bool on) {
    L.useTexture = on;
    canvas_->update();
  });

  auto addImageButton = [this](QBoxLayout *lay, const QString &name,
                               QImage &target, QAbstractButton *enable) {
    auto *b = new QPushButton(name);
    connect(b, &QPushButton::clicked, this, [this, name, &target, enable] {
      const QString path = QFileDialog::getOpenFileName(
          this, name, "data", "Obrazy (*.png *.jpg *.jpeg *.bmp)");
      if (path.isEmpty())
        return;
      if (!render::loadImage(target, path)) {
        QMessageBox::warning(this, "Błąd", "Nie można wczytać: " + path);
        return;
      }
      enable->setChecked(true);
      canvas_->update();
    });
    lay->addWidget(b);
  };

  ioLayout->addWidget(colorRadio);
  ioLayout->addWidget(texRadio);
  addImageButton(ioLayout, "Wczytaj teksturę…", L.texture, texRadio);
  layout->addWidget(ioBox);

  auto *nmBox = new QGroupBox("Mapa normalnych", panel);
  auto *nmLayout = new QVBoxLayout(nmBox);
  auto *nmCheck = new QCheckBox("Włączona (N)", nmBox);
  nmCheck->setChecked(L.useNormalMap);
  nmCheck->setShortcut(Qt::Key_N);
  connect(nmCheck, &QCheckBox::toggled, this, [this, &L](bool on) {
    L.useNormalMap = on;
    canvas_->update();
  });
  nmLayout->addWidget(nmCheck);
  addImageButton(nmLayout, "Wczytaj mapę normalnych…", L.normalMap, nmCheck);
  layout->addWidget(nmBox);

  auto *srcBox = new QGroupBox("Źródło światła", panel);
  auto *srcLayout = new QVBoxLayout(srcBox);

  auto *animCheck = new QCheckBox("Animacja (A)", srcBox);
  animCheck->setChecked(true);
  animCheck->setShortcut(Qt::Key_A);
  connect(animCheck, &QCheckBox::toggled, canvas_, &CanvasWidget::setAnimating);
  srcLayout->addWidget(animCheck);

  model::LightCurve &C = canvas_->lightCurve();
  addSlider(srcLayout, "z (×0.1)", -30, 30, int(C.z * 10),
            [&C](int v) { C.z = v / 10.0f; });
  layout->addWidget(srcBox);

  layout->addStretch();
  auto *resetButton = new QPushButton("Resetuj widok (R)", panel);
  resetButton->setShortcut(Qt::Key_R);
  connect(resetButton, &QPushButton::clicked, this, [this] {
    scene_.resetOrientation();
    canvas_->update();
  });
  layout->addWidget(resetButton);
  auto *helpButton = new QPushButton("Pomoc (F1)", panel);
  helpButton->setShortcut(Qt::Key_F1);
  connect(helpButton, &QPushButton::clicked, this, &MainWindow::showHelp);
  layout->addWidget(helpButton);
  return panel;
}
void MainWindow::showHelp() {
  QMessageBox::information(this, "Pomoc", R"(<h3>Sterowanie myszą</h3>
<ul>
<li><b>Przeciąganie lewym przyciskiem</b> w obszarze rysowania – obrót sceny.</li>
<li><b>Przeciąganie punktu kontrolnego krzywej światła</b> (pomarańczowe kropki) – edycja krzywej.
Przesunięcie punktu łączenia przesuwa też jego uchwyty; przesunięcie uchwytu ustawia drugi uchwyt symetrycznie (krzywa pozostaje gładka, C1).</li>
</ul>
<h3>Skróty klawiszowe</h3>
<table>
<tr><td><b>F1</b></td><td>pomoc</td></tr>
<tr><td><b>R</b></td><td>reset obrotu do widoku początkowego</td></tr>
<tr><td><b>B</b></td><td>wielobok Béziera</td></tr>
<tr><td><b>S</b></td><td>siatka trójkątów</td></tr>
<tr><td><b>F</b></td><td>wypełnione trójkąty</td></tr>
<tr><td><b>C</b> / <b>T</b></td><td>kolor obiektu: jednolity / tekstura</td></tr>
<tr><td><b>N</b></td><td>mapa normalnych</td></tr>
<tr><td><b>A</b></td><td>animacja światła</td></tr>
</table>
<h3>Ruch myszy → obrót</h3>
<p>Orientacja sceny jest pamiętana jako macierz obrotu R (bez ograniczeń kątów).
Przy każdym ruchu myszy liczymy przesunięcie kursora (dx, dy) od <i>poprzedniej</i> pozycji, nie od miejsca kliknięcia.</p>
<ul>
<li>ruch pionowy dy → obrót wokół osi x ekranu o kąt α = 0.5° · dy,</li>
<li>ruch poziomy dx → obrót wokół osi y ekranu o kąt β = 0.5° · dx.</li>
</ul>
<p>Przyrost R<sub>przyrost</sub> = R<sub>x</sub>(α) · R<sub>y</sub>(β) składamy z bieżącą orientacją:
R<sub>nowa</sub> = R<sub>przyrost</sub> · R<sub>aktualna</sub>.
Mnożenie przyrostu z lewej strony oznacza obrót względem osi <i>ekranu</i>, więc przeciągnięcie w danym kierunku
zawsze obraca bryłę w tę samą stronę, niezależnie od jej aktualnego położenia
(bryła „podąża” za kursorem jak kula toczona ręką).
Następnie wszystkie wierzchołki siatki (P, Pu, Pv, N) są mnożone przez R; N jest normalizowany.</p>)");
}
