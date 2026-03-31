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
		PClass* getClassObj() const;

		virtual void __setattribute__(const QString& attrName, const pointer& obj) override;
		virtual pointer __getattribute__(const QString& attrName) override;

		virtual pointer __eq__(const pointer& other) const override;
		virtual pointer __ne__(const pointer& other) const override;

	private:
		PClass* classObj;
		QHash<QString, pointer> privateMembers;
	};
}



