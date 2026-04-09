#include "PClass.h"
#include "core/utils/functions.h"
#include "core/objects/runtime/pfunction.h"
#include "core/objects/runtime/plist.h"
#include "core/objects/runtime/pstr.h"
#include "PInstance.h"
#include "PDict.h"

namespace Py {
	PClass* PClass::object = nullptr;
	PClass::PClass(
		const QString& className, 
		MemberMap functions, 
		MemberMap staticMembers,
		const QVector<pointer>& parents
	)
		: PObject(typeMap.at(Type::Class)),
		className(className), functions(functions), staticMembers(staticMembers), parents(parents)
	{
	}
	void PClass::init()
	{
		auto initFunc = __getattribute__("__init__");
		if (initFunc == nullptr) {
			auto self = makeShared<PStr>("self");
			QVector<pointer> listParams;
			listParams.push_back(self);
			auto param = makeShared<PList>(listParams);
			QVector<vm::Instruction> temp;
			temp.push_back(vm::Code::RETURN);
			auto name = makeShared<PStr>("__init__");
			auto functionObj = makeShared<PFunction>(param, makeShared<PDict>(), vm::Instruction::toByteArray(temp), name);
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
	pointer PClass::__instance__(
		const pointer& listParams,
		const pointer& dictParams,
		QSharedPointer<Environment> envir)
	{
		auto obj = makeShared<PInstance>(envir, sharedFromThis());
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
		if (!mroList) {
			mrolist = mro(sharedFromThis());
			mroList = &mrolist;
		}
		auto& mrovec = *mroList;
		for (const auto& c : mrovec) {
			auto ret = ((PClass*)(c.get()))->onlygeattributehere(attrName);
			if (ret)
				return ret;
		}
		return nullptr;
	}
	pointer PClass::__call__(
		const pointer& listParams,
		const pointer& dictParams, 
		QSharedPointer<Environment> envir)
	{
		return __instance__(listParams, dictParams, envir);
	}
	QVector<pointer> PClass::__mro__()
	{
		return *mroList;
	}
	MemberMap& PClass::getFunctions()
	{
		return functions;
	}
	MemberMap& PClass::getStaticMembers()
	{
		return staticMembers;
	}
	pointer PClass::onlygeattributehere(const QString& attrName)
	{
		auto iter = functions.find(attrName);
		if (iter != functions.end()) {
			return iter.value();
		}
		iter = staticMembers.find(attrName);
		if (iter != staticMembers.end()) {
			return iter.value();
		}
		return nullptr;
	}
	bool PClass::neverInTail(pointer c, QVector<pointer>& l)
	{
		if (l.size() <= 1)
			return true;

		// 检查 c 是否出现在 list[1...] 里面
		for (int i = 1; i < l.size(); i++)
		{
			if (l[i] == c)
				return false;
		}
		return true;
	}
	QVector<pointer> PClass::mro(pointer cls)
	{
		if (cls->getType().type != Type::Class)
			throw std::runtime_error("mro遇到意外的参数");
		const auto const classobj = (PClass*)(cls.get());
		if (classobj->parents.isEmpty()) {
			return { cls };
		}
		QVector<QVector<pointer>> toMerge;

		for (auto& pObj : classobj->parents) {
			toMerge.append(mro(pObj));
		}

		QVector<pointer> directParents;
		for (auto& pObj : classobj->parents) {
			directParents.append(pObj);
		}
		toMerge.append(directParents);

		QVector<pointer> result;
		result.append(cls);

		while (!toMerge.isEmpty())
		{
			// 清除空列表
			for (auto it = toMerge.begin(); it != toMerge.end();) {
				if (it->isEmpty())
					it = toMerge.erase(it);
				else
					++it;
			}

			if (toMerge.isEmpty())
				break;
			pointer head = nullptr;
			for (auto& list : toMerge) {
				pointer candidate = list.first();

				bool ok = true;
				for (auto& l : toMerge) {
					if (!neverInTail(candidate, l)) {
						ok = false;
						break;
					}
				}

				if (ok) {
					head = candidate;
					break;
				}
			}

			if (!head) {
				log("C3 MRO 错误：无法合并继承");
				return result;
			}

			result.append(head);

			for (auto& list : toMerge) {
				if (!list.isEmpty() && list.first() == head) {
					list.removeFirst();
				}
			}
		}

		return result;
	}
}