#pragma once
#include "core/objects/runtime/pobject.h"

namespace Py {
	class PSuper : public PObject
	{
		using pointer = PObject::pointer;
	public:
		PSuper(PClass* currClass, pointer instance);

		pointer getNextClass();

	private:
		pointer instance;
		PClass* currClass;
	};
}


