#include "piterator.h"

#include "core/objects/runtime/pnone.h"

namespace Py {

PIterator::PIterator(PObject::pointer object)
    : PObject(typeMap.at(Type::Int)), object(object), currentIndex(0), listCache(nullptr)
{
}

PObject::pointer PIterator::__next__()
{
    auto type = object->getType().type;
    switch (type) {
    case Type::List: {
        QSharedPointer<PList> listObj = nullptr;
        if (!listCache) {
            listObj = dynamicPointerCast<PList>(object);
            listCache = listObj.get();
        }
        if (currentIndex >= listCache->getTrueValue().size())
            return makeShared<PNone>();
        auto obj = listCache->getTrueValue()[currentIndex];
        currentIndex++;
        return obj;
        break;
    }
    default:
        return nullptr;
    }
}

}


