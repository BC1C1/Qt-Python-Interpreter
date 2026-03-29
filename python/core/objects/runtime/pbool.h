#ifndef PBOOL_H
#define PBOOL_H
#include "core/objects/runtime/pobject.h"

namespace Py {
class PBool : public PObject
{
public:
    PBool(bool value = false);
    QString toString() const override;
    virtual QVariant getValue() const override;
    virtual ~PBool();

    virtual pointer asInt() const override;
    virtual pointer asFloat() const override;
    virtual pointer asBool() const override;
private:
    bool value;
};
}


#endif // PBOOL_H
