#pragma once
#include <qhash.h>

#include "core/objects/runtime/pobject.h"

namespace Py {
	using pointer = Py::PObject::pointer;
	using MemberMap = QHash<QString, pointer>;
	class PClass : public PObject
	{
	public:
		PClass(
			const QString& className, 
			MemberMap functions, 
			MemberMap staticMembers, 
			const QVector<pointer>& parents = QVector<pointer>());
		virtual QString toString() const override;
		QVariant getValue() const override;

		virtual pointer __instance__(
			const pointer& listParams,
			const pointer& dictParams,
			QSharedPointer<Environment> envir) override;
		virtual void __setattribute__(const QString& atrrName, const pointer& obj) override;
		virtual pointer __getattribute__(const QString& attrName) override;
		virtual pointer __call__(
			const pointer& listParams,
			const pointer& dictParams,
			QSharedPointer<Environment> envir
		) override;
		virtual QVector<PClass*> __mro__() override;
		MemberMap& getFunctions();
		MemberMap& getStaticMembers();
		pointer onlygeattributehere(const QString& attrName);
	private:
		bool neverInTail(PClass* c, QVector<PClass*>& l);
		QVector<PClass*> mro(PClass* cls);
	private:
		QString className;
		QHash<QString, pointer> functions;
		QHash<QString, pointer> staticMembers;
		QVector<pointer> parents;
		QVector<PClass*>* mroList = nullptr;
		QVector<PClass*> mrolist;
	public:
		static PClass* object;
	};
}



