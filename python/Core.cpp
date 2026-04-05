#include "Core.h"

Core::Core(QString pyfilepath, QString projectDir, QObject* parent) 
    : QObject(parent), pyfilepath(pyfilepath), projectDir(projectDir), 
    lexer(nullptr), parser(nullptr), compiler(nullptr), pvm(nullptr)
{
}

void Core::execute()
{
    QFile file(pyfilepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件：" << pyfilepath;
        return;
    }
    auto code = file.readAll().toStdString();
    file.close();
	if (!lexer) lexer = new Lexer(this);
    auto tokens = lexer->scanTokens(code);
    if (!parser) parser = new Parser();
    auto ast = parser->parse(tokens);
    if (!compiler) compiler = new Compiler();
    compiler->setAst(ast);
    auto codes = compiler->compileAST();
    if (!pvm) pvm = new PVM(codes, this);
    
    pvm->start();
    return;
}

Core::~Core()
{
}
