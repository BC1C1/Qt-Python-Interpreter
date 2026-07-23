#ifndef PITERATOR_H
#define PITERATOR_H

#include "core/objects/runtime/pobject.h"
#include "core/utils/functions.h"
#include "core/objects/runtime/plist.h"
#include "PDict.h"

namespace Py {
class PIterator : public PObject
{
public:
    PIterator(pointer object);
    virtual pointer __next__() override;
    QString toString() const override;
    virtual QVector<pointer> getChildren() const override;
private:
    pointer object;
    int currentIndex;
    PList* listCache;
    PDict* dictCache;
};
}



#endif // PITERATOR_H
