#include "pvm.h"
namespace vm {
PVM::PVM(const QVector<Instruction> &codes, QObject *parent) : QObject(parent), codes(codes), PC(0), isRunning(false)
{
    globalEnvir = makeShared<Environment>(nullptr, this);
}

void PVM::start()
{
    isRunning = true;
    run();
}

void PVM::stop()
{
    isRunning = false;
}

void PVM::executeSingleCode()
{
    auto instruction = codes[PC];
    auto currentCode = instruction.code;
    auto operand = instruction.operand;
    qDebug() << "执行指令：PC=" << PC << "，指令类型=" <<  CodeToQString(currentCode);

    switch (currentCode)
    {
    case Code::HALT: {
        qDebug() << "收到HALT停机指令";
        stop();
        return;
    }
    case Code::LOAD_INT: {
        int value = operand.toInt();
        pushValue(makeShared<PInt>(value));
        break;
    }
    case Code::ADD: {
        auto obj2 = popValue();
        auto obj1 = popValue();
        pushValue(obj1->__add__(obj2));
        break;
    }
    case Code::PRINT: {
        auto obj = popValue();
        qDebug() << obj->toString();
        break;
    }
    case Code::LOAD_NAME: {
        QString name = operand.toString();
        pushValue(globalEnvir->getObj(name));
        break;
    }
    case Code::STORE_NAME: {
        QString name = operand.toString();
        auto obj = popValue();
        globalEnvir->assign(name, obj);
        break;
    }
    default:
        break;
    }

    PC++;
}

void PVM::pushValue(pointer object)
{
    valueStack.push(object);
}

pointer PVM::popValue()
{
    if (valueStack.isEmpty()) {
        throwErrMsg("栈为空，弹栈失败");
    }
    return valueStack.pop();
}

void PVM::throwErrMsg(const std::string &msg)
{
    throw std::runtime_error(msg);
}

void PVM::run()
{
    while (isRunning && PC < codes.length()) {
        executeSingleCode(); // 这个函数在处理halt情况的时候会把isStart弄成false终止循环
    }
    if (PC >= codes.size())     qDebug() << "虚拟机停机：指令执行完毕（PC越界）";
    else                        qDebug() << "虚拟机停机：收到停止指令";
}

QString CodeToQString(Code code)
{
    switch (code)
    {
    case Code::LOAD_INT:    return "LOAD_INT";
    case Code::LOAD_FLOAT:  return "LOAD_FLOAT";
    case Code::LOAD_STRING: return "LOAD_STRING";
    case Code::LOAD_NAME:   return "LOAD_NAME";
    case Code::STORE_NAME:  return "STORE_NAME";
    case Code::ADD:         return "ADD";
    case Code::PRINT:       return "PRINT";
    case Code::HALT:        return "HALT";
    default:                return "UNKNOWN_CODE";
    }
}

}
