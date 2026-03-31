#include "pfunction.h"

#include "core/objects/runtime/pstr.h"
#include "core/objects/runtime/pint.h"
#include "core/objects/runtime/plist.h"
#include "core/objects/runtime/pnone.h"

namespace Py {

PFunction::PFunction(PObject::pointer params, const QByteArray &code, PObject::pointer name, bool isClassFunction)
: PObject(typeMap.at(Type::FunctionDefine)),
  name(name), params(params), code(code), isClassFunction(isClassFunction)
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

bool PFunction::getIsClassFunction() const
{
    return this->isClassFunction;
}

Py::PObject::pointer PFunction::__call__(const pointer& params, QSharedPointer<Environment> newEnvir)
{
    auto param = dynamicPointerCast<Py::PList>(params);
    auto sizeFalse = getParamsObj()->size();
    auto sizeTrue = param->size();
    if (sizeTrue != sizeFalse) {
        QString errMsg = QString(u8"参数数量不匹配: 形参数量：%1，实参数量：%2")
            .arg(sizeFalse).arg(sizeTrue);
        throw std::runtime_error(errMsg.toStdString());
    }
    // 参数处理
    auto pIter = params->__iter__();
    auto fIter = getParamsObj()->__iter__();
    auto NoneObj = makeShared<PNone>(); // 未来搞成全局对象
    for (int i = 0; i < param->size(); i++) {
        auto value1 = fIter->__next__();
    //if (i == 0 && getIsClassFunction()) {
    //    newEnvir->assign(value1->toString(), sharedFromThis()); // 绑定self
    //    continue;
    //}
    newEnvir->assign(value1->toString(), pIter->__next__());
    }
    return nullptr;
}

QVector<vm::Instruction> PFunction::getCodeObj() const
{
    return vm::Instruction::fromByteArray(code);
}


}

