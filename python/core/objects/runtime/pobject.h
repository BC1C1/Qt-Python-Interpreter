#ifndef POBJECT_H
#define POBJECT_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QSharedPointer>
#include <QEnableSharedFromThis>
#include <QException>
#include <stdexcept>
#include <string>
#include <QVariant>
namespace Py {
// exception define begin
using runtime_error = std::runtime_error;
// exception define end

enum class Type {
    Int, Float, Str, Bool, None, Undefined, Iterator, List, FunctionDefine,
    ReturnValue, Class, Instance, Dict, Super
};
QString TypeToString(Type type);
struct PyType {
    PyType(Type type);
    QString __str__();
    Type type;
    bool operator==(PyType other);
};
class Environment;
class PInt;
class PFloat;
class PBool;
class PStr;
class PNone;
class Iterator;
class PList;
class PFunction;
class PReturnValue;
class PClass;
class PInstance;
class PDict;
class PSuper;
class PObject : public QObject, public QEnableSharedFromThis<PObject>
{
    Q_OBJECT
public:
    using pointer = QSharedPointer<PObject>;
public:
    PObject(PyType type, QObject *parent = nullptr);
    PyType getType() const;
    virtual ~PObject();
    virtual QString toString() const = 0;
    virtual QVariant getValue() const;

    // 类型转换声明
    virtual pointer asInt() const;
    virtual pointer asFloat() const;
    virtual pointer asBool() const;
    virtual pointer asList() const;
    virtual pointer asString() const;

    // 算术运算符声明
    virtual pointer __add__(const pointer& other) const;
    virtual pointer __sub__(const pointer& other) const;
    virtual pointer __mul__(const pointer& other) const;
    virtual pointer __truediv__(const pointer& other) const;
    virtual pointer __mod__(const pointer& other) const;
    virtual pointer __pow__(const pointer& other) const;

    // 比较运算符声明
    virtual pointer __eq__(const pointer& other) const;
    virtual pointer __ne__(const pointer& other) const;
    virtual pointer __lt__(const pointer& other) const;
    virtual pointer __le__(const pointer& other) const;
    virtual pointer __gt__(const pointer& other) const;
    virtual pointer __ge__(const pointer& other) const;

    // 逻辑运算符声明
    virtual pointer __and__(const pointer& other) const;
    virtual pointer __or__(const pointer& other) const;
    virtual pointer __not__(const pointer& other) const;

    // 获取迭代器
    virtual pointer __iter__();
    virtual pointer __next__();

    // 内部元素
    virtual void __setitem__(const pointer& index, pointer obj);
    virtual pointer __getitem__(const pointer& index);

    // 对象的调用办法
    virtual pointer __call__(
        const pointer& listParams,
        const pointer& dictParams,
        QSharedPointer<Environment> envir
    );

    // 对象实例化、属性访问和存储
    virtual pointer __instance__(
        const pointer& listParams,
        const pointer& dictParams,
        QSharedPointer<Environment> envir);
    virtual pointer __getattribute__(const QString& attributeName);
    virtual void __setattribute__(const QString& attributeName, const pointer& obj);

    // mro获取
    virtual QVector<PClass*> __mro__();
protected:
    // 错误处理函数声明
    pointer noSuchCast(const std::string& name) const;

    pointer defaultOpError(const QString& op, const pointer& other) const;

    pointer defaultNoIterError() const;

    pointer defaultNotIterError() const;

    void defaultNoSetItemError() const;

    pointer defaultNoGetItemError() const;

    pointer defaultNoCallFuncError() const;

    pointer defaultNoInstanceError() const;

    pointer defaultNoGetAttributeError() const;

    pointer defaultNoSetAttributeError() const;

    pointer defaultNoMroError() const;
private:
    PyType type;
signals:

};
static const std::map<Type, PyType> typeMap = {
    {Type::Int, PyType(Type::Int)},
    {Type::Float, PyType(Type::Float)},
    {Type::Str, PyType(Type::Str)},
    {Type::Bool, PyType(Type::Bool)},
    {Type::None, PyType(Type::None)},
    {Type::Iterator, PyType(Type::Iterator)},
    {Type::List, PyType{Type::List}},
    {Type::FunctionDefine, PyType{Type::FunctionDefine}},
    {Type::ReturnValue, PyType{Type::ReturnValue}},
    {Type::Class, PyType{Type::Class}},
    {Type::Instance, PyType{Type::Instance}},
    {Type::Dict, PyType{Type::Dict}},
    {Type::Super, PyType{Type::Super}},
};
class PInt;
class PFloat;
class PBool;
class PStr;
class PNone;
class Iterator;
class PList;
class PFunction;
class PReturnValue;
class PClass;
class PInstance;
class PDict;
class PSuper;
}


#endif // POBJECT_H
