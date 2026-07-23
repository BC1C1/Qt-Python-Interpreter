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
		virtual void init() override;
		virtual QString toString() const override;
		QVariant getValue() const override;
		virtual QVector<pointer> getChildren() const override;

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
		virtual QVector<pointer> __mro__() override;
		MemberMap& getFunctions();
		MemberMap& getStaticMembers();
		pointer onlygeattributehere(const QString& attrName);
	private:
		bool neverInTail(pointer c, QVector<pointer>& l);
		QVector<pointer> mro(pointer cls);
	private:
		QString className;
		QHash<QString, pointer> functions;
		QHash<QString, pointer> staticMembers;
		QVector<pointer> parents;
		QVector<pointer>* mroList = nullptr;
		QVector<pointer> mrolist;
	public:
		static PClass* object;
	};
}



