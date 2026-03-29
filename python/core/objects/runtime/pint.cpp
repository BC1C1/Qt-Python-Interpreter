#include "pint.h"
#include "core/objects/runtime/pstr.h"
#include "core/objects/runtime/pfloat.h"
#include "core/objects/runtime/pbool.h"

namespace Py {
PInt::PInt(int value) : PObject(typeMap.at(Type::Int)), value(value)
{

}

QString PInt::toString() const
{
    return QString(u8"%1").arg(value);
}

QVariant PInt::getValue() const
{
    return QVariant(value);
}

PObject::pointer PInt::asInt() const
{
    return makeShared<PInt>(this->value);
}

PObject::pointer PInt::asFloat() const
{
    return makeShared<PFloat>(this->value);
}

PObject::pointer PInt::asBool() const
{
    return makeShared<PBool>(this->value == 0 ? false : true);
}

PObject::pointer PInt::asString() const
{
    return makeShared<PStr>(QString("%1").arg(this->value));
}

PObject::pointer PInt::__add__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Type::Int: {
        return makeShared<PInt>(value + other->getValue().toInt());
    }
    case Type::Float: {
        return makeShared<PFloat>(double(value) + other->getValue().toDouble());
    }
    case Type::Bool: {
        int val = other->getValue().toBool() ? 1 : 0;
        return makeShared<PInt>(value + val);
    }
    case Type::Undefined:
    default:
        defaultOpError(u8"+", other);
        return nullptr; // 无效语句用于消除警告
    }
}

PObject::pointer PInt::__sub__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Type::Int: {
        return makeShared<PInt>(value - other->getValue().toInt());
    }
    case Type::Float: {
        return makeShared<PFloat>(double(value) - other->getValue().toDouble());
    }
    case Type::Bool: {
        int val = other->getValue().toBool() ? 1 : 0;
        return makeShared<PInt>(value - val);
    }
    case Type::None: {
        return makeShared<PInt>(-value);
    }
    case Type::Undefined:
    default:
        defaultOpError(u8"-", other);
        return nullptr;
    }
}

PObject::pointer PInt::__mul__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Type::Int: {
        return makeShared<PInt>(value * other->getValue().toInt());
    }
    case Type::Float: {
        return makeShared<PFloat>(double(value) * other->getValue().toDouble());
    }
    case Type::Bool: {
        int val = other->getValue().toBool() ? 1 : 0;
        return makeShared<PInt>(value * val);
    }
    case Type::Undefined:
    default:
        defaultOpError(u8"*", other);
        return nullptr;
    }
}

PObject::pointer PInt::__truediv__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Type::Int: {
        int otherVal = other->getValue().toInt();
        if (otherVal == 0)
            throw std::runtime_error(u8"ZeroDivisionError: 不可除以0");
        return makeShared<PFloat>(double(value) / otherVal);
    }
    case Type::Float: {
        double otherVal = other->getValue().toDouble();
        if (qFuzzyCompare(otherVal, 0.0)) {
            throw std::runtime_error(u8"ZeroDivisionError: 不可除以0");
        }
        return makeShared<PFloat>(double(value) / otherVal);
    }
    case Type::Bool: {
        bool otherVal = other->getValue().toBool();
        if (!otherVal) {
            throw std::runtime_error(u8"ZeroDivisionError: 不可除以0");
        }
        return makeShared<PFloat>(double(value) / 1.0);
    }
    case Type::Undefined:
    default:
        defaultOpError(u8"/", other);
        return nullptr;
    }
}

PObject::pointer PInt::__mod__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Type::Int: {
        int otherVal = other->getValue().toInt();
        if (otherVal == 0) {
            throw std::runtime_error(u8"ZeroDivisionError: 取模不可除以0");
        }
        return makeShared<PInt>(value % otherVal);
    }
    case Type::Float: {
        double otherVal = other->getValue().toDouble();
        if (qFuzzyCompare(otherVal, 0.0)) {
            throw std::runtime_error(u8"ZeroDivisionError: 取模不可除以0");
        }
        double result = fmod(double(value), otherVal);
        return makeShared<PFloat>(result);
    }
    case Type::Undefined:
    default:
        defaultOpError(u8"%", other);
        return nullptr;
    }
}

PObject::pointer PInt::__pow__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Type::Int: {
        int otherVal = other->getValue().toInt();
        if (value == 0 && otherVal < 0) {
            throw std::runtime_error(u8"ZeroDivisionError: 0的负数次幂无意义");
        }
        return makeShared<PInt>(static_cast<int>(pow(value, otherVal)));
    }
    case Type::Float: {
        double otherVal = other->getValue().toDouble();
        if (value == 0 && otherVal < 0) {
            throw std::runtime_error(u8"ZeroDivisionError: 0的负数次幂无意义");
        }
        double result = pow(double(value), otherVal);
        return makeShared<PFloat>(result);
    }
    case Type::Bool: {
        bool otherVal = other->getValue().toBool();
        int exp = otherVal ? 1 : 0;
        return makeShared<PInt>(static_cast<int>(pow(value, exp)));
    }
    case Type::Undefined:
    default:
        defaultOpError(u8"**", other);
        return nullptr;
    }
}

