#include "Core.h"

#include "core/objects/runtime/environment.h"
#include "Exception.h"

Core::Core(QObject* parent) 
    : QObject(parent), 
    lexer(nullptr), parser(nullptr), compiler(nullptr), pvm(nullptr)
{
}

void Core::execute(const Pro& project)
{
    try {
        auto& pyfilepath = project.currFilePath;
        QFile file(pyfilepath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            log("无法打开文件：" + pyfilepath);
            return;
        }
        auto code = file.readAll().toStdString();
        file.close();
        if (!lexer) lexer = new Lexer(this);
        auto tokens = lexer->scanTokens(code);
        if (!parser) parser = new Parser(this);
        auto ast = parser->parse(tokens);
        if (!compiler) compiler = new Compiler(this);
        compiler->setAst(ast);
        auto codes = compiler->compileAST();
#ifdef COMPILE_DEBUG
        log("编译输出已开启");
        for (const auto& i : codes) {
            log(i.vm::Instruction::toString());
        }
        log("\n");
#endif // COMPILE_DEBUG

        if (!pvm) pvm = new PVM(this);
        pvm->setImportSrcPath(project.srcPath);
        pvm->setCode(codes);
        pvm->start();
        return;
    }
    catch (LexerError& e) {
        log("分词错误: ");
        log(e.what());
        throw;
    }
    catch (ParserError& e) {
        log("语法分析错误: ");
        log(e.what());
        throw;
    }
    catch (CompilerError& e) {
        log("编译错误: ");
        log(e.what());
        throw;
    }
    catch (VMError& e) {
        log("执行错误: ");
        log(e.what());
        throw;
    }
    catch (...) {
        log("未知的错误");
    }
}

void Core::execute(QString srcFilePath)
{
    try {
        auto rootPath = Project::findProjectRoot(srcFilePath);
        auto srcPath = Project::getSrcPath(rootPath);
        Pro p{ rootPath, srcFilePath, srcPath };
        execute(p);
    }
    catch (InvalidProject& e) {
        log(e.what());
    }

}

Core::EPointer Core::getResultEnvir()
{
    if (pvm) return pvm->currEnvir();
    return nullptr; 
}

Core::~Core()
{
}
