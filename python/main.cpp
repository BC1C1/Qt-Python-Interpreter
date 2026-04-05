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
#include "Core.h"

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
int test_main() {
    std::string code1 = R"(
l = []
print(l)
l.append(1)
print(l)
)";

    try {
        // 0. 原始文本
        qDebug() << "<---------- text ---------->";
        qDebug() << QString::fromStdString(code1);
        qDebug() << "<---------- text end ---------->";

        // 1. 分词
        Lexer lexer;
        std::vector<Token> tokens = lexer.scanTokens(code1);

        qDebug() << "<---------- tokens ---------->";
        for (const Token& token : tokens) {
            qDebug() << token.TokenToQString(token) << Qt::endl;
        }

        // 2. 解析
        Parser parser;
        QSharedPointer<ANode> ast = parser.parse(tokens);
        if (!ast) {
            qDebug() << "解析失败：AST 为空";
            return 1;
        }
        auto jsonObj = ast->toJson();
        QJsonDocument doc(jsonObj);
        QString prettyStr = QString::fromUtf8(doc.toJson(QJsonDocument::Indented));

        // 按换行符拆分，逐行输出
        QStringList lines = prettyStr.split("\n");
        for (const QString& line : lines) {
            qDebug().noquote() << line;
        }

        qDebug() << "\n<---------- parse success ---------->";


        // 3. 编译
        Compiler compiler;
        compiler.setAst(ast);
        auto instrucntions = compiler.compileAST();
        int cnt = 0;
        for (const auto& ins : instrucntions) {
            qDebug() << "Line: " << cnt << ins.toString();
            cnt++;
        }

        qDebug() << "\n<---------- compile success ---------->";

        // 4. 运行
        PVM pythonVirtualMachine(instrucntions);
        pythonVirtualMachine.start();

        qDebug() << "\n<---------- running success ---------->";
        qDebug() << "=== 测试跑通！===";

        return 0;
    }
    catch (const std::exception& e) {
        qDebug() << "\n=== 测试失败：" << e.what();
        return 1;
    }
    catch (const QString& errMsg) {
        qDebug() << "\n=== q测试失败：" << errMsg;
        return 1;
    }
    catch (...) {
        qDebug() << "\n=== 测试失败：未知异常 ===";
        return 1;
    }

    //    字节码测试方式
    //    QVector<Instruction> ins;

    //    // PC=0  【正确】跳去主程序入口 = PC=1
    //    ins.push_back({Code::JUMP, 1});

    //    // ==================== 主程序 ====================
    //    // PC=1  i = 0
    //    ins.push_back({Code::LOAD_INT, 0});
    //    // PC=2
    //    ins.push_back({Code::STORE_VAR, "i"});

    //    // ==================== WHILE 准备 ====================
    //    // PC=3  压 breakPC → 最终 END 位置
    //    ins.push_back({Code::LOAD_INT, 27});
    //    // PC=4  压 continuePC → 条件判断位置 PC=6
    //    ins.push_back({Code::LOAD_INT, 6});
    //    // PC=5  开始循环
    //    ins.push_back({Code::LOOP_START_WHILE});

    //    // ==================== 循环条件 ====================
    //    // PC=6  ✅ continue 必须跳回这里
    //    ins.push_back({Code::LOAD_NAME, "i"});
    //    // PC=7
    //    ins.push_back({Code::LOAD_INT, 3});
    //    // PC=8
    //    ins.push_back({Code::LT});
    //    // PC=9  条件不成立跳去 PC=27
    //    ins.push_back({Code::JUMP_IF_FALSE, 27});

    //    // ==================== 循环体 ====================
    //    // PC=10  i += 1
    //    ins.push_back({Code::LOAD_NAME, "i"});
    //    ins.push_back({Code::LOAD_INT, 1});
    //    ins.push_back({Code::ADD});
    //    ins.push_back({Code::STORE_VAR, "i"});

    //    // PC=14  if i == 1: continue
    //    ins.push_back({Code::LOAD_NAME, "i"});
    //    ins.push_back({Code::LOAD_INT, 1});
    //    ins.push_back({Code::EQ});
    //    ins.push_back({Code::JUMP_IF_FALSE, 19});
    //    // PC=18
    //    ins.push_back({Code::CONTINUE});  // → 跳 PC=6

    //    // PC=19  print(i)
    //    ins.push_back({Code::LOAD_NAME, "i"});
    //    ins.push_back({Code::PRINT});

    //    // PC=21  if i == 2: break
    //    ins.push_back({Code::LOAD_NAME, "i"});
    //    ins.push_back({Code::LOAD_INT, 2});
    //    ins.push_back({Code::EQ});
    //    ins.push_back({Code::JUMP_IF_FALSE, 25});
    //    // PC=25
    //    ins.push_back({Code::BREAK});     // → 跳 PC=27

    //    // PC=26  回到条件判断
    //    ins.push_back({Code::JUMP, 6});

    //    // ==================== 循环结束 ====================
    //    // PC=27
    //    ins.push_back({Code::LOOP_WHILE_END});
    //    // PC=28
    //    ins.push_back({Code::HALT});

    //    PVM pythonVirtualMachine(ins);
    //    pythonVirtualMachine.start();
}
int run(QString filePath, QString projectName = QString()) {
    Core* core = new Core(filePath, projectName);
    core->execute();
    delete core;
    return 0;
}
int createProject(QString name, QString path) {
    // 1如果filepath是dir，在目录下创建刚才说的项目文件夹（其实只需要一个名字为name的文件夹）
    QDir d(path);
    if (!QFileInfo(path).isDir()) {
        qDebug() << "项目需要创建在文件夹下";
        return -1;
    }
    QString proPath = path + QString("/%1").arg(name);
    QDir dir1;
    if (dir1.mkdir(proPath)) {
        qDebug() << "项目文件夹创建成功:" << proPath;
    }
    else {
        qDebug() << "项目文件夹创建失败:" << proPath;
        return -1;
    }
    QString srcPath = proPath + "/src";
    QDir dir2;
    if (dir2.mkdir(srcPath)) {
        qDebug() << "src文件夹创建成功:" << srcPath;
    }
    else {
        qDebug() << "src文件夹创建失败:" << srcPath;
        return -1;
    }
    // 创建__init__.py文件
    QString pyFilePath = srcPath + "/__init__.py";
    QFile pyFile(pyFilePath);
    if (pyFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        pyFile.close();
        qDebug() << "创建__init__.py文件成功:" << pyFilePath;
    }
    return 0;
}

