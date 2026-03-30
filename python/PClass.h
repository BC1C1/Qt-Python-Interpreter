#pragma once
#include <qhash.h>

#include "core/objects/runtime/pobject.h"

namespace Py {
	using pointer = Py::PObject::pointer;
	using MemberMap = QHash<QString, pointer>;
	class PClass : public PObject
	{
	public:
		PClass(const QString& className, MemberMap functions, MemberMap staticMembers);
		virtual QString toString() const override;
		QVariant getValue() const override;

		virtual pointer __instance__(const pointer& params, QSharedPointer<Environment> envir) override;
		virtual void __setattribute__(const QString& atrrName, const pointer& obj) override;
		virtual pointer __getattribute__(const QString& attrName) override;
		MemberMap& getFunctions();
		MemberMap& getStaticMembers();

	private:
		QString className;
		QHash<QString, pointer> functions;
		QHash<QString, pointer> staticMembers;
	};
}



