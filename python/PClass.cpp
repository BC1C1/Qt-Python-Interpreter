#include "PClass.h"

namespace Py {
	PClass::PClass() : PObject(typeMap.at(Type::Class))
	{
	}
}