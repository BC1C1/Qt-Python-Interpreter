#include "plist.h"

#include "core/objects/runtime/pstr.h"
#include "core/objects/runtime/pint.h"
#include "core/objects/runtime/pbool.h"
#include "core/objects/runtime/piterator.h"
#include "core/objects/runtime/pfunction.h"
#include "core/utils/functions.h"
#include "core/objects/runtime/pnone.h"
#include "PDict.h"

namespace Py {
    using BuiltinFuncPtr =
        pointer(*)(
            const pointer& self,
            const pointer& args,
            QSharedPointer<Environment> env
            );
    using EPointer = QSharedPointer<Environment>;
QHash<QString, pointer> PList::functions = QHash<QString, pointer>();
PList::PList(
    const QVector<PObject::pointer> &value,
    bool isSkipRegister
) : PObject(typeMap.at(Type::List)), value(value)
{
    if (!isSkipRegister)
        registerInnerFunc();
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

int PList::size() const
{
    return value.size();
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
    case Type::None: {
        return makeShared<PBool>(false);
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
    case Type::None: {
        return makeShared<PBool>(true);
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
    return makeShared<PIterator>(sharedFromThis());
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

pointer PList::__getattribute__(const QString& attrName)
{
    auto iter = functions.find(attrName);
    if (iter != functions.end()) {
        return iter.value();
    }
    return makeShared<PNone>();
}

void PList::__setattribute__(const QString& attrName, const pointer& attr)
{
    functions[attrName] = attr;
}

void PList::registerInnerFunc()
{
    registerAppend();
}

void PList::registerAppend()
{
    auto listObj = makeShared<PList>(QVector<pointer>{makeShared<PStr>("new_element")}, true);
    auto func = makeShared<PFunction>(
        listObj,                    // 位置参数形参
        makeShared<PDict>(),        // 空字典
        QByteArray(),               // 空字节码（不会调用）
        makeShared<PStr>("append"), // 函数名
        false,                      // 不是类方法
        true                        // 是内置方法
    );
    this->functions["append"] = func;
    // 制造一个函数
    BuiltinFuncPtr appendFuncPtr = [](const pointer& self, const pointer& arg, EPointer env)->pointer {
        auto newElement = env->getObj("new_element");
        // 不用检查因为前面调用__call__检查过了，如果这个对象不存在那前面都错了，为了效率不层层检查，
        // 而且把错误只放到一层是合适的，尤其是未来对于这里的记忆模糊时
        auto listself = (PList*)(self.get());
        listself->getTrueValue().append(newElement);
        return makeShared<PNone>();
        };
    func->setBuildinFunc(appendFuncPtr);
    return;
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

