#include "Project.h"
#include <QDir>
#include <qdebug.h>

Pro Project::createProject(const QString& projName, const QString& basePath)
{
    if (isFindProject(projName, basePath)) {
        throw std::runtime_error("该项目已存在");
    }

    try {
        createDir(basePath, projName);
        QString proPath = QDir(basePath).filePath(projName);
        createHiddenFolder(proPath, ".pyproject");
        QString srcPath = createDir(proPath, "src");

        // 返回 Pro 结构体
        return Pro(proPath, QString(), srcPath);
    }
    catch (...) {
        throw std::runtime_error("创建项目失败");
    }
}

QString Project::findProjectRoot(const QString& filePath)
{
    auto currPath = filePath;
    while (!currPath.isEmpty()) {
        if (isFindProFile(currPath)) {
            return currPath; // 如果上级找到该目录下有标志文件，则立刻返回该路径
        }
        currPath = getUpper(currPath);
    }
    throw std::runtime_error("无效的项目");
}

QString Project::getSrcPath(const QString& basePath)
{
    if (basePath.isEmpty()) {
        return QString();
    }

    QDir dir(basePath);

    if (!dir.exists()) {
        return QString();
    }

    QString fullPath = dir.absoluteFilePath("src");
    QFileInfo fileInfo(fullPath);

    if (fileInfo.exists() && fileInfo.isDir()) {
        return fileInfo.absoluteFilePath();
    }

    return QString();
}

QString Project::createDir(const QString& parentPath, const QString& dirName)
{
    QString path = QDir(parentPath).filePath(dirName);
    QDir dir;

    if (dir.mkdir(path)) {
        qDebug() << "创建成功:" << path;
        return path;
    }
    else {
        throw std::runtime_error(QString("创建目录失败: %1").arg(path).toStdString());
    }
}

inline QString Project::createHiddenFolder(const QString& parentPath, const QString& folderName)
{
    QFileInfo parentInfo(parentPath);
    if (!parentInfo.exists()) {
        throw std::runtime_error(QString("路径不存在: %1").arg(parentPath).toStdString());
    }

    if (!parentInfo.isDir()) {
        throw std::runtime_error(QString("不是文件夹: %1").arg(parentPath).toStdString());
    }

    QString targetPath = QDir(parentPath).filePath(folderName);
    QDir dir;

    if (!dir.mkpath(targetPath)) {
        throw std::runtime_error(QString("创建文件夹失败: %1").arg(targetPath).toStdString());
    }

#ifdef Q_OS_WIN
    if (!SetFileAttributes(reinterpret_cast<LPCWSTR>(targetPath.utf16()), FILE_ATTRIBUTE_HIDDEN)) {
        throw std::runtime_error(QString("设置隐藏属性失败: %1").arg(targetPath).toStdString());
    }
#else
    QFileInfo createdInfo(targetPath);
    QString parent = createdInfo.absolutePath();
    QString hiddenPath = parent + "/." + folderName;

    if (!QDir().rename(targetPath, hiddenPath)) {
        throw std::runtime_error(QString("重命名为隐藏文件夹失败: %1").arg(targetPath).toStdString());
    }
    targetPath = hiddenPath;
#endif

    qDebug() << "隐藏文件夹创建成功:" << targetPath;
    return targetPath;
}

QString Project::getUpper(const QString& path)
{
    if (path.isEmpty()) {
        qWarning() << "getParentDirectory: 输入路径为空";
        return QString();
    }

    QFileInfo fileInfo(path);
    QString absolutePath = fileInfo.absolutePath();

#ifdef Q_OS_WIN
    if (absolutePath == "/" || absolutePath == "\\" ||
        absolutePath.endsWith(":/") || absolutePath.endsWith(":\\") ||
        (absolutePath.length() == 2 && absolutePath[1] == ':')) {
        return QString();
    }
#endif

#ifdef Q_OS_UNIX
    if (absolutePath == "/") {
        return QString();
    }
#endif

    QDir dir(absolutePath);
    QString parentPath = dir.filePath("..");

    QFileInfo parentInfo(parentPath);
    QString canonicalParent = parentInfo.canonicalFilePath();

    if (canonicalParent.isEmpty() || canonicalParent == absolutePath) {
        return QString();
    }

    return canonicalParent;
}

bool Project::isFindProFile(const QString& basePath)
{
    if (basePath.isEmpty()) {
        return false;
    }

    QDir dir(basePath);
    return dir.exists(".pyproject");
}

bool Project::isFindProject(const QString& proName, const QString& basePath)
{
    if (basePath.isEmpty()) {
        return false;
    }

    QDir baseDir(basePath);

    QString targetFullPath = baseDir.filePath(proName);

    return QFileInfo::exists(targetFullPath) && QFileInfo(targetFullPath).isDir();
}
