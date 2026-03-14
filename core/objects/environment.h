#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <QObject>
#include "core/objects/pobject.h"
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
public:
    QMap<QString, QSharedPointer<PObject>> vars;
    EPointer parentEnvir;
signals:

};
}


#endif // ENVIRONMENT_H
