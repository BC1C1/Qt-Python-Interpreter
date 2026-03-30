#include "pvm.h"
#include "core/utils/functions.h"
#include "core/objects/runtime/pint.h"
#include "core/objects/runtime/pstr.h"
#include "core/objects/runtime/pnone.h"
#include "core/objects/runtime/pbool.h"
#include "core/objects/runtime/pfloat.h"
#include "core/objects/runtime/plist.h"
#include "core/objects/runtime/pnone.h"
#include "core/objects/runtime/piterator.h"
#include "core/objects/runtime/environment.h"
#include "core/objects/runtime/pfunction.h"
#include "PClass.h"
namespace vm {
PVM::PVM(const QVector<Instruction> &codes, QObject *parent) : QObject(parent), PC(0), isRunning(false)
{
    auto defaultEnvir = makeShared<Environment>(nullptr, this);
    callFrameStack.push(makeCallFrame(0, defaultEnvir, Instruction::toByteArray(codes)));
    currCodes = codes;
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

    auto instruction = currCodes[PC];
    auto currentCode = instruction.code;
    auto operand = instruction.operand;
    qDebug() << "执行指令：PC=" << PC << "，指令类型=" <<  CodeToQString(currentCode)
             << "操作数：operand= " << operand.toString();
    auto isGo = true;
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
    case Code::LOAD_FLOAT: {
        double value = operand.toDouble();
        pushValue(makeShared<PFloat>(value));
        break;
    }
    case Code::LOAD_STRING: {
        auto obj = makeShared<Py::PStr>(operand.toString());
        pushValue(obj);
        break;
    }
    case Code::LOAD_LIST: {
        auto sizeObj = popValue();
        int size = sizeObj->getValue().toInt();
        QVector<pointer> inner;
        inner.reserve(size);
        for (int i = 0; i < size; i++) {
            inner.push_back(popValue());
        }
        std::reverse(inner.begin(), inner.end());
        auto obj = makeShared<Py::PList>(inner);
        //        qDebug() << "type of list is:" << TypeToString(obj->getType().type);
        pushValue(obj);
        break;
    }
    case Code::LOAD_NONE: {
        pushValue(makeShared<Py::PNone>());
        break;
    }
    case Code::LOAD_NAME: {
        //dumpStack("loadname begin");
        QString name = operand.toString();
        auto obj = currEnvir()->getObj(name);
        if (!obj)
            qDebug() << "obj is nullptr";
        qDebug() << "obj.name is" << name << "\nobj.type is"
                 << TypeToString(obj->getType().type)
                 << "obj.value is(toString)" << obj->toString();
        if (obj)
            pushValue(obj);
        if (!obj) {
            throwErrMsg(QString("变量 '%1' 未定义").arg(name).toUtf8().data());
        }
        //dumpStack("loadname end");
        break;
    }
    case Code::LOAD_INDEX: {
        auto index = popValue();
        auto toIndex = popValue();
        auto obj = toIndex->__getitem__(index);
        pushValue(obj);
        break;
    }
    case Code::LOAD_ATTR: {
        auto toAttr = popValue();
        auto attrName = operand.toString();
        auto obj = toAttr->__getattribute__(attrName);
        pushValue(obj);
        break;
    }
    case Code::STORE_VAR: {
        auto left = operand.toString();
        auto right = popValue();
        currEnvir()->assign(left, right);
        //        qDebug() << "var name : " << left << "get value: " << right->toString();
        //        qDebug() << "var type is:" << TypeToString(right->getType().type);
        break;
    }
    case Code::STORE_INDEX: {
        auto index = popValue();
        auto toIndex = popValue();
        auto rightValue = popValue();
        toIndex->__setitem__(index, rightValue);
        break;
    }
    case Code::STORE_ATTR: {
        auto toAttr = popValue();
        auto attrName = operand.toString();
        auto value = popValue();
        toAttr->__setattribute__(attrName, value);
        break;
    }
    case Code::CREATE_ITER: {
//        dumpStack("before create iterator");
        auto obj = popValue();
        qDebug() << TypeToString(obj->getType().type);
        auto iter = obj->__iter__();
        pushValue(iter);
        qDebug() << "迭代器是否为空：" << (iter == nullptr);
//        dumpStack("after create iterator");
        break;
    }
    case Code::ITER_NEXT: {
//        dumpStack("before next iterator");
        auto loopVarName = operand.toString();
        auto iter = topValue();
        auto obj = iter->__next__();
        currEnvir()->assign(loopVarName, obj);
//        dumpStack("after next iterator");
        break;
    }
    case Code::LOOP_START_FOR: {
        int breakPC = popValue()->getValue().toInt();
        int continuePC = popValue()->getValue().toInt();
        pushFrame(makeForLoopFrame(continuePC, breakPC));
        break;
    }
    case Code::LOOP_START_WHILE: {
        int breakPC = popValue()->getValue().toInt();
        int continuePC = popValue()->getValue().toInt();
        pushFrame(makeWhileLoopFrame(continuePC, breakPC));
        break;
    }
    case Code::CONTINUE: {
        BlockFrame currFrame;
        for (auto c = blockFrameStack.rbegin(); c != blockFrameStack.rend(); ++c) {
            if (c->blockType == BlockType::LOOP_FOR || c->blockType == BlockType::LOOP_WHILE) {
                currFrame = *c;
                break;
            }
        }

        isGo = false;
        PC = currFrame.continuePC;
        break;
    }
    case Code::BREAK: {
        BlockFrame currFrame;
        for (auto c = blockFrameStack.rbegin(); c != blockFrameStack.rend(); ++c) {
            if (c->blockType == BlockType::LOOP_FOR || c->blockType == BlockType::LOOP_WHILE) {
                currFrame = *c;
                break;
            }
        }

        isGo = false;
        PC = currFrame.breakPC;
        break;
    }
    case Code::LOOP_FOR_END: {
        if (topFrame().blockType != BlockType::LOOP_FOR) {
            throwErrMsg("不匹配的块"); // 编译期错误
        }
        popValue(); // 弹出迭代器

        popFrame();
        break; // 统一的PC++自己会跑
    }
    case Code::LOOP_WHILE_END: {
        if (topFrame().blockType != BlockType::LOOP_WHILE) {
            throwErrMsg("不匹配的块"); // 编译期错误
        }

        popFrame();
        break; // 统一的PC++自己会跑
    }
    case Code::CREATE_FUNCTION: {
        auto code = operand.toByteArray();
        auto params = popValue();
        auto name = popValue();
        auto functionObj = makeShared<Py::PFunction>(params, code, name);
        pushValue(functionObj);
        qDebug() << "以下是函数内部字节码";
        for (const auto& innerCode : Instruction::fromByteArray(code)) {
            qDebug() << innerCode.toString();
        }
        qDebug() << "函数字节码结束";
        break;
    }
    case Code::CALL: {
        //dumpStack("call begin");
        auto caller = popValue();
        auto params = popValue();
        auto newEnvir = createNewEnvironment(currEnvir());
        auto function = dynamicPointerCast<Py::PFunction>(caller);
        if (function) {
            auto param = dynamicPointerCast<Py::PList>(params);
            if (param->size() != function->getParamsObj()->size()) {
                throwErrMsg(u8"参数数量不匹配");
            }
            // 新字节码
            auto newCode = function->getCodeObj();
            // 参数处理
            auto pIter = params->__iter__();
            auto fIter = function->getParamsObj()->__iter__();
            auto NoneObj = makeShared<PNone>(); // 未来搞成全局对象
            for (int i = 0; i < param->size(); i++) {
                auto value1 = fIter->__next__();
                newEnvir->assign(value1->toString(), pIter->__next__());
            }
            callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
            currCodes = newCode;
            isGo = false;
            PC = 0;
        }
        else {
            auto classObj = dynamicPointerCast<Py::PClass>(caller);
            if (!classObj) {
                throwErrMsg(u8"无效的caller");
            }
            pushValue(classObj->__instance__(params, currEnvir()));
        }

        //dumpStack("call end");
        break;
    }
    case Code::RETURN: {
        //dumpStack("return begin");
        // auto returnValue = popValue(); 这个栈是同一个，无所谓
        auto frame = callFrameStack.pop();
        currCodes = Instruction::fromByteArray(callFrameStack.top().codes);
        // 环境自动回复，currEnvir是栈顶的
        isGo = false;
        PC = frame.fromWhere;
        //dumpStack("return end");
        break;
    }
    case Code::CREATE_CLASS: {
        auto className = operand.toString();
        auto staticNumber = popValue()->getValue().toInt();
        QHash<QString, pointer> staticMembers;
        for (int i = 0; i < staticNumber; i++) {
            auto varName = popValue()->getValue().toString();
            auto obj = popValue();
            staticMembers[varName] = obj;
        }
        auto functionsNumber = popValue()->getValue().toInt();
        QHash<QString, pointer> functions;
        for (int i = 0; i < functionsNumber; i++) {
            auto functionName = popValue()->getValue().toString();
            auto functionobj = popValue();
            functions[functionName] = functionobj;
        }
        auto classObj = makeShared<Py::PClass>(className, functions, staticMembers);
        currEnvir()->assign(operand.toString(), classObj);
        break;
    }
    case Code::ADD: {
       // dumpStack("add begin");
        auto obj2 = popValue();
        auto obj1 = popValue();
        pushValue(obj1->__add__(obj2));
        //dumpStack("add end");
        break;
    }
    case Code::SUB: {
        auto obj2 = popValue();
        auto obj1 = popValue();
        pushValue(obj1->__sub__(obj2));
        break;
    }
    case Code::MUL: {
        auto obj2 = popValue();
        auto obj1 = popValue();
        pushValue(obj1->__mul__(obj2));
        break;
    }
    case Code::DIV: {
        auto obj2 = popValue();
        auto obj1 = popValue();
        pushValue(obj1->__truediv__(obj2));
        break;
    }
    case Code::MOD: {
        auto obj2 = popValue();
        auto obj1 = popValue();
        pushValue(obj1->__mod__(obj2));
        break;
    }
    case Code::POW: {
        auto obj2 = popValue();
        auto obj1 = popValue();
        pushValue(obj1->__pow__(obj2));
        break;
    }
    case Code::NEQ: {
        auto obj2 = popValue();
        auto obj1 = popValue();
        pushValue(obj1->__ne__(obj2));
        break;
    }
    case Code::GT: {
        auto obj2 = popValue();
        auto obj1 = popValue();
        pushValue(obj1->__gt__(obj2));
        break;
    }
    case Code::GE: {
        auto obj2 = popValue();
        auto obj1 = popValue();
        pushValue(obj1->__ge__(obj2));
        break;
    }
    case Code::LE: {
        auto obj2 = popValue();
        auto obj1 = popValue();
        pushValue(obj1->__le__(obj2));
        break;
    }
    case Code::LT: {
        auto obj2 = popValue();
        auto obj1 = popValue();
        pushValue(obj1->__lt__(obj2));
        break;
    }
    case Code::EQ: {
        auto obj2 = popValue();
        auto obj1 = popValue();
        pushValue(obj1->__eq__(obj2));
        break;
    }
    case Code::POP: {
//        dumpStack("🔥 真正要执行 POP 了！！！");

        popValue();
        break;
    }
    case Code::PRINT: {
        auto toPrint = popValue();
        qDebug() << toPrint->toString();
        break;
    }
    case Code::JUMP_IF_FALSE: {
        auto value = popValue()->asBool()->getValue().toBool();
        if (!value) {
            isGo = false;
            PC = operand.toInt();
        }
        break;
    }
    case Code::JUMP: {
        isGo = false;
        PC = operand.toInt();
        break;
    }
    default:
        break;
    }

