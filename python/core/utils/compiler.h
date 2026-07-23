#ifndef COMPILER_H
#define COMPILER_H

#include <QObject>
#include <QPair>
#include <QMap>
#include <QDateTime>

#include "core/objects/ast/astnode.h"
#include "core/objects/runtime/pvm.h"
#include "Exception.h"

namespace Compile {
using APointer = Parse::pointer;
using Instruction = vm::Instruction;
using NodeType = Parse::NodeType;
using Code = vm::Code;

enum class NeedType {
    Begin,
    End,
    RETURN
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
    void compileBlock(APointer node);
    void compileFunctions(APointer node);
    void compileFunctionInClass(APointer node);
    void compileClasses(APointer node);

    void compileLeftValue(APointer node);
    void compileExpression(APointer node);

    void compileStatement(APointer node);

private:
    APointer ast;
    QVector<Instruction> cache;
    QMap<APointer, int> functionPlace;
signals:

};
}


#endif // COMPILER_H
