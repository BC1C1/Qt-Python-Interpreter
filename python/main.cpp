#include "mainwindow.h"

#include <QApplication>
#include <QJsonDocument>
#include <qdebug.h>
#include <qhash.h>
#include "core/utils/lexer.h"
#include "core/utils/parser.h"
#include "core/objects/runtime/pvm.h"
#include "core/utils/compiler.h"

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


int main(int argc, char *argv[])
{
    std::string code1 = R"(
class Person :
    # 所有属性赋值都会走这里
    def __setattr__(self, attr_name, value) :
        print("正在设置属性：" + attr_name + " = " + value)
        # 真正把值存进去
        self.__dict__[attr_name] = value

# 测试
p = Person()
p.name = "Tom"   # 触发 __setattr__
p.age = 18       # 触发 __setattr__
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

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
