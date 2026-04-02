#include "PSuper.h"

#include "PClass.h"
#include "PDict.h"
#include "PInstance.h"
#include "core/objects/runtime/plist.h"
#include "core/objects/runtime/environment.h"

namespace Py {
	using pointer = PObject::pointer;
	PSuper::PSuper(pointer currClass, pointer instance) : PObject(typeMap.at(Type::Super)),
		currClass(currClass), instance(instance)
	{
	}
	QString PSuper::toString() const
	{
		return "super";
	}
	pointer PSuper::getNextClass()
	{
		QVector<PClass*> mro = instance->__mro__();
		auto currclass = (PClass*)(currClass.get());
		auto index = mro.indexOf(currclass);
		if (mro[index + 1] != PClass::object) {
			return mro[index + 1]->__getattribute__("__init__");
		}
		return nullptr;
	}
	pointer PSuper::__call__(const pointer& listParams, const pointer& dictParams, QSharedPointer<Environment> envir)
	{
		//auto listObj = (PList*)(listParams.get());
		//auto dictObj = (PDict*)(dictParams.get());
		//auto& tl = listObj->getTrueValue();
		//auto& td = dictObj->getTrueValue();

		//if (td.size() != 0) {
		//	QString errMsg("super不允许关键字传参");
		//	throw std::runtime_error(errMsg.toStdString());
		//}
		//if (tl.size() > 2) {
		//	QString errMsg("super不允许多余的参数");
		//	throw std::runtime_error(errMsg.toStdString());
		//}
		//QStringList paramName = { "cls", "instance" };
		//int i = 0;
		//for (i = 0; i < tl.size(); i++) {
		//	envir->assign(paramName[i], tl[i]);
		//}
		//if (i < 2) {
		//	auto obj = envir->getObj("self");
		//	envir->assign(paramName[1], obj);
		//	if (i < 1)
		//		envir->assign(paramName[0], ((PInstance*)(obj.get()))->__cls__());
		//}
		return nullptr;
	}
	pointer PSuper::__getattribute__(const QString& attrName)
	{
		return pointer();
	}
}
