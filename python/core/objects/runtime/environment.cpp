#include "environment.h"
namespace Py {

Environment::Environment(Environment::EPointer p, QObject *): parentEnvir(p)
{

}

Environment::~Environment()
{
}

void Environment::assign(const QString &name, Environment::Pointer obj)
{
    // from here
    auto iter = vars.find(name);
    if (iter != vars.end()) {
        iter.value() = obj;
        return;
    }
    // from parent
    auto currentEnvir = this->sharedFromThis();
    while (currentEnvir->parentEnvir) {
        currentEnvir = currentEnvir->parentEnvir;
        auto iter = currentEnvir->vars.find(name);
        if (iter != currentEnvir->vars.end()) {
            currentEnvir->vars[name] = obj;
            return;
        }
    }
    // create here
    vars[name] = obj;
}

void Environment::createThere(const QString &name, Environment::Pointer obj)
{
    vars[name] = obj;
}

void Environment::throwParent()
{
    parentEnvir = nullptr;
}

void Environment::setParent(Environment::EPointer parentEnvir)
{
    this->parentEnvir = parentEnvir;
}

void Environment::clear()
{
    vars.clear();
}

Environment::Pointer Environment::getObj(const QString &name)
{
    // 1. 先找自己
    auto iter = vars.find(name);
    if (iter != vars.end()) {
        return iter.value();
    }

    // 2. 再递归找父环境
    if (parentEnvir) {
        return parentEnvir->getObj(name);
    }

    // 3. 完全找不到
    return nullptr;
}

}

