#include "PModel.h"

#include "core/objects/runtime/environment.h"

namespace Py {
	Py::PModel::PModel(EPointer envir) : PObject(typeMap.at(Type::Model)), table(envir)
	{
	}

	QString Py::PModel::toString() const
	{
		return "a model";
	}

	Py::pointer Py::PModel::__getattribute__(const QString& attrName)
	{
		return table->getObj(attrName);
	}
	QVector<pointer> PModel::getChildren() const
	{
		return table->getChildren(); // table is instance of Environment
	}
}


