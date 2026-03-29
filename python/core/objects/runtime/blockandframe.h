#ifndef BLOCKANDFRAME_H
#define BLOCKANDFRAME_H
#include "core/objects/runtime/pobject.h"
#include "core/objects/runtime/environment.h"
#include <QByteArray>
namespace vm {
using pointer = Py::PObject::pointer;
using EPointer = QSharedPointer<Py::Environment>;
enum class BlockType {
    LOOP_FOR,
    LOOP_WHILE,
};

struct BlockFrame {
    BlockType blockType;
    int breakPC;
    int continuePC; //暂时这两个
};

struct CallFrame{
    int fromWhere;
    pointer returnValue;
    EPointer innerEnvir;
    QByteArray codes;
};
CallFrame makeCallFrame(int fromWhere, EPointer newEnvir, const QByteArray &codes);
BlockFrame makeWhileLoopFrame(int beginPC, int endPC);
BlockFrame makeForLoopFrame(int beginPC, int endPC);
}






#endif // BLOCKANDFRAME_H
