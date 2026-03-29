#include "pfunction.h"

#include "core/objects/runtime/pstr.h"
#include "core/objects/runtime/pint.h"

namespace Py {

PFunction::PFunction(PObject::pointer params, const QByteArray &code, PObject::pointer name)
: PObject(typeMap.at(Type::FunctionDefine)),
  name(name), params(params), code(std::move(code))
{

}

QVariant PFunction::getValue() const
{
    return QVariant::fromValue(code);
}

QString PFunction::toString() const
{
    return QString("Function: %1").arg(getNameObj()->getValue().toString());
}

PStr* PFunction::getNameObj() const
{
    return (PStr*)name.get();
}

PList* PFunction::getParamsObj() const
{
    return (PList*)params.get();
}

QVector<vm::Instruction> PFunction::getCodeObj() const
{
    return vm::Instruction::fromByteArray(code);
}



}

