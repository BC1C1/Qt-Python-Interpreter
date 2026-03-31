#pragma once

#include <qhash.h>
#include "core/objects/runtime/pobject.h"

namespace Py {
	using pointer = Py::PObject::pointer;
	using TableType = QHash<pointer, pointer>;
	class PDict : public PObject
	{
	public:
		PDict(TableType&& hTable = TableType());
		int size() const { return hashTable.size(); }
		QList<pointer> getKeyList();
		QList<pointer> getValueList();
		pointer getKeyAt(int index);

		virtual QString toString() const override;
		virtual pointer asString() const override;

		virtual pointer __getitem__(const pointer& index) override;
		virtual void __setitem__(const pointer& index, pointer obj) override;

		virtual pointer __iter__() override;


	private:
		QHash<pointer, pointer> hashTable;
	};
}


