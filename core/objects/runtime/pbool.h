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
private:
    bool value;
};
}


#endif // PBOOL_H
