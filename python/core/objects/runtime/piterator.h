#ifndef PITERATOR_H
#define PITERATOR_H

#include "core/objects/runtime/pobject.h"
#include "core/utils/functions.h"
#include "core/objects/runtime/plist.h"

namespace Py {
class PIterator : public PObject
{
public:
    PIterator(pointer object);
    virtual pointer __next__() override;
    QString toString() const override;
private:
    pointer object;
    int currentIndex;
    PList* listCache;
};
}



#endif // PITERATOR_H
