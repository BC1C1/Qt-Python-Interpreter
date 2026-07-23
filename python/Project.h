#pragma once

#include <QString>
#include <stdexcept>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

struct Pro {
    Pro() {}
    Pro(const QString& proPath, const QString& currFilePath, const QString& srcPath) :
        proPath(proPath), currFilePath(currFilePath), srcPath(srcPath) 
    {
    }
    QString proPath; // .pyproject 所在
    QString currFilePath; // 当前文件
    QString srcPath; // src文件夹
};

class Project
{
public:
    // 创建项目
    static Pro createProject(const QString& projName, const QString& basePath, bool isEmpty = true);

    // 从某个文件路径向上查找项目根
    static QString findProjectRoot(const QString& filePath);

    // 获取项目的 src 路径
    static QString getSrcPath(const QString& projectRoot);

    static QString createDir(const QString& parentPath, const QString& dirName);

    static QString createHiddenFolder(const QString& parentPath, const QString& folderName);

    static QString getUpper(const QString& currPath);

    static bool isFindProFile(const QString& path);

    static bool isFindProject(const QString& proName, const QString& path);
};
