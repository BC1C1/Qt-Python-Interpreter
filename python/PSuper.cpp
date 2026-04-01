#include "PSuper.h"

#include "PClass.h"

namespace Py {
	using pointer = PObject::pointer;
	PSuper::PSuper(PClass* currClass, pointer instance) : PObject(typeMap.at(Type::Super)),
		currClass(currClass), instance(instance)
	{
	}
	pointer PSuper::getNextClass()
	{
		QVector<PClass*> mro = instance->__mro__();
		auto index = mro.indexOf(currClass);
		if (mro[index + 1] != PClass::object) {
			return mro[index + 1]->__getattribute__("__init__");
		}
	}
}