    if (isGo)
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

pointer PVM::topValue()
{
    return valueStack.top();
}

pointer PVM::getBasicObject(QVariant value)
{
    switch (value.type()) {
    case QVariant::Int: {
        return makeShared<Py::PInt>(value.toInt());
        break;
    }
    case QVariant::Double: {
        return makeShared<Py::PFloat>(value.toDouble());
        break;
    }
    case QVariant::String: {
        return makeShared<Py::PStr>(value.toString());
        break;
    }
    default: {
        break;
    }
    }
    return nullptr;
}

void PVM::dumpStack(const QString &hint)
{
    qDebug() << "\n==================== 栈调试 dump ====================";
    qDebug() << "提示：" << hint;
    qDebug() << "当前栈大小：" << valueStack.size();

    // QStack 遍历（从栈底 → 栈顶）
    for (int i = 0; i < valueStack.size(); ++i) {
        auto obj = valueStack[i];
        QString typeStr = TypeToString(obj->getType().type);
        QString valueStr = obj->toString();

        qDebug().noquote()
            << "栈[" << i << "]"
            << "类型：" << typeStr
            << " | 值：" << valueStr;
    }
    qDebug() << "=====================================================\n";
}

EPointer PVM::createNewEnvironment(EPointer currEnvir)
{
    auto ret = makeShared<Environment>(currEnvir, this);
    return ret;
}

BlockFrame PVM::popFrame()
{
    if (!blockFrameStack.isEmpty())
        return blockFrameStack.pop();
    throwErrMsg("块栈已空，弹栈失败");
    return BlockFrame(); // 无效语句用于消除警告
}

void PVM::pushFrame(const BlockFrame &blockFrame)
{
    blockFrameStack.push(blockFrame);
}

BlockFrame PVM::topFrame()
{
    if (!blockFrameStack.isEmpty())
        return blockFrameStack.top();
    throwErrMsg("块栈已空，查看失败");
    return BlockFrame(); // 无效语句用于消除警告
}

EPointer PVM::currEnvir()
{
    return callFrameStack.top().innerEnvir;
}

void PVM::throwErrMsg(const std::string &msg)
{
    throw std::runtime_error(msg);
}

void PVM::run()
{
    try {
        while (isRunning && PC < currCodes.length()) {
            executeSingleCode();
        }
        if (PC >= currCodes.size())     qDebug() << "虚拟机停机：指令执行完毕（PC越界）";
        else                        qDebug() << "虚拟机停机：收到停止指令";
    } catch (std::runtime_error& e) {
        qDebug() << e.what();
    }

}

QString CodeToQString(Code code)
{
    switch (code) {
    case Code::LOAD_INT:        return "LOAD_INT";
    case Code::LOAD_FLOAT:      return "LOAD_FLOAT";
    case Code::LOAD_STRING:     return "LOAD_STRING";
    case Code::LOAD_LIST:       return "LOAD_LIST";
    case Code::LOAD_NONE:       return "LOAD_NONE";

    case Code::LOAD_NAME:       return "LOAD_NAME";
    case Code::LOAD_INDEX:      return "LOAD_INDEX";
    case Code::LOAD_ATTR:       return "LOAD_ATTR";

    case Code::STORE_VAR:       return "STORE_VAR";
    case Code::STORE_INDEX:     return "STORE_INDEX";
    case Code::STORE_ATTR:      return "STORE_ATTR";

    case Code::JUMP_IF_FALSE:   return "JUMP_IF_FALSE";
    case Code::JUMP:            return "JUMP";

    case Code::CREATE_ITER:     return "CREATE_ITER";
    case Code::ITER_NEXT:       return "ITER_NEXT";

    case Code::LOOP_START_FOR:  return "LOOP_START_FOR";
    case Code::LOOP_START_WHILE:return "LOOP_START_WHILE";
    case Code::CONTINUE:        return "CONTINUE"; 
    case Code::BREAK:           return "BREAK";
    case Code::LOOP_FOR_END:    return "LOOP_FOR_END";
    case Code::LOOP_WHILE_END:  return "LOOP_WHILE_END";

    case Code::CALL:            return "CALL";
    case Code::RETURN:          return "RETURN";

    case Code::ADD:             return "ADD";
    case Code::SUB:             return "SUB";
    case Code::MUL:             return "MUL";
    case Code::DIV:             return "DIV";
    case Code::MOD:             return "MOD";
    case Code::POW:             return "POW";

    case Code::EQ:              return "EQ";
    case Code::NEQ:             return "NEQ";
    case Code::GT:              return "GT";
    case Code::GE:              return "GE";
    case Code::LT:              return "LT";
    case Code::LE:              return "LE";

    case Code::POP:             return "POP";
    case Code::PRINT:           return "PRINT";
    case Code::HALT:            return "HALT";
    case Code::INVALID:         return "INVALID";
    case Code::CREATE_FUNCTION: return "CREATE_FUNCTION";
    case Code::CREATE_CLASS:    return "CREATE_CLASS";

    default:                    return "UNKNOWN_CODE";
    }
}

QString Instruction::toString() const {
    QString codeStr = CodeToQString(this->code) + " ";
    if (this->operand.isValid() && !this->operand.isNull()) {
        return QString("%1 %2 ").arg(codeStr).arg(this->operand.toString());
    }
    return codeStr;
}

QVector<Instruction> Instruction::fromByteArray(const QByteArray &array)
{
    QVector<vm::Instruction> ins;
    QDataStream stream(array);
    stream >> ins;
    return ins;
}

QByteArray Instruction::toByteArray(const QVector<Instruction> &ins)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);

    stream << ins;

    return bytes;
}

