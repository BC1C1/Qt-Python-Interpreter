#pragma once

#include "core/objects/runtime/pobject.h"

namespace Py {
	class PClass : public PObject
	{
	public:
		PClass();


	private:
		QString className;

	};
}



