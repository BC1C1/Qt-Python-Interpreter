#pragma once

#include "core/objects/runtime/pobject.h"


namespace Py {
	using pointer = PObject::pointer;
	using EPointer = QSharedPointer<Environment>;
	class PModel : public PObject
	{
	public:
		PModel(EPointer envir);
		virtual QString toString() const override;
		virtual pointer __getattribute__(const QString& attrName) override;
		virtual QVector<pointer> getChildren() const override;

	private:
		EPointer table;
	};
}


