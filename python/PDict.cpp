#include "PDict.h"

#include "core/objects/runtime/pstr.h"
#include "core/utils/functions.h"
#include "core/objects/runtime/piterator.h"
#include <qdebug.h>

namespace Py {
	PDict::PDict(TableType&& hTable) 
		: PObject(typeMap.at(Type::Dict)), hashTable(std::move(hTable))
	{

	}
	QList<pointer> PDict::getKeyList()
	{
		return hashTable.keys();
	}
	QList<pointer> PDict::getValueList()
	{
		return hashTable.values();
	}
	pointer PDict::getKeyAt(int index)
	{
		auto it = hashTable.begin() + index;
		return it.key();
	}
	QString PDict::toString() const
	{
		QString l;
		for (auto iter = hashTable.begin(); iter != hashTable.end(); ++iter) {
			l.append("pair: { key: " +
					iter.key()->toString() +
					" value : " +
					iter.value()->toString() +
					" }\n");
		}
		return l;
	}
	pointer PDict::asString() const
	{
		return makeShared<PStr>(toString());
	}
	pointer PDict::__getitem__(const pointer& index)
	{
		auto iter = hashTable.find(index);
		if (iter == hashTable.end()) return nullptr;
		return iter.value();
	}
	void PDict::__setitem__(const pointer& index, pointer obj)
	{
		hashTable[index] = obj;
	}
	pointer PDict::__iter__()
	{
		return makeShared<PIterator>(sharedFromThis());
	}
}
