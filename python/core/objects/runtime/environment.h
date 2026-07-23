#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <QObject>
#include <qset.h>
#include "core/objects/runtime/pobject.h"
namespace Py {
class Environment : public QObject, public QEnableSharedFromThis<Environment>
{
    Q_OBJECT
private:
    using EPointer = QSharedPointer<Environment>;
    using Pointer = QSharedPointer<PObject>;
public:
    Environment(EPointer p = nullptr, QObject *parentEnvir = nullptr);
    ~Environment();
    void assign(const QString& name, Pointer obj);
    void createThere(const QString& name, Pointer obj);
    void throwParent();
    void setParent(EPointer parentEnvir);
    void clear();
    Pointer getObj(const QString& name);
    void addGlobal(const QString& name);
    bool isGlobal(const QString& name) const;
    QVector<Py::PObject::pointer> getChildren() const;
public:
    QMap<QString, QSharedPointer<PObject>> vars;
    QSet<QString> globalVars;
    EPointer parentEnvir;
signals:

};
}


#endif // ENVIRONMENT_H
