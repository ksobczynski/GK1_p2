#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QWidget>

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

  QWidget w;
  w.show();
  return app.exec();
}
