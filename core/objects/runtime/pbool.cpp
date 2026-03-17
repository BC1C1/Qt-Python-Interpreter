#include "pbool.h"

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

}

