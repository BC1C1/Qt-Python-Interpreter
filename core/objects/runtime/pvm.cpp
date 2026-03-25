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
        auto list = operand.toList();
        QVector<pointer> inner;
        for (const auto& o : list) {
            auto p = getBasicObject(o);
            if (p)
                inner.push_back(p);
        }
        auto obj = makeShared<Py::PList>(inner);
        pushValue(obj);
        break;
    }
    case Code::LOAD_NAME: {
        QString name = operand.toString();
        auto obj = globalEnvir->getObj(name);
        if (obj)
            pushValue(obj);
        if (!obj) {
            throwErrMsg(QString("变量 '%1' 未定义").arg(name).toUtf8().data());
        }
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
        globalEnvir->assign(left, right);
        //        qDebug() << "var name : " << left << "get value: " << right->toString();
        break;
    }
    case Code::STORE_INDEX: {
        auto rightValue = popValue();
        auto index = popValue();
        auto toIndex = popValue();
        toIndex->__setitem__(index, rightValue);
        break;
    }
    case Code::STORE_ATTR: {
        auto value = popValue();
        auto toAttr = popValue();
        auto attrName = operand.toString();
        toAttr->__setattribute__(attrName, value);
        break;
    }
    case Code::CREATE_ITER: {
        auto obj = popValue();
        pushValue(obj->__iter__());
    }
    case Code::ITER_NEXT: {
        auto iter = popValue();
        pushValue(iter->__next__());
    }
    case Code::ADD: {
        auto obj2 = popValue();
        auto obj1 = popValue();
        pushValue(obj1->__add__(obj2));
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

void PVM::throwErrMsg(const std::string &msg)
{
    throw std::runtime_error(msg);
}

void PVM::run()
{
    try {
        while (isRunning && PC < codes.length()) {
            executeSingleCode(); // 这个函数在处理halt情况的时候会把isStart弄成false终止循环
        }
        if (PC >= codes.size())     qDebug() << "虚拟机停机：指令执行完毕（PC越界）";
        else                        qDebug() << "虚拟机停机：收到停止指令";
    } catch (std::runtime_error& e) {
        qDebug() << e.what();
    }

}

QString CodeToQString(Code code)
{
    switch (code)
    {
    case Code::LOAD_INT:    return "LOAD_INT";
    case Code::LOAD_FLOAT:  return "LOAD_FLOAT";
    case Code::LOAD_STRING: return "LOAD_STRING";
    case Code::LOAD_NAME:   return "LOAD_NAME";
    case Code::STORE_VAR:   return "STORE_VAR";
    case Code::ADD:         return "ADD";
    case Code::SUB:         return "SUB";
    case Code::MUL:         return "MUL";
    case Code::DIV:         return "DIV";
    case Code::MOD:         return "MOD";
    case Code::POW:         return "POW";
    case Code::PRINT:       return "PRINT";
    case Code::JUMP:        return "JUMP";
    case Code::JUMP_IF_FALSE:return "JUMP_IF_FALSE";
    case Code::EQ:          return "EQ";
    case Code::NEQ:         return "NEQ";
    case Code::GT:          return "GT";
    case Code::GE:          return "GE";
    case Code::LT:          return "LT";
    case Code::LE:          return "LE";
    case Code::HALT:        return "HALT";
    default:                return "UNKNOWN_CODE";
    }
}

QString Instruction::toString() const {
    QString codeStr = CodeToQString(this->code) + " ";
    if (this->operand.isValid() && !this->operand.isNull()) {
        return QString("%1 %2 ").arg(codeStr).arg(this->operand.toString());
    }
    return codeStr;
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
    Code ret = Code::INVALID;

    // 加载字面量
    if (string == "LOAD_INT")           ret = Code::LOAD_INT;
    else if (string == "LOAD_FLOAT")    ret = Code::LOAD_FLOAT;
    else if (string == "LOAD_STRING")   ret = Code::LOAD_STRING;
    else if (string == "LOAD_NAME")     ret = Code::LOAD_NAME;

    // 赋值变量
    else if (string == "STORE_VAR")     ret = Code::STORE_VAR;
    else if (string == "STORE_INDEX")   ret = Code::STORE_INDEX;
    else if (string == "STORE_ATTR")    ret = Code::STORE_ATTR;

    // 跳转
    else if (string == "JUMP_IF_FALSE") ret = Code::JUMP_IF_FALSE;
    else if (string == "JUMP")          ret = Code::JUMP;

    // 算数
    else if (string == "ADD")           ret = Code::ADD;
    else if (string == "SUB")           ret = Code::SUB;
    else if (string == "MUL")           ret = Code::MUL;
    else if (string == "DIV")           ret = Code::DIV;
    else if (string == "MOD")           ret = Code::MOD;
    else if (string == "POW")           ret = Code::POW;

    // 比较
    else if (string == "EQ")            ret = Code::EQ;
    else if (string == "NEQ")           ret = Code::NEQ;
    else if (string == "GT")            ret = Code::GT;
    else if (string == "GE")            ret = Code::GE;
    else if (string == "LT")            ret = Code::LT;
    else if (string == "LE")            ret = Code::LE;

    // 打印/停机
    else if (string == "PRINT")         ret = Code::PRINT;
    else if (string == "HALT")          ret = Code::HALT;

    // 无匹配则返回默认的 Code::INVALID
    return ret;
}

}
