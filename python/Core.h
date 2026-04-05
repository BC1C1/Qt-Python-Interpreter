#pragma once

#include "core/utils/lexer.h"
#include "core/utils/parser.h"
#include "core/utils/compiler.h"
#include "core/objects/runtime/pvm.h"
#include "qobject.h"

using Lex::Lexer;
using Lex::Token;
using Parse::Parser;
using Compile::Compiler;
using vm::PVM;

class Core : public QObject
{
	Q_OBJECT
public:
	Core(QString pyfilepath, QString projectDir = QString(), QObject* parent = nullptr);
	void execute();
	~Core();

private:
	QString pyfilepath;
	QString projectDir;
	Lexer* lexer;
	Parser* parser;
	Compiler* compiler;
	PVM* pvm;
};