PObject::pointer PInt::__eq__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Type::Int: {
        bool val = (value == other->getValue().toInt());
        return makeShared<PBool>(val);
    }
    case Type::Float: {
        bool val = (double(value) == other->getValue().toDouble());
        return makeShared<PBool>(val);
    }
    case Type::Bool: {
        bool otherVal = other->getValue().toBool();
        bool val = (value == (otherVal ? 1 : 0));
        return makeShared<PBool>(val);
    }
    case Type::None: {
        return makeShared<PBool>(false);
    }
    case Type::Undefined:
    default:
        defaultOpError(u8"==", other);
        return nullptr;
    }
}

PObject::pointer PInt::__ne__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Type::Int: {
        bool val = (value != other->getValue().toInt());
        return makeShared<PBool>(val);
    }
    case Type::Float: {
        bool val = (double(value) != other->getValue().toDouble());
        return makeShared<PBool>(val);
    }
    case Type::Bool: {
        bool otherVal = other->getValue().toBool();
        bool val = (value != (otherVal ? 1 : 0));
        return makeShared<PBool>(val);
    }
    case Type::None: {
        return makeShared<PBool>(true);
    }
    case Type::Undefined:
    default:
        defaultOpError(u8"!=", other);
        return nullptr;
    }
}

PObject::pointer PInt::__lt__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Type::Int: {
        bool val = (value < other->getValue().toInt());
        return makeShared<PBool>(val);
    }
    case Type::Float: {
        bool val = (double(value) < other->getValue().toDouble());
        return makeShared<PBool>(val);
    }
    case Type::Bool: {
        bool otherVal = other->getValue().toBool();
        bool val = (value < (otherVal ? 1 : 0));
        return makeShared<PBool>(val);
    }
    case Type::Undefined:
    default:
        defaultOpError(u8"<", other);
        return nullptr;
    }
}

PObject::pointer PInt::__le__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Type::Int: {
        bool val = (value <= other->getValue().toInt());
        return makeShared<PBool>(val);
    }
    case Type::Float: {
        bool val = (double(value) <= other->getValue().toDouble());
        return makeShared<PBool>(val);
    }
    case Type::Bool: {
        bool otherVal = other->getValue().toBool();
        bool val = (value <= (otherVal ? 1 : 0));
        return makeShared<PBool>(val);
    }
    case Type::Undefined:
    default:
        defaultOpError(u8"<=", other);
        return nullptr;
    }
}

PObject::pointer PInt::__gt__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Type::Int: {
        bool val = (value > other->getValue().toInt());
        return makeShared<PBool>(val);
    }
    case Type::Float: {
        bool val = (double(value) > other->getValue().toDouble());
        return makeShared<PBool>(val);
    }
    case Type::Bool: {
        bool otherVal = other->getValue().toBool();
        bool val = (value > (otherVal ? 1 : 0));
        return makeShared<PBool>(val);
    }
    case Type::Undefined:
    default:
        defaultOpError(u8">", other);
        return nullptr;
    }
}

PObject::pointer PInt::__ge__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Type::Int: {
        bool val = (value >= other->getValue().toInt());
        return makeShared<PBool>(val);
    }
    case Type::Float: {
        bool val = (double(value) >= other->getValue().toDouble());
        return makeShared<PBool>(val);
    }
    case Type::Bool: {
        bool otherVal = other->getValue().toBool();
        bool val = (value >= (otherVal ? 1 : 0));
        return makeShared<PBool>(val);
    }
    case Type::Undefined:
    default:
        defaultOpError(u8">=", other);
        return nullptr;
    }
}

PObject::pointer PInt::__and__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Type::Int: {
        int otherVal = other->getValue().toInt();
        return makeShared<PInt>(value & otherVal);
    }
    case Type::Bool: {
        bool otherVal = other->getValue().toBool();
        return makeShared<PInt>(value & (otherVal ? 1 : 0));
    }
    case Type::Undefined:
    default:
        defaultOpError(u8"&/and", other);
        return nullptr;
    }
}

PObject::pointer PInt::__or__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Type::Int: {
        int otherVal = other->getValue().toInt();
        return makeShared<PInt>(value | otherVal);
    }
    case Type::Bool: {
        bool otherVal = other->getValue().toBool();
        return makeShared<PInt>(value | (otherVal ? 1 : 0));
    }
    case Type::Undefined:
    default:
        defaultOpError(u8"|/or", other);
        return nullptr;
    }
}

PObject::pointer PInt::__not__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type)
    {
    case Type::None: {
        int result = ~value;
        return makeShared<PInt>(result);
    }
    case Type::Undefined:
    default:
        defaultOpError(u8"~", other);
        return nullptr;
    }
}

PInt::~PInt()
{

}
}


