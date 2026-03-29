#ifndef PFLOAT_H
#define PFLOAT_H
#include "core/objects/runtime/pobject.h"
#include "core/utils/functions.h"
namespace Py {
class PFloat : public PObject
{
public:
    PFloat(double value = 0);
    virtual QString toString() const override;
    virtual QVariant getValue() const override;

    // 类型转换
    virtual pointer asInt() const override;
    virtual pointer asFloat() const override;
    virtual pointer asBool() const override;
    virtual pointer asString() const override;

    // 算术运算符声明
    virtual pointer __add__(const pointer &other) const override;
    virtual pointer __sub__(const pointer& other) const override;
    virtual pointer __mul__(const pointer& other) const override;
    virtual pointer __truediv__(const pointer& other) const override;
    virtual pointer __mod__(const pointer& other) const override;
    virtual pointer __pow__(const pointer& other) const override;

    // 比较运算符声明
    virtual pointer __eq__(const pointer& other) const override;
    virtual pointer __ne__(const pointer& other) const override;
    virtual pointer __lt__(const pointer& other) const override;
    virtual pointer __le__(const pointer& other) const override;
    virtual pointer __gt__(const pointer& other) const override;
    virtual pointer __ge__(const pointer& other) const override;

    // 逻辑运算符声明
    virtual pointer __and__(const pointer& other) const override;
    virtual pointer __or__(const pointer& other) const override;
    virtual pointer __not__(const pointer& other) const override;
private:
    double value;
};
}


#endif // PFLOAT_H