bool saveInstructionsToTXT(const QVector<Instruction> &instructions, const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "文件打开失败：" << file.errorString();
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    for (const Instruction& ins : instructions) {
        out << ins.toString() << "\n";
    }

    file.close();
    return true;
}

QVector<Instruction> loadInstructionsFromTXT(const QString &filePath)
{
    QVector<Instruction> instructions;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "文件打开失败：" << file.errorString();
        return instructions;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");

    // 逐行读取并解析
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed(); // 去除首尾空格/换行
        if (line.isEmpty()) continue; // 跳过空行

        // 拆分指令行
        QStringList parts = line.split(" ", Qt::SkipEmptyParts);
        if (parts.isEmpty()) continue;

        Instruction ins;
        // 1. 先解析Code
        ins.code = QStringToCode(parts[0]);

        // 2. 解析operand
        if (parts.size() >= 2) {
            ins.operand = QVariant(parts[1]);
        }

        instructions.append(ins);
    }

    file.close();
    return instructions;
}

Code QStringToCode(QString string)
{
    static const QHash<QString, Code> map = {
        {"LOAD_INT",        Code::LOAD_INT},
        {"LOAD_FLOAT",      Code::LOAD_FLOAT},
        {"LOAD_STRING",     Code::LOAD_STRING},
        {"LOAD_LIST",       Code::LOAD_LIST},
        {"LOAD_NONE",       Code::LOAD_NONE},

        {"LOAD_NAME",       Code::LOAD_NAME},
        {"LOAD_INDEX",      Code::LOAD_INDEX},
        {"LOAD_ATTR",       Code::LOAD_ATTR},

        {"STORE_VAR",       Code::STORE_VAR},
        {"STORE_INDEX",     Code::STORE_INDEX},
        {"STORE_ATTR",      Code::STORE_ATTR},

        {"JUMP_IF_FALSE",   Code::JUMP_IF_FALSE},
        {"JUMP",            Code::JUMP},

        {"CREATE_ITER",     Code::CREATE_ITER},
        {"ITER_NEXT",       Code::ITER_NEXT},

        {"LOOP_START_FOR",  Code::LOOP_START_FOR},
        {"LOOP_START_WHILE",Code::LOOP_START_WHILE},
        {"CONTINUE",        Code::CONTINUE},
        {"BREAK",           Code::BREAK},
        {"LOOP_FOR_END",    Code::LOOP_FOR_END},
        {"LOOP_WHILE_END",  Code::LOOP_WHILE_END},

        {"CALL",            Code::CALL},
        {"RETURN",          Code::RETURN},

        {"ADD",             Code::ADD},
        {"SUB",             Code::SUB},
        {"MUL",             Code::MUL},
        {"DIV",             Code::DIV},
        {"MOD",             Code::MOD},
        {"POW",             Code::POW},

        {"EQ",              Code::EQ},
        {"NEQ",             Code::NEQ},
        {"GT",              Code::GT},
        {"GE",              Code::GE},
        {"LT",              Code::LT},
        {"LE",              Code::LE},

        {"POP",             Code::POP},
        {"PRINT",           Code::PRINT},
        {"HALT",            Code::HALT},
        {"INVALID",         Code::INVALID},
    };

    auto it = map.find(string);
    if (it != map.end()) {
        return it.value();
    }
    return Code::INVALID;
}

}
