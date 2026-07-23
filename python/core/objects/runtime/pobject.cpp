#include "pobject.h"

namespace Py {
QString TypeToString(Type type)
{
    switch (type)
    {
    case Type::Int: return "Int";
    case Type::Float: return "Float";
    case Type::Str: return "Str";
    case Type::Bool: return "Bool";
    case Type::None: return "None";
    case Type::Iterator: return "Iterator";
    case Type::List: return "List";
    case Type::FunctionDefine: return "Function";
    case Type::ReturnValue: return "returnValue";
    case Type::Class: return "class";
    case Type::Instance: return "instance";
    case Type::Dict:  return "dict";
    default: return "undefine";
    }
}
PyType::PyType(Type type) : type(type)
{
}

QString PyType::__str__()
{
    return TypeToString(type);
}

bool PyType::operator==(PyType other)
{
    return type == other.type;
}

PObject::PObject(PyType type, QObject*): type(type)
{
}

PyType PObject::getType() const
{
    return type;
}

PObject::~PObject()
{
}

QVariant PObject::getValue() const
{
    throw std::runtime_error((QString(u8"类型：%1不可获取值").arg(TypeToString(type.type))).toUtf8().data());
}

QVector<PObject::pointer> PObject::getChildren() const
{
    return QVector<pointer>();
}

PObject::pointer PObject::asInt() const
{
    this->noSuchCast(u8"int");
    return nullptr;
}

PObject::pointer PObject::asFloat() const
{
    this->noSuchCast(u8"float");
    return nullptr;
}

PObject::pointer PObject::asBool() const
{
    this->noSuchCast(u8"bool");
    return nullptr;
}

PObject::pointer PObject::asList() const
{
    this->noSuchCast(u8"list");
    return nullptr;
}

PObject::pointer PObject::asString() const
{
    this->noSuchCast(u8"string");
    return nullptr;
}

PObject::pointer PObject::__add__(const PObject::pointer &other) const
{
    defaultOpError(u8"+", other);
    return nullptr;
}

PObject::pointer PObject::__sub__(const PObject::pointer &other) const
{
    defaultOpError(u8"-", other);
    return nullptr;
}

PObject::pointer PObject::__mul__(const PObject::pointer &other) const
{
    defaultOpError(u8"*", other);
    return nullptr;
}

PObject::pointer PObject::__truediv__(const PObject::pointer &other) const
{
    defaultOpError(u8"/", other);
    return nullptr;
}

PObject::pointer PObject::__mod__(const PObject::pointer &other) const
{
    defaultOpError(u8"%", other);
    return nullptr;
}

PObject::pointer PObject::__pow__(const PObject::pointer &other) const
{
    defaultOpError(u8"**", other);
    return nullptr;
}

PObject::pointer PObject::__eq__(const PObject::pointer &other) const
{
    defaultOpError(u8"==", other);
    return nullptr;
}

PObject::pointer PObject::__ne__(const PObject::pointer &other) const
{
    defaultOpError(u8"!=", other);
    return nullptr;
}

PObject::pointer PObject::__lt__(const PObject::pointer &other) const
{
    defaultOpError(u8"<", other);
    return nullptr;
}

PObject::pointer PObject::__le__(const PObject::pointer &other) const
{
    defaultOpError(u8"<=", other);
    return nullptr;
}

PObject::pointer PObject::__gt__(const PObject::pointer &other) const
{
    defaultOpError(u8">", other);
    return nullptr;
}

PObject::pointer PObject::__ge__(const PObject::pointer &other) const
{
    defaultOpError(u8">=", other);
    return nullptr;
}

PObject::pointer PObject::__and__(const PObject::pointer &other) const
{
    defaultOpError(u8"and", other);
    return nullptr;
}

PObject::pointer PObject::__or__(const PObject::pointer &other) const
{
    defaultOpError(u8"or", other);
    return nullptr;
}

PObject::pointer PObject::__not__(const PObject::pointer &other) const
{
    defaultOpError(u8"not", other);
    return nullptr;
}

PObject::pointer PObject::__iter__()
{
    defaultNoIterError();
    return nullptr;
}

PObject::pointer PObject::__next__()
{
    defaultNoIterError();
    return nullptr;
}

void PObject::__setitem__(const PObject::pointer&, PObject::pointer)
{
    defaultNoSetItemError();
}

PObject::pointer PObject::__getitem__(const PObject::pointer &)
{
    defaultNoGetItemError();
    return nullptr;
}

PObject::pointer PObject::__call__(
    const PObject::pointer&, 
    const pointer&, 
    QSharedPointer<Environment> 
)
{
    defaultNoCallFuncError();
    return nullptr;
}

PObject::pointer PObject::__instance__(const pointer& listParams, 
    const pointer& dictParams, QSharedPointer<Environment> envir)
{
    defaultNoInstanceError();
    return nullptr;
}

PObject::pointer PObject::__getattribute__(const QString &)
{
    defaultNoGetAttributeError();
    return nullptr;
}

void PObject::__setattribute__(const QString &, const PObject::pointer &)
{
    defaultNoSetAttributeError();
}

QVector<PObject::pointer> PObject::__mro__()
{
    return QVector<PObject::pointer>();
}

PObject::pointer PObject::noSuchCast(const std::string &name) const
{
    throw runtime_error(name);
    return nullptr;
}

PObject::pointer PObject::defaultOpError(const QString &, const PObject::pointer &) const
{
    throw runtime_error(u8"没有匹配的运算符");
}

PObject::pointer PObject::defaultNoIterError() const
{
    throw runtime_error(u8"__iter__未实现");
}

PObject::pointer PObject::defaultNotIterError() const
{
    throw runtime_error(u8"该对象不是迭代器");
}

void PObject::defaultNoSetItemError() const
{
    throw runtime_error(u8"__setitem__未实现");
}

PObject::pointer PObject::defaultNoGetItemError() const
{
    throw runtime_error(u8"__getitem__未实现");
}

PObject::pointer PObject::defaultNoCallFuncError() const
{
    throw runtime_error("u8__call__未实现");
}

PObject::pointer PObject::defaultNoInstanceError() const
{
    throw runtime_error(u8"__instance__未实现");
}

PObject::pointer PObject::defaultNoGetAttributeError() const
{
    throw runtime_error(u8"__getattribute__未实现");
}

PObject::pointer PObject::defaultNoSetAttributeError() const
{
    throw runtime_error(u8"__setattribute__未实现");
}

PObject::pointer PObject::defaultNoMroError() const
{
    throw runtime_error(u8"__mro__未实现");
}



}
