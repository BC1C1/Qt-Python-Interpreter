#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <QSharedPointer>
#include <type_traits>
#include "core/objects/runtime/pobject.h"
#include <qdir.h>
#include "Logger.h"
#include <qmainwindow.h>
#include "SecondaryWindow.h"

#define COMPILE_DEBUG

using PObject = Py::PObject;
// utils begin
template <typename T>
constexpr bool is_pobject_v = std::is_base_of_v<PObject, T>;

template <typename T>
constexpr bool is_mainwindow_v = std::is_base_of_v<QMainWindow, T>;

template <typename T, typename... Args>
std::enable_if_t<is_pobject_v<T> && !is_mainwindow_v<T>, QSharedPointer<T>>
makeShared(Args&&... args) {
    auto obj = QSharedPointer<T>::create(std::forward<Args>(args)...);
    obj->init();
    return obj;
}

template <typename T, typename... Args>
std::enable_if_t<is_mainwindow_v<T> && !is_pobject_v<T>, QSharedPointer<T>>
makeShared(Args&&... args) {
    auto obj = QSharedPointer<T>::create(std::forward<Args>(args)...);
    obj->initAll();
    return obj;
}

template <typename T, typename... Args>
std::enable_if_t<!is_mainwindow_v<T> && !is_pobject_v<T>, QSharedPointer<T>>
makeShared(Args&&... args) {
    return QSharedPointer<T>::create(std::forward<Args>(args)...);
}

template <typename T>
constexpr bool isSecondaryWindow_v = std::is_base_of_v<SecondaryWindow, T>;

template <typename T, typename... Args>
std::enable_if_t<isSecondaryWindow_v<T>, T*>
newWindow(Args&&... args) {
    auto obj = new T(std::forward<Args>(args)...);
    obj->connectToMainWindow();
    return obj;
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
inline static QString findFile(const QString& searchPath, const QString& baseName, const QString& suffix)
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
inline static QStringList getAllFilesAbsolutePath(const QString& directoryPath)
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
