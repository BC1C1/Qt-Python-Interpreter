#ifndef PNONE_H
#define PNONE_H
#include "core/objects/runtime/pobject.h"
#include "core/utils/functions.h"

namespace Py {
class PNone : public PObject
{
public:
    PNone();
    virtual QString toString() const override;

    virtual pointer __eq__(const pointer& other) const override;
    virtual pointer __ne__(const pointer& other) const override;
};
}


#endif // PNONE_H
