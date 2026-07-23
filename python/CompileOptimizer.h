#pragma once

#include <qvector.h>

namespace vm {
	class Instruction;
}

namespace vm {
	class CompileOptimizer
	{
	public:
		CompileOptimizer();

		void setCodes(QVector<Instruction>* codes);

		~CompileOptimizer();

	private:
		QVector<Instruction>* codes;

	};
}


