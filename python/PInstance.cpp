#include "PInstance.h"

#include "PClass.h"

namespace Py {
	PInstance::PInstance(EPointer envir, PClass* classObj) : PObject(typeMap.at(Type::Class)),
		classObj(classObj)
	{
		auto& classStaticMembers = classObj->getStaticMembers();
		privateMembers = MemberMap(classStaticMembers.begin(), classStaticMembers.end());
	}

	QString PInstance::toString() const
	{
		return QString("a instance of %1").arg(classObj->toString());
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
		auto& functions = classObj->getFunctions();
		iter = functions.find(attrName);
		if (iter != functions.end()) {
			return iter.value();
		}
		//auto& members = classObj->getStaticMembers();
		//iter = members.find(attrName);
		//if (iter != members.end()) {
		//	return iter.value();
		//} // 不需要，因为复制过一遍了
		Py::PObject::__getattribute__(attrName); // 调用默认报错的方法
		return nullptr; // 其实到不了这一句
	}
}


