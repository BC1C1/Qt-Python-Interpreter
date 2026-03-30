#include "PClass.h"
#include "core/utils/functions.h"
#include "core/objects/runtime/pfunction.h"
#include "core/objects/runtime/plist.h"
#include "core/objects/runtime/pstr.h"
#include "PInstance.h"

namespace Py {
	PClass::PClass(const QString& className, MemberMap functions, MemberMap staticMembers)
		: PObject(typeMap.at(Type::Class)),
		className(className), functions(functions), staticMembers(staticMembers)
	{
		auto iter = this->functions.find("__init__");
		if (iter == this->functions.end()) {
			auto self = makeShared<PStr>("self");
			QVector<pointer> params;
			params.push_back(self);
			auto param = makeShared<PList>(params);
			QVector<vm::Instruction> temp;
			temp.push_back(vm::Code::RETURN);
			auto name = makeShared<PStr>("__init__");
			auto functionObj = makeShared<PFunction>(param, vm::Instruction::toByteArray(temp), name);
			this->functions["__init__"] = functionObj;
		}
	}
	QString PClass::toString() const
	{
		return QString(u8"class: %1").arg(className);
	}
	QVariant PClass::getValue() const
	{
		return QVariant(u8"no value in class");
	}
	pointer PClass::__instance__(const pointer& params, QSharedPointer<Environment> envir)
	{
		auto obj = makeShared<PInstance>(envir, this);
		QVector<pointer> param = { obj };
		auto list = dynamicPointerCast<PList>(params);
		param.append(list->getTrueValue());
		auto newParam = makeShared<PList>(param);
		functions["__init__"]->__call__(newParam, envir);
		return obj;
	}
	void PClass::__setattribute__(const QString& attrName, const pointer& obj)
	{
		auto type = obj->getType().type;
		switch (type)
		{
		case Type::FunctionDefine: {
			functions.insert(attrName, obj);
			break;
		}
		default: {
			staticMembers.insert(attrName, obj);
			break;
		}
		}
	}
	pointer PClass::__getattribute__(const QString& attrName)
	{
		auto iter = functions.find(attrName);
		if (iter != functions.end()) {
			return iter.value();
		}
		iter = staticMembers.find(attrName);
		if (iter != staticMembers.end()) {
			return iter.value();
		}
		Py::PObject::__getattribute__(attrName);
		return nullptr;
	}
	MemberMap& PClass::getFunctions()
	{
		return functions;
	}
	MemberMap& PClass::getStaticMembers()
	{
		return staticMembers;
	}
}