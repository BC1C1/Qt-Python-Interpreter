#include "CompileOptimizer.h"

#include "core/objects/runtime/pvm.h"
using vm::Instruction;
namespace vm {
	CompileOptimizer::CompileOptimizer() : codes(nullptr)
	{
	}

	void CompileOptimizer::setCodes(QVector<Instruction>* codes)
	{
		delete this->codes;
		this->codes = new QVector<Instruction>(codes->begin(), codes->end());
	}

	CompileOptimizer::~CompileOptimizer()
	{
		delete codes;
	}
}


