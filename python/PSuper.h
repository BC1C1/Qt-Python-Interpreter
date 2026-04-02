#pragma once
#include "core/objects/runtime/pobject.h"

namespace Py {
	class PSuper : public PObject
	{
		using pointer = PObject::pointer;
	public:
		PSuper(pointer currClass, pointer instance);
		virtual QString toString() const override;

		pointer getNextClass();

		virtual pointer __call__(const pointer& listParams,
			const pointer& dictParams,
			QSharedPointer<Environment> envir) override;

		virtual pointer __getattribute__(const QString& attrName) override;

	private:
		pointer instance;
		pointer currClass;
	};
}


