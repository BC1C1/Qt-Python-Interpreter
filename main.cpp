#include "mainwindow.h"

#include <QApplication>
#include <qdebug.h>
#include "core/utils/lexer.h"
#include "core/utils/parser.h"
#include <qdebug.h>
#include "core/objects/runtime/pvm.h"

Q_DECLARE_METATYPE(Lex::Token)
Q_DECLARE_METATYPE(QVector<Lex::Token>)

using Lex::Lexer;
using Parse::Parser;
using Lex::Token;
using Parse::ANode;
using vm::Code;
using vm::PVM;
using vm::Instruction;
int main(int argc, char *argv[])
{
//    std::string code1 = R"(
//a = 1 + 2
//print(a)
//)";

//    try {
//        // 1. 分词
//        Lexer lexer;
//        std::vector<Token> tokens = lexer.scanTokens(code1); // 假设scanTokens已适配QString/QVector

//        // 输出分词结果（Qt风格输出）
//        qDebug() << "<---------- tokens ---------->";
//        for (const Token& token : tokens) {
//            // 如果你的Token类有toQString()：cout << token.toQString() << Qt::endl;
//            // 如果保留operator<<：cout << token << Qt::endl;
//            qDebug() << token.TokenToQString(token) << Qt::endl; // 根据实际Token类调整
//        }

//        // 2. 解析
//        Parser parser;
//        QSharedPointer<ANode> ast = parser.parse(tokens); // 用QSharedPointer替代原生shared_ptr（Qt推荐）
//        if (!ast) {
//            qDebug() << "解析失败：AST 为空";
//            return 1;
//        }
//        qDebug() << ast->toJson();

//        qDebug() << "\n<---------- parse success ---------->";
//        qDebug() << "=== 分词+解析测试跑通！===";

//        return 0;
//    }
//    catch (const std::exception& e) {
//        // Qt输出C++标准异常
//        qDebug() << "\n=== 测试失败：" << e.what();
//        return 1;
//    }
//    catch (const QString& errMsg) {
//        // 适配Qt字符串异常（如果你的解析器抛QString类型错误）
//        qDebug() << "\n=== 测试失败：" << errMsg;
//        return 1;
//    }
//    catch (...) {
//        qDebug() << "\n=== 测试失败：未知异常 ===";
//        return 1;
//    }
    QVector<Instruction> codes;
    codes << Instruction{Code::LOAD_INT, QVariant(1)};
    codes << Instruction{Code::LOAD_INT, QVariant(2)};
    codes << Instruction{Code::ADD};
    codes << Instruction{Code::STORE_NAME, QVariant("a")};
    codes << Instruction{Code::LOAD_NAME, QVariant("a")};
    codes << Instruction{Code::PRINT};
    codes << Instruction{Code::HALT};
    PVM pythonVirtualMachine(codes);
    pythonVirtualMachine.start();


    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
