#include "pstr.h"
#include "core/objects/runtime/pint.h"
#include "core/objects/runtime/pfloat.h"
#include "core/objects/runtime/pbool.h"

namespace Py {

PStr::PStr(QString value) : PObject(typeMap.at(Type::Str)), value(value)
{

}

PStr::~PStr()
{

}

QVariant PStr::getValue() const
{
    return QVariant(value);
}

QString PStr::toString() const
{
    return value;
}

PObject::pointer PStr::__add__(const pointer& other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Py::Type::Str: {
        QString otherVal = other->getValue().toString();
        return makeShared<PStr>(value + otherVal);
    }
    case Py::Type::Int:
    case Py::Type::Float:
    case Py::Type::Bool: {
        QString otherVal = other->toString();
        return makeShared<PStr>(value + otherVal);
    }
    case Py::Type::Undefined:
    default:
        defaultOpError("+", other);
        return nullptr; // 消除警告
    }
}

PObject::pointer PStr::__mul__(const pointer& other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Py::Type::Int: {
        int times = other->getValue().toInt();
        if (times < 0) {
            throw runtime_error("TypeError: 字符串乘法不支持负数次数");
        }
        QString result = value.repeated(times);
        return makeShared<PStr>(result);
    }
    case Py::Type::Bool: {
        int times = other->getValue().toBool() ? 1 : 0;
        QString result = value.repeated(times);
        return makeShared<PStr>(result);
    }
    case Py::Type::Undefined:
    default:
        defaultOpError("*", other);
        return nullptr;
    }
}

// 等于（==）
PObject::pointer PStr::__eq__(const pointer& other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Py::Type::Str: {
        QString otherVal = other->getValue().toString();
        bool val = (value == otherVal);
        return makeShared<PBool>(val);
    }
    default: {
        return makeShared<PBool>(false);
    }
    }
}

// 不等于（!=）
PObject::pointer PStr::__ne__(const pointer& other) const
{
    auto eqResult = this->__eq__(other);
    if (!eqResult) {
        defaultOpError("!=", other);
        return nullptr;
    }
    bool val = !eqResult->getValue().toBool();
    return makeShared<PBool>(val);
}

// 小于
PObject::pointer PStr::__lt__(const pointer& other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Py::Type::Str: {
        QString otherVal = other->getValue().toString();
        bool val = (value < otherVal);
        return makeShared<PBool>(val);
    }
    case Py::Type::Undefined:
    default:
        defaultOpError("<", other);
        return nullptr;
    }
}

// 小于等于（<=）
PObject::pointer PStr::__le__(const pointer& other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Py::Type::Str: {
        QString otherVal = other->getValue().toString();
        bool val = (value <= otherVal);
        return makeShared<PBool>(val);
    }
    case Py::Type::Undefined:
    default:
        defaultOpError("<=", other);
        return nullptr;
    }
}

// 大于（>）
PObject::pointer PStr::__gt__(const pointer& other) const
{
    auto leResult = this->__le__(other);
    if (!leResult) {
        defaultOpError(">", other);
        return nullptr;
    }
    bool val = !leResult->getValue().toBool();
    return makeShared<PBool>(val);
}

// 大于等于（>=）
PObject::pointer PStr::__ge__(const pointer& other) const
{
    auto ltResult = this->__lt__(other);
    if (!ltResult) {
        defaultOpError(">=", other);
        return nullptr;
    }
    bool val = !ltResult->getValue().toBool();
    return makeShared<PBool>(val);
}

}

