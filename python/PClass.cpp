#include "PClass.h"
#include "core/utils/functions.h"
#include "core/objects/runtime/pfunction.h"
#include "core/objects/runtime/plist.h"
#include "core/objects/runtime/pstr.h"
#include "PInstance.h"

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
		//auto iter = functions.find(attrName);
		//if (iter != functions.end()) {
		//	return iter.value();
		//}
		//iter = staticMembers.find(attrName);
		//if (iter != staticMembers.end()) {
		//	return iter.value();
		//}
		auto mrovec = mro(this);
		for (const auto& c : mrovec) {
			auto ret = c->onlygeattributehere(attrName);
			if (ret)
				return ret;
		}
		return nullptr;
	}
	pointer PClass::__call__(const pointer& params, QSharedPointer<Environment> envir)
	{
		return __instance__(params, envir);
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
	bool PClass::neverInTail(PClass* c, QVector<PClass*>& l)
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
	QVector<PClass*> PClass::mro(PClass* cls)
	{
		// ----------------------------------------
		// 终止条件：根类（object）
		// ----------------------------------------
		if (cls->parents.isEmpty()) {
			return { cls };
		}

		// ----------------------------------------
		// 第一步：收集所有要合并的列表
		// ----------------------------------------
		QVector<QVector<PClass*>> toMerge;

		// 1. 加入所有父类的 MRO
		for (auto& pObj : cls->parents) {
			auto p = dynamicPointerCast<PClass>(pObj);
			toMerge.append(mro(p.get()));
		}

		// 2. 加入直接父类列表（C3 必需）
		QVector<PClass*> directParents;
		for (auto& pObj : cls->parents) {
			directParents.append((PClass*)pObj.get());
		}
		toMerge.append(directParents);

		// ----------------------------------------
		// 第二步：C3 合并核心
		// ----------------------------------------
		QVector<PClass*> result;
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

			// ------------------------------------
			// 找一个合法的 head
			// ------------------------------------
			PClass* head = nullptr;
			for (auto& list : toMerge) {
				PClass* candidate = list.first();

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
				// 没有合法候选 → 继承冲突
				qDebug() << "C3 MRO 错误：无法合并继承";
				return result;
			}

			// ------------------------------------
			// 把 head 加入结果，并从所有列表删除
			// ------------------------------------
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