int createPyFile(QString filename, QString path) {
    QDir d(path);
    if (!QFileInfo(path).isDir()) {
        qDebug() << "项目需要创建在文件夹下";
        return -1;
    }
    QString fileName = path + QString("/%1.py").arg(filename);
    QFile pyFile(fileName);
    if (pyFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        pyFile.close();
        qDebug() << "创建py文件成功:" << fileName;
    }
    return 0;
}

int createModelFile(QString filename, QString path) {
    QDir d(path);
    if (!QFileInfo(path).isDir()) {
        qDebug() << "项目需要创建在文件夹下";
        return -1;
    }
    QString fileName = path + QString("/%1.py").arg(filename);
    QFile pyFile1(fileName);
    if (pyFile1.open(QIODevice::WriteOnly | QIODevice::Text)) {
        pyFile1.close();
        qDebug() << "创建py文件成功:" << fileName;
    }
    QString initFilePath = path + "/__init__.py";
    QFile pyFile(initFilePath);
    if (pyFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        pyFile.close();
        qDebug() << "创建__init__.py文件成功:" << initFilePath;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    // ------------------------------
    // 命令格式说明
    // ------------------------------
    // 1. 运行脚本      : python <file.mypy> [<projectName>]
    // 2. 创建项目      : createpypro <projectName> <basePath>
    // 3. 创建py文件    : createpyfile <filename> <dirPath>
    // 4. 创建模块文件   : createpymodelfile <filename> <dirPath>

    if (argc < 2) {
        qDebug() << "Usage:";
        qDebug() << "  python <file.mypy> [<projectName>]   Run script";
        qDebug() << "  createpypro <name> <path>            Create project";
        qDebug() << "  createpyfile <name> <path>           Create py file";
        qDebug() << "  createpymodelfile <name> <path>      Create module file";
        return -1;
    }

    QString cmd = argv[1];

    // ------------------------------
    // 运行脚本
    // ------------------------------
    if (cmd == "python" && (argc == 3 || argc == 4)) {
        if (argc == 4) {
            return run(argv[2], argv[3]);
        }
        return run(argv[2]);
    }

    // ------------------------------
    // 创建项目
    // ------------------------------
    else if (cmd == "createpypro" && argc == 4) {
        QString projName = argv[2];
        QString basePath = argv[3];
        return createProject(projName, basePath);
    }

    // ------------------------------
    // 创建 py 文件
    // ------------------------------
    else if (cmd == "createpyfile" && argc == 4) {
        QString fileName = argv[2];
        QString dirPath = argv[3];
        return createPyFile(fileName, dirPath);
    }

    // ------------------------------
    // 创建模块文件
    // ------------------------------
    else if (cmd == "createpymodelfile" && argc == 4) {
        QString fileName = argv[2];
        QString dirPath = argv[3];
        return createModelFile(fileName, dirPath);
    }

    // ------------------------------
    // 无效命令
    // ------------------------------
    else {
        qDebug() << "Error: Invalid command or wrong number of arguments!";
        qDebug() << "Usage:";
        qDebug() << "  python <file.mypy> [<projectName>]   Run script";
        qDebug() << "  createpypro <name> <path>            Create project";
        qDebug() << "  createpyfile <name> <path>           Create py file";
        qDebug() << "  createpymodelfile <name> <path>      Create module file";
        return -1;
    }

    return a.exec();
}
