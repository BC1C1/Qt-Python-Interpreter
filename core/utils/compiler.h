#ifndef COMPILER_H
#define COMPILER_H

#include <QObject>
#include <QPair>

#include "core/objects/ast/astnode.h"
#include "core/objects/runtime/pvm.h"

namespace Compile {
using APointer = Parse::pointer;
using Instruction = vm::Instruction;
using NodeType = Parse::NodeType;
using Code = vm::Code;

enum class NeedType {
    Begin,
    End
};
using pairVector = QVector<QPair<NeedType, int>>;

class Compiler : public QObject
{
    Q_OBJECT

public:
    explicit Compiler(QObject *parent = nullptr);
    void setAst(APointer ast);
    QVector<Instruction> compileAST();
private:
    pairVector compileBlock(APointer node);


//    void compileLeftValue(APointer node);
    void compileExpression(APointer node);

    pairVector compileStatement(APointer node);

private:
    void throwErrorLine(int line1, int line2 = -1);
private:
    APointer ast;
    QVector<Instruction> cache;
signals:

};
}


#endif // COMPILER_H
