#include "model/Scene.h"
#include "ui/MainWindow.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QWidget>
#include <cstddef>
#include <iostream>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QCommandLineParser parser;
  parser.setApplicationDescription("Powierzchnie Beziera");
  parser.addHelpOption();
  QCommandLineOption fileOpt({"f", "file"}, "Plik z punktami kontrolnymi.",
                             "plik", "data/surfaces.txt");
  parser.addOption(fileOpt);
  parser.process(app);
  QString path = parser.value(fileOpt);

  model::Scene scene{};
  QString e;
  if (!scene.load(path, &e)) {
    QMessageBox::critical(nullptr, "Błąd", e);
    return 1;
  }

  MainWindow window(scene);
  window.show();
  return app.exec();
}
