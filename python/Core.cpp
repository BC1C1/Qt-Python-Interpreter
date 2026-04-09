#include "Core.h"

#include "core/objects/runtime/environment.h"

Core::Core(QObject* parent) 
    : QObject(parent), 
    lexer(nullptr), parser(nullptr), compiler(nullptr), pvm(nullptr)
{
}

void Core::execute(const Pro& project)
{
    auto& pyfilepath = project.currFilePath;
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
    if (!pvm) pvm = new PVM(this);
    pvm->setImportSrcPath(project.srcPath);
    pvm->setCode(codes);
    pvm->start();
    return;
}

void Core::execute(QString srcFilePath)
{
    auto rootPath = Project::findProjectRoot(srcFilePath);
    auto srcPath = Project::getSrcPath(rootPath);
    Pro p{ rootPath, srcFilePath, srcPath };
    execute(p);
}

Core::EPointer Core::getResultEnvir()
{
    if (pvm) return pvm->currEnvir();
    return nullptr; 
}

Core::~Core()
{
}
