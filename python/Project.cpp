#include "Project.h"
#include "Exception.h"
#include <QDir>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

Pro Project::createProject(const QString& projName, const QString& basePath, bool isEmpty)
{
    // 参数检查
    if (projName.isEmpty()) {
        throw InvalidPath("项目名称不能为空");
    }

    if (basePath.isEmpty()) {
        throw InvalidPath("基础路径不能为空");
    }

    // 检查项目是否已存在
    if (isFindProject(projName, basePath)) {
        throw FileAlreadyExists(QString("项目已存在: %1").arg(projName));
    }

    createDir(basePath, projName);
    QString proPath = QDir(basePath).filePath(projName);
    createHiddenFolder(proPath, ".pyproject");
    QString srcPath = createDir(proPath, "src");
    QString route = QDir(srcPath).filePath("main.py");
    QFile file(route);
    if (!file.open(QIODevice::WriteOnly)) {
        throw CreateFileFail(QString("创建文件失败: %1").arg(route));
    }
    if (!isEmpty) {
        file.write(R"(print('Hello Python!'))");
    }
        
    file.close();

    // 返回 Pro 结构体
    return Pro(proPath, route, srcPath);
}

QString Project::findProjectRoot(const QString& filePath)
{
    if (filePath.isEmpty()) {
        throw InvalidPath("文件路径不能为空");
    }

    auto currPath = filePath;
    while (!currPath.isEmpty()) {
        if (isFindProFile(currPath)) {
            return currPath; // 如果上级找到该目录下有标志文件，则立刻返回该路径
        }
        currPath = getUpper(currPath);
    }
    throw InvalidProject(QString("无效的项目，未找到.pyproject文件: %1").arg(filePath));
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
    // 参数检查
    if (parentPath.isEmpty()) {
        throw InvalidPath("父路径不能为空");
    }

    if (dirName.isEmpty()) {
        throw InvalidPath("目录名称不能为空");
    }

    QString path = QDir(parentPath).filePath(dirName);
    QDir dir;

    if (dir.mkdir(path)) {
        return path;
    }
    else {
        throw DirectoryCreationFailed(QString("创建目录失败: %1").arg(path));
    }
}

QString Project::createHiddenFolder(const QString& parentPath, const QString& folderName)
{
    // 参数检查
    if (parentPath.isEmpty()) {
        throw InvalidPath("父路径不能为空");
    }

    if (folderName.isEmpty()) {
        throw InvalidPath("文件夹名称不能为空");
    }

    QFileInfo parentInfo(parentPath);
    if (!parentInfo.exists()) {
        throw FileNotFound(QString("路径不存在: %1").arg(parentPath));
    }

    if (!parentInfo.isDir()) {
        throw NotADirectory(QString("不是文件夹: %1").arg(parentPath));
    }

    QString targetPath = QDir(parentPath).filePath(folderName);
    QDir dir;

    if (!dir.mkpath(targetPath)) {
        throw DirectoryCreationFailed(QString("创建文件夹失败: %1").arg(targetPath));
    }

#ifdef Q_OS_WIN
    if (!SetFileAttributes(reinterpret_cast<LPCWSTR>(targetPath.utf16()), FILE_ATTRIBUTE_HIDDEN)) {
        throw HiddenAttributeFailed(QString("设置隐藏属性失败: %1").arg(targetPath));
    }
#else
    QFileInfo createdInfo(targetPath);
    QString parent = createdInfo.absolutePath();
    QString hiddenPath = parent + "/." + folderName;

    if (!QDir().rename(targetPath, hiddenPath)) {
        throw RenameFailed(QString("重命名为隐藏文件夹失败: %1").arg(targetPath));
    }
    targetPath = hiddenPath;
#endif

    return targetPath;
}

QString Project::getUpper(const QString& path)
{
    if (path.isEmpty()) {
        logWarn("getParentDirectory: 输入路径为空");
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
    if (basePath.isEmpty() || proName.isEmpty()) {
        return false;
    }

    QDir baseDir(basePath);
    QString targetFullPath = baseDir.filePath(proName);

    return QFileInfo::exists(targetFullPath) && QFileInfo(targetFullPath).isDir();
}