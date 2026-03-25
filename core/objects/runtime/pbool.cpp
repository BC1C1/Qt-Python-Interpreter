#include "pbool.h"

#include "core/objects/runtime/pint.h"
#include "core/objects/runtime/pfloat.h"

namespace Py {
PBool::PBool(bool value) : PObject(typeMap.at(Type::Bool)), value(value)
{

}

QString PBool::toString() const
{
    return value ? u8"true" : u8"false";
}

QVariant PBool::getValue() const
{
    return QVariant(value);
}

PBool::~PBool()
{

}

PObject::pointer PBool::asInt() const
{
    return makeShared<PInt>(value ? 1 : 0);
}

PObject::pointer PBool::asFloat() const
{
    return makeShared<PFloat>(value ? 1 : 0);
}

PObject::pointer PBool::asBool() const
{
    return makeShared<PBool>(value);
}

}

