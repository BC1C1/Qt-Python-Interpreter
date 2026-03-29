#include "pnone.h"

#include "core/objects/runtime/pbool.h"
namespace Py {
PNone::PNone() : PObject(typeMap.at(Type::None))
{

}

QString Py::PNone::toString() const
{
    return "None";
}

PObject::pointer PNone::__eq__(const PObject::pointer &other) const
{
    return makeShared<PBool>(other->getType() == this->getType());
}

PObject::pointer PNone::__ne__(const PObject::pointer &other) const
{
    return makeShared<PBool>(other->getType().type != this->getType().type);
}
}

