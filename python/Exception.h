#pragma once
#include <exception>
#include <string>
#include <QString>
#include "core/utils/functions.h"

class MyLangError : public std::exception
{
public:
    explicit MyLangError(const std::string& msg) : m_msg(msg) {}
    explicit MyLangError(const QString& msg) : m_msg(qt2std(msg)) {}
    explicit MyLangError(const char* msg) : m_msg(std::string(msg)) {}

    const char* what() const noexcept override {
        return m_msg.c_str();
    }

private:
    std::string m_msg;
};

class LexerError : public MyLangError {
public:
    using MyLangError::MyLangError;
};

class ParserError : public MyLangError {
public:
    using MyLangError::MyLangError;
};

class CompilerError : public MyLangError {
public:
    using MyLangError::MyLangError;
};

class VMError : public MyLangError {
public:
    using MyLangError::MyLangError;
};

class FileError : public std::exception
{
public:
    explicit FileError(const std::string& msg) : m_msg(msg) {}
    explicit FileError(const QString& msg) : m_msg(qt2std(msg)) {}
    explicit FileError(const char* msg) : m_msg(std::string(msg)) {}

    const char* what() const noexcept override {
        return m_msg.c_str();
    }

private:
    std::string m_msg;
};

// 文件已存在异常
class FileAlreadyExists : public FileError
{
public:
    using FileError::FileError;
};

// 文件不存在异常
class FileNotFound : public FileError
{
public:
    using FileError::FileError;
};

class CreateFileFail : public FileError
{
public:
    using FileError::FileError;
};

// 目录创建失败异常
class DirectoryCreationFailed : public FileError
{
public:
    using FileError::FileError;
};

// 隐藏属性设置失败异常
class HiddenAttributeFailed : public FileError
{
public:
    using FileError::FileError;
};

// 重命名失败异常
class RenameFailed : public FileError
{
public:
    using FileError::FileError;
};

// 项目无效异常
class InvalidProject : public FileError
{
public:
    using FileError::FileError;
};

// 路径无效异常
class InvalidPath : public FileError
{
public:
    using FileError::FileError;
};

// 不是目录异常
class NotADirectory : public FileError
{
public:
    using FileError::FileError;
};
