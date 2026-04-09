#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <QSharedPointer>
#include <type_traits>
#include "core/objects/runtime/pobject.h"
#include <qdir.h>

using PObject = Py::PObject;
// utils begin
template <typename T>
constexpr bool is_pobject_v = std::is_base_of_v<PObject, T>;
template <typename T, typename... Args>
std::enable_if_t<is_pobject_v<T>, QSharedPointer<T>>
makeShared(Args&&... args) {
    auto obj = QSharedPointer<T>::create(std::forward<Args>(args)...);
    obj->init();
    return obj;
}
template <typename T, typename... Args>
std::enable_if_t<!is_pobject_v<T>, QSharedPointer<T>>
makeShared(Args&&... args) {
    return QSharedPointer<T>::create(std::forward<Args>(args)...);
}

//template <typename T, typename... Args>
//QSharedPointer<T> makeShared(Args&&... args) {
//    return QSharedPointer<T>::create(std::forward<Args>(args)...);
//}
//template <typename Base, typename Derived, typename... Args>
//QSharedPointer<Base> makeShared(Args&&... args) {
//    return QSharedPointer<Base>(new Derived(std::forward<Args>(args)...));
//}
template <typename Target, typename Source>
QSharedPointer<Target> dynamicPointerCast(const QSharedPointer<Source>& source)
{
    return qSharedPointerDynamicCast<Target>(source);
//    if (source.isNull()) {
//        return QSharedPointer<Target>();
//    }

//    Target* rawTarget = qobject_cast<Target*>(source.data());
//    if (rawTarget == nullptr) {
//        return QSharedPointer<Target>();
//    }

//    return QSharedPointer<Target>(source, rawTarget); // 这里的构造注意
}
// string cast. UTF-8 is the only storage type
inline std::string qt2std(const QString& qStr)
{
    QByteArray utf8Bytes = qStr.toUtf8();
    return std::string(utf8Bytes.constData(), utf8Bytes.size());
}

inline QString std2qt(const std::string& sStr)
{
    return QString::fromUtf8(sStr.data(), static_cast<int>(sStr.size()));
}
static QString findFile(const QString& searchPath, const QString& baseName, const QString& suffix)
{
    QDir directory(searchPath);

    if (!directory.exists()) {
        return QString();
    }

    QString fileName = baseName + suffix;
    QString fullPath = directory.absoluteFilePath(fileName);

    QFileInfo info(fullPath);
    if (info.exists() && info.isFile()) {
        return fullPath;
    }

    return QString();
}
static QStringList getAllFilesAbsolutePath(const QString& directoryPath)
{
    QDir directory(directoryPath);

    if (!directory.exists()) {
        return QStringList();
    }

    directory.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    QStringList absolutePaths;
    QStringList fileNames = directory.entryList();

    for (const QString& fileName : fileNames) {
        absolutePaths.append(directory.absoluteFilePath(fileName));
    }

    return absolutePaths;
}
// utils end

#endif // FUNCTIONS_H
