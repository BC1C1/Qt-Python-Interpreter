#pragma once

#include <qhash.h>

#include "core/objects/runtime/pobject.h"


namespace Py {
	using EPointer = QSharedPointer<Environment>;
	class PInstance : public PObject
	{
	public:
		PInstance(EPointer envir, PClass* classObj);
		virtual QString toString() const override;

		virtual void __setattribute__(const QString& attrName, const pointer& obj) override;
		virtual pointer __getattribute__(const QString& attrName) override;
	private:
		PClass* classObj;
		QHash<QString, pointer> privateMembers;
	};
}



