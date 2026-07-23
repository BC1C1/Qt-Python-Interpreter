#include "mainwindow.h"

#include <QApplication>
#include <QJsonDocument>
#include <qdebug.h>
#include <qhash.h>
#include "core/utils/lexer.h"
#include "core/utils/parser.h"
#include "core/objects/runtime/pvm.h"
#include "core/utils/compiler.h"
#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h>
#include "Core.h"
#include "Project.h"
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include "Logger.h"

Q_DECLARE_METATYPE(Lex::Token)
Q_DECLARE_METATYPE(QVector<Lex::Token>)
Q_DECLARE_METATYPE(vm::Instruction)
Q_DECLARE_METATYPE(QVector<vm::Instruction>)

using Lex::Lexer;
using Parse::Parser;
using Lex::Token;
using Parse::ANode;
using vm::Code;
using vm::PVM;
using vm::Instruction;
using Compile::Compiler;
using PObject = Py::PObject;
using pointer = QSharedPointer<PObject>;

inline uint qHash(const pointer& p, uint seed = 0)
{
    return qHash(p.data(), seed);
}

//class A :
//    def __init__(self) :
//    self.a = 1
//    class B(A) :
//    def __init__(self) :
//    super().__init__(self)
//    self.b = 2
//    b = B()
//    print(b.b)
//    print(b.a)
//    print(b.A.a)
void generateTestFiles(const QString& projRoot)
{
    QString src = Project::getSrcPath(projRoot);

    // 生成 test.py
    QString testFile = QDir(src).filePath("test.py");
    QFile f1(testFile);
    f1.open(QIODevice::WriteOnly | QIODevice::Text);
    f1.write("def hello():\n    print('Hello from test.py')\n");
    f1.close();

    // 生成 main.py
    QString mainFile = QDir(src).filePath("main.py");
    QFile f2(mainFile);
    f2.open(QIODevice::WriteOnly | QIODevice::Text);
    f2.write("import test\n""test.hello()\n");
    f2.close();
}

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    // ======================
    // Logger 极简测试（3行）
    // ======================
    logDebug("main 启动成功");
    logDebug("Logger 测试输出 1");
    logDebug("Logger 测试输出 2");

    //MainWindow w;
    auto w = makeShared<MainWindow>();
    w->show();
    log("普通黑色日志");
    logDebug("灰色调试");
    logInfo("黑色信息");
    logWarn("橙色警告");
    logError("红色错误");
    return a.exec();
}
