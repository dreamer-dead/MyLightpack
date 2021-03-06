#include <QDir>

namespace QtUtils {
QString pathCombine(const QString& path1, const QString& path2) {
    return QDir::cleanPath(path1 + QDir::separator() + path2);
}
}
