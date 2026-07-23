#pragma once

#include "core/utils/lexer.h"
#include "core/utils/parser.h"
#include "core/utils/compiler.h"
#include "core/objects/runtime/pvm.h"
#include "core/utils/functions.h"
#include "qobject.h"
#include "Project.h"
#include "Logger.h"

using Lex::Lexer;
using Lex::Token;
using Parse::Parser;
using Compile::Compiler;
using vm::PVM;
using Py::Environment;

class Core : public QObject
{
	Q_OBJECT
private:
	using EPointer = QSharedPointer<Environment>;

public:
	Core(QObject* parent = nullptr);
	void execute(const Pro& project);
	void execute(QString srcFilePath);
	EPointer getResultEnvir();
	~Core();

private:
	Lexer* lexer;
	Parser* parser;
	Compiler* compiler;
	PVM* pvm;
};

