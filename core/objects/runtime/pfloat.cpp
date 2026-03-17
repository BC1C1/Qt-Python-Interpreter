#include "pfloat.h"
#include "core/objects/runtime/pstr.h"
#include "core/objects/runtime/pint.h"
#include "core/objects/runtime/pbool.h"

namespace Py {

PFloat::PFloat(double value) : PObject(typeMap.at(Type::Float)), value(value)
{

}

QString PFloat::toString() const
{
    return QString(u8"%1").arg(value);
}

QVariant PFloat::getValue() const
{
    return QVariant(value);
}

PObject::pointer PFloat::asInt() const {
    return makeShared<PInt>(value);
}

PObject::pointer PFloat::asFloat() const {
    return makeShared<PFloat>(value);
}

PObject::pointer PFloat::asBool() const {
    return makeShared<PBool>(!qFuzzyCompare(value, 0.0));
}

PObject::pointer PFloat::asString() const {
    return makeShared<PStr>(toString());
}

PObject::pointer PFloat::__add__(const PObject::pointer &other) const {
    auto type = other->getType().type;
    switch (type) {
    case Type::Int: {
        double otherVal = other->getValue().toInt();
        return makeShared<PFloat>(value + otherVal);
    }
    case Type::Float: {
        double otherVal = other->getValue().toDouble();
        return makeShared<PFloat>(value + otherVal);
    }
    case Type::Bool: {
        double otherVal = other->getValue().toBool() ? 1.0 : 0.0;
        return makeShared<PFloat>(value + otherVal);
    }
    default:
        defaultOpError(u8"+", other);
        return nullptr;
    }
}

PObject::pointer PFloat::__sub__(const PObject::pointer &other) const {
    auto type = other->getType().type;
    switch (type) {
    case Type::Int: {
        double otherVal = other->getValue().toInt();
        return makeShared<PFloat>(value - otherVal);
    }
    case Type::Float: {
        double otherVal = other->getValue().toDouble();
        return makeShared<PFloat>(value - otherVal);
    }
    case Type::Bool: {
        double otherVal = other->getValue().toBool() ? 1.0 : 0.0;
        return makeShared<PFloat>(value - otherVal);
    }
    case Type::None: {
        return makeShared<PFloat>(-value); // 单目减
    }
    default:
        defaultOpError(u8"-", other);
        return nullptr;
    }
}

PObject::pointer PFloat::__mul__(const PObject::pointer &other) const {
    auto type = other->getType().type;
    switch (type) {
    case Type::Int: {
        double otherVal = other->getValue().toInt();
        return makeShared<PFloat>(value * otherVal);
    }
    case Type::Float: {
        double otherVal = other->getValue().toDouble();
        return makeShared<PFloat>(value * otherVal);
    }
    case Type::Bool: {
        double otherVal = other->getValue().toBool() ? 1.0 : 0.0;
        return makeShared<PFloat>(value * otherVal);
    }
    default:
        defaultOpError(u8"*", other);
        return nullptr;
    }
}

PObject::pointer PFloat::__truediv__(const PObject::pointer &other) const {
    auto type = other->getType().type;
    switch (type) {
    case Type::Int: {
        double otherVal = other->getValue().toInt();
        if (otherVal == 0.0) {
            throw std::runtime_error(u8"ZeroDivisionError: 不可除以0");
        }
        return makeShared<PFloat>(value / otherVal);
    }
    case Type::Float: {
        double otherVal = other->getValue().toDouble();
        if (qFuzzyCompare(otherVal, 0.0)) {
            throw std::runtime_error(u8"ZeroDivisionError: 不可除以0");
        }
        return makeShared<PFloat>(value / otherVal);
    }
    case Type::Bool: {
        bool otherVal = other->getValue().toBool();
        if (!otherVal) {
            throw std::runtime_error(u8"ZeroDivisionError: 不可除以0");
        }
        return makeShared<PFloat>(value / 1.0);
    }
    default:
        defaultOpError(u8"/", other);
        return nullptr;
    }
}

PObject::pointer PFloat::__mod__(const PObject::pointer &other) const {
    auto type = other->getType().type;
    switch (type) {
    case Type::Int: {
        double otherVal = other->getValue().toInt();
        if (otherVal == 0.0) {
            throw std::runtime_error(u8"ZeroDivisionError: 取模不可除以0");
        }
        return makeShared<PFloat>(fmod(value, otherVal));
    }
    case Type::Float: {
        double otherVal = other->getValue().toDouble();
        if (qFuzzyCompare(otherVal, 0.0)) {
            throw std::runtime_error(u8"ZeroDivisionError: 取模不可除以0");
        }
        return makeShared<PFloat>(fmod(value, otherVal));
    }
    default:
        defaultOpError(u8"%", other);
        return nullptr;
    }
}

PObject::pointer PFloat::__pow__(const PObject::pointer &other) const {
    auto type = other->getType().type;
    switch (type) {
    case Type::Int: {
        double otherVal = other->getValue().toInt();
        if (qFuzzyCompare(value, 0.0) && otherVal < 0) {
            throw std::runtime_error(u8"ZeroDivisionError: 0的负数次幂无意义");
        }
        return makeShared<PFloat>(pow(value, otherVal));
    }
    case Type::Float: {
        double otherVal = other->getValue().toDouble();
        if (qFuzzyCompare(value, 0.0) && otherVal < 0) {
            throw std::runtime_error(u8"ZeroDivisionError: 0的负数次幂无意义");
        }
        return makeShared<PFloat>(pow(value, otherVal));
    }
    case Type::Bool: {
        double otherVal = other->getValue().toBool() ? 1.0 : 0.0;
        return makeShared<PFloat>(pow(value, otherVal));
    }
    default:
        defaultOpError(u8"**", other);
        return nullptr;
    }
}

// ------------------------------
// 比较运算符
// ------------------------------
PObject::pointer PFloat::__eq__(const PObject::pointer &other) const {
    auto type = other->getType().type;
    switch (type) {
    case Type::Int: {
        double otherVal = other->getValue().toInt();
        return makeShared<PBool>(qFuzzyCompare(value, otherVal));
    }
    case Type::Float: {
        double otherVal = other->getValue().toDouble();
        return makeShared<PBool>(qFuzzyCompare(value, otherVal));
    }
    case Type::Bool: {
        double otherVal = other->getValue().toBool() ? 1.0 : 0.0;
        return makeShared<PBool>(qFuzzyCompare(value, otherVal));
    }
    default:
        defaultOpError(u8"==", other);
        return nullptr;
    }
}

PObject::pointer PFloat::__ne__(const PObject::pointer &other) const {
    auto eqResult = this->__eq__(other);
    if (!eqResult) {
        defaultOpError(u8"!=", other);
        return nullptr;
    }
    bool val = !eqResult->getValue().toBool();
    return makeShared<PBool>(val);
}

PObject::pointer PFloat::__lt__(const PObject::pointer &other) const {
    auto type = other->getType().type;
    switch (type) {
    case Type::Int: {
        double otherVal = other->getValue().toInt();
        return makeShared<PBool>(value < otherVal);
    }
    case Type::Float: {
        double otherVal = other->getValue().toDouble();
        return makeShared<PBool>(value < otherVal);
    }
    case Type::Bool: {
        double otherVal = other->getValue().toBool() ? 1.0 : 0.0;
        return makeShared<PBool>(value < otherVal);
    }
    default:
        defaultOpError(u8"<", other);
        return nullptr;
    }
}

PObject::pointer PFloat::__le__(const PObject::pointer &other) const {
    auto type = other->getType().type;
    switch (type) {
    case Type::Int: {
        double otherVal = other->getValue().toInt();
        return makeShared<PBool>(value <= otherVal);
    }
    case Type::Float: {
        double otherVal = other->getValue().toDouble();
        return makeShared<PBool>(value <= otherVal);
    }
    case Type::Bool: {
        double otherVal = other->getValue().toBool() ? 1.0 : 0.0;
        return makeShared<PBool>(value <= otherVal);
    }
    default:
        defaultOpError(u8"<=", other);
        return nullptr;
    }
}

PObject::pointer PFloat::__gt__(const PObject::pointer &other) const {
    auto leResult = this->__le__(other);
    if (!leResult) {
        defaultOpError(u8">", other);
        return nullptr;
    }
    bool val = !leResult->getValue().toBool();
    return makeShared<PBool>(val);
}

PObject::pointer PFloat::__ge__(const PObject::pointer &other) const {
    auto ltResult = this->__lt__(other);
    if (!ltResult) {
        defaultOpError(u8">=", other);
        return nullptr;
    }
    bool val = !ltResult->getValue().toBool();
    return makeShared<PBool>(val);
}

// ------------------------------
// 逻辑/位运算符（PFloat仅兼容，Python中浮点数不支持位运算）
// ------------------------------
PObject::pointer PFloat::__and__(const PObject::pointer &other) const {
    // Python中浮点数不支持位运算，直接抛错
    defaultOpError(u8"&/and", other);
    return nullptr;
}

PObject::pointer PFloat::__or__(const PObject::pointer &other) const {
    // Python中浮点数不支持位运算，直接抛错
    defaultOpError(u8"|/or", other);
    return nullptr;
}

PObject::pointer PFloat::__not__(const PObject::pointer &other) const {
    auto type = other->getType().type;
    switch (type) {
    case Type::None: {
        // Python中浮点数不支持~取反，抛错（对齐原生行为）
        throw std::runtime_error(u8"TypeError: 不支持对浮点数执行按位取反");
    }
    default:
        defaultOpError(u8"~", other);
        return nullptr;
    }
}

}
