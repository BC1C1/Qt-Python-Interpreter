#ifndef PSTR_H
#define PSTR_H
#include "core/objects/runtime/pobject.h"
namespace Py {
class PStr : public PObject
{
public:
    PStr(QString value = "");
    virtual ~PStr();
    virtual QVariant getValue() const override;
    virtual QString toString() const override;

public:
    // 算术运算符声明
    virtual pointer __add__(const pointer& other) const override;
    virtual pointer __mul__(const pointer& other) const override;

    // 比较运算符声明
    virtual pointer __eq__(const pointer& other) const override;
    virtual pointer __ne__(const pointer& other) const override;
    virtual pointer __lt__(const pointer& other) const override;
    virtual pointer __le__(const pointer& other) const override;
    virtual pointer __gt__(const pointer& other) const override;
    virtual pointer __ge__(const pointer& other) const override;
private:
    QString value;
};
}


#endif // PSTR_H
