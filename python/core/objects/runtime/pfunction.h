#ifndef PFUNCTION_H
#define PFUNCTION_H

#include <QByteArray>

#include "core/objects/runtime/pobject.h"
#include "core/objects/runtime/pvm.h"  // just for definition of vm::Code

namespace Py {
using vm::Code;
class PFunction : public PObject
{
    using pointer = Py::PObject::pointer;
public:
    PFunction(pointer listObj, pointer dictObj, const QByteArray& code, pointer name = nullptr, bool isClassFunction = false);
    QVariant getValue() const override;
    QString toString() const override;
    PStr* getNameObj() const;
    PList* getListParamsObj() const;
    PDict* getDictParamsObj() const;
    bool getIsClassFunction() const;
    virtual pointer __call__(const pointer& listParams, 
        const pointer& dictParams,
        QSharedPointer<Environment> envir) override;
    QVector<vm::Instruction> getCodeObj() const;
private:
    pointer name;
    pointer listParams;
    pointer dictParams;
    QByteArray code;
    bool isClassFunction;
};
}



#endif // PFUNCTION_H
