#include "core/objects/runtime/blockandframe.h"

#include "core/objects/runtime/pnone.h"
#include "core/objects/runtime/pvm.h"

namespace vm {


CallFrame makeCallFrame(int fromWhere, EPointer newEnvir, const QByteArray &codes)
{
    return CallFrame {
        fromWhere,
                makeShared<Py::PNone>(),
                newEnvir,
                codes
    };
}

BlockFrame makeWhileLoopFrame(int beginPC, int endPC)
{
    return BlockFrame{
      BlockType::LOOP_WHILE,
              endPC,
              beginPC
    };
}

BlockFrame makeForLoopFrame(int beginPC, int endPC)
{
    return BlockFrame{
      BlockType::LOOP_FOR,
              endPC,
              beginPC
    };
}

}

