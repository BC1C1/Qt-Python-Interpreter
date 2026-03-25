#include "mainwindow.h"

#include <QApplication>
#include <QJsonDocument>
#include <qdebug.h>
#include "core/utils/lexer.h"
#include "core/utils/parser.h"
#include <qdebug.h>
#include "core/objects/runtime/pvm.h"
#include "core/utils/compiler.h"

Q_DECLARE_METATYPE(Lex::Token)
Q_DECLARE_METATYPE(QVector<Lex::Token>)

using Lex::Lexer;
using Parse::Parser;
using Lex::Token;
using Parse::ANode;
using vm::Code;
using vm::PVM;
using vm::Instruction;
using Compile::Compiler;
int main(int argc, char *argv[])
{
    std::string code1 = R"(
list = [1,2,3]
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
        for (const auto& ins : instrucntions) {
            qDebug() << ins.toString();
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


    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
