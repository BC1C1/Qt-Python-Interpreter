#ifndef PLIST_H
#define PLIST_H

#include "core/objects/runtime/pobject.h"
#include "core/utils/functions.h"
#include <QVector>

namespace Py {
class PList : public PObject
{
public:
    PList(QVector<pointer>& list);
    QString toString() const override;
    QVariant getValue() const override;
    QVector<pointer>& getTrueValue();

    virtual pointer asString() const override;

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

    // 逻辑运算符声明
    virtual pointer __not__(const pointer& other) const override;

    // 获取迭代器
    virtual pointer __iter__() override;

    // 内部元素
    virtual void __setitem__(const pointer& index, pointer obj) override;
    virtual pointer __getitem__(const pointer& index) override;

protected:
    void throwInvalidTypeForIndex();
    void throwOutOfRange();

private:
    QVector<pointer> value;
};
}

#endif // PLIST_H
