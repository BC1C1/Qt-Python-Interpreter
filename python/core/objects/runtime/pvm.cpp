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
#include "PInstance.h"
#include "PClass.h"
namespace vm {
PVM::PVM(const QVector<Instruction> &codes, QObject *parent) : QObject(parent), PC(0), isRunning(false)
{
    auto defaultEnvir = makeShared<Environment>(nullptr, this);
    callFrameStack.push(makeCallFrame(0, defaultEnvir, Instruction::toByteArray(codes)));
    currCodes = codes;
    initRootObject();
    PClass::object = object;
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
    case Code::HALT:                halt_execute();                     return;

    case Code::LOAD_INT:            load_int_execute(operand);          break;
    case Code::LOAD_TRUE:           load_true_execute();                break;
    case Code::LOAD_FALSE:          load_false_execute();               break;
    case Code::LOAD_FLOAT:          load_float_execute(operand);        break;
    case Code::LOAD_STRING:         load_string_execute(operand);       break;
    case Code::LOAD_LIST:           load_list_execute();                break;
    case Code::LOAD_DICT:           load_dict_execute();                break;
    case Code::LOAD_NONE:           load_none_execute();                break;
    case Code::LOAD_NAME:           load_name_execute(operand);         break;
    case Code::LOAD_INDEX:          load_index_execute();               break;
    case Code::LOAD_ATTR:           load_attr_execute(operand, isGo);   break;

    case Code::STORE_VAR:           store_var_execute(operand);         break;
    case Code::STORE_INDEX:         store_index_execute();              break;
    case Code::STORE_ATTR:          store_attr_execute(operand, isGo);        break;

    case Code::CREATE_ITER:         create_iter_execute();              break;
    case Code::ITER_NEXT:           iter_next_execute(operand);         break;

    case Code::LOOP_START_FOR:      loop_start_for_execute();           break;
    case Code::LOOP_START_WHILE:    loop_start_while_execute();         break;
    case Code::CONTINUE:            continue_execute(isGo);             break;
    case Code::BREAK:               break_execute(isGo);                break;
    case Code::LOOP_FOR_END:        loop_for_end_execute();             break;
    case Code::LOOP_WHILE_END:      loop_while_end_execute();           break;

    case Code::CREATE_FUNCTION:     create_function_execute(operand);   break;
    case Code::CALL:                call_execute(isGo);                 break;
    case Code::RETURN:              return_execute(isGo);               break;
    case Code::CREATE_CLASS:        create_class_execute(operand);      break;

    case Code::ADD:                 add_execute(isGo);                  break;
    case Code::SUB:                 sub_execute(isGo);                  break;
    case Code::MUL:                 mul_execute(isGo);                  break;
    case Code::DIV:                 div_execute(isGo);                  break;
    case Code::MOD:                 mod_execute(isGo);                  break;
    case Code::POW:                 pow_execute(isGo);                  break;

    case Code::EQ:                  eq_execute(isGo);                   break;
    case Code::NEQ:                 neq_execute(isGo);                  break;
    case Code::GT:                  gt_execute(isGo);                   break;
    case Code::GE:                  ge_execute(isGo);                   break;
    case Code::LT:                  lt_execute(isGo);                   break;
    case Code::LE:                  le_execute(isGo);                   break;

    case Code::POP:                 popValue();                         break;
    case Code::PRINT:               print_execute();                    break;

    case Code::JUMP_IF_FALSE:       jump_if_false_execute(operand, isGo);    break;
    case Code::JUMP:                jump_execute(operand, isGo);             break;

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

void PVM::halt_execute()
{
    qDebug() << "收到HALT停机指令";
    stop();
}

inline void PVM::initRootObject() {
    object = new PClass(
        "!object",                // 不可能被使用的类名
        QHash<QString, pointer>(),
        QHash<QString, pointer>(),
        QVector<pointer>()       
    );
}

void PVM::load_list_execute()
{
    auto sizeObj = popValue();
    int size = sizeObj->getValue().toInt();
    QVector<pointer> inner;
    inner.reserve(size);
    for (int i = 0; i < size; i++) {
        inner.push_back(popValue());
    }
    std::reverse(inner.begin(), inner.end());
    auto obj = makeShared<Py::PList>(inner);
    pushValue(obj);
}

void PVM::load_dict_execute()
{
    auto size = popValue()->getValue().toInt();
    QHash<pointer, pointer> inner;
    inner.reserve(size);
    for (int i = 0; i < size; i++) {
        inner[popValue()] = popValue();
    }
    auto obj = makeShared<Py::PDict>(std::move(inner));
    pushValue(obj);
}

void PVM::load_name_execute(QVariant operand)
{
    //dumpStack("loadname begin");
    QString name = operand.toString();
    auto obj = currEnvir()->getObj(name);
    //if (!obj)
    //    qDebug() << "obj is nullptr";
    //else
    //    qDebug() << "obj.name is" << name << "\nobj.type is"
    //            << TypeToString(obj->getType().type)
    //            << "obj.value is(toString)" << obj->toString();
    if (obj)
        pushValue(obj);
    if (!obj) {
        throwErrMsg(QString("变量 '%1' 未定义").arg(name).toUtf8().data());
    }
    //dumpStack("loadname end");
}

void PVM::load_int_execute(QVariant operand)
{
    int value = operand.toInt();
    pushValue(makeShared<PInt>(value));
}

void PVM::load_float_execute(QVariant operand)
{
    double value = operand.toDouble();
    pushValue(makeShared<PFloat>(value));
}

void PVM::load_string_execute(QVariant operand)
{
    auto obj = makeShared<Py::PStr>(operand.toString());
    pushValue(obj);
}

void PVM::store_attr_execute(QVariant operand, bool& isGo)
{
    auto toAttr = popValue();
    auto attrName = operand.toString();
    auto value = popValue();
    if (toAttr->getType().type == Type::Instance) {
        auto result = toAttr->__getattribute__(attrName);
        auto instance = dynamicPointerCast<Py::PInstance>(toAttr);
        auto functions = instance->getClassObj()->getFunctions();
        auto iter = functions.find("__setattr__");
        if (iter != functions.end()) {
            auto newEnvir = createNewEnvironment(currEnvir());
            auto funcObj = dynamicPointerCast<PFunction>(iter.value());
            auto param = QVector<pointer>{ toAttr, makeShared<PStr>(attrName), value };
            funcObj->__call__(makeShared<PList>(param), newEnvir);
            auto newCode = funcObj->getCodeObj();
            callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
            currCodes = newCode;
            isGo = false;
            PC = 0;
            return;
        }
    }
    toAttr->__setattribute__(attrName, value);
}

void PVM::store_index_execute()
{
    auto index = popValue();
    auto toIndex = popValue();
    auto rightValue = popValue();
    toIndex->__setitem__(index, rightValue);
}

void PVM::loop_start_for_execute()
{
    int breakPC = popValue()->getValue().toInt();
    int continuePC = popValue()->getValue().toInt();
    pushFrame(makeForLoopFrame(continuePC, breakPC));
}

void PVM::loop_start_while_execute()
{
    int breakPC = popValue()->getValue().toInt();
    int continuePC = popValue()->getValue().toInt();
    pushFrame(makeWhileLoopFrame(continuePC, breakPC));

}

void PVM::load_attr_execute(QVariant operand, bool& isGo)
{
    auto toAttr = popValue();
    auto attrName = operand.toString();
    qDebug() << "value of toAttr is: " << toAttr->toString();
    if (toAttr->getType().type == Type::Instance) {
        auto result = toAttr->__getattribute__(attrName);
        if (result) {
            pushValue(result);
            return;
        }
        auto instance = dynamicPointerCast<Py::PInstance>(toAttr);
        auto functions = instance->getClassObj()->getFunctions();
        auto iter = functions.find("__getattr__");
        if (iter != functions.end()) {
            auto newEnvir = createNewEnvironment(currEnvir());
            auto funcObj = dynamicPointerCast<PFunction>(iter.value());
            auto param = QVector<pointer>{ toAttr, makeShared<PStr>(attrName) };
            funcObj->__call__(makeShared<PList>(param), newEnvir);
            auto newCode = funcObj->getCodeObj();
            callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
            currCodes = newCode;
            isGo = false;
            PC = 0;
            return;
        }
    }
    auto obj = toAttr->__getattribute__(attrName);
    pushValue(obj);
}

void PVM::store_var_execute(QVariant operand)
{
    auto left = operand.toString();
    auto right = popValue();
    currEnvir()->assign(left, right);
    //qDebug() << "var name : " << left << "get value: " << right->toString();
    //qDebug() << "var type is:" << TypeToString(right->getType().type);
}

void PVM::create_iter_execute()
{
    //        dumpStack("before create iterator");
    auto obj = popValue();
    qDebug() << TypeToString(obj->getType().type);
    auto iter = obj->__iter__();
    pushValue(iter);
    qDebug() << "迭代器是否为空：" << (iter == nullptr);
    //        dumpStack("after create iterator");
}

void PVM::iter_next_execute(QVariant operand)
{
    //        dumpStack("before next iterator");
    auto loopVarName = operand.toString();
    auto iter = topValue();
    auto obj = iter->__next__();
    currEnvir()->assign(loopVarName, obj);
    //        dumpStack("after next iterator");
}

void PVM::load_index_execute()
{
    auto index = popValue();
    auto toIndex = popValue();
    auto obj = toIndex->__getitem__(index);
    pushValue(obj);
}

void PVM::load_none_execute()
{
    pushValue(makeShared<Py::PNone>());
}

void PVM::load_true_execute()
{
    pushValue(makeShared<PBool>(true));
}

void PVM::load_false_execute()
{
    pushValue(makeShared<PBool>(false));
}

void PVM::continue_execute(bool& isGo)
{
    BlockFrame currFrame;
    for (auto c = blockFrameStack.rbegin(); c != blockFrameStack.rend(); ++c) {
        if (c->blockType == BlockType::LOOP_FOR || c->blockType == BlockType::LOOP_WHILE) {
            currFrame = *c;
            break;
        }
    }

    isGo = false;
    PC = currFrame.continuePC;
}

void PVM::break_execute(bool& isGo)
{
    BlockFrame currFrame;
    for (auto c = blockFrameStack.rbegin(); c != blockFrameStack.rend(); ++c) {
        if (c->blockType == BlockType::LOOP_FOR || c->blockType == BlockType::LOOP_WHILE) {
            currFrame = *c;
            break;
        }
    }

    isGo = false;
    PC = currFrame.breakPC;
}

void PVM::loop_for_end_execute()
{
    if (topFrame().blockType != BlockType::LOOP_FOR) {
        throwErrMsg("不匹配的块"); // 编译期错误
    }
    popValue(); // 弹出迭代器

    popFrame();
}

void PVM::loop_while_end_execute()
{
    if (topFrame().blockType != BlockType::LOOP_WHILE) {
        throwErrMsg("不匹配的块"); // 编译期错误
    }

    popFrame();
}

void PVM::create_function_execute(QVariant operand)
{
    auto code = operand.toByteArray();
    auto isClassFunction = popValue()->getValue().toBool();
    auto params = popValue();
    auto name = popValue();
    auto functionObj = makeShared<Py::PFunction>(params, code, name, isClassFunction);
    pushValue(functionObj);
    qDebug() << "以下是函数内部字节码";
    for (const auto& innerCode : Instruction::fromByteArray(code)) {
        qDebug() << innerCode.toString();
    }
    qDebug() << "函数字节码结束";
}

void PVM::call_execute(bool& isGo)
{
    auto caller = popValue();
    pointer trueParam = popValue();
    auto funcCaller = popValue();
    auto newEnvir = createNewEnvironment(currEnvir());
    if (funcCaller->getType().type == Type::Instance)
    {
        auto listobj = dynamicPointerCast<PList>(trueParam);
        QVector<pointer> p = { funcCaller };
        p.append(listobj->getTrueValue());
        trueParam = makeShared<PList>(p);
    }
    else if (caller->getType().type == Type::Class) {
        auto classObj = dynamicPointerCast<PClass>(caller);
        auto obj = caller->__call__(trueParam, currEnvir());
        QVector<pointer> p = { obj };
        pushValue(obj);
        p.append(dynamicPointerCast<PList>(trueParam)->getTrueValue());
        trueParam = makeShared<PList>(p);
        auto initFunc = classObj->getFunctions()["__init__"];
        initFunc->__call__(makeShared<PList>(p), newEnvir);
        auto initFObj = dynamicPointerCast<PFunction>(initFunc);
        auto newCode = initFObj->getCodeObj();
        callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
        currCodes = newCode;
        isGo = false;
        PC = 0;
        return;
    }
    // 其它情况不需要加
    auto function = dynamicPointerCast<PFunction>(caller);
    auto newCode = function->getCodeObj();
    callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
    currCodes = newCode;
    isGo = false;
    PC = 0;
    return;
    //auto caller = popValue(); 
    //qDebug() << "caller.type is: " << TypeToString(caller->getType().type);
    //qDebug() << "caller.value is(toString): " << caller->toString();
    //auto params = popValue();
    //auto newEnvir = createNewEnvironment(currEnvir());
    //switch (caller->getType().type)
    //{
    //case Type::FunctionDefine: {
    //    auto function = dynamicPointerCast<Py::PFunction>(caller);
    //    caller->__call__(params, newEnvir);
    //    auto newCode = function->getCodeObj();
    //    callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
    //    currCodes = newCode;
    //    isGo = false;
    //    PC = 0;
    //    break;
    //}
    //case Type::Class: {
    //    auto classObj = dynamicPointerCast<Py::PClass>(caller);
    //    if (!classObj) {
    //        throwErrMsg(u8"无效的caller");
    //    }
    //    for (const auto& p : classObj->getFunctions()) {
    //        qDebug() << p->toString();
    //    }
    //    auto obj = classObj->__call__(params, newEnvir);
    //    pushValue(obj); // 先压入栈，后面的是闭合操作，会把操作数栈顶重新变回obj，就能return回去
    //    auto param = dynamicPointerCast<PList>(params);
    //    auto initFunc = classObj->getFunctions()["__init__"];
    //    QVector<pointer> p = { obj };
    //    p.append(param->getTrueValue());
    //    initFunc->__call__(makeShared<PList>(p), newEnvir);
    //    auto initFObj = dynamicPointerCast<PFunction>(initFunc);
    //    auto newCode = initFObj->getCodeObj();
    //    callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
    //    currCodes = newCode;
    //    isGo = false;
    //    PC = 0;
    //    break;
    //}
    //default:
    //    break;
    //}
}

void PVM::return_execute(bool& isGo)
{
    //dumpStack("return begin");
    // auto returnValue = popValue(); 这个栈是同一个，无所谓
    auto frame = callFrameStack.pop();
    currCodes = Instruction::fromByteArray(callFrameStack.top().codes);
    // 环境自动回复，currEnvir是栈顶的
    isGo = false;
    PC = frame.fromWhere;
    //dumpStack("return end");
}

void PVM::create_class_execute(QVariant operand)
{
    auto className = operand.toString();
    auto parentNumber = popValue()->getValue().toInt();
    QVector<pointer> parents;
    for (int i = 0; i < parentNumber; i++) {
        parents.append(popValue());
    }
    std::reverse(parents.begin(), parents.end());
    if (parents.size() == 0) {
        parents.append(QSharedPointer<PClass>(object));
    }
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
    auto classObj = makeShared<Py::PClass>(className, functions, staticMembers, parents);
    currEnvir()->assign(operand.toString(), classObj);
}

void PVM::add_execute(bool& isGo)
{
    auto obj2 = popValue();
    auto obj1 = popValue();
    if (obj1->getType().type == Type::Instance) {
        auto instance = dynamicPointerCast<PInstance>(obj1);
        auto& functions = instance->getClassObj()->getFunctions();
        auto iter = functions.find("__add__");
        if (iter != functions.end()) {
            auto func = dynamicPointerCast<PFunction>(iter.value());
            auto newCode = func->getCodeObj();
            auto newEnvir = createNewEnvironment(currEnvir());
            auto param = QVector<pointer>{ obj1, obj2 };
            func->__call__(makeShared<PList>(param), newEnvir);
            callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
            currCodes = newCode;
            isGo = false;
            PC = 0;
            return;
        }
    }
    else
        pushValue(obj1->__add__(obj2));
}
void PVM::sub_execute(bool& isGo)
{
    auto obj2 = popValue();
    auto obj1 = popValue();
    if (obj1->getType().type == Type::Instance) {
        auto instance = dynamicPointerCast<PInstance>(obj1);
        auto& functions = instance->getClassObj()->getFunctions();
        auto iter = functions.find("__sub__");
        if (iter != functions.end()) {
            auto func = dynamicPointerCast<PFunction>(iter.value());
            auto newCode = func->getCodeObj();
            auto newEnvir = createNewEnvironment(currEnvir());
            auto param = QVector<pointer>{ obj1, obj2 };
            func->__call__(makeShared<PList>(param), newEnvir);
            callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
            currCodes = newCode;
            isGo = false;
            PC = 0;
            return;
        }
    }
    else
        pushValue(obj1->__sub__(obj2));
}
void PVM::mul_execute(bool& isGo)
{
    auto obj2 = popValue();
    auto obj1 = popValue();
    if (obj1->getType().type == Type::Instance) {
        auto instance = dynamicPointerCast<PInstance>(obj1);
        auto& functions = instance->getClassObj()->getFunctions();
        auto iter = functions.find("__mul__");
        if (iter != functions.end()) {
            auto func = dynamicPointerCast<PFunction>(iter.value());
            auto newCode = func->getCodeObj();
            auto newEnvir = createNewEnvironment(currEnvir());
            auto param = QVector<pointer>{ obj1, obj2 };
            func->__call__(makeShared<PList>(param), newEnvir);
            callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
            currCodes = newCode;
            isGo = false;
            PC = 0;
            return;
        }
    }
    else
        pushValue(obj1->__mul__(obj2));
}
void PVM::div_execute(bool& isGo)
{
    auto obj2 = popValue();
    auto obj1 = popValue();
    if (obj1->getType().type == Type::Instance) {
        auto instance = dynamicPointerCast<PInstance>(obj1);
        auto& functions = instance->getClassObj()->getFunctions();
        auto iter = functions.find("__truediv__");
        if (iter != functions.end()) {
            auto func = dynamicPointerCast<PFunction>(iter.value());
            auto newCode = func->getCodeObj();
            auto newEnvir = createNewEnvironment(currEnvir());
            auto param = QVector<pointer>{ obj1, obj2 };
            func->__call__(makeShared<PList>(param), newEnvir);
            callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
            currCodes = newCode;
            isGo = false;
            PC = 0;
            return;
        }
    }
    else
        pushValue(obj1->__truediv__(obj2));
}
// 取模
void PVM::mod_execute(bool& isGo)
{
    auto obj2 = popValue();
    auto obj1 = popValue();
    if (obj1->getType().type == Type::Instance) {
        auto instance = dynamicPointerCast<PInstance>(obj1);
        auto& functions = instance->getClassObj()->getFunctions();
        auto iter = functions.find("__mod__");
        if (iter != functions.end()) {
            auto func = dynamicPointerCast<PFunction>(iter.value());
            auto newCode = func->getCodeObj();
            auto newEnvir = createNewEnvironment(currEnvir());
            auto param = QVector<pointer>{ obj1, obj2 };
            func->__call__(makeShared<PList>(param), newEnvir);
            callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
            currCodes = newCode;
            isGo = false;
            PC = 0;
            return;
        }
    }
    pushValue(obj1->__mod__(obj2));
}

// 幂运算
void PVM::pow_execute(bool& isGo)
{
    auto obj2 = popValue();
    auto obj1 = popValue();
    if (obj1->getType().type == Type::Instance) {
        auto instance = dynamicPointerCast<PInstance>(obj1);
        auto& functions = instance->getClassObj()->getFunctions();
        auto iter = functions.find("__pow__");
        if (iter != functions.end()) {
            auto func = dynamicPointerCast<PFunction>(iter.value());
            auto newCode = func->getCodeObj();
            auto newEnvir = createNewEnvironment(currEnvir());
            auto param = QVector<pointer>{ obj1, obj2 };
            func->__call__(makeShared<PList>(param), newEnvir);
            callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
            currCodes = newCode;
            isGo = false;
            PC = 0;
            return;
        }
    }
    pushValue(obj1->__pow__(obj2));
}

// 等于 ==
void PVM::eq_execute(bool& isGo)
{
    auto obj2 = popValue();
    auto obj1 = popValue();
    if (obj1->getType().type == Type::Instance) {
        auto instance = dynamicPointerCast<PInstance>(obj1);
        auto& functions = instance->getClassObj()->getFunctions();
        auto iter = functions.find("__eq__");
        if (iter != functions.end()) {
            auto func = dynamicPointerCast<PFunction>(iter.value());
            auto newCode = func->getCodeObj();
            auto newEnvir = createNewEnvironment(currEnvir());
            auto param = QVector<pointer>{ obj1, obj2 };
            func->__call__(makeShared<PList>(param), newEnvir);
            callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
            currCodes = newCode;
            isGo = false;
            PC = 0;
            return;
        }
    }
    pushValue(obj1->__eq__(obj2));
}

// 不等于 !=
void PVM::neq_execute(bool& isGo)
{
    auto obj2 = popValue();
    auto obj1 = popValue();
    if (obj1->getType().type == Type::Instance) {
        auto instance = dynamicPointerCast<PInstance>(obj1);
        auto& functions = instance->getClassObj()->getFunctions();
        auto iter = functions.find("__ne__");
        if (iter != functions.end()) {
            auto func = dynamicPointerCast<PFunction>(iter.value());
            auto newCode = func->getCodeObj();
            auto newEnvir = createNewEnvironment(currEnvir());
            auto param = QVector<pointer>{ obj1, obj2 };
            func->__call__(makeShared<PList>(param), newEnvir);
            callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
            currCodes = newCode;
            isGo = false;
            PC = 0;
            return;
        }
    }
    pushValue(obj1->__ne__(obj2));
}

// 大于 >
void PVM::gt_execute(bool& isGo)
{
    auto obj2 = popValue();
    auto obj1 = popValue();
    if (obj1->getType().type == Type::Instance) {
        auto instance = dynamicPointerCast<PInstance>(obj1);
        auto& functions = instance->getClassObj()->getFunctions();
        auto iter = functions.find("__gt__");
        if (iter != functions.end()) {
            auto func = dynamicPointerCast<PFunction>(iter.value());
            auto newCode = func->getCodeObj();
            auto newEnvir = createNewEnvironment(currEnvir());
            auto param = QVector<pointer>{ obj1, obj2 };
            func->__call__(makeShared<PList>(param), newEnvir);
            callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
            currCodes = newCode;
            isGo = false;
            PC = 0;
            return;
        }
    }
    pushValue(obj1->__gt__(obj2));
}

// 大于等于 >=
void PVM::ge_execute(bool& isGo)
{
    auto obj2 = popValue();
    auto obj1 = popValue();
    if (obj1->getType().type == Type::Instance) {
        auto instance = dynamicPointerCast<PInstance>(obj1);
        auto& functions = instance->getClassObj()->getFunctions();
        auto iter = functions.find("__ge__");
        if (iter != functions.end()) {
            auto func = dynamicPointerCast<PFunction>(iter.value());
            auto newCode = func->getCodeObj();
            auto newEnvir = createNewEnvironment(currEnvir());
            auto param = QVector<pointer>{ obj1, obj2 };
            func->__call__(makeShared<PList>(param), newEnvir);
            callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
            currCodes = newCode;
            isGo = false;
            PC = 0;
            return;
        }
    }
    pushValue(obj1->__ge__(obj2));
}

// 小于 <
void PVM::lt_execute(bool& isGo)
{
    auto obj2 = popValue();
    auto obj1 = popValue();
    if (obj1->getType().type == Type::Instance) {
        auto instance = dynamicPointerCast<PInstance>(obj1);
        auto& functions = instance->getClassObj()->getFunctions();
        auto iter = functions.find("__lt__");
        if (iter != functions.end()) {
            auto func = dynamicPointerCast<PFunction>(iter.value());
            auto newCode = func->getCodeObj();
            auto newEnvir = createNewEnvironment(currEnvir());
            auto param = QVector<pointer>{ obj1, obj2 };
            func->__call__(makeShared<PList>(param), newEnvir);
            callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
            currCodes = newCode;
            isGo = false;
            PC = 0;
            return;
        }
    }
    pushValue(obj1->__lt__(obj2));
}

// 小于等于 <=
void PVM::le_execute(bool& isGo)
{
    auto obj2 = popValue();
    auto obj1 = popValue();
    if (obj1->getType().type == Type::Instance) {
        auto instance = dynamicPointerCast<PInstance>(obj1);
        auto& functions = instance->getClassObj()->getFunctions();
        auto iter = functions.find("__le__");
        if (iter != functions.end()) {
            auto func = dynamicPointerCast<PFunction>(iter.value());
            auto newCode = func->getCodeObj();
            auto newEnvir = createNewEnvironment(currEnvir());
            auto param = QVector<pointer>{ obj1, obj2 };
            func->__call__(makeShared<PList>(param), newEnvir);
            callFrameStack.push(makeCallFrame(PC + 1, newEnvir, Instruction::toByteArray(newCode)));
            currCodes = newCode;
            isGo = false;
            PC = 0;
            return;
        }
    }
    pushValue(obj1->__le__(obj2));
}

void PVM::print_execute()
{
    auto toPrint = popValue();
    qDebug() << toPrint->toString();
}

void PVM::jump_if_false_execute(const QVariant& operand, bool& isGo)
{
    auto value = popValue()->asBool()->getValue().toBool();
    if (!value) {
        isGo = false;
        PC = operand.toInt();
    }
}

void PVM::jump_execute(const QVariant& operand, bool& isGo)
{
    isGo = false;
    PC = operand.toInt();
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

PVM::~PVM()
{
    delete object;
}

QString CodeToQString(Code code)
{
    switch (code) {
    case Code::LOAD_INT:        return "LOAD_INT";
    case Code::LOAD_FLOAT:      return "LOAD_FLOAT";
    case Code::LOAD_STRING:     return "LOAD_STRING";
    case Code::LOAD_LIST:       return "LOAD_LIST";
    case Code::LOAD_NONE:       return "LOAD_NONE";
    case Code::LOAD_TRUE:       return "LOAD_TRUE";
    case Code::LOAD_FALSE:      return "LOAD_FALSE";

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
