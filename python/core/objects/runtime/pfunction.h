#ifndef PFUNCTION_H
#define PFUNCTION_H

#include <QByteArray>

#include "core/objects/runtime/pobject.h"
#include "core/objects/runtime/pvm.h"  // just for definition of vm::Code

namespace Py {
using vm::Code;
class PFunction : public PObject
{
public:
    PFunction(pointer params, const QByteArray& code, pointer name = nullptr);
    QVariant getValue() const override;
    QString toString() const override;
    PStr* getNameObj() const;
    PList* getParamsObj() const;
    QVector<vm::Instruction> getCodeObj() const;
private:
    pointer name;
    pointer params;
    QByteArray code;
};
}



#endif // PFUNCTION_H
