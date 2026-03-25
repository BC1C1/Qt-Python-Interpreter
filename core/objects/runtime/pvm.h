#ifndef PVM_H
#define PVM_H

#include <QObject>
#include <QVector>
#include <QDebug>
#include <QStack>
#include <QFile>
#include "core/objects/runtime/pobject.h"
#include "core/utils/functions.h"
#include "core/objects/runtime/pint.h"
#include "core/objects/runtime/pstr.h"
#include "core/objects/runtime/pnone.h"
#include "core/objects/runtime/pbool.h"
#include "core/objects/runtime/pfloat.h"
#include "core/objects/runtime/plist.h"
#include "core/objects/runtime/environment.h"
// code define begin
namespace vm {
enum class Code {
    // 加载字面量
    LOAD_INT,       // need 数字           栈：[] → [值]
    LOAD_FLOAT,     // need 数字           栈：[] → [值]
    LOAD_STRING,    // need 字符串         栈：[] → [值]
    LOAD_LIST,      // need 列表           栈：[] → [值]

    // 加载变量
    LOAD_NAME,      // need 变量名         栈：[] → [值]
    LOAD_INDEX,     // need 无             栈：[对象, 下标] → [值]
    LOAD_ATTR,      // need 属性名         栈：[对象] → [属性值]

    // 赋值
    STORE_VAR,      // need 变量名         栈：[值] → []
    STORE_INDEX,    // need 无             栈：[对象, 下标, 值] → []
    STORE_ATTR,     // need 属性名         栈：[对象, 值] → []

    // 跳转
    JUMP_IF_FALSE,  // 弹栈，false则跳转, need where to jump
    JUMP,           // 无条件跳转, need, need where to jump

    // 迭代器
    CREATE_ITER,    // 从栈中取obj，尝试获取其迭代器压入栈中
    ITER_NEXT,      // 从栈中获取迭代器，调用next

    // 算数
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    POW,

    // 比较
    EQ,
    NEQ,
    GT,
    GE,
    LT,
    LE,

    // 打印测试
    PRINT,

    // 停机
    HALT,

    INVALID

};

QString CodeToQString(Code code);
Code QStringToCode(QString string);

// 测试用
//QVector<Code> fromTextToVector(const QString& text);


// code define end

struct Instruction {
    Code code;
    QVariant operand;

    Instruction() {}
    Instruction(Code c, const QVariant& op = QVariant())
        : code(c), operand(op) {}
    QString toString() const;

};

bool saveInstructionsToTXT(const QVector<Instruction>& instructions, const QString& filename);
QVector<Instruction> loadInstructionsFromTXT(const QString& filename);

using namespace Py;
using pointer = Py::PObject::pointer;
using ValueStack = QStack<pointer>;
using EPointer = QSharedPointer<Environment>;
class PVM : public QObject
{
    Q_OBJECT
public:
    PVM(const QVector<Instruction>& codes, QObject *parent = nullptr);
    void start();
    void stop();
    void run();
private:
    void executeSingleCode();
    void pushValue(pointer object);
    pointer popValue();
    pointer getBasicObject(QVariant value);
private:
    void throwErrMsg(const std::string& msg);
private:
    QVector<Instruction> codes;
    int PC;
    bool isRunning;
    ValueStack valueStack;
    EPointer globalEnvir;
signals:

};
}


#endif // PVM_H
