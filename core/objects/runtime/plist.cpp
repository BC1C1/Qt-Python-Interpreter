#include "plist.h"

#include "core/objects/runtime/pstr.h"
#include "core/objects/runtime/pint.h"
#include "core/objects/runtime/pbool.h"

namespace Py {

PList::PList(QVector<PObject::pointer> &value) : PObject(typeMap.at(Type::Int)), value(value)
{

}

QString PList::toString() const
{
    QString ret = "[";
    for (int i = 0; i < value.size(); i++) {
        ret.append(value[i]->toString());
        if (i != value.size() - 1)
            ret.append(", ");
    }
    ret.append("]");
    return ret;
}

QVariant PList::getValue() const
{
    QList<QVariant> ret;
    for (const auto& p : value) {
        ret.push_back(p->getValue());
    }
    return QVariant(ret);
}

QVector<PObject::pointer> &PList::getTrueValue()
{
    return value;
}

PObject::pointer PList::asString() const
{
    return makeShared<Py::PStr>(toString());
}

PObject::pointer PList::__add__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type) {
    case Type::List: {
        auto otherObj = dynamicPointerCast<PList>(other);
        return makeShared<PList>(value + otherObj->getTrueValue());
        break;
    }
    default:
        defaultOpError(u8"+", other);
        return nullptr; // 无效语句用于消除警告
    }
}

PObject::pointer PList::__mul__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type) {
    case Type::Int: {
        int value = other->getValue().toInt();
        QVector<pointer> ret;
        for (int i = 0; i < value; i++) {
            ret.append(this->value);
        }
        return makeShared<PList>(ret);
        break;
    }
    default:{
        defaultOpError(u8"*", other);
        return nullptr; // 无效语句用于消除警告
    }
    }
}

PObject::pointer PList::__eq__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type) {
    case Type::List: {
        auto listObj = dynamicPointerCast<PList>(other);
        if (value.size() != listObj->getTrueValue().size())
            return makeShared<PBool>(false);
        for (int i = 0; i < value.size(); i ++) {
            if (value[i]->__ne__(listObj->getTrueValue()[i])->getValue().toBool())
                return makeShared<PBool>(false);
        }
        return makeShared<PBool>(true);
    }
    default: {
        return makeShared<PBool>(false);
    }
    }
}

PObject::pointer PList::__ne__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type) {
    case Type::List: {
        auto listObj = dynamicPointerCast<PList>(other);
        if (value.size() != listObj->getTrueValue().size())
            return makeShared<PBool>(true);
        for (int i = 0; i < value.size(); i ++) {
            if (value[i]->__ne__(listObj->getTrueValue()[i])->getValue().toBool())
                return makeShared<PBool>(true);
        }
        return makeShared<PBool>(false);
    }
    default: {
        return makeShared<PBool>(true);
    }
    }
}

PObject::pointer PList::__lt__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type) {
    case Type::List: {
        auto listObj = dynamicPointerCast<PList>(other);
        auto size = fmin(value.size(), listObj->getTrueValue().size());
        for (int i = 0; i < size; i ++) {
            if (value[i]->__lt__(listObj->getTrueValue()[i])->getValue().toBool())
                return makeShared<PBool>(true);
        }
        return makeShared<PBool>(value.size() < listObj->getTrueValue().size());
    }
    default: {
        defaultOpError(u8"<", other);
        return nullptr; // 无效语句用于消除警告
    }
    }
}

PObject::pointer PList::__le__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type) {
    case Type::List: {
        auto listObj = dynamicPointerCast<PList>(other);
        auto size = fmin(value.size(), listObj->getTrueValue().size());
        for (int i = 0; i < size; i ++) {
            if (value[i]->__le__(listObj->getTrueValue()[i])->getValue().toBool())
                return makeShared<PBool>(true);
        }
        return makeShared<PBool>(value.size() <= listObj->getTrueValue().size());
    }
    default: {
        defaultOpError(u8"<=", other);
        return nullptr; // 无效语句用于消除警告
    }
    }
}

PObject::pointer PList::__gt__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type) {
    case Type::List: {
        auto listObj = dynamicPointerCast<PList>(other);
        auto size = fmin(value.size(), listObj->getTrueValue().size());
        for (int i = 0; i < size; i ++) {
            if (value[i]->__gt__(listObj->getTrueValue()[i])->getValue().toBool())
                return makeShared<PBool>(true);
        }
        return makeShared<PBool>(value.size() > listObj->getTrueValue().size());
    }
    default: {
        defaultOpError(u8">", other);
        return nullptr; // 无效语句用于消除警告
    }
    }
}

PObject::pointer PList::__ge__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type) {
    case Type::List: {
        auto listObj = dynamicPointerCast<PList>(other);
        auto size = fmin(value.size(), listObj->getTrueValue().size());
        for (int i = 0; i < size; i ++) {
            if (value[i]->__ge__(listObj->getTrueValue()[i])->getValue().toBool())
                return makeShared<PBool>(true);
        }
        return makeShared<PBool>(value.size() >= listObj->getTrueValue().size());
    }
    default: {
        defaultOpError(u8">=", other);
        return nullptr; // 无效语句用于消除警告
    }
    }
}

PObject::pointer PList::__not__(const PObject::pointer &other) const
{
    auto type = other->getType().type;
    switch (type) {
    case Type::None: {
        return makeShared<PBool>(this->value.isEmpty());
    }
    default: {
        defaultOpError(u8"!", other);
        return nullptr;
    }
    }
}

PObject::pointer PList::__iter__()
{
    return nullptr;
}

void PList::__setitem__(const PObject::pointer &index, PObject::pointer obj)
{
    if (index->getType().type != Type::Int)
        throwInvalidTypeForIndex();
    auto idx = index->getValue().toInt();
    if (idx < 0 || idx >= this->value.size()) {
        throwOutOfRange();
    }
    this->value[idx] = obj;
}

PObject::pointer PList::__getitem__(const PObject::pointer &index)
{
    if (index->getType().type != Type::Int)
        throwInvalidTypeForIndex();
    auto idx = index->getValue().toInt();
    if (idx < 0 || idx >= this->value.size()) {
        throwOutOfRange();
    }
    return this->value[idx];
}

void PList::throwInvalidTypeForIndex()
{
    throw std::runtime_error("type of index is wrong!");
}

void PList::throwOutOfRange()
{
    throw std::runtime_error("index out of range");
}
}

