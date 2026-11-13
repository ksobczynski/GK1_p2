#pragma once

#include "model/BezierSurface.h"
#include <QString>
#include <vector>
namespace model {
bool loadSurfaces(const QString &path, std::vector<BezierSurface> &out,
                  QString *Error);
}