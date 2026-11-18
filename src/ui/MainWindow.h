#pragma once

#include "model/Scene.h"
#include <QMainWindow>

class CanvasWidget;

class MainWindow : public QMainWindow {
public:
  explicit MainWindow(model::Scene &scene, QWidget *parent = nullptr);

private:
  QWidget *createControlPanel();
  model::Scene &scene_;
  CanvasWidget *canvas_;
};
