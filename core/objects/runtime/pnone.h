#ifndef PNONE_H
#define PNONE_H
#include "core/objects/runtime/pobject.h"

namespace Py {
class PNone : public PObject
{
public:
    PNone();
    virtual QString toString() const override;
};
}


#endif // PNONE_H
