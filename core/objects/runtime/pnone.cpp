#include "pnone.h"
namespace Py {
PNone::PNone() : PObject(typeMap.at(Type::None))
{

}

QString Py::PNone::toString() const
{
    return "None";
}
}

