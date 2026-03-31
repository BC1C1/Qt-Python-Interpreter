#include "PInstance.h"

#include "PClass.h"

#include "core/utils/functions.h"
#include "core/objects/runtime/pbool.h"
#include "PDict.h"

namespace Py {
	PInstance::PInstance(EPointer envir, PClass* classObj) : PObject(typeMap.at(Type::Instance)),
		classObj(classObj)
	{
		auto& classStaticMembers = classObj->getStaticMembers();
		privateMembers = MemberMap(classStaticMembers.begin(), classStaticMembers.end());
		auto dict_obj = makeShared<PDict>();
		privateMembers.insert("__dict__", dict_obj);
	}

	QString PInstance::toString() const
	{
		return QString("a instance of %1").arg(classObj->toString());
	}

	PClass* PInstance::getClassObj() const
	{
		return classObj;
	}

	void PInstance::__setattribute__(const QString& attrName, const pointer& obj)
	{
		privateMembers[attrName] = obj;
	}

	pointer Py::PInstance::__getattribute__(const QString& attrName)
	{
		auto iter = this->privateMembers.find(attrName);
		if (iter != this->privateMembers.end()) {
			return iter.value();
		}
		//auto& functions = classObj->getFunctions();
		//iter = functions.find(attrName);
		//if (iter != functions.end()) {
		//	return iter.value();
		//}
		return classObj->__getattribute__(attrName);
	}
	pointer PInstance::__eq__(const pointer& other) const
	{
		auto type = other->getType().type;
		switch (type)
		{
		case Type::None: {
			return makeShared<PBool>(false);
			break;
		}
		default:
			return makeShared<PBool>(false);
			break;
		}
	}
	pointer PInstance::__ne__(const pointer& other) const
	{
		auto type = other->getType().type;
		switch (type)
		{
		case Type::None: {
			return makeShared<PBool>(true);
			break;
		}
		default:
			return makeShared<PBool>(true);
			break;
		}
	}
}


