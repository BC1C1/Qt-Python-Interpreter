#include "pfunction.h"

#include "core/objects/runtime/pstr.h"
#include "core/objects/runtime/pint.h"
#include "core/objects/runtime/plist.h"
#include "core/objects/runtime/pnone.h"
#include "PDict.h"
#include <qset.h>

namespace Py {

PFunction::PFunction(pointer listObj, pointer dictObj, const QByteArray &code, PObject::pointer name, bool isClassFunction)
: PObject(typeMap.at(Type::FunctionDefine)),
  name(name), listParams(listObj), dictParams(dictObj), code(code), isClassFunction(isClassFunction)
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

PList* PFunction::getListParamsObj() const
{
    return (PList*)listParams.get();
}

PDict* PFunction::getDictParamsObj() const
{
    return (PDict*)dictParams.get();
}

bool PFunction::getIsClassFunction() const
{
    return this->isClassFunction;
}

Py::PObject::pointer PFunction::__call__(const pointer& listParams, 
    const pointer& dictParams,
    QSharedPointer<Environment> newEnvir)
{
    auto listObj = (PList*)(listParams.get());
    auto dictObj = (PDict*)(dictParams.get());

    auto& fl = ((PList*)(this->listParams.get()))->getTrueValue();
    auto& tl = listObj->getTrueValue();
    auto& fd = ((PDict*)(this->dictParams.get()))->getTrueValue();
    auto& td = dictObj->getTrueValue();

    QSet<pointer> assigned;

    // pos assign
    if (tl.size() > fl.size() + fd.size()) {
        auto dsize = tl.size() - fl.size();
        QString errMsg = QString(u8"位置参数过多: 形参数量：%1，实参数量：%2")
            .arg(fl.size() + fd.size()).arg(tl.size());
        throw std::runtime_error(errMsg.toStdString());
    }
    // 逐个赋值
    int assignCount = qMin(tl.size(), fl.size());
    for (int i = 0; i < assignCount; i++) {
        assigned.insert(fl[i]);
        newEnvir->assign(fl[i]->toString(), tl[i]);
    }

    int extraStart = fl.size();
    for (int i = extraStart; i < tl.size(); i++) {
        // 拿到第 i 个形参
        int defIdx = i - fl.size();
        auto fpair = fd.begin() + defIdx;

        assigned.insert(fpair.key());
        newEnvir->assign(fpair.key()->toString(), tl[i]);
    }

    // key assign
    // 对传入的关键字赋值
    for (int i = 0; i < td.size(); i++) {
        auto tpair = td.begin() + i;
        if (assigned.contains(tpair.key())) {
            QString errMsg = QString(u8"关键字参数: %1不可重复赋值").arg(tpair.key()->toString());
            throw std::runtime_error(errMsg.toStdString());
        }
        if (!fd.contains(tpair.key()) && !fl.contains(tpair.key())) {
            throw std::runtime_error(QString(u8"参数'%1'不是函数的形参")
                .arg(tpair.key()->toString()).toStdString());
        }
        newEnvir->assign(tpair.key()->toString(), tpair.value());
    }
    // 对默认参数赋值
    for (int i = 0; i < fd.size(); i++) {
        auto fpair = fd.begin() + i;
        if (assigned.contains(fpair.key()))
            continue;
        newEnvir->assign(fpair.key()->toString(), fpair.value());
    }
    // 检查为赋值的参数
    for (const auto& o : fl) {
        if (!assigned.contains(o)) {
            throw std::runtime_error(QString(u8"缺少必选参数：%1").arg(o->toString()).toStdString());
        }
    }
    return makeShared<PNone>();
}

QVector<vm::Instruction> PFunction::getCodeObj() const
{
    return vm::Instruction::fromByteArray(code);
}


}

