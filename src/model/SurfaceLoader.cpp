
#include "SurfaceLoader.h"
#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <array>
#include <vector>
namespace model {
namespace {
bool fail(QString *error, const QString &message) {
  if (error)
    *error = message;
  return false;
}
} // namespace
bool loadSurfaces(const QString &path, std::vector<BezierSurface> &out,
                  QString *error) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return fail(error, QString("Nie mozna otworzyć pliku \"%1\": %2")
                           .arg(path, file.errorString()));
  std::vector<BezierSurface> surfaces;
  std::array<QVector3D, 16> pts;
  int count = 0;
  QTextStream in(&file);
  int lineNo = 0;
  while (!in.atEnd()) {
    const QString line = in.readLine().trimmed();
    ++lineNo;

    if (line.isEmpty())
      continue;
    const QStringList parts = line.simplified().split(' ');
    if (parts.size() != 3)
      return fail(error,
                  QString("Linia %1: oczekiwano 3 liczb (x y z), jest %2.")
                      .arg(lineNo)
                      .arg(parts.size()));
    float c[3];
    for (int k = 0; k < 3; k++) {
      bool ok = false;
      c[k] = parts[k].toFloat(&ok);
      if (!ok)
        return fail(error, QString("Linia %1: \"%2\" nie jest liczbą.")
                               .arg(lineNo)
                               .arg(parts[k]));
    }
    pts[count++] = QVector3D(c[0], c[1], c[2]);
    if (count == 16) {
      surfaces.emplace_back(pts);
      count = 0;
    }
  }

  if (count != 0)
    return fail(
        error, QString("Niepełna powierzchnia na końcu pliku: %1 z 16 punktów.")
                   .arg(count));
  if (surfaces.size() < 2)
    return fail(
        error,
        QString("Plik zawiera %1 powierzchni, wymagane są co najmniej 2.")
            .arg(surfaces.size()));

  out = std::move(surfaces);
  return true;
}
} // namespace